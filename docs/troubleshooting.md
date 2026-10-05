# Sorun Giderme

## Ekran hiç açılmıyor

- OLED VCC -> 3V3
- OLED GND -> GND
- OLED SCL -> D1
- OLED SDA -> D2
- I2C adresinin `0x3C` olduğundan emin ol.
- Ekranın SH1106 olup olmadığını kontrol et.

## Ekranda karıncalanma / bozuk görüntü var

SSD1306 kodu SH1106 ekranda bozuk görüntü verebilir. Bu projede `Adafruit_SH110X` ve `Adafruit_SH1106G` kullanılır.

## TTP223 çalışmıyor

- OUT -> D5
- VCC -> 3V3
- GND -> GND
- Sensöre dokunduğunda üzerindeki LED'in tepki verip vermediğini kontrol et.

## Kod derlenmiyor

Şu kütüphanelerin kurulu olduğundan emin ol:
- Adafruit GFX Library
- Adafruit SH110X

## Kart bilgisayarda görünmüyor

NodeMCU V3 CH340 kullanıyorsan bilgisayarda CH340 sürücüsü gerekebilir.

## Oyun çok hızlı / yavaş

Kod içindeki:
- `gameSpeed`
- `gravity`
- `jumpPower`

değerleri değiştirilebilir.
