#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <DHT.h>
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// Definícia veľkosti displeja
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Inicializácia displeja (I2C) s nastavenou I2C adresou 0x3C
#define OLED_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Nastavenie pinov pre DHT senzor
#define DHT_PIN 4  // Zmeň podľa pripojenia
#define DHT_TYPE DHT11  // Typ senzora (DHT11 alebo DHT22)

// Inicializácia DHT senzora
DHT dht(DHT_PIN, DHT_TYPE);

// Inicializácia DS3231 RTC
RTC_DS3231 rtc;

// WiFi a NTP nastavenia
const char* ssid = "TP-Link";       // Nastavte na vašu Wi-Fi sieť
const char* password = "Moje_Heslo"; // Heslo vymyslene

WiFiUDP udp;
NTPClient timeClient(udp, "pool.ntp.org", 3600, 60000); // Získava čas z NTP servera, posunutie o 1 hodinu (3600 sekúnd)

unsigned long lastMeasurementTime = 0;
unsigned long measurementInterval = 1000;  // Interval na meranie (1 sekundu)

void setup() {
  // Inicializácia sériového monitora
  Serial.begin(115200);
  Serial.println("Zacinam...");

  // Inicializácia I2C pre OLED displej
  Wire.begin();
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS)) {
    Serial.println("Nepodarilo sa inicializovať OLED displej");
    for (;;);  // Nekonečná slučka, ak inicializácia zlyhá
  }
  display.clearDisplay();

  // Inicializácia DHT senzora
  dht.begin();

  // Inicializácia DS3231 RTC
  if (!rtc.begin()) {
    Serial.println("Nepodarilo sa inicializovať RTC modul!");
    for (;;);
  }

  // Skontroluj, či RTC beží
  if (rtc.lostPower()) {
    Serial.println("RTC stratilo napájanie, nastavujem aktuálny čas...");
    rtc.adjust(DateTime(2025, 2, 1, 12, 50, 0)); // Nastavte čas podľa potreby (napr. 2025, 2, 1, 12:50:00)
  } else {
    Serial.println("RTC je nastavený a beží.");
  }

  // Pripojenie na WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Pripajam sa na WiFi...");
  }
  Serial.println("Pripojenie na WiFi úspešné!");

  // Inicializácia NTP klienta
  timeClient.begin();
  timeClient.update();

  // Nastavenie času na RTC z NTP
  DateTime now = DateTime(timeClient.getEpochTime());
  rtc.adjust(now);

  Serial.println("Čas bol nastavený na RTC podľa NTP.");
}

void loop() {
  // Skontrolujeme, či uplynul interval na meranie
  if (millis() - lastMeasurementTime >= measurementInterval) {
    lastMeasurementTime = millis();  // Aktualizuj čas posledného merania

    // Meranie teploty a vlhkosti
    float temperature = dht.readTemperature();  // Meranie teploty v °C
    float humidity = dht.readHumidity();        // Meranie vlhkosti v %

    // Čítanie aktuálneho času z DS3231
    DateTime now = rtc.now();

    // Zobrazenie údajov na OLED displeji
    display.clearDisplay();
    display.setTextSize(1);    // Nastavenie veľkosti textu
    display.setTextColor(SSD1306_WHITE);  // Nastavenie farby textu

    // Zobrazenie dátumu
    display.setCursor(0, 0);
    display.print("Datum: ");
    display.print(now.day());
    display.print("/");
    display.print(now.month());
    display.print("/");
    display.print(now.year());

    // Zobrazenie času
    display.setCursor(0, 10);
    display.print("Cas: ");
    display.print(now.hour() < 10 ? "0" : "");
    display.print(now.hour());
    display.print(":");
    display.print(now.minute() < 10 ? "0" : "");
    display.print(now.minute());
    display.print(":");
    display.print(now.second() < 10 ? "0" : "");
    display.print(now.second());

    // Zobrazenie teploty
    display.setCursor(0, 30);
    display.print("Teplota: ");
    display.print(temperature);
    display.println(" C");

    // Zobrazenie vlhkosti
    display.setCursor(0, 40);
    display.print("Vlhkost: ");
    display.print(humidity);
    display.println(" %");

    display.display();  // Aktualizácia displeja

    // Vypísať údaje na sériový monitor
    Serial.print("Datum: ");
    Serial.print(now.day());
    Serial.print("/");
    Serial.print(now.month());
    Serial.print("/");
    Serial.println(now.year());

    Serial.print("Cas: ");
    Serial.print(now.hour() < 10 ? "0" : "");
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute() < 10 ? "0" : "");
    Serial.print(now.minute());
    Serial.print(":");
    Serial.print(now.second() < 10 ? "0" : "");
    Serial.println(now.second());

    Serial.print("Teplota: ");
    Serial.print(temperature);
    Serial.println(" C");

    Serial.print("Vlhkost: ");
    Serial.print(humidity);
    Serial.println(" %");

    delay(100); // Krátka oneskorenie
  }
}
