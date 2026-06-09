#include "http_parser.h"
#include "http/constants.h"

inline std::string HttpParser::trim(const std::string& s) {
    auto start = s.find_first_not_of(" \t");
    auto end = s.find_last_not_of(" \t");
    if (start == std::string::npos) return "";
    return s.substr(start, end - start + 1);
}

bool HttpParser::parse_request(const std::string& raw_request, HttpRequest& request) {
    request.clear();

    size_t header_body_separator = raw_request.find("\r\n\r\n");
    if (header_body_separator == std::string::npos) {
        return false;
    }

    std::string headers = raw_request.substr(0, header_body_separator);
    std::string body = raw_request.substr(header_body_separator + 4);

    
    size_t line_end = raw_request.find("\r\n");
    if (line_end == std::string::npos) {
        return false;
    }

    std::string line = raw_request.substr(0, line_end);
    

    size_t first_space = line.find(' ');
    if (first_space == std::string::npos) {
        return false;
    }

    size_t second_space = line.find(' ', first_space + 1);
    if (second_space == std::string::npos) {
        return false;
    }

    size_t third_space = line.find(' ', second_space + 1);
    if (third_space != std::string::npos) {
        return false;
    }

    request.method = line.substr(0, first_space);

    request.path = line.substr(first_space + 1,second_space - first_space - 1);

    request.version = line.substr(second_space + 1);

    if (request.method.empty() ||request.path.empty() ||request.version.empty()) {
        return false;
    }


    size_t current_pos = line_end + 2;

    while (current_pos < headers.size()) {

    size_t next_line_end = headers.find("\r\n", current_pos);

    if (next_line_end == std::string::npos) {
        next_line_end = headers.size();
    }

    std::string header_line =
        headers.substr(current_pos,
                       next_line_end - current_pos);

    if (header_line.empty()) {
        break;
    }

    size_t colon_pos = header_line.find(':');

    if (colon_pos == std::string::npos) {
        return false;
    }

    std::string key =
        trim(header_line.substr(0, colon_pos));

    std::string value =
        trim(header_line.substr(colon_pos + 1));

    request.headers[key] = value;

    current_pos = next_line_end + 2;
    }

    request.body = body;

    auto a = request.headers.find(Constants::HEADER_CONTENT_LENGTH);

    if (a != request.headers.end()) {

        size_t expected_length = 0;

        try {
            expected_length = std::stoul(a->second);
        }
        catch (...) {
            return false;
        }

        if (request.body.size() != expected_length) {
            return false;
        }
    }
    return true;
}
