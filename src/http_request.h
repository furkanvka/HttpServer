#pragma once

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