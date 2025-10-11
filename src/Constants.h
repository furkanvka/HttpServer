#pragma once

#include <string>

namespace Constants {
    // ------------------------------------
    // HTTP VERSION
    // ------------------------------------
    const std::string HTTP_VERSION = "HTTP/1.1";
    
    // ------------------------------------
    // STATUS CODES (DURUM KODLARI)
    // ------------------------------------
    // Başarılı Kodlar (Success)
    const std::string STATUS_200 = "200 OK";
    const std::string STATUS_201 = "201 Created";
    const std::string STATUS_204 = "204 No Content";
    
    // Yönlendirme Kodları (Redirection)
    const std::string STATUS_301 = "301 Moved Permanently";
    const std::string STATUS_302 = "302 Found";

    // İstemci Hata Kodları (Client Error)
    const std::string STATUS_400 = "400 Bad Request";
    const std::string STATUS_401 = "401 Unauthorized";
    const std::string STATUS_403 = "403 Forbidden";
    const std::string STATUS_404 = "404 Not Found";
    const std::string STATUS_405 = "405 Method Not Allowed";
    const std::string STATUS_411 = "411 Length Required"; // POST'ta Content-Length yoksa

    // Sunucu Hata Kodları (Server Error)
    const std::string STATUS_500 = "500 Internal Server Error";
    const std::string STATUS_501 = "501 Not Implemented";

    // ------------------------------------
    // HTTP METOTLARI (Methods)
    // ------------------------------------
    const std::string METHOD_GET = "GET";
    const std::string METHOD_POST = "POST";
    const std::string METHOD_PUT = "PUT";
    const std::string METHOD_DELETE = "DELETE";
    const std::string METHOD_HEAD = "HEAD";
    
    // ------------------------------------
    // GENEL BAŞLIK DEĞERLERİ (Common Header Values)
    // ------------------------------------
    const std::string HEADER_CONNECTION = "Connection";
    const std::string HEADER_CONTENT_TYPE = "Content-Type";
    const std::string HEADER_CONTENT_LENGTH = "Content-Length";
    
    const std::string VALUE_CONN_CLOSE = "close";
    const std::string VALUE_CONN_KEEPALIVE = "keep-alive";
    
    // ------------------------------------
    // MINE TİPLERİ (Common Content-Types)
    // ------------------------------------
    const std::string MIME_HTML = "text/html";
    const std::string MIME_PLAIN = "text/plain";
    const std::string MIME_JSON = "application/json";
    const std::string MIME_OCTET_STREAM = "application/octet-stream";
    
} // namespace Constants