#include "file_handler.h"
#include <fstream>
#include <sstream>
#include <iostream>

HttpResponse FileHandler::serve(const std::string& request_path) {
    HttpResponse response;

    std::string path = (request_path == "/") ? "/index.html" : request_path;

    // Dinamik root_directory kullanımı
    std::string full_path = root_directory + path;

    std::string content;
    if (readFile(full_path, content)) {
        response.statusCode = "200 OK";
        response.body = content;
        response.headers["Content-Type"] = getMimeType(path);
        response.headers["Content-Length"] = std::to_string(content.size());
    } else {
        response.statusCode = "404 Not Found";
        response.body = "<h1>404 Not Found</h1>";
        response.headers["Content-Type"] = "text/html";
    }

    return response;
}

// readFile ve getMimeType artık üye fonksiyon (statik değiller)
bool FileHandler::readFile(const std::string& path, std::string& content) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return false;
    std::ostringstream ss;
    ss << file.rdbuf();
    content = ss.str();
    return true;
}

std::string FileHandler::getMimeType(const std::string& path) {
    if (path.ends_with(".html")) return "text/html";
    if (path.ends_with(".css"))  return "text/css";
    if (path.ends_with(".js"))   return "application/javascript";
    if (path.ends_with(".png"))  return "image/png";
    if (path.ends_with(".jpg"))  return "image/jpeg";
    return "application/octet-stream";
}