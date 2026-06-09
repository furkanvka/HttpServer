#pragma once

#include "../http/http_message.h"
#include <string>

class FileHandler {
public:
    // Constructor ile kök dizini alıyoruz
    explicit FileHandler(const std::string& root) : root_directory(root) {}

    HttpResponse serve(const std::string& path);

private:
    std::string root_directory; // Kök dizini sakla

    bool readFile(const std::string& path, std::string& content);
    std::string getMimeType(const std::string& path);
};