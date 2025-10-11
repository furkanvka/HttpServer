#pragma once

#include "Constants.h"
#include <string>
#include <map>

class HttpResponse {
public:
    std::string statusCode;
    std::string body;
    
    std::map<std::string, std::string> headers;

    HttpResponse() {
        statusCode = Constants::STATUS_200;
        headers["Server"] = "MyCppServer/1.0";
    }

    std::string toString(); 
};