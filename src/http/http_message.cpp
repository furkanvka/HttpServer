#include "http_message.h"
#include "constants.h"
#include <sstream>
#include <algorithm> 

std::string HttpResponse::toString() const {
    std::stringstream response_stream;

    response_stream << Constants::HTTP_VERSION << " " << statusCode << "\r\n";

    response_stream << "Content-Length: " << std::to_string(body.length()) << "\r\n";
    
    bool has_content_type = false;
    for (const auto& pair : headers) {
        if (pair.first == Constants::HEADER_CONTENT_TYPE) {
            has_content_type = true;
        }
        response_stream << pair.first << ": " << pair.second << "\r\n";
    }
    
    if (!has_content_type) {
        response_stream << Constants::HEADER_CONTENT_TYPE << ": " << Constants::MIME_HTML << "\r\n";
    }

    response_stream << "\r\n";

    response_stream << body;

    return response_stream.str();
}
