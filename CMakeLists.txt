cmake_minimum_required(VERSION 3.10)
project(SimpleHttpServer LANGUAGES CXX)

# C++ standardı
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Derleme uyarıları
add_compile_options(-Wall -Wextra -Wpedantic)

# Include klasörleri
# Başlık dosyalarının (header) bulunabilmesi için hem src hem de app dizinlerini ekliyoruz
include_directories(src)
# Kaynak dosyalar
set(SOURCES
    src/core/http_server.cpp
    src/core/http_parser.cpp
    src/core/file_handler.cpp
    src/http/http_message.cpp
    src/core/thread_pool.cpp
    src/cli/cli.cpp
    src/main.cpp
)

# Çıktı dosyası
add_executable(server ${SOURCES})

# POSIX socket ve thread işlemleri için gerekli kütüphaneler
target_link_libraries(server PRIVATE pthread)