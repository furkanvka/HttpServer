#include "http_response.h"
#include "Constants.h"
#include <sstream>
#include <algorithm> 

std::string HttpResponse::toString() {
    std::stringstream response_stream;

    response_stream << Constants::HTTP_VERSION << " " << statusCode << "\r\n";

    response_stream << "Content-Length: " << std::to_string(body.length()) << "\r\n";
    
    if (headers.find(Constants::HEADER_CONTENT_TYPE) == headers.end()) {
        headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
    }
    
    for (const auto& pair : headers) {
        response_stream << pair.first << ": " << pair.second << "\r\n";
    }

    response_stream << "\r\n";

    response_stream << body;

    return response_stream.str();
}