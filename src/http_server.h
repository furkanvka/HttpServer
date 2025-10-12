#pragma once

#include "http_request.h"
#include "http_response.h"
#include <iostream>



/**
 * @class HttpServer
 * @brief Temel TCP soket işlemlerini ve HTTP/1.1 bağlantı döngüsünü yönetir.
 */
class HttpServer {
public:
    /**
     * @brief Sunucuyu belirtilen portta başlatır.
     * @param port Dinlenecek port numarası.
     */
    HttpServer(int port);
    
    /**
     * @brief Sunucunun ana döngüsünü başlatır ve gelen bağlantıları dinler.
     */
    void start();

private:
    int server_fd; // Ana sunucu soket tanımlayıcısı
    int port;
    void handle_connection(int client_socket);
    void send_response(int client_socket, const HttpResponse& response);
    
    bool parse_request(const std::string& raw_request, HttpRequest& request); 


    std::string get_mime_type(const std::string& extension);
    bool read_file_to_body(const std::string& filename, std::string& out_body);
    HttpResponse process_request(const HttpRequest& request);
};