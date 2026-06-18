#include "http_server.h"
#include "http_parser.h"

HttpServer::HttpServer(int port, const std::string& root_path) : file_handler(root_path),pool(std::thread::hardware_concurrency()){
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

void HttpServer::start() {
    if (server_fd < 0) {
        std::cerr << "Hata: Sunucu soketi baslatilmadi. start() iptal ediliyor." << std::endl;
        return;
    }

    while (true) {
        struct sockaddr_in client_address;
        socklen_t client_addr_size = sizeof(client_address);
        
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, &client_addr_size);   

        if (client_socket < 0) {
            perror("accept hatası"); 
            continue; 
        }
        
        std::cout << "[+] Yeni bağlantı kabul edildi. Soket FD: " << client_socket << std::endl;
        
        // ESKİ KOD: std::thread client_thread(&HttpServer::handle_connection, this, client_socket);
        // ESKİ KOD: client_thread.detach(); 

        // YENİ KOD: İşi doğrudan thread havuzuna fırlatıyoruz!
        pool.enqueue(&HttpServer::handle_connection, this, client_socket);
        
    }
}

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

        std::string raw_request(buffer, bytes_read); 
        request.clear(); 
        
        if (HttpParser::parse_request(raw_request, request)) {
            response = process_request(request);
            send_response(client_socket,response);
            
            auto it = response.headers.find(Constants::HEADER_CONNECTION);
            if(it != response.headers.end() && it->second == Constants::VALUE_CONN_CLOSE){
                keep_alive = false;
            } else {
                auto req_conn_it = request.headers.find(Constants::HEADER_CONNECTION);
                if(req_conn_it == request.headers.end() || req_conn_it->second == Constants::VALUE_CONN_CLOSE) {
                    keep_alive = false;
                }
            }
        } else {
            break; 
        }
    }

    close(client_socket);
    std::cout << "[-] Bağlantı kapatıldı. Soket FD: " << client_socket << std::endl;
}

void HttpServer::send_response(
    int client_socket,
    const HttpResponse& response)
{
    std::string raw_response = response.toString();

    send(client_socket,raw_response.c_str(),raw_response.size(),0);
}


// http_server.cpp içinde
HttpResponse HttpServer::process_request(const HttpRequest& request)
{
    // Request path'i alıyoruz (Örn: /index.html veya /style.css)
    std::string path = request.path;

    // FileHandler kullanarak dosyayı servis et
    HttpResponse response = file_handler.serve(path);

    // Eğer istersen buraya ek header'lar ekleyebilirsin
    response.headers[Constants::HEADER_CONNECTION] = Constants::VALUE_CONN_CLOSE;

    return response;
}