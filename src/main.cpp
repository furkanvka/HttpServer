#include "http_server.h"
#include <iostream>

int main() {
    // 8080 portunda bir HTTP sunucusu başlat
    HttpServer server(8080);

    // Bağlantıları kabul etmeye başla
    server.start();

    return 0;
}