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
- Çoklu istemci desteği

## 📁 Proje Yapısı

├── http_server.h

├── http_server.cpp

├── http_request.h

├── http_response.h

├── main.cpp

└── Makefile


## 🧱 Kullanım

### 🔹 1. CMake yapı klasörü oluştur

```bash
mkdir build && cd build
```

### 🔹 2. CMake ile yapılandır

```bash
cmake ..
```

### 🔹 3. Derle

```bash
make
```

### 🔹 4. Sunucuyu çalıştır

```bash
./server
```


## 📚 Öğrenme Odakları

Bu proje aşağıdaki konuların öğrenilmesi için uygundur:

* TCP/IP temelleri
* HTTP protokol yapısı
* Soket programlama (bind, listen, accept, recv, send)
* HTTP isteklerinin parse edilmesi
* C++ stream ve string işlemleri


## 🛠️ Gelecek Geliştirmeler

* [x] Çoklu istemci desteği (`std::thread` veya `select()`)
* [ ] POST istek desteği
* [ ] Statik dosya sunumu (HTML, CSS, JS)
* [ ] Loglama sistemi
* [ ] Basit web API yapısı

