//
// Created by 陆清 on 2/23/20.
//

#ifndef H2D1_CACHE_H
#define H2D1_CACHE_H
#include <iostream>
#include <fstream>
#include <sstream>
#include "Request.h"
#include "Response.h"
#include "ServerSocket.h"
#include "ClientSocket.h"
#include <unordered_map>
#include "list"
#include "Logger.h"
#include "PostHandler.h"

class cache{
    class node{
    public:
        std::string cache_url;
        Response cache_response;

    public:
        node(const std::string k, const Response v): cache_url(k), cache_response(v){}
    };
private:
    std::mutex mtx_cache;//std::mutex to protect cache
    std::list<node> cache_list;
    std::unordered_map<std::string, std::list<node>::iterator> cache_map;
    size_t capacity = 2000;
    ServerSocket & serverSocket;
public:
    // connect_pair_t & connectPair;
    cache(const int & c, ServerSocket & serverSocket) : capacity(c), serverSocket(serverSocket) {}
    // virtual connect_pair_t & getConnectPair() {return this->getConnectPair;}
    virtual void check_cache(Request & request, int ID, Logger & logger, connect_pair_t & connectPair);
    virtual void put_cache(const std::string key, Response value, const datetime_zone_t time);
    virtual bool getResponseByUrl(Response & response_in_cache, std::string & key);
    virtual int revalidate_header(Request & request, Response & response, int ID, Logger & logger, connect_pair_t & connectPair);
    virtual void response_helper(Request & request, int id, Logger & file, connect_pair_t & connectPair);
    //virtual void reconstructRequest(Request& request, Response& response);
};

bool cache::getResponseByUrl(Response &response_in_cache, std::string &key){
    std::lock_guard<std::mutex> guard(mtx_cache);
    //Response response(id);
    if(cache_map.find(key)!=cache_map.end()){
        response_in_cache = cache_map[key]->cache_response;
        cache_map[key]->cache_response.iscached=true;

        cache_list.erase(cache_map[key]);
        cache_list.push_front({key,response_in_cache});
        cache_map[key] = cache_list.begin();

        return true;
    } 
    
    return false;
}

void cache::put_cache(const std::string key, Response value, const datetime_zone_t r_store_time) {
    std::lock_guard<std::mutex> guard(mtx_cache);
    //if map(cache) has key
    if (cache_map.find(key) != cache_map.end())
        cache_list.erase(cache_map[key]);
    else if (cache_list.size() >= capacity) {
        cache_map.erase(cache_list.back().cache_url);
        cache_list.pop_back();
    }
    cache_list.push_front({ key, value });
    cache_map[key] = cache_list.begin();
    cache_map[key]->cache_response.getStoredTime()=r_store_time;//TODO:maybe have mistatke

}

int cache::revalidate_header(Request & request, Response &response, int ID, Logger & file, connect_pair_t & connectPair) {
    mapped_field_t response_header_map_prev=response.getHeaderFields();
    auto it_etag = response_header_map_prev.find("Etag");
    std::string response_etag_material;
    if (it_etag!=response_header_map_prev.end()){
        //TODO:reconstruct request header with etag
        request.getHeaderFields()["If-None-Match"] = it_etag->second;
        auto it_last_modified = response_header_map_prev.find("Last-Modified");
        if (it_last_modified != response_header_map_prev.end()) {
            request.getHeaderFields()["If-Modified-Since"] = it_last_modified->second;
        }
    }
    else{
        auto it_last_modified = response_header_map_prev.find("Last-Modified");
        if (it_last_modified != response_header_map_prev.end()) {
            request.getHeaderFields()["If-Modified-Since"] = it_last_modified->second;
        }
    }
    //connect with web server
    ClientSocket clientSocket(request.getHostName(), request.getPort());
    std::vector<char> requestMsg = request.reconstruct();
    file.sendingRequest(request);
    clientSocket.socketSend(requestMsg);//send to server

    std::vector<char> responseMsgNew;
    Response NewResponse(request.getId());
    clientSocket.socketRecv(responseMsgNew, NewResponse);//recv server new response
    cleanVectorChar(NewResponse.getContent());
    NewResponse.getContent() = obtainContent(responseMsgNew);
    file.receivedResponse(NewResponse, request);//TODO:Maybe has mistake

    //response status code is 304 then just return old response and update cache
    if(strcmp(NewResponse.getStatusCode().data(), "304") == 0){
        std::string url = request.getUri().data();
        datetime_zone_t currTime = getCurrentTime();
        put_cache(url, response, currTime);//put old response
        std::vector<char> reconRespMsg =response.reconstruct();
        serverSocket.socketSend(reconRespMsg, connectPair);//send to browser
        //TODO:MAYBE NOT CORRECT
        file.sendingResponse(NewResponse);
        return 0;
    }

    mapped_field_t response_header_map_new=response.getHeaderFields();
    auto it_response_new = response_header_map_new.find("Cache-Control");
    std::string response_cache_control_material;
    if (it_response_new != response_header_map_new.end()) {
        response_cache_control_material = it_response_new->second.data();
        //if response(server) 200,no store->send to browser
        if (response_cache_control_material.find("no-store") != std::string::npos &&
            strcmp(NewResponse.getStatusCode().data(), "200") == 0) {
            std::stringstream ss4;
            ss4 << ID << ": not cacheable because \"no-store\"" << std::endl;
            file.write(ss4.str());
            //将server response 返回 browser,且不放进cache
            std::vector<char> reconRespMsg = NewResponse.reconstruct();
            file.receivedResponse(NewResponse, request);
            serverSocket.socketSend(reconRespMsg, connectPair);//send to browser
            file.sendingResponse(NewResponse);
            return 1;
        }
        else { // first time to put into cache
            std::string url = request.getUri().data();
            datetime_zone_t currTime=getCurrentTime();
            put_cache(url, NewResponse, currTime);

            //if new response's head has no-cache
            if(response_cache_control_material.find("no-cache")!=std::string::npos ||
               response_cache_control_material.find("max-age=0") != std::string::npos){
                std::stringstream ss3;
                ss3 << request.getId() << ": cached, but requires re-validation\r\n";
                file.write(ss3.str());
            }
            else if (response_cache_control_material.find("max-age") != std::string::npos) {//update expire time?
                size_t pos = response_cache_control_material.find("max-age=");
                pos += 8;
                size_t pos1 = response_cache_control_material.find(",", pos);
                std::string age_content = response_cache_control_material.substr(pos, pos1 - pos);
                int age = stoi(age_content);
                time_t res = std::mktime(&currTime.first) + (time_t) age;
                std::string res_expire = asctime(gmtime(&res));
                std::stringstream ss5;
                ss5 << request.getId() << ": cached, expires at " << res_expire;
                file.write(ss5.str());
            }
            auto it_response_expires = response_header_map_new.find("Expires");
            std::vector<char> response_expire_material;
            if(it_response_expires!=response_header_map_new.end()) {//header hase expires
                response_expire_material = it_response_expires->second;//expire time
                // int expire=stoi(response_expire_material);
                std::tm expire = getDatetimeAndZone(response_expire_material).first;
                time_t res = std::mktime(&currTime.first) + std::mktime(&expire);
                std::string res_expire = asctime(gmtime(&res));
                std::stringstream ss5;
                ss5 << request.getId() << ": cached, expires at " << res_expire;
                file.write(ss5.str());
            }
            else{
                //TODO:no expire tag
            }
            //send to browser
            std::vector<char> reconRespMsg = response.reconstruct();
            file.receivedResponse(response, request);
            serverSocket.socketSend(reconRespMsg, connectPair);
            file.sendingResponse(response);
            return 2;
        }
    }
    else {
        //no cache control in new response
        std::vector<char> reconRespMsg = response.reconstruct();
        file.receivedResponse(response, request);
        serverSocket.socketSend(reconRespMsg, connectPair);
        file.sendingResponse(response);
        return 3;
    }
}

void cache::response_helper(Request & request, int id, Logger & file, connect_pair_t & connectPair){
    std::string url = request.getUri().data();
    // id=request.getId();
    datetime_zone_t currTime = getCurrentTime();
    Response response(request.getId());

    if (!getResponseByUrl(response, url)){ // cache_list does not contain what was requested (not in cache)
        //request uri not stored in cache
        std::stringstream ss1;
        ss1 << response.getId() <<": not in cache\r\n";
        file.write(ss1.str());
        //connect to server and get response then store in cache
        ClientSocket clientSocket(request.getHostName(), request.getPort());
        std::vector<char> requestMsg = request.reconstruct();
        file.sendingRequest(request);
        clientSocket.socketSend(requestMsg);//send to server

        std::vector<char> responseMsg;
        // Response response(request.getId());
        response.clearResponse();
        clientSocket.socketRecv(responseMsg, response);//recv server response

        if (responseMsg.size() == 0) {
            closeSockfd(connectPair.first);
            return;
        } // commented when responseMst resized in client socketRecv

        cleanVectorChar(response.getContent());
        response.getContent() = obtainContent(responseMsg);//get response content

        // check cache control material for response
        mapped_field_t response_header_map=response.getHeaderFields();
        auto it_response = response_header_map.find("Cache-Control");
        std::string response_cache_control_material;
        if(it_response!=response_header_map.end()) {// response has cache-control
            response_cache_control_material = it_response->second.data();
            //if response(server) 200 && (no-store || private) -> not cacheable -> send response to browser directly
            if ((response_cache_control_material.find("no-store") != std::string::npos || 
            response_cache_control_material.find("private")!=std::string::npos)&&
            strcmp(response.getStatusCode().data(), "200") == 0){
                std::stringstream ss4;
                ss4 << response.getId() << ": not cacheable because \"no-store\" or \"private\"\r\n";
                file.write(ss4.str());
                //将server response 返回 browser,且不放进cache
                std::vector<char> reconRespMsg = response.reconstruct();
                file.receivedResponse(response, request);
                serverSocket.socketSend(reconRespMsg, connectPair);//send to browser
                file.sendingResponse(response);
                return;
            }
        }
        //put response(from server) to cache
        put_cache(url, response, currTime);

        // cached, but expires at 
        // cached, requires revalidation

        //send response from server to client;
        std::vector<char> reconRespMsg = response.reconstruct();
        file.receivedResponse(response, request);
        serverSocket.socketSend(reconRespMsg, connectPair);
        file.sendingResponse(response);

    } 
    else {// cache_list contains what was requested (in cache), which was stored in response
        //request store in cache and response has cache control?
        mapped_field_t response_header_map = response.getHeaderFields();
        auto it_response = response_header_map.find("Cache-Control");
        std::string response_cache_control_material;
        if(it_response != response_header_map.end()) { // cache-control field exists in cached response
            // std::cout<<"no cache lalalla"<<std::endl;
            response_cache_control_material = it_response->second.data();
            //find no-cache or max-age=0 tag in cache
            if (response_cache_control_material.find("no-cache") != std::string::npos ||
                response_cache_control_material.find("max-age=0") != std::string::npos) {
                // log for no cache
                std::stringstream ss3;
                ss3 << id << ": in cache, requires re-validation\r\n";
                file.write(ss3.str());
                int n = revalidate_header(request, response, id, file,connectPair);
                if (n==0) {
                    //304
                }
                else if (n==1) {
                    //no-store
                }
                else if (n==2) {
                    //put in cache
                }
                else if (n==3) {
                    //no cache control tag in new response
                }
            }
            //find max-age in cache
            else if (response_cache_control_material.find("max-age") != std::string::npos){
                size_t pos = response_cache_control_material.find("max-age=");
                pos += 8;
                size_t pos1 = response_cache_control_material.find(",", pos);
                std::string age_content = response_cache_control_material.substr(pos, pos1 - pos);
                int age = stoi(age_content);
                //TODO:check whether fresh
                datetime_zone_t store_time = response.getStoredTime();
                time_t expiretime = std::mktime(&store_time.first) + (time_t) age;
                if (expiretime > std::mktime(&currTime.first)){ // fresh
                    // log for valid
                    std::stringstream ss_ic_valid;
                    ss_ic_valid << id << ": in cache, valid\r\n";
                    file.write(ss_ic_valid.str());
                    std::vector<char> reconRespMsg =response.reconstruct();
                    serverSocket.socketSend(reconRespMsg, connectPair);//send to browser
                    //TODO:MAYBE NOT CORRECT
                    file.sendingResponse(response);
                    return;
                }
                else { // not fresh
                    // log for re-validation
                    std::stringstream ss_not_fresh;
                    ss_not_fresh << id << ": in cache, requires re-validation\r\n";
                    file.write(ss_not_fresh.str());
                    int n = revalidate_header(request, response, id, file,connectPair);
                    if (n==0) {
                        //304
                    }
                    else if (n==1) {
                        //no-store
                    }
                    else if (n==2) {
                        //put in cache
                    }
                    else if (n==3) {
                        //no cache control tag in new response
                    }
                }
            }
        }
        else {  // cache-control field not exists in cached response
            //check the expires tag
            time_t expire_time;
            auto it_response_expires = response_header_map.find("Expires");
            std::vector<char> response_expire_material;
            if (it_response_expires != response_header_map.end()) {// cached response header has expires
                response_expire_material = it_response_expires->second;//expire time
                // int expire=stoi(response_expire_material);
                std::tm expire = getDatetimeAndZone(response_expire_material).first;
                expire_time = std::mktime(&currTime.first) + std::mktime(&expire);
            }
            else {// cached response header does not have expires
                //TODO:no expire tag
                time_t store_time = std::mktime(&response.getStoredTime().first);
                auto it_response_last_modified = response_header_map.find("Last-Modified");
                std::vector<char> response_last_modified_material;
                if (it_response_last_modified != response_header_map.end()) { // cached response has last-modified
                    std::tm datetm = getDatetimeAndZone(response.getDatetimeVectorChar()).first;
                    time_t date = std::mktime(&datetm);
                    std::tm last_modified_tm = getDatetimeAndZone(it_response_last_modified->second).first;
                    time_t last_modified = std::mktime(&last_modified_tm);
                    expire_time = store_time + (date - last_modified) * 0.1;
                }
                else { // cached response no has last-modified
                    expire_time = store_time - (time_t) 10;
                }
            }

            if (expire_time > std::mktime(&currTime.first)) { // fresh cached response
                std::stringstream ss_expires_valid;
                ss_expires_valid << request.getId() << ": in cache, valid\r\n";
                file.write(ss_expires_valid.str());
                std::vector<char> reconrespMsg = response.reconstruct();
                serverSocket.socketSend(reconrespMsg, connectPair);
                file.sendingResponse(response);
            }
            else {
                std::string res_expire = asctime(gmtime(&expire_time));
                std::stringstream ss5;
                ss5 << request.getId() << ": in cached, but expires at " << res_expire;
                file.write(ss5.str()); 
                int n = revalidate_header(request, response, id, file,connectPair);
                if (n==0) {
                    //304
                }
                else if (n==1) {
                    //no-store
                }
                else if (n==2) {
                    //put in cache
                }
                else if (n==3) {
                    //no cache control tag in new response
                }
            }
        }
    }
}

void cache::check_cache(Request & request, int id, Logger & file, connect_pair_t & connectPair) {
    //1. first check request header
    std::unordered_map<std::string, std::vector<char> > request_header_map = request.getHeaderFields();
    auto it_request = request_header_map.find("Cache-Control");
    std::string request_cache_control_material;
    if (it_request != request_header_map.end()) {
        request_cache_control_material = it_request->second.data();
    }
    // if (request_cache_control_material.find("no-cache") != std::string::npos ||
    //     request_cache_control_material.find("max-age=0") != std::string::npos) {
    //     response_helper(request, id, file, connectPair);
    // } else {
    //     response_helper(request, id, file, connectPair);
    // }
    response_helper(request, id, file, connectPair);
}

#endif //H2D1_CACHE_H
