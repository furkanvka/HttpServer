cmake_minimum_required(VERSION 3.10)
project(SimpleHttpServer LANGUAGES CXX)

# C++ standardı
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Derleme uyarıları
add_compile_options(-Wall -Wextra -Wpedantic)

# Kaynak dosyalar
set(SOURCES
    main.cpp
    http_server.cpp
    http_request.cpp
    http_response.cpp
)

# Çıktı dosyası
add_executable(server ${SOURCES})

# POSIX socket fonksiyonları için gerekli kütüphane
target_link_libraries(server PRIVATE pthread)
