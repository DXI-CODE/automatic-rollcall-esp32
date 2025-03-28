#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h> // Driver Library for the LCD Module

//PRUEBA rama
// Definición de pines para I2C
#define SDA_PIN D2
#define SCL_PIN D3
#define BUZZER_PIN D5

// WiFi parameters to be configured
const char* ssid = "LAB ELECTRONICA E IA"; // Write here your router's username
const char* password = "Electro2024.#."; // Write here your router's passward

// Construct an LCD object and pass it the 
// I2C address, width (in characters) and
// height (in characters). Depending on the
// Actual device, the IC2 address may change.
LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

void imprimir(const char* cad){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(cad);
}

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
    lcd.print(data2); // Mostrar en LCD
}


void setup() {
    Serial.begin(115200);
    lcd.begin(20,4);
    lcd.init();
    lcd.backlight();

    nfc.begin();
    nfc.SAMConfig(); 

    if (!nfc.getFirmwareVersion()) {
        Serial.println("No se encontró el módulo NFC");
        while (1);
    }

    // Conexión a WiFi
    imprimir("Conectando a:");
    lcd.setCursor(0, 1);
    lcd.print(ssid);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }

    imprimir("Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(1000);
    lcd.clear();
}

void loop() {
    uint8_t success;
    uint8_t uid[7]; 
    uint8_t uidLength;

    lcd.setCursor(0, 0);
    lcd.print("21/03/21"); 
    lcd.setCursor(0, 1);
    lcd.print("Sistemas embebidos");
    lcd.setCursor(0, 2);
    lcd.print("Cierre: 8:15 AM");
    lcd.setCursor(0, 3);
    lcd.print("Escanea tu tarjeta");

    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {
        tone(BUZZER_PIN, 3000, 500);

        Serial.print("Tarjeta detectada con UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            Serial.print(uid[i], HEX);
            Serial.print(" ");
        }
        Serial.println();

        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Hora: 8:10 AM");
        imprimirdatosdetarjeta();

        delay(4000);
        lcd.clear();
    }

    delay(1000);
}