#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "utils.h"
#include "Request.h"
#include "Response.h"
#include <fstream>

class Logger {
    public:
    Logger();
    virtual ~Logger();
    virtual void receivedRequest(Request & request, const std::vector<char> & ip);
    virtual void sendingRequest(Request & request);
    virtual void sendingResponse(Response & response);
    virtual void receivedResponse(Response & response, Request & request);
    virtual void tunnelClosed(const u_long & id);

    protected:
    std::ofstream log;
};

Logger::Logger() {
    try {
        log.open("./proxy.log");
    }
    catch (std::exception & e) {
        throw std::invalid_argument(e.what());
    }
}

Logger::~Logger() {
    log.close();
}

void Logger::receivedRequest(Request & request, const std::vector<char> & ip) {
    std::stringstream loggedReq;
    datetime_zone_t datetimeZone = getCurrentTime();
    loggedReq << request.getId() << ": \"" << request.getMethod().data() << " " << \
    request.getUri().data() << " " << request.getProtocal().data() << "\" from " << \
    ip.data() << " @ " << std::put_time(&datetimeZone.first, "%c") << "\r\n";
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << loggedReq.str();
}

void Logger::sendingRequest(Request & request) {
    std::stringstream loggedReq;
    loggedReq << request.getId() << ": Requesting \"" << request.getMethod().data() << " " << \
    request.getUri().data() << " " << request.getProtocal().data() << "\" from " << \
    request.getHostName().data() << "\r\n";
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << loggedReq.str();
}

void Logger::sendingResponse(Response & response) {
    std::stringstream loggedresp;
    loggedresp << response.getId() << ": Responding \"" << response.getProtocal().data() << \
    " " << response.getStatusCode().data() << " " << response.getReasonPhrase().data() << \
    "\"" << "\r\n";
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << loggedresp.str();
}

void Logger::receivedResponse(Response & response, Request & request) {
    std::stringstream loggedresp;
    loggedresp << response.getId() << ": Received \"" << response.getProtocal().data() << \
    " " << response.getStatusCode().data() << " " << response.getReasonPhrase().data() << \
    "\"" << " from " << request.getHostName().data() << "\r\n";
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << loggedresp.str();
}

void Logger::tunnelClosed(const u_long & id) {
    std::stringstream loggedresp;
    loggedresp << id << ": Tunnel closed\r\n";
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << loggedresp.str();
}

#endif