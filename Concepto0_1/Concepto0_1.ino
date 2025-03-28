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
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000); // Ajuste UTC-6 (México)

void imprimirdatosdetarjeta() {
   unsigned char data[4]; // Solo se leen 4 bytes por página
    char data2[32] = {0};  // Inicializar el buffer con ceros
    uint8_t success;
    uint8_t i, j;
    uint8_t index = 0;
    bool dentroDelimitador = false; // Bandera para saber si estamos entre delimitadores

    for (i = 7; i < 23; i++) {
        success = nfc.ntag2xx_ReadPage(i, data);
        
        if (success) {
            for (j = 0; j < 4; j++) {
                char caracter = (char)data[j];
                
                // Detectar el delimitador de inicio y fin
                if (caracter == '|') {
                    if (dentroDelimitador) {
                        // Si ya estábamos dentro, cerrar la cadena y salir
                        data2[index] = '\0';
                        goto fin_lectura;
                    } else {
                        // Detecta el primer delimitador
                        dentroDelimitador = true;
                        continue; // Ignorar el delimitador inicial
                    }
                }

                // Solo almacenar si estamos entre delimitadores
                if (dentroDelimitador) {
                    data2[index++] = caracter;
                }
            }
        } else {
            Serial.println("Error al leer la página NFC");
        }
    }

fin_lectura:
    data2[index] = '\0'; // Asegurar la terminación de cadena

    Serial.println("\nDatos leídos de la tarjeta:");
    Serial.println(data2);

  lcd.clear();
  lcd.setCursor(0, 0);
  // Imprimir solo una parte del nombre en la primera línea si es largo
  lcd.print(data2); 

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
  lcd.print("Conectando a:");
  lcd.setCursor(0, 1);
  lcd.print(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.clear();
  lcd.print("Conectado!");
  delay(1000);
  lcd.clear();
  timeClient.begin(); // Iniciar NTP
}

void loop() {
  timeClient.update(); // Obtener hora actual

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

  lcd.setCursor(0, 0);
    lcd.print("21/03/21"); 
    lcd.setCursor(0, 1);
    lcd.print("Sistemas embebidos");
    lcd.setCursor(0, 2);
    lcd.print("Cierre: 8:15");
    lcd.setCursor(8, 2);
    lcd.print(timeClient.getFormattedTime());
    lcd.setCursor(0, 3);
    lcd.print("Escanea tu tarjeta");

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    tone(BUZZER_PIN, 3000, 500);
    lcd.clear();
    imprimirdatosdetarjeta();
    delay(4000);
    lcd.clear();
  }else{
    delay(1000);
    lcd.clear();
    }

}