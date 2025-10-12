
#include "http_server.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <cstring>

#include <fstream>
#include <filesystem> // C++17 ve sonrası için
#include <algorithm> 

namespace fs = std::filesystem;

// ------------------------------------
// HTTP Server oluşmu ve tcp socket işlemleri
// ------------------------------------
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

// ------------------------------------
// Server döngüsü "ilerde multi-thread eklencek"
// ------------------------------------
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

// ------------------------------------
// Baglantı işlemleri 
// ------------------------------------
void HttpServer::handle_connection(int client_socket){
    bool keep_alive = true;
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


// ------------------------------------
// Http request parser
// ------------------------------------
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

// ------------------------------------
// Http responser 
// ------------------------------------
void HttpServer::send_response(int client_socket, const HttpResponse& response) {
    std::stringstream response_stream;

    response_stream << "HTTP/1.1 " << response.statusCode << " OK\r\n";

    response_stream << "Content-Length: " << response.body.length() << "\r\n";
    
    for (const auto& pair : response.headers) {
        response_stream << pair.first << ": " << pair.second << "\r\n";
    }

    response_stream << "\r\n";

    response_stream << response.body;

    std::string final_response = response_stream.str();
    
    ssize_t bytes_sent = send(client_socket, final_response.c_str(), final_response.length(), 0);

    if (bytes_sent < 0) {
        perror("send hatası");
    } else {
        std::cout << "[+] Yanıt gönderildi. Status: " << response.statusCode << std::endl;
    }
}

// ------------------------------------
// Http Processer
// ------------------------------------

// html ve css bulundugu kısım
const std::string PUBLIC_ROOT = "/home/furkan/Documents/http/public"; 

HttpResponse HttpServer::process_request(const HttpRequest& request) {
    HttpResponse response;

    // 1. Connection Header Yönetimi
    auto it = request.headers.find(Constants::HEADER_CONNECTION);
    if (it != request.headers.end() && it->second == Constants::VALUE_CONN_KEEPALIVE) {
        response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_KEEPALIVE;
    } else {
        response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_CLOSE;
    }

    // 2. Sadece GET metodu destekleniyor
    if (request.method != Constants::METHOD_GET) {
        response.statusCode = Constants::STATUS_405;
        response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
        response.body =
            "<html><body><h1>" + Constants::STATUS_405 + "</h1>"
            "<p>Bu sunucu sadece GET metodunu destekler.</p></body></html>";
        return response;
    }

    // 3. Statik Dosya Servisi
    std::string requested_path = request.path;
    if (requested_path == "/") {
        requested_path = "/index.html"; 
    }

    fs::path full_path = fs::path(PUBLIC_ROOT) / fs::path(requested_path).relative_path();

    std::cout << "[i] Statik dosya araniyor: " << full_path.string() << std::endl;

    if (fs::exists(full_path) && fs::is_regular_file(full_path)) {
        response.headers[Constants::HEADER_CONTENT_TYPE] = get_mime_type(full_path.extension().string());
        if (read_file_to_body(full_path.string(), response.body)) {
            response.statusCode = Constants::STATUS_200;
            return response;
        } else {
            response.statusCode = Constants::STATUS_500;
            response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
            response.body =
                "<h1>" + Constants::STATUS_500 + "</h1>"
                "<p>Sunucu dosya okuma hatasi: Izin yetersiz veya okunamiyor.</p>";
            return response;
        }
    }

    // 4. Statik dosya yoksa: 404 NOT FOUND
    response.statusCode = Constants::STATUS_404;
    response.headers[Constants::HEADER_CONTENT_TYPE] = Constants::MIME_HTML;
    response.body =
        "<html><body>"
        "<h1>" + Constants::STATUS_404 + "</h1>"
        "<p>Istediginiz kaynak bulunamadi: <code>" + request.path + "</code></p>"
        "</body></html>";

    return response;
}


// helper function 

bool HttpServer::read_file_to_body(const std::string& filename, std::string& out_body) {
    
    // 1. Dosyayı ikili modda aç
    std::ifstream file(filename, std::ios::in | std::ios::binary); 
    
    if (!file.is_open()) {
        std::cerr << "[-] HATA: Dosya acilamadi veya bulunamadi: " << filename << std::endl;
        return false;
    }

    try {
        // 2. Dosya boyutunu öğren (fs::file_size C++17 ile çok daha hızlı)
        // Bu, string'i önceden ayırmamızı sağlar.
        size_t file_size = fs::file_size(filename); 
        
        // 3. String'i önceden ayır (Tek bir bellek tahsisi)
        out_body.resize(file_size);

        // 4. Dosyanın tamamını tek bir okuma işleminde string buffer'ına oku
        // out_body.data(), C++17'den itibaren mutable (değiştirilebilir) bir pointer sağlar.
        file.read(out_body.data(), file_size);

        // Okuma başarılı mıydı kontrol et (Dosyanın sonuna ulaştı mı?)
        if (!file) {
            std::cerr << "[-] HATA: Dosya tam olarak okunamadi (I/O hatasi): " << filename << std::endl;
            return false;
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "[-] HATA: Dosya boyutu alinirken hata: " << e.what() << std::endl;
        file.close();
        return false;
    }
    
    file.close(); 
    return true;
}

std::string HttpServer::get_mime_type(const std::string& extension) {
    if (extension == ".html" || extension == ".htm") return Constants::MIME_HTML;
    if (extension == ".css") return "text/css";
    if (extension == ".js") return "application/javascript";
    if (extension == ".json") return Constants::MIME_JSON;
    if (extension == ".png") return "image/png";
    if (extension == ".jpg" || extension == ".jpeg") return "image/jpeg";
    if (extension == ".gif") return "image/gif";
    
    return Constants::MIME_OCTET_STREAM; 
}