#include "http_server.h"
#include "http_request.h"
#include "http_response.h"
#include <iostream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <thread> // KRİTİK EKLENTİ: Multi-threading için
#include <fstream>
#include <filesystem> // C++17 ve sonrası için
#include <algorithm> 

namespace fs = std::filesystem;

// ------------------------------------
// HTTP Server oluşumu ve TCP socket işlemleri
// ------------------------------------
HttpServer::HttpServer(int port){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    this->port = port;
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen hatası");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "[+] Sunucu " << port << " portunda dinliyor..." << std::endl;
}

// ------------------------------------
// Server döngüsü (MULTI-THREAD AKTİF)
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
        
        // KRİTİK ADIM: handle_connection'ı yeni bir iş parçacığına taşı
        std::thread client_thread(&HttpServer::handle_connection, this, client_socket);
        
        // İş parçacığını ana döngüden ayır (detach)
        client_thread.detach(); 
    }
}

// ------------------------------------
// Bağlantı işlemleri 
// ------------------------------------
void HttpServer::handle_connection(int client_socket){
    bool keep_alive = true;
    char buffer[4096];

    HttpRequest request;
    HttpResponse response;

    while(keep_alive){
        // recv, bağlantı kesilene kadar bekler. Geleneksel HTTP/1.1 için bu mantık biraz basittir,
        // ama tarayıcılar kısa istekler için bunu tolere eder.
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
            
            // Connection başlığını kontrol et, ancak performansı artırmak için kapatmayı zorlayacağız.
            auto it = response.headers.find(Constants::HEADER_CONNECTION);
            if(it != response.headers.end() && it->second == Constants::VALUE_CONN_CLOSE){
                keep_alive = false;
            } else {
                // Eğer yanıt başlığında close yoksa, mevcut connection durumuna bak.
                auto req_conn_it = request.headers.find(Constants::HEADER_CONNECTION);
                if(req_conn_it == request.headers.end() || req_conn_it->second == Constants::VALUE_CONN_CLOSE) {
                    keep_alive = false;
                }
            }

        } else {
            // Parsing hatası durumunda bağlantıyı kapat
            break; 
        }
    }

    close(client_socket);
    std::cout << "[-] Bağlantı kapatıldı. Soket FD: " << client_socket << std::endl;
}


// ------------------------------------
// Http request parser
// ... (parse_request fonksiyonu aynı kalır) ...

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

    // HTTP Yanıtı Formatı Düzeltildi: response.statusCode (Örn: "200 OK") arkasına tekrar " OK" eklenmiyor.
    response_stream << "HTTP/1.1 " << response.statusCode << "\r\n"; 

    // Content-Length zorunludur.
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

const std::string PUBLIC_ROOT = "/home/furkan/Documents/http/public"; 

HttpResponse HttpServer::process_request(const HttpRequest& request) {
    HttpResponse response;

    // PERFORMANS OPTİMİZASYONU: Bağlantıyı her zaman kapatmaya zorla (Gecikmeyi önler)
    // keep-alive yönetimi karmaşık ve hatalı implementasyonda yavaşlığa neden olur.
    response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_CLOSE;
    
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
    // Baştaki '/' karakterini kaldırarak dosya sistemi birleşimini güvenli hale getiriyoruz.
    while (requested_path.length() > 0 && requested_path.front() == '/') {
        requested_path.erase(0, 1);
    }
    // Ana sayfa ise index.html olarak ayarla
    if (requested_path.empty()) {
        requested_path = "index.html"; 
    }

    fs::path full_path = fs::path(PUBLIC_ROOT) / requested_path;

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
// Dosya okuma optimizasyonu (tek seferde okuma)
bool HttpServer::read_file_to_body(const std::string& filename, std::string& out_body) {
    
    std::ifstream file(filename, std::ios::in | std::ios::binary); 
    
    if (!file.is_open()) {
        std::cerr << "[-] HATA: Dosya acilamadi veya bulunamadi: " << filename << std::endl;
        return false;
    }

    try {
        // Dosya okuma optimizasyonu
        size_t file_size = fs::file_size(filename); 
        out_body.resize(file_size);
        file.read(out_body.data(), file_size);

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
