#pragma once

#include <string>
#include "http/http_message.h"

class HttpParser {
public:
    static bool parse_request(const std::string& raw_request, HttpRequest& request);
private:
    static std::string trim(const std::string& s);
};
