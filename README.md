# 🧩 Simple HTTP Server (C++)

Bu proje, yalnızca **C++ ve POSIX soketleri** kullanılarak sıfırdan yazılmış basit bir **HTTP 1.1 sunucusudur**.  
Amaç, düşük seviye ağ programlama ve HTTP protokolü mantığını öğrenmektir.

## 🚀 Özellikler
- HTTP/1.1 desteği  
- GET isteklerini işleme  
- Temel yönlendirme ("/" ve "/about")  
- Connection: keep-alive & close yönetimi  
- Tarayıcı (Firefox, Chrome, curl) uyumlu  
- Tamamen STL + POSIX ile yazılmıştır

## 📁 Proje Yapısı
.
├── http_server.h
├── http_server.cpp
├── http_request.h
├── http_response.h
├── main.cpp
└── Makefile

## 🧱 Kullanım
** CMake yapı klasörü oluştur **
mkdir build && cd build

** CMake ile yapılandır **
cmake ..

** Derle**
make

**Çalıştır**
./server

## 📚 Öğrenme Odakları

Bu proje aşağıdaki konuların öğrenilmesi için uygundur:

* TCP/IP temelleri
* HTTP protokol yapısı
* Soket programlama (bind, listen, accept, recv, send)
* HTTP isteklerinin parse edilmesi
* C++ stream ve string işlemleri

---

## 🛠️ Gelecek Geliştirmeler

* [ ] Çoklu istemci desteği (`std::thread` veya `select()`)
* [ ] POST istek desteği
* [ ] Statik dosya sunumu (HTML, CSS, JS)
* [ ] Loglama sistemi
* [ ] Basit web API yapısı


