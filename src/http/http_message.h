#pragma once

#include "constants.h"
#include <string>
#include <map>

class HttpRequest {
public:
    std::string method;
    std::string path;
    std::string version;
    
    std::map<std::string, std::string> headers; 
    
    std::string body;

    void clear() {
        method.clear();
        path.clear();
        version.clear();
        headers.clear();
        body.clear();
    }
};


class HttpResponse {
public:
    std::string statusCode;
    std::string body;
    
    std::map<std::string, std::string> headers;

    HttpResponse() {
        statusCode = Constants::STATUS_200;
        headers["Server"] = "MyCppServer/1.0";
    }

    std::string toString() const; 
};
