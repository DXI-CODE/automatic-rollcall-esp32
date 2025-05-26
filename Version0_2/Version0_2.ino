//Librerias del códgio
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
#include <ESPAsyncWebServer.h>

//Constantes simbólicas
#define SDA_PIN 21
#define SCL_PIN 22
#define BUZZER_PIN 4
#define PIN_CS 5

//Constantes de variables de internet
const char *ssid = "LAB ELECTRONICA E IA";
const char *password = "Electro2024.#.";

AsyncWebServer server(80);

//Constantes del código
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

//Variables globales
char *archivo;
JsonDocument clases;
JsonDocument alumnos;
DS3231 rtc;
uint8_t horaInicio;
uint8_t minutoInicio;
uint8_t horaCierre;
uint8_t minutoCierre;
uint8_t claseActual;
uint16_t grupo;
const char *nombre;
bool internet = true;
bool hayClases = true;
bool reseteando = false;


LiquidCrystal_I2C lcd(0x27, 20, 4);
Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -21600, 60000);  // Ajuste UTC-6 (México)

//Función de lectura de datos de tarjeta
void imprimirdatosdetarjeta(int8_t hora, int8_t min, int8_t sec) {

  unsigned char data[4];
  char data2[32] = { 0 };
  uint8_t success;
  uint8_t i, j;
  uint8_t index = 0;
  bool dentroDelimitador = false;


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
  char horaString[32];

  if (WiFi.status() == WL_CONNECTED) {
    timeClient.forceUpdate();
    strncpy(horaString, timeClient.getFormattedTime().c_str(), sizeof(horaString));
    horaString[sizeof(horaString) - 1] = '\0';  // Ensure null-termination
  } else {
    if (hora < 10 && min < 10 && sec < 10) {
      sprintf(horaString, "0%d:0%d:0%d", hora, min, sec);
    } else if (hora < 10 && min < 10) {
      sprintf(horaString, "0%d:0%d:%d", hora, min, sec);
    } else if (hora < 10 && sec < 10) {
      sprintf(horaString, "0%d:%d:0%d", hora, min, sec);
    } else if (hora < 10) {
      sprintf(horaString, "0%d:%d:%d", hora, min, sec);
    } else if (min < 10 && sec < 10) {
      sprintf(horaString, "%d:0%d:0%d", hora, min, sec);
    } else if (min < 10) {
      sprintf(horaString, "%d:0%d:%d", hora, min, sec);
    } else if (sec < 10) {
      sprintf(horaString, "%d:%d:0%d", hora, min, sec);
    } else {
      sprintf(horaString, "%d:%d:%d", hora, min, sec);
    }
  }

  lcd.print(horaString);  // Mostrar hora de registro

  for (int i = 0, j = 0; data2[j + (i * 20)] != '\0'; i++) {
    char aux[32] = { 0 };
    for (j = 0; j < 20 && data2[j + (i * 20)] != '\0'; j++) {
      aux[j] = data2[j + (i * 20)];
      Serial.println(j);
      if (data2[j] == '\0') {
        aux[j] = data2[j + (i * 20)];
      }
    }
    lcd.setCursor(0, i + 1);
    lcd.print(aux);
  }
}

//Función para verificar la hora y si se esta en el periodo de alguna materia
void chequeoHora(int8_t dia, int8_t hora, int8_t min, int8_t sec) {
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

      bool hayClasesAux = clases[day[dia]][numClase[claseActual]]["status"];
      //Serial.println(hayClasesAux);

      if (!hayClasesAux && hayClases) {
        hayClases = false;
      }

      if (hayClases) {
        lcd.setCursor(0, 3);
        lcd.print("Escanea tu tarjeta");  //Escaneo

        success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 1000);
        if (success) {
          lcd.clear();
          tono();
          imprimirdatosdetarjeta(hora, min, sec);
          delay(3000);
          lcd.clear();
        }
      } else {
        lcd.setCursor(0, 3);
        lcd.print("Clase cancelada");  //No habra clase, fue cancelada
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
        hayClases = true;
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

//Función para imprimir simbolo de conexión a internet
void imprimirSimboloInternet() {
  lcd.setCursor(19, 0);
  if (WiFi.status() == WL_CONNECTED) {
    lcd.write(0);
  } else {
    lcd.write(1);
  }
}

//Función para emitir tono de chequeo
void tono() {
  if (1) {
    //Tono no permitido
    tone(BUZZER_PIN, 2500, 200);
    delay(300);
    tone(BUZZER_PIN, 2500, 200);
    delay(300);
    tone(BUZZER_PIN, 2500, 200);
  } else {
    //Tono permitido
    tone(BUZZER_PIN, 2500, 500);
  }
}

//Función que envia y recibe datos a una página web para su registro
void servidor() {
  server.on("/enviar", HTTP_GET, [](AsyncWebServerRequest *request) {
    String archivo = "";  // Use String to hold file content safely

    File dataFile = SD.open("/Lunes.json");
    if (dataFile) {
      while (dataFile.available()) {
        archivo += (char)dataFile.read();  // Append each char to String
      }
      dataFile.close();
    } else {
      request->send(500, "text/plain", "Failed to open file");
      return;
    }

    AsyncWebServerResponse *response = request->beginResponse(200, "application/json", archivo);
    response->addHeader("Access-Control-Allow-Origin", "*");
    response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    response->addHeader("Access-Control-Allow-Headers", "*");
    request->send(response);
  });

  String receivedText = ""; // Global or static to persist across chunks

server.on("/recibir", HTTP_POST, [](AsyncWebServerRequest *request) {}, NULL,
[](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    static String bodyBuffer;  // Static so it persists across chunks

    for (size_t i = 0; i < len; i++) {
        bodyBuffer += (char)data[i];
    }

    if (index + len == total) {
        Serial.println("Full body received:");
        Serial.println(bodyBuffer);
        Serial.print("Size: ");
        Serial.println(total);

        // Save to file if needed
        
        File file = SD.open("/Lunes.json", FILE_WRITE);
        if (file) {
            file.println(bodyBuffer);
            file.close();
            Serial.println("File written successfully.");
        } else {
            Serial.println("Failed to open file for writing.");
        }
        // Send response
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "Datos recibidos en ESP32! Reiniciando...");
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        response->addHeader("Access-Control-Allow-Headers", "*");
        request->send(response);

        // Reset buffer
        bodyBuffer = "";
        reseteando = true;
        lcd.clear();
        delay(100);
        lcd.setCursor(0, 0);
        lcd.print("Datos actualizados");
        lcd.setCursor(0, 1);
        lcd.print("de horarios.");
        lcd.setCursor(0, 2);
        lcd.print("Reseteando el ESP32.");
        delay(5000);
        WiFi.disconnect(true);  // Optional
        delay(100);             // Allow cleanup
        ESP.restart();
    }
});

  server.begin();
}

//Función de inicialización de código
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
        lcd.print("1  segundo  mas");
      } else {
        lcd.print(cont2);
      }
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.print("Conectado!");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP());
    delay(2000);
    servidor();

  } else {
    lcd.clear();
    lcd.print("Sin internet...");
    delay(1000);
  }

  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  internet = false;

  lcd.clear();
  //imprimirSimboloInternet();
  //Fin de conexion de internet inicial

  timeClient.begin();  // Iniciar NTP

  //Buffer de lectura de la SD del json de clases
  archivo = (char *)malloc(sizeof(char) * 2500);

  //Inicializacion de la tarjeta SD y obtencion de JSON (clases)
  lcd.setCursor(0, 0);
  lcd.print("Leyendo SD (clases)");
  int i = 0;
  File dataFile2 = SD.open("/Lunes.json");
  if (dataFile2) {
    while (dataFile2.available()) {
      archivo[i] = dataFile2.read();
      i++;
    }
    dataFile2.close();
  }
  deserializeJson(clases, archivo);  //Obtencion de las asignaturas y sus horas dentro del objeto llamado clases
  free(archivo);
  delay(500);

  if (clases == NULL) {  //Si no se encuentra el archivo JSON Lunes.json en la raiz
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No se encontro");
    lcd.setCursor(0, 1);
    lcd.print("un archivo JSON");
    while (1) delay(100);
  }
  //Fin de inicializacion de la tarjeta SD y obtencion de JSON

  //Inicializacion de la tarjeta SD y obtencion de JSON (alumnos)
  //Buffer de lectura de la SD del json de alumnos
  archivo = (char *)malloc(sizeof(char) * 2500);
  lcd.setCursor(0, 0);
  lcd.print("Leyendo SD (alumnos)");
  i = 0;
  File dataFile = SD.open("/alumnos.json");
  if (dataFile) {
    while (dataFile.available()) {
      archivo[i] = dataFile.read();
      i++;
    }
    dataFile.close();
  }
  deserializeJson(alumnos, archivo);  //Obtencion de los alumnos y sus grupos dentro del objeto llamado alumnos
  free(archivo);
  delay(500);

  if (alumnos == NULL) {  //Si no se encuentra el archivo JSON alumnos.json en la raiz
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("No se encontro");
    lcd.setCursor(0, 1);
    lcd.print("un archivo JSON");
    while (1) delay(100);
  }
  //Fin de inicializacion de la tarjeta SD y obtencion de JSON alumnos

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
  }else{
    lcd.clear();
  }
}

//Función que ejecuta el código principal
void loop() {

  bool test;
  bool test2;
  //Obtencion de dia, minuto y clase actual
  int8_t dia;
  int8_t min;
  int8_t hora;
  int8_t sec;

  if(!reseteando){
  //Comprobacion de internet
  if (WiFi.status() != WL_CONNECTED) { //Si no hay internet
    if (internet == true) {
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
      sec = timeClient.getSeconds();
    } else {
      dia = rtc.getDoW() - 1;
      min = rtc.getMinute();
      hora = rtc.getHour(test, test2);
      sec = rtc.getSecond();
    }
    chequeoHora(dia, hora, min, sec);

  } else {
    if (internet == false) {
      lcd.clear();
      delay(50);
      internet = true;
    }
    timeClient.forceUpdate();  //Actualizacion de la hora actual
    //Serial.println(timeClient.getEpochTime());
    if (timeClient.getEpochTime() < 2085956896) {
      //Serial.println("Con internet");
      dia = timeClient.getDay();
      min = timeClient.getMinutes();
      hora = timeClient.getHours();
      sec = timeClient.getSeconds();
      chequeoHora(dia, hora, min, sec);
    }else{
      //Hora invalida, notifica al usuario
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Obteniendo la hora...");
    }
  }
  }
}