#include "cli.h"
#include <iostream>
#include <string>
#include <vector>

namespace cli {

    void print_help() {
        std::cout << "Kullanım: my_server <komut> [seçenekler]\n\n";
        std::cout << "Komutlar:\n";
        std::cout << "  server       HTTP sunucusunu başlatır.\n\n";
        std::cout << "Global Seçenekler:\n";
        std::cout << "  -h, --help   Yardım menüsünü gösterir.\n";
        std::cout << "  -v, --version Versiyon bilgisini gösterir.\n\n";
        std::cout << "Sunucu Seçenekleri (server komutu ile kullanılır):\n";
        std::cout << "  -p, --port   Sunucunun dinleyeceği portu belirler (Varsayılan: 8080).\n";
    }

    void print_version() {
        std::cout << "MyCppServer v1.0.0\n";
    }

    void create_server(int port, const std::string& root_path) {
        std::cout << "[INFO] Root dizini: " << root_path << std::endl;        
        HttpServer server(port, root_path);
        server.start();
    }
    int parse_arguments(int argc, char* argv[]) {
        std::vector<std::string> args(argv + 1, argv + argc);

        if (args.empty()) {
            print_help();
            return 1;
        }

        std::string command = args[0];

        if (command == "-h" || command == "--help") {
            print_help();
            return 0;
        } else if (command == "-v" || command == "--version") {
            print_version();
            return 0;
        } else if (command == "server") {
            int port = 8080;
            std::string root_path = "../public"; // Varsayılan değer

            // Döngü burada başlıyor
            for (size_t i = 1; i < args.size(); ++i) {
                if (args[i] == "-p" || args[i] == "--port") {
                    if (i + 1 < args.size()) {
                        port = std::stoi(args[++i]);
                    } else {
                        std::cerr << "[-] Hata: --port için değer belirtilmedi!\n";
                        return 1;
                    }
                } else if (args[i] == "-r" || args[i] == "--root") {
                    if (i + 1 < args.size()) {
                        root_path = args[++i];
                    } else {
                        std::cerr << "[-] Hata: --root için değer belirtilmedi!\n";
                        return 1;
                    }
                } else {
                    std::cerr << "[-] Bilinmeyen seçenek: " << args[i] << "\n";
                    return 1;
                }
            } // Döngü parantezi burada kapanıyor

            // Döngüden çıktıktan sonra sunucuyu başlat
            create_server(port, root_path);

        } else {
            std::cerr << "[-] Bilinmeyen komut: " << command << "\n\n";
            print_help();
            return 1;
        }

        return 0;
    }
}
