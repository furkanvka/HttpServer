cmake_minimum_required(VERSION 3.10)
project(SimpleHttpServer LANGUAGES CXX)

# C++ standardı
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Derleme uyarıları
add_compile_options(-Wall -Wextra -Wpedantic)

# Include klasörleri
include_directories(src)

# Kaynak dosyalar
set(SOURCES
    src/main.cpp
    src/http_server.cpp
    src/http_request.cpp
    src/http_response.cpp
)

# Çıktı dosyası
add_executable(server ${SOURCES})

# POSIX socket için gerekli kütüphane
target_link_libraries(server PRIVATE pthread)
