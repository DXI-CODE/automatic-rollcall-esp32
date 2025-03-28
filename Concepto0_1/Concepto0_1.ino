#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define SDA_PIN D2
#define SCL_PIN D3
#define BUZZER_PIN D5

const char* ssid = "LAB ELECTRONICA E IA";
const char* password = "Electro2024.#.";

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000); // Ajuste UTC-6 (México)

void imprimirdatosdetarjeta() {
  unsigned char data[4];
  char data2[32] = {0};
  uint8_t success;
  uint8_t i, j;

  for (i = 7; i < 23; i++) {
      success = nfc.ntag2xx_ReadPage(i, data);

      if (success) {
          for (j = 0; j < 4; j++) {
              data2[(i - 15) * 4 + j] = (char)data[j];
              if (data[j] == '|') {
                  data2[(i - 15) * 4 + j] = '\0';
                  break;
              }
          }
      } else {
          Serial.println("Error al leer la página NFC");
      }
  }

  data2[(i - 15) * 4] = '\0';
  Serial.println("\nDatos leídos de la tarjeta:");
  Serial.println(data2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(data2); // Mostrar nombre

  lcd.setCursor(0, 1);
  lcd.print("Hora: ");
  lcd.print(timeClient.getFormattedTime()); // Mostrar hora de registro
}

void setup() {
  Serial.begin(115200);
  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();

  nfc.begin();
  nfc.SAMConfig();

  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print("Conectando a WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("Conectado!");

  timeClient.begin(); // Iniciar NTP
}

void loop() {
  timeClient.update(); // Obtener hora actual

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  lcd.setCursor(0, 0);
  lcd.print("Escanea tu credencial escolar");

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    tone(BUZZER_PIN, 3000, 500);
    lcd.clear();
    imprimirdatosdetarjeta();
    delay(4000);
    lcd.clear();
  }

  delay(1000);
}
