#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RTClib.h>
#include <DHT.h>

// Definícia veľkosti displeja
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Inicializácia displeja (I2C) s nastavenou I2C adresou 0x3C
#define OLED_I2C_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Nastavenie pinov pre DHT senzor
#define DHT_PIN 4  // Zmeň podľa pripojenia
#define DHT_TYPE DHT11  // Typ senzora (DHT11 alebo DHT22)

// Nastavenie pinov pre SGW signál
#define SGW_PIN 16  // Tento pin bude používať SGW signál (zmeň podľa pripojenia)

// Inicializácia DHT senzora
DHT dht(DHT_PIN, DHT_TYPE);

// Inicializácia DS3231 RTC
RTC_DS3231 rtc;

volatile bool updateTime = false;  // Flag na označenie, že čas je potrebné aktualizovať

unsigned long lastMeasurementTime = 0;
unsigned long measurementInterval = 1000;  // Interval na meranie (1 sekundu)

// Funkcia pre obsluhu prerušenia SGW signálu
void IRAM_ATTR sgwInterrupt() {
  updateTime = true;  // Nastavíme flag, že je potrebné aktualizovať čas
}

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
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Nastaví aktuálny čas na čas kompilácie
  }

  // Nastavenie SGW pin ako vstup s prerušením na vzostupnú hranu
  pinMode(SGW_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(SGW_PIN), sgwInterrupt, RISING);  // Aktivuje prerušenie na vzostupnú hranu signálu

  Serial.println("SGW interrupt setup complete.");
}

void loop() {
  // Skontrolujeme, či uplynul interval na meranie
  if (millis() - lastMeasurementTime >= measurementInterval) {
    lastMeasurementTime = millis();  // Aktualizuj čas posledného merania

    // Meranie teploty a vlhkosti
    float temperature = dht.readTemperature();  // Meranie teploty v °C
    float humidity = dht.readHumidity();        // Meranie vlhkosti v %

    // Ak je požiadavka na aktualizáciu času (signal SGW bol detekovaný)
    if (updateTime) {
      // Resetujeme flag na neaktualizovaný čas
      updateTime = false;

      // Čítanie aktuálneho času z DS3231
      DateTime now = rtc.now();

      // Pridáme jednu sekundu k času
      now = now + 1;

      // Aktualizujeme čas na RTC
      rtc.adjust(now);

      // Zobrazenie dátumu a času na sériový monitor
      Serial.print("Updated Time: ");
      Serial.print(now.hour() < 10 ? "0" : "");
      Serial.print(now.hour());
      Serial.print(":");
      Serial.print(now.minute() < 10 ? "0" : "");
      Serial.print(now.minute());
      Serial.print(":");
      Serial.print(now.second() < 10 ? "0" : "");
      Serial.println(now.second());
    }

    // Zobrazenie údajov na OLED displeji
    DateTime now = rtc.now();
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

    display.println ("Miestnost: Spajza");

    display.display();  // Aktualizácia displeja

    delay(100); // Krátka oneskorenie
  }
}
