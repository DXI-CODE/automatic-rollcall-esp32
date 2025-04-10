#include "ESP8266WiFi.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>

#define SDA_PIN D2
#define SCL_PIN D3
#define BUZZER_PIN D0
#define PIN_CS D8

const char* ssid = "LAB ELECTRONICA E IA";
const char* password = "Electro2024.#.";

char day[7][32] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};


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


    for (i = 7; i < 26; i++) {

        success = nfc.ntag2xx_ReadPage(i, data);
        
        if (success) {
            for (j = 0; j < 4; j++) {
                char caracter = (char)data[j];
                
                // Detectar el delimitador de inicio y fin
                if (caracter == '|') {
                    if (dentroDelimitador) {
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

  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.print(timeClient.getFormattedTime()); // Mostrar hora de registro

  for(int i=0, j=0; data2[j+(i*20)] != '\0'; i++){
    char aux[32] = {0};
    for(j=0; j<20 && data2[j+(i*20)] != '\0'; j++){
      aux[j] = data2[j+(i*20)];
      if(data2[j] == '\0'){
        aux[j] = data2[j+(i*20)];
      }
    }
    lcd.setCursor(0, i+1);
    lcd.print(aux); 
  }

}


void setup() {
  Serial.begin(115200);

  Serial.println("Iniciando");

  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();
  Serial.println("Pantalla lista");
  nfc.begin();
  nfc.SAMConfig();


  if (!SD.begin(PIN_CS)) {
    lcd.clear();
    lcd.print("No se encontro");
    lcd.setCursor(0, 1);
    lcd.print("una tarjeta SD!");

    while(1){
      delay(1000);
      if (SD.begin(PIN_CS)) {
        lcd.clear();
        lcd.print("Tarjeta SD");
        lcd.setCursor(0, 1);
        lcd.print("encontrada!");
        delay(1000);
        break;
      }
    }
  }else{
    lcd.clear();
    lcd.print("Tarjeta SD");
    lcd.setCursor(0, 1);
    lcd.print("encontrada!");
    delay(1000);
  }

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

  timeClient.update();
  char horaString[32];
  int8_t dia = timeClient.getDay();
  int8_t min = timeClient.getMinutes();
  int8_t hora = timeClient.getHours(); 
  if(hora < 10 && min < 10){
    sprintf(horaString, "%s 0%d:0%d Grupo:%d", day[dia], hora, min, 604);
  }else if(hora < 10 && min >= 10){
    sprintf(horaString, "%s 0%d:%d Grupo:%d", day[dia], hora, min, 604);
  }else if(hora >= 10 && min < 10){
    sprintf(horaString, "%s %d:0%d Grupo:%d", day[dia], hora, min, 604);
  }else{
    sprintf(horaString, "%s %d:%d Grupo:%d", day[dia], hora, min, 604);
  }
  

  uint8_t success;
  uint8_t uid[] = {0, 0, 0, 0, 0, 0, 0};
  uint8_t uidLength;

    if(WiFi.status() != WL_CONNECTED){
      lcd.setCursor(0, 0);
      lcd.print("Conectando a:");
      lcd.setCursor(0, 1);
      lcd.print(ssid);
      while (WiFi.status() != WL_CONNECTED) {
      delay(5000);
      WiFi.reconnect();
      }
      lcd.clear();
      lcd.print("Conectado!");
    }
    lcd.setCursor(0, 0);
    lcd.print(horaString);
    lcd.setCursor(0, 1);
    lcd.print("Sistemas embebidos");
    lcd.setCursor(0, 2);
    lcd.print("Cierre: 8:15");

    lcd.setCursor(0, 3);
    lcd.print("Escanea tu tarjeta");

  
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
    if (success) {
      lcd.clear();
    tone(BUZZER_PIN, 2500, 500);
    imprimirdatosdetarjeta();
    delay(3000);
    lcd.clear();
  }
  delay(50);
  
    
    
}