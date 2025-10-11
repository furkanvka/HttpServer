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

    /**
     * @brief Yeni bir istemci bağlantısını işler (Keep-Alive döngüsü burada gerçekleşir).
     * @param client_socket İstemciye ait soket tanımlayıcısı.
     */
    void handle_connection(int client_socket);
    
    /**
     * @brief Cevabı soket üzerinden gönderir.
     * @param client_socket Hedef soket.
     * @param response Gönderilecek HttpResponse nesnesi.
     */
    void send_response(int client_socket, const HttpResponse& response);
    
    /**
     * @brief Ham isteği (string) ayrıştırır ve HttpRequest nesnesini doldurur.
     * @param raw_request Ham HTTP isteği verisi.
     * @param request Doldurulacak HttpRequest nesnesi.
     * @return Ayrıştırma başarılıysa true, değilse false.
     */
    bool parse_request(const std::string& raw_request, HttpRequest& request); 

    /**
     * @brief Ayrıştırılmış isteği işler ve bir cevap oluşturur.
     * @param request Gelen HttpRequest nesnesi.
     * @return İstemciye gönderilmeye hazır HttpResponse nesnesi.
     */
    HttpResponse process_request(const HttpRequest& request);
};