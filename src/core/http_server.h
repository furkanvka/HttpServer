#pragma once

#include "http/http_message.h"
#include "file_handler.h"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sstream>
#include <cstring>
#include <thread>
#include <string>

#include "thread_pool.h"

class HttpServer {
public:
    HttpServer(int port, const std::string& root_path);
    void start();

private:
    int server_fd;
    int port;
    FileHandler file_handler;
    ThreadPool pool; // <-- Thread pool nesnemiz eklendi

    void handle_connection(int client_socket);
    void send_response(int client_socket, const HttpResponse& response);
    HttpResponse process_request(const HttpRequest& request);
};