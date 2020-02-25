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

class cache{
    class node{
    public:
        std::string cache_url;
        Response cache_response;

    public:
        node(const std::string& k, const Response& v): cache_url(k), cache_response(v){}
    };
private:
    std::mutex mtx_cache;//std::mutex to protect cache
    std::list<node> cache_list;
    std::unordered_map<std::string, std::list<node>::iterator> cache_map;
    size_t capacity=2000;
    ServerSocket & serverSocket;
public:
    // connect_pair_t & connectPair;
    cache(const int&c, ServerSocket & serverSocket) : capacity(c), serverSocket(serverSocket) {}
    // virtual connect_pair_t & getConnectPair() {return this->getConnectPair;}
    virtual void check_cache(Request request,int ID, Logger & logger, connect_pair_t & connectPair);
    virtual void put_cache(const std::string& key,const Response& value,const datetime_zone_t time);
    virtual bool getResponseByUrl(Response &response_in_cache, std::string& key);
};

bool cache::getResponseByUrl(Response &response_in_cache, std::string& key){
    std::lock_guard<std::mutex> guard(mtx_cache);
    //Response response(id);
    if(cache_map.find(key)!=cache_map.end()){
        cache_list.erase(cache_map[key]);
        cache_map[key]->cache_response.iscached=true;
    } else{
        return false;
    }
    cache_list.push_front({key,cache_map[key]->cache_response});
    cache_map[key]=cache_list.begin();
    response_in_cache= cache_map[key]->cache_response;
    return true;
}

void cache::put_cache(const std::string &key, const Response &value, const datetime_zone_t r_store_time) {
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

void cache::check_cache(Request request,int id,Logger & file,connect_pair_t & connectPair){
    //1. first check request header
    std::unordered_map<std::string, std::vector<char> > request_header_map=request.getHeaderFields();
    auto it_request = request_header_map.find("Cache-Control");
    std::string request_cache_control_material;

    std::string url = request.getUri().data();//

    datetime_zone_t currTime=getCurrentTime();
    Response response(id);
    if (!getResponseByUrl(response,url)){
        //request uri not stored in cache
        std::stringstream ss1;
        ss1 << id <<": not in cache"<<std::endl;
        file.write(ss1.str());
        //TODO:connect to server and get response then store in cache
        ClientSocket clientSocket(request.getHostName(), request.getPort());
        std::vector<char> requestMsg = request.reconstruct();
        file.sendingRequest(request);
        clientSocket.socketSend(requestMsg);

        std::vector<char> responseMsg;
        Response response(request.getId());
        clientSocket.socketRecv(responseMsg, response);

        if (responseMsg.size() == 0) {
            closeSockfd(connectPair.first);
            return;
        } // commented when responseMst resized in client socketRecv

        cleanVectorChar(response.getContent());
        response.getContent() = obtainContent(responseMsg);
        
        //cache control material for response
        std::unordered_map<std::string, std::vector<char> > response_header_map=response.getHeaderFields();
        auto it_response = response_header_map.find("Cache-Control");
        std::string response_cache_control_material;
        if(it_response!=response_header_map.end()) {
            response_cache_control_material = it_response->second.data();
            //if response(server) 200,no store->send to browser
            if (response_cache_control_material.find("no-store") != std::string::npos &&
                strcmp(response.getStatusCode().data(), "200") == 0) {
                // std::stringstream ss4;
                // ss4 << id << ": cannot be stored in cache and return to browser immediately" << std::endl;
                // file.write(ss4.str());
                //TODO:将server response 返回 browser,且不放进cache
                std::vector<char> reconRespMsg = response.reconstruct();
                file.sendingResponse(response);
                serverSocket.socketSend(reconRespMsg, connectPair);
                file.receivedResponse(response, request);
                return;
            }
            //if response(server) no-cache, revalidate request
            if (response_cache_control_material.find("no-cache") != std::string::npos ||
                response_cache_control_material.find("max-age=0") != std::string::npos) {
                //log for no cache
                std::stringstream ss3;
                ss3 << id << ": cached, but requires re-validation";
                file.write(ss3.str());
            } else if (response_cache_control_material.find("max-age") != std::string::npos) {
                size_t pos = response_cache_control_material.find("max-age=");
                pos += 8;
                size_t pos1 = response_cache_control_material.find(",", pos);
                std::string age_content = response_cache_control_material.substr(pos, pos1 - pos);
                int age = stoi(age_content);
                time_t res = std::mktime(&currTime.first) + (time_t) age;
                std::string res_expire = asctime(gmtime(&res));
                std::stringstream ss5;
                ss5 << id << ": cached, expires at " << res_expire;
                file.write(ss5.str());
            }
            //put response(from server) to cache
            put_cache(url,response,currTime);
        }else{
            std::vector<char> reconRespMsg = response.reconstruct();
            file.sendingResponse(response);
            serverSocket.socketSend(reconRespMsg, connectPair);
            file.receivedResponse(response, request);
            return;
        }


        //send response from server to client;
        std::vector<char> reconRespMsg = response.reconstruct();
        file.sendingResponse(response);
        serverSocket.socketSend(reconRespMsg, connectPair);
        file.receivedResponse(response, request);

    } else {//in cache

        if (it_request != request_header_map.end()) {
            request_cache_control_material = it_request->second.data();
            std::cout <<"cache_control: "<<request_cache_control_material<<std::endl;
        } else {
            std::cout << "request head cannot find cache-control" << std::endl;
        }
        //no-store--log--don't cache
        if (request_cache_control_material.find("no-store")!=std::string::npos){
            // std::stringstream ss;
            // ss << id <<": cannot be stored in cache and return to browser immediately"<<std::endl;
            // file.write(ss.str());
            //TODO:正常返回server内容
            return;
        }
    }
}




#endif //H2D1_CACHE_H
