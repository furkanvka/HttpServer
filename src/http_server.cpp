
#include "http_server.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <cstring>

HttpServer::HttpServer(int port){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // Tüm IP'lerden bağlantı kabul et
    address.sin_port = htons(port);        // Portu ağ byte sırasına çevir

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) { // 10 = bekleyen maksimum bağlantı sayısı
        perror("listen hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "[+] Sunucu " << port << " portunda dinliyor..." << std::endl;
}


void HttpServer::start() {
    if (server_fd < 0) {
        std::cerr << "Hata: Sunucu soketi baslatilmadi. start() iptal ediliyor." << std::endl;
        return;
    }

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_addr_size = sizeof(client_address);
        
        // 1. Yeni bağlantıyı kabul et
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addr_size);   

        if (client_socket < 0) {
            perror("accept hatası"); 
            continue; 
        }
        
        std::cout << "[+] Yeni bağlantı kabul edildi. Soket FD: " << client_socket << std::endl;
        handle_connection(client_socket);
    }
}

void HttpServer::handle_connection(int client_socket){
    bool keep_alive = true;
    int size,flag;
    char buffer[4096];

    HttpRequest request;
    HttpResponse response;

    while(keep_alive){
        ssize_t bytes_read = recv(client_socket,buffer,sizeof(buffer), 0);
        if (bytes_read <= 0) {
            break; 
        }

        // burda bitler string oluyor.
        std::string raw_request(buffer, bytes_read); 

        request.clear(); 

        
        if (parse_request(raw_request, request)) {
            response = process_request(request);
            send_response(client_socket,response);
            
            auto it = request.headers.find("Connection");
            if(it != request.headers.end() && it->second == "close"){
                keep_alive = false;
            }

        } else {
            // hata çözcez burda
            break; 
        }
    }

    close(client_socket);
}


    
bool HttpServer::parse_request(const std::string& raw_request, HttpRequest& request) {
    std::istringstream stream(raw_request);
    std::string line;

    if (!std::getline(stream, line))
        return false;
    if (!line.empty() && line.back() == '\r')
        line.pop_back();

    std::istringstream request_line(line);
    request_line >> request.method >> request.path >> request.version;

    if (request.method.empty() || request.path.empty() || request.version.empty())
        return false;

    while (std::getline(stream, line)) {
        if (line == "\r" || line == "" || line == "\n")
            break;
        if (line.back() == '\r')
            line.pop_back();

        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            while (!value.empty() && value.front() == ' ')
                value.erase(0, 1);

            request.headers[key] = value;
        }
    }

    std::string body;
    while (std::getline(stream, line))
        body += line + "\n";

    if (!body.empty() && body.back() == '\n')
        body.pop_back();

    request.body = body;

    std::cout << "=== HTTP Request Parsed ===\n";
    std::cout << "Method: " << request.method << "\n";
    std::cout << "Path: " << request.path << "\n";
    std::cout << "Version: " << request.version << "\n";
    for (auto& [k, v] : request.headers)
        std::cout << k << ": " << v << "\n";
    if (!request.body.empty())
        std::cout << "Body: " << request.body << "\n";
    std::cout << "===========================\n";

    return true;
}

void HttpServer::send_response(int client_socket, const HttpResponse& response) {
    std::stringstream response_stream;

    // 1. Status Line (Durum Satırı)
    // Örn: HTTP/1.1 200 OK
    response_stream << "HTTP/1.1 " << response.statusCode << " OK\r\n";

    // Yanıtın gövdesi varsa Content-Length eklenmeli
    response_stream << "Content-Length: " << response.body.length() << "\r\n";
    
    // 2. Headers (Başlıklar)
    for (const auto& pair : response.headers) {
        response_stream << pair.first << ": " << pair.second << "\r\n";
    }

    // 3. Başlıklar ve Gövde Arasındaki Boş Satır
    response_stream << "\r\n";

    // 4. Body (Gövde)
    response_stream << response.body;

    std::string final_response = response_stream.str();
    
    // İstemciye yanıtı gönder
    ssize_t bytes_sent = send(client_socket, final_response.c_str(), final_response.length(), 0);

    if (bytes_sent < 0) {
        perror("send hatası");
    } else {
        std::cout << "[+] Yanıt gönderildi. Status: " << response.statusCode << std::endl;
    }
}

HttpResponse HttpServer::process_request(const HttpRequest& request) {
    HttpResponse response;

    auto it = request.headers.find(Constants::HEADER_CONNECTION);
    if(it != request.headers.end() && it->second == Constants::VALUE_CONN_KEEPALIVE){
        response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_KEEPALIVE;
    } else {
        response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_CLOSE;
    }

    // --- TEMEL HATA YÖNETİMİ: Metot Kontrolü (Sadece GET destekliyoruz) ---
    if (request.method != Constants::METHOD_GET) {
        response.statusCode = Constants::STATUS_405; // 405 Method Not Allowed
        
        response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
        response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_CLOSE; // Hata sonrası kapat
        
        response.body = "<html><body><h1>" + Constants::STATUS_405 + "</h1><p>Bu sunucu sadece GET metodunu destekler.</p></body></html>";
        return response;
    }
    
    // --- URL YÖNLENDİRME (ROUTING) MANTIĞI ---

    if (request.path == "/") {
        response.statusCode = Constants::STATUS_200; // 200 OK
        response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
        
        response.body = 
            "<html><body>"
            "<h1>Hos geldiniz! Bu C++ Sunucusu</h1>"
            "<p>Ana sayfa basarili.</p>"
            "<p><a href='/about'>Hakkinda</a></p>"
            "</body></html>";

    } else if (request.path == "/about") {
        response.statusCode = Constants::STATUS_200; // 200 OK
        response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
        
        response.body = 
            "<html><body>"
            "<h1>Hakkinda</h1>"
            "<p>Bu sunucu sifirdan C++ ile yazilmistir.</p>"
            "<p><a href='/'>Ana Sayfa</a></p>"
            "</body></html>";

    } else {
        // --- 404 Not Found (Bulunamadı) ---
        response.statusCode = Constants::STATUS_404;
        response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
        
        response.body = 
            "<html><body>"
            "<h1>" + Constants::STATUS_404 + "</h1>"
            "<p>Istediginiz kaynak bulunamadi: <code>" + request.path + "</code></p>"
            "</body></html>";
    }
    
    return response;
}