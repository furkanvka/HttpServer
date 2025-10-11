# 🧩 Simple HTTP Server (C++)

Bu proje, yalnızca **C++ ve POSIX soketleri** kullanılarak sıfırdan yazılmış basit bir **HTTP 1.1 sunucusudur**.  
Amaç, düşük seviye ağ programlama ve HTTP protokolü mantığını öğrenmektir.

---

## 🚀 Özellikler
- HTTP/1.1 desteği  
- GET isteklerini işleme  
- Temel yönlendirme ("/" ve "/about")  
- Connection: keep-alive & close yönetimi  
- Tarayıcı (Firefox, Chrome, curl) uyumlu  
- Tamamen STL + POSIX ile yazılmıştır (ek kütüphane yok)

---

## 📁 Proje Yapısı
.
├── http_server.h
├── http_server.cpp
├── http_request.h
├── http_response.h
├── main.cpp
└── Makefile

## 🧱 Kullanım

# 1️⃣ CMake yapı klasörü oluştur
mkdir build && cd build

# 2️⃣ CMake ile yapılandır
cmake ..

# 3️⃣ Derle
make

# 4️⃣ Çalıştır
./server
Süper fikir ⚡
Aşağıda projen için sade, profesyonel ve açıklayıcı bir **README.md** örneği verdim.
Bu, senin şu anda yazdığın basit C++ HTTP sunucusuna göre hazırlandı 👇

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


