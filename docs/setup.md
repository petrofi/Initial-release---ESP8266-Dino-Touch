# Kurulum Rehberi

## 1. Donanım bağlantısı

### OLED
- GND -> GND
- VCC -> 3V3
- SCL -> D1 / GPIO5
- SDA -> D2 / GPIO4

### TTP223
- GND -> GND
- VCC -> 3V3
- OUT -> D5 / GPIO14

## 2. Arduino IDE

Kart:
`NodeMCU 1.0 (ESP-12E Module)`

Gerekli kütüphaneler:
- Adafruit GFX Library
- Adafruit SH110X

## 3. Yükleme

1. NodeMCU'yu USB veri kablosu ile bilgisayara bağla.
2. Doğru COM portunu seç.
3. `ESP8266_Dino_Touch.ino` dosyasını aç.
4. Yükle düğmesine bas.
5. Yükleme sonunda `Hash of data verified` mesajı görülmesi normaldir.

## 4. Kullanım

TTP223 sensörüne dokun:
- Başlangıç ekranında oyun başlar.
- Oyun sırasında Dino zıplar.
- Game Over ekranında yeni oyun başlar.
