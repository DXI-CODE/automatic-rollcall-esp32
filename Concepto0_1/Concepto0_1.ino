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

const char* ssid = "CGA2121_eCAWRvh";
const char* password = "fRpG5gJA2HY9sAgzK3";

char day[7][32] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
char dayEleccion[7][32] = {"\"Dom\"", "\"Lun\"", "\"Mar\"", "\"Mie\"", "\"Jue\"", "\"Vie\"", "\"Sab\""};
char numClase[7][32] = {"\"0\"", "\"1\"", "\"2\"", "\"3\"", "\"4\"", "\"5\"", "\"6\""};

char *archivo;
JsonDocument clases;
uint8_t horaInicio;
uint8_t minutoInicio;
uint8_t horaCierre;
uint8_t minutoCierre;
uint8_t claseActual;
uint8_t grupo;
String nombre;

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

  archivo = (char*)malloc(sizeof(char) * 2500);
  //Tarjeta SD
  lcd.print("Leyendo SD");
  int i = 0;
  File dataFile = SD.open("Lunes2.json");
  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) { 
      archivo[i] = dataFile.read(); 
      Serial.write(archivo[i]);
      i++;
      }
    dataFile.close();
  }
  deserializeJson(clases, archivo);
  free(archivo);
  //serializeJson(clases, Serial);
  lcd.clear();
  claseActual = 0;
}

void loop() {

  timeClient.update();
  char horaString[32];
  int8_t dia = timeClient.getDay();
  int8_t min = timeClient.getMinutes();
  int8_t hora = timeClient.getHours(); 
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
  
  if(dia != 0 && dia != 6){
      if(claseActual < 2){
        horaInicio = clases[dayEleccion[dia-1]][numClase[claseActual]]["h_inicio"];
        minutoInicio = clases[dayEleccion[dia-1]][numClase[claseActual]]["m_inicio"];
        horaCierre = clases[dayEleccion[dia-1]][numClase[claseActual]]["h_fin"];
        minutoCierre = clases[dayEleccion[dia-1]][numClase[claseActual]]["m_fin"];
        grupo = clases[dayEleccion[dia-1]][numClase[claseActual]]["grupo"];
      }
    
    //nombre =  clases[dia-1][claseActual]["nombre"]).as<String>();
    

    lcd.setCursor(0, 0);
    if(hora < 10 && min < 10){
      sprintf(horaString, "%s 0%d:0%d Grupo:%d", day[dia], hora, min, grupo);
    }else if(hora < 10 && min >= 10){
      sprintf(horaString, "%s 0%d:%d Grupo:%d", day[dia], hora, min, grupo);
    }else if(hora >= 10 && min < 10){
      sprintf(horaString, "%s %d:0%d Grupo:%d", day[dia], hora, min, grupo);
    }else{
      sprintf(horaString, "%s %d:%d Grupo:%d", day[dia], hora, min, grupo);
    }
    lcd.print(horaString); //Dia xx:xx grupo:yyyy

    lcd.setCursor(0, 1);
    lcd.print("Sistemas embebidos");
    
    if( ((hora >= horaInicio && hora < horaCierre) && (min >= minutoInicio)) ||         
        (hora == horaCierre && min <= minutoCierre))   
        {

    lcd.setCursor(0, 2);
    if(horaCierre < 10 && minutoCierre < 10){
    sprintf(horaString, "Cierre: 0%d:0%d", horaCierre, minutoCierre);
    }else if(horaCierre < 10 && minutoCierre >= 10){
    sprintf(horaString, "Cierre: 0%d:%d",horaCierre, minutoCierre);
    }else if(horaCierre >= 10 && minutoCierre < 10){
    sprintf(horaString, "Cierre: %d:0%d",horaCierre, minutoCierre);
    }else{
    sprintf(horaString, "Cierre: %d:%d", horaCierre, minutoCierre);
    }
    lcd.print(horaString);// Cierre: xx:xx
      lcd.setCursor(0, 3);
      lcd.print("Escanea tu tarjeta");//Escaneo

      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
      if (success) {
        lcd.clear();
        tone(BUZZER_PIN, 2500, 500);
        imprimirdatosdetarjeta();
        delay(3000);
        lcd.clear();
      }
    }else{
      
      if(hora < horaInicio || ((hora == horaInicio) && (min < minutoInicio))){
        lcd.setCursor(0, 2);
        if(horaInicio < 10 && minutoInicio < 10){
        sprintf(horaString, "Abre: 0%d:0%d", horaInicio, minutoInicio);
        }else if(horaInicio < 10 && minutoInicio >= 10){
        sprintf(horaString, "Abre: 0%d:%d",horaInicio,minutoInicio);
        }else if(horaInicio >= 10 && minutoInicio < 10){
        sprintf(horaString, "Abre: %d:0%d",horaInicio, minutoInicio);
        }else{
        sprintf(horaString, "Abre: %d:%d", horaInicio, minutoInicio);
        }
        lcd.print(horaString);// Abre: xx:xx

      }else if((hora > horaCierre) && (min > minutoInicio) && claseActual < 2){
        claseActual++;
      }else{
        lcd.setCursor(0, 2);
        lcd.print(claseActual);
        lcd.setCursor(0, 3);
        lcd.print("Ya paso el tiempo");
      }
      
    }
    
  
  } else{
    lcd.setCursor(0, 0);
    if(hora < 10 && min < 10){
      sprintf(horaString, "%s 0%d:0%d Grupo:---", day[dia], hora, min);
    }else if(hora < 10 && min >= 10){
      sprintf(horaString, "%s 0%d:%d Grupo:---", day[dia], hora, min);
    }else if(hora >= 10 && min < 10){
      sprintf(horaString, "%s %d:0%d Grupo:---", day[dia], hora, min);
    }else{
      sprintf(horaString, "%s %d:%d Grupo:---", day[dia], hora, min);
    }
    lcd.print(horaString); //Dia xx:xx grupo:yyyy

    lcd.setCursor(0, 2);
    lcd.print("No hay clases");
    lcd.setCursor(0, 3);
    lcd.print("disponibles");
  }

    
    
    
    

    

    
  
    
    delay(50);
}