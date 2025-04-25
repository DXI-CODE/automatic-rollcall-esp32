#include "WiFi.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include <LiquidCrystal_I2C.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <SD.h>
#include <ArduinoJson.h>
#include <DS3231.h>

#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 4
#define PIN_CS 5

const char* ssid = "L3";
const char* password = "";

char day[7][8] = { "Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab" };
char numClase[7][3] = { "0", "1", "2", "3", "4", "5", "6" };

byte charWifi[] = {
  B00000,
  B01110,
  B10001,
  B00100,
  B01010,
  B00000,
  B00100,
  B00000
};

byte charNoWifi[] = {
  B10001,
  B01010,
  B01010,
  B00100,
  B01010,
  B01010,
  B10001,
  B00000
};

char* archivo;
JsonDocument clases;
DS3231 rtc;
uint8_t horaInicio;
uint8_t minutoInicio;
uint8_t horaCierre;
uint8_t minutoCierre;
uint8_t claseActual;
uint16_t grupo;
const char* nombre;
bool internet = true;

LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000);  // Ajuste UTC-6 (México)

void imprimirdatosdetarjeta() {

  unsigned char data[4];   // Solo se leen 4 bytes por página
  char data2[32] = { 0 };  // Inicializar el buffer con ceros
  uint8_t success;
  uint8_t i, j;
  uint8_t index = 0;
  bool dentroDelimitador = false;  // Bandera para saber si estamos entre delimitadores


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
            continue;  // Ignorar el delimitador inicial
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

  data2[index] = '\0';  // Asegurar la terminación de cadena

  Serial.println("\nDatos leídos de la tarjeta:");
  Serial.println(data2);

  lcd.setCursor(0, 0);
  lcd.print("Hora: ");
  lcd.print(timeClient.getFormattedTime());  // Mostrar hora de registro

  for (int i = 0, j = 0; data2[j + (i * 20)] != '\0'; i++) {
    char aux[32] = { 0 };
    for (j = 0; j < 20 && data2[j + (i * 20)] != '\0'; j++) {
      aux[j] = data2[j + (i * 20)];
      if (data2[j] == '\0') {
        aux[j] = data2[j + (i * 20)];
      }
    }
    lcd.setCursor(0, i + 1);
    lcd.print(aux);
  }
}

void chequeoHora(int8_t dia, int8_t hora, int8_t min) {
  //Valores para el escaneo rfid
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  //Cadena de hora
  char horaString[32];

  if (dia != 0 && dia != 6) {  //Se comprueba si hoy no es sabado o domingo

    if (clases[day[dia]][numClase[claseActual]]["grupo"]) {  //Si la clase actual existe, se obtienen sus valores

      //Obtencion de variables
      horaInicio = clases[day[dia]][numClase[claseActual]]["h_inicio"];
      minutoInicio = clases[day[dia]][numClase[claseActual]]["m_inicio"];
      horaCierre = clases[day[dia]][numClase[claseActual]]["h_fin"];
      minutoCierre = clases[day[dia]][numClase[claseActual]]["m_fin"];
      grupo = clases[day[dia]][numClase[claseActual]]["grupo"];
      nombre = clases[day[dia]][numClase[claseActual]]["nombre"];
      //Fin de obtencion de variables

      //Inicia formateo de dia y grupo
      lcd.setCursor(0, 0);
      if (hora < 10 && min < 10) {
        sprintf(horaString, "%s 0%d:0%d Gpo:%d", day[dia], hora, min, grupo);  //DIA 0x:0x Gpo:xxxx
      } else if (hora < 10 && min >= 10) {
        sprintf(horaString, "%s 0%d:%d Gpo:%d", day[dia], hora, min, grupo);  //DIA 0x:xx Gpo:xxxx
      } else if (hora >= 10 && min < 10) {
        sprintf(horaString, "%s %d:0%d Gpo:%d", day[dia], hora, min, grupo);  //DIA xx:0x Gpo:xxxx
      } else {
        sprintf(horaString, "%s %d:%d Gpo:%d", day[dia], hora, min, grupo);  //DIA xx:xx Gpo:xxxx
      }
      lcd.print(horaString);  //Dia xx:xx grupo:yyyy
      imprimirSimboloInternet();
      //Termina formateo de dia y grupo

      lcd.setCursor(0, 1);
      lcd.print(nombre);  //Nombre de clase
    }

    //If que checa si hay clases actualmente
    if (((hora >= horaInicio && hora < horaCierre) && (min >= minutoInicio)) ||  //Caso de hora actual sea xx:xx y la hora de apertura sea xx:yy                       (ej 11:15 y 11:00)
        ((hora >= horaInicio && hora == horaCierre) && min <= minutoCierre) ||   //Caso de hora actual sea xx:xx y la hora de cierre sea xx:yy                         (ej 11:15 y 11:30)
        (hora > horaInicio && hora < horaCierre))                                //Caso de hora actual sea xx:yy y la hora sea horaApertura < horaActual < horaCierre  (ej 12:15 y 11:30 - 13:00)
    {

      lcd.setCursor(0, 2);
      //Inicia formateo de hora
      if (horaCierre < 10 && minutoCierre < 10) {
        sprintf(horaString, "Cierre: 0%d:0%d", horaCierre, minutoCierre);  //Caso 0x:0x
      } else if (horaCierre < 10 && minutoCierre >= 10) {
        sprintf(horaString, "Cierre: 0%d:%d", horaCierre, minutoCierre);  //Caso 0x:xx
      } else if (horaCierre >= 10 && minutoCierre < 10) {
        sprintf(horaString, "Cierre: %d:0%d", horaCierre, minutoCierre);  //Caso xx:0x
      } else {
        sprintf(horaString, "Cierre: %d:%d", horaCierre, minutoCierre);  //Caso xx:xx
      }
      lcd.print(horaString);  // Cierre: xx:xx
      imprimirSimboloInternet();
      //Termina formateo de hora

      lcd.setCursor(0, 3);
      lcd.print("Escanea tu tarjeta");  //Escaneo

      success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
      if (success) {
        lcd.clear();
        tone(BUZZER_PIN, 2500, 500);
        imprimirdatosdetarjeta();
        delay(3000);
        lcd.clear();
      }
    } else {
      //Si no hay clases, decide que hacer


      if (hora < horaInicio || ((hora == horaInicio) && (min < minutoInicio))) {  //Si se detecta una clase que no ha comenzado, se pone su horario de apertura

        //Inicia formateo de hora
        lcd.setCursor(0, 2);
        if (horaInicio < 10 && minutoInicio < 10) {
          sprintf(horaString, "Abre: 0%d:0%d", horaInicio, minutoInicio);  //Caso 0x:0x
        } else if (horaInicio < 10 && minutoInicio >= 10) {
          sprintf(horaString, "Abre: 0%d:%d", horaInicio, minutoInicio);  //Caso 0x:xx
        } else if (horaInicio >= 10 && minutoInicio < 10) {
          sprintf(horaString, "Abre: %d:0%d", horaInicio, minutoInicio);  //Caso xx:0x
        } else {
          sprintf(horaString, "Abre: %d:%d", horaInicio, minutoInicio);  //Caso xx:xx
        }
        lcd.print(horaString);  // Abre: xx:xx
        imprimirSimboloInternet();
        //Termina formateo de hora

      } else if (((hora == horaCierre) && (min > minutoCierre) || (hora > horaCierre)) && clases[day[dia]][numClase[claseActual]]["grupo"]) {  //Caso si ya acabo el tiempo de escaneo de una clase, para que se vaya a avanzar a la siguiente hora
        claseActual++;
        lcd.clear();
        delay(100);
        Serial.println(claseActual);
      } else {  //Caso de que se hayan acabado las clases para el area especifica

        //Formateo de dia y grupo
        lcd.setCursor(0, 0);
        if (hora < 10 && min < 10) {
          sprintf(horaString, "%s 0%d:0%d Gpo:---", day[dia], hora, min);  //DIA 0x:0x Gpo:---
        } else if (hora < 10 && min >= 10) {
          sprintf(horaString, "%s 0%d:%d Gpo:---", day[dia], hora, min);  //DIA 0x:xx Gpo:---
        } else if (hora >= 10 && min < 10) {
          sprintf(horaString, "%s %d:0%d Gpo:---", day[dia], hora, min);  //DIA xx:0x Gpo:---
        } else {
          sprintf(horaString, "%s %d:%d Gpo:---", day[dia], hora, min);  //DIA xx:xx Gpo:---
        }
        lcd.print(horaString);  //Dia xx:xx grupo:---
        imprimirSimboloInternet();
        //Termina formateo de dia

        lcd.setCursor(0, 1);
        lcd.print("Sin clases restantes");
        lcd.setCursor(0, 2);
        lcd.print("para este dia.");
      }
    }


  } else {  //Caso de dia sin clases (sabado o domingo)
            //Formateo de hora
    lcd.setCursor(0, 0);
    if (hora < 10 && min < 10) {
      sprintf(horaString, "%s 0%d:0%d Gpo:---", day[dia], hora, min);  //Caso 0x:0x
    } else if (hora < 10 && min >= 10) {
      sprintf(horaString, "%s 0%d:%d Gpo:---", day[dia], hora, min);  //Caso 0x:xx
    } else if (hora >= 10 && min < 10) {
      sprintf(horaString, "%s %d:0%d Gpo:---", day[dia], hora, min);  //Caso xx:0x
    } else {
      sprintf(horaString, "%s %d:%d Gpo:---", day[dia], hora, min);  //Caso xx:xx
    }
    lcd.print(horaString);  //Dia xx:xx grupo:---
                            //Termina formateo de hora
    imprimirSimboloInternet();

    lcd.setCursor(0, 2);
    lcd.print("No hay clases");
    lcd.setCursor(0, 3);
    lcd.print("disponibles");
  }
  delay(50);
}

void imprimirSimboloInternet(){
  lcd.setCursor(19, 0);
  if(WiFi.status() == WL_CONNECTED){
    lcd.write(0);
  }else{
    lcd.write(1);
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Iniciando");
  //Inicialzacion de pantalla
  lcd.begin(20, 4);
  lcd.init();
  lcd.backlight();
  Serial.println("Pantalla lista");
  nfc.begin();
  nfc.SAMConfig();
  //Fin de inicializacion de pantalla
  lcd.createChar(0, charWifi);
  lcd.createChar(1, charNoWifi);

  //Inicializacion de tarjeta sd
  if (!SD.begin(PIN_CS)) {
    lcd.clear();
    lcd.print("No se encontro");
    lcd.setCursor(0, 1);
    lcd.print("una tarjeta SD!");

    while (1) {
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
  } else {
    lcd.clear();
    lcd.print("Tarjeta SD");
    lcd.setCursor(0, 1);
    lcd.print("encontrada!");
    delay(1000);
  }
  //Fin de inicializacion de SD

  //Inicio de conexion de internet inicial
  lcd.clear();
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print("Conectando a:");
  lcd.setCursor(0, 1);
  lcd.print(ssid);
  lcd.setCursor(0, 2);
  lcd.print("Intentando por:");
  lcd.setCursor(0, 3);
  lcd.print("10 segundos mas");

  unsigned long startAttemptTime = millis();
  const unsigned long timeout = 10000;  // 10 seconds max wait
  int cont = 20;

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
    delay(500);
    lcd.setCursor(0, 3);
    cont -= 1;
    if (cont % 2 == 0) {
      int cont2 = cont / 2;
      if (cont == 18) {
        lcd.print(cont2);
        lcd.print(" ");
      } else if (cont == 2) {
        lcd.print("1 segundo  mas");
      }else {
        lcd.print(cont2);
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.print("Conectado!");
    delay(1000);
  } else {
    lcd.clear();
    lcd.print("Sin internet...");
    delay(1000);
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  internet = false;

  lcd.clear();
  imprimirSimboloInternet();
  //Fin de conexion de internet inicial

  timeClient.begin();  // Iniciar NTP

  //Buffer de lectura de la SD del json de clases
  archivo = (char*)malloc(sizeof(char) * 2500);

  //Inicializacion de la tarjeta SD y obtencion de JSON
  lcd.print("Leyendo SD");
  int i = 0;
  File dataFile = SD.open("/Lunes.json");
  if (dataFile) {
    while (dataFile.available()) {
      archivo[i] = dataFile.read();
      i++;
    }
    dataFile.close();
  }
  deserializeJson(clases, archivo);  //Obtencion de las asignaturas y sus horas dentro del objeto llamado clases
  free(archivo);
  serializeJson(clases[day[2]][numClase[2]]["grupo"], Serial);
  lcd.clear();

  if (clases == NULL) {  //Si no se encuentra el archivo JSON Lunes.json en la raiz
    lcd.setCursor(0, 0);
    lcd.print("No se encontro");
    lcd.setCursor(0, 1);
    lcd.print("un archivo JSON");
    while (1) delay(100);
  }
  //Fin de inicializacion de la tarjeta SD y obtencion de JSON

  claseActual = 0;  //Se pone la claseActual como 0 para leer de la primera clase

  Wire.begin();
  //Si el tiempo esta desconfigurado
  //if(RTClib::now() <= timeClient.now){
  //Poniendo el tiempo
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    imprimirSimboloInternet();
    lcd.setCursor(0, 0);
    lcd.print("Ajustando hora");
    lcd.setCursor(0, 1);
    lcd.print("en el modulo rtc");
    delay(1000);

    if (WiFi.status() == WL_CONNECTED) {
      timeClient.update();  // This fetches time from the NTP server
      time_t rawTime = timeClient.getEpochTime();

      while (rawTime == 0 || rawTime > 4102444800UL) {  // > year 2100 is obviously wrong
        Serial.println("Waiting for valid NTP time...");
        delay(500);
        timeClient.update();
        rawTime = timeClient.getEpochTime();
      }

      Serial.println(rawTime);
      rtc.setEpoch(rawTime);  // Or rtc.adjust(DateTime(rawTime)) if using RTClib

    } else {
      Serial.println("WiFi not connected, can't sync NTP");
    }

    lcd.clear();
    imprimirSimboloInternet();
    lcd.setCursor(0, 0);
    lcd.print("Listo!");
    delay(1500);
    lcd.clear();
    timeClient.update();
    //}
  }
}

void loop() {

  bool test;
  bool test2;
  //Obtencion de dia, minuto y clase actual
  int8_t dia;
  int8_t min;
  int8_t hora;

  //Comprobacion de internet
  if (WiFi.status() != WL_CONNECTED) {
    if(internet == true){
      lcd.clear();
      delay(50);
      internet = false;
    }
    Serial.println("Sin internet");
    WiFi.disconnect();           // Clear any previous connection
    WiFi.begin(ssid, password);  // Re-initiate connection

    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 1000;  // 10 seconds max wait

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
      delay(500);
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Reconnected!");
      timeClient.update();  //Actualizacion de la hora actual
      lcd.clear();
      delay(500);  // Optional delay after reconnect
      dia = timeClient.getDay();
      min = timeClient.getMinutes();
      hora = timeClient.getHours();
    } else {
      dia = rtc.getDoW() - 1;
      min = rtc.getMinute();
      hora = rtc.getHour(test, test2);
      Serial.println(rtc.getYear());
    }
    chequeoHora(dia, hora, min);

  }else {
    if(internet == false){
      lcd.clear();
      delay(50);
      internet = true;
    }
    timeClient.forceUpdate();  //Actualizacion de la hora actual
    Serial.println(timeClient.getEpochTime());
    if(timeClient.getEpochTime() < 4294945700){
    Serial.println("Con internet");
    dia = timeClient.getDay();
    min = timeClient.getMinutes();
    hora = timeClient.getHours();
    chequeoHora(dia, hora, min);
    }
    
  }
}