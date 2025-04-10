#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h> // Driver Library for the LCD Module

//PRUEBA hola
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

void imprimirdatosdetarjeta(){
  unsigned char data[600];
  char data2[32];
  uint8_t success;
  uint8_t i = 15;
  uint8_t j=0;
  for (; i < 23; i++)
      {
        success = nfc.ntag2xx_ReadPage(i, data);
        
        // Display the results, depending on 'success'
        if (success)
        {
          for(j=0; j<4;j++){
            data2[j+((i-15)*4)] = (signed)data[j];
            Serial.println((char)data[j], data[j]);
            Serial.print(" ");
            Serial.print(data[j]);
            
          }
          // Dump the page data
          
        }
        else
        {
          Serial.println("Unable to read the requested page!");
        }
      }
      data2[j+((i-15)*4) +1] = '\0';
      Serial.println(data2);
      //imprimir(data2);
      //lcd.clear();
}

void setup() {
  Serial.begin(115200);
  // The begin call takes the width and height. This
  // Should match the number provided to the constructor.
  lcd.begin(20,4);
  lcd.init();// Turn on the backlight.
  lcd.backlight();

  nfc.begin();
  nfc.SAMConfig(); // Configura el módulo NFC para modo normal

  uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        Serial.println("No se encontró el módulo NFC");
        while (1); // Detiene el programa si no hay conexión
    }

  
  // Connect to WiFi
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  imprimir("Conectandose a");
      lcd.setCursor(0, 1);
      lcd.print(ssid);

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
    uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0}; // Buffer para almacenar el UID
    uint8_t uidLength;                     // Tamaño del UID
    char UID[8] = "";                      // Aseguramos que el tamaño sea suficiente

    // Actualizar la pantalla LCD con información estática
    lcd.setCursor(0, 0);
    lcd.print("21/03/21"); // Fecha
    lcd.setCursor(0, 1);
    lcd.print("Sistemas embebidos");
    lcd.setCursor(0, 2);
    lcd.print("Cierre: 8:15 AM");
    lcd.setCursor(0, 3);
    lcd.print("Escanea tu tarjeta");

    // Verifica si hay una tarjeta presente
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

    if (success) {
        // Reproducir tono de confirmación
        tone(BUZZER_PIN, 3000, 500);

        // Mostrar el UID en el monitor serial
        Serial.print("Tarjeta detectada con UID: ");
        for (uint8_t i = 0; i < uidLength; i++) {
            UID[i] = uid[i]; // Guardar el UID
            Serial.print(uid[i], HEX); // Mostrar cada byte del UID en hexadecimal
            Serial.print(" ");
        }
        Serial.println();

        // Mostrar el UID en la pantalla LCD
        lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Hora: 8:10 AM"); // Fecha
            imprimirdatosdetarjeta();
        /*for (uint8_t i = 0; i < uidLength; i++) {
            lcd.print(uid[i], HEX); // Mostrar cada byte del UID en hexadecimal
            lcd.print(" ");
        }*/
            delay(4000);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("21/03/21"); // Fecha
            lcd.setCursor(0, 1);
              lcd.print("Sistemas embebidos");
              lcd.setCursor(0, 2);
            lcd.print("Cierre: 8:15 AM");
            lcd.setCursor(0, 3);
            lcd.print("Escanea tu tarjeta");
    } else {
        // Si no hay tarjeta, actualizar el LCD con un mensaje
        lcd.setCursor(0, 3);
        lcd.print("Esperando tarjeta...");
    }

    delay(1000); // Esperar un segundo antes de volver a leer
}

-------------------------------------------------------
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <ArduinoJson.h>

#define SD_CS_PIN D8  // Pin CS para el SD

void setup() {
  Serial.begin(115200);

  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Fallo al iniciar la tarjeta SD");
    return;
  }

  Serial.println("Tarjeta SD iniciada correctamente");

  File file = SD.open("/datos.json");
  if (!file) {
    Serial.println("No se pudo abrir el archivo datos.json");
    return;
  }

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, file);

  if (error) {
    Serial.print("Error al deserializar JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* nombre = doc["nombre"];
  int edad = doc["edad"];

  Serial.print("Nombre: ");
  Serial.println(nombre);
  Serial.print("Edad: ");
  Serial.println(edad);

  file.close();
}

void loop() {
  // Nada en loop
}

--------------------------------------------------------------
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = D8; // Pin CS conectado al lector SD

void setup() {
  Serial.begin(115200);
  while (!Serial); // Espera a que el puerto serial esté listo (solo para debug)

  Serial.println("Inicializando tarjeta SD...");
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Fallo en la inicialización!");
    Serial.println("Verifica:");
    Serial.println("1. Que la tarjeta SD esté insertada");
    Serial.println("2. Las conexiones del lector SD");
    Serial.println("3. Que el archivo exista y tenga formato correcto");
    return;
  }
  Serial.println("Tarjeta SD inicializada correctamente.");

  // Abrir el archivo JSON
  File jsonFile = SD.open("/config.json");
  if (!jsonFile) {
    Serial.println("Error al abrir config.json");
    return;
  }

  // Verificar tamaño del archivo
  size_t size = jsonFile.size();
  if (size > 1024) {
    Serial.println("Archivo JSON demasiado grande");
    return;
  }

  // Leer y parsear el JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonFile);
  
  if (error) {
    Serial.print("Error al parsear JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Extraer valores del JSON
  const char* nombre = doc["nombre"];
  int valor = doc["valor"];
  bool activo = doc["activo"];

  // Mostrar valores
  Serial.println("\nValores leídos del JSON:");
  Serial.print("Nombre: "); Serial.println(nombre);
  Serial.print("Valor: "); Serial.println(valor);
  Serial.print("Activo: "); Serial.println(activo ? "SI" : "NO");

  jsonFile.close();
}

void loop() {
  // Nada aquí
}
---------------------------------------------------------------
#include <ArduinoJson.h>
#include <SPI.h>
#include <SD.h>

// Configuración del pin CS para el lector de tarjeta SD
const int chipSelect = 10; // Ajusta este valor según tu módulo

void setup() {
  Serial.begin(115200);

  // Inicializar la tarjeta SD
  if (!SD.begin(chipSelect)) {
    Serial.println("Error al inicializar la tarjeta SD");
    return;
  }

  // Abrir el archivo JSON
  File file = SD.open("datos.json");
  if (!file) {
    Serial.println("Error al abrir el archivo");
    return;
  }

  // Deserializar el archivo JSON
  DynamicJsonDocument doc(1024); // Ajusta el tamaño según tu archivo JSON
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    Serial.print("Error al leer JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Acceder a los datos del JSON
  const char* nombre = doc["nombre"];
  int edad = doc["edad"];
  Serial.println(nombre);
  Serial.println(edad);

  file.close(); // Cerrar el archivo
}

void loop() {
  // Nada que hacer aquí
}
