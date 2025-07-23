#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// VARIABILI PIN ARDUINO
int ledRosso = D1;
int ledBlu = D2;
int pinCLK = D6;
int pinDT = D5;
int sensore = A0;

// VARIABILI PER CONNESSIONE WIFI
const char *ssid = "iPhone di Riccardo";
const char *password = "caccapupo";
int isConnected;

// VARIABILI PER UDP
IPAddress ipServer(172, 20, 10, 5);  
WiFiUDP Udp;
// char msg[100];
long pcktCount = 0;

// VARIABILI PER SENSORE UMIDITA'
char statoSensore[100];
int valoreSensore;
int dry = 1000;
int wet = 1600;
int very_wet = 2400;

// VARIABILI PER ROTARY ENCODER
int pinCLK_value;
int pinCLK_prev;
bool bool_CW;

// VARIABILI PER IL TEMPO DI CAMPIONAMENTO
int count;
int tempoMinimo = 1;
int tempoMassimo = 30;
unsigned long interval;
unsigned long previousMillis = 0;

int connectToWiFi() {
  Serial.begin(115200);
  Serial.print("Connessione a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  int tentativiConnessione = 0;
  while (WiFi.status() != WL_CONNECTED && tentativiConnessione < 30) {
    delay(500);
    Serial.print(".");
    tentativiConnessione++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnessione WiFi stabilita");
    Serial.print("Indirizzo IP assegnato: ");
    Serial.println(WiFi.localIP());
    return 1;
  } else {
    Serial.println("\nConnessione WiFi fallita. Verifica le credenziali.");
  }
  return 0;
}

void checkEncoder() {
  int pinCLK_value = digitalRead(pinCLK);

  if (pinCLK_value != pinCLK_prev) {
    if (digitalRead(pinDT) != pinCLK_value) {
      bool_CW = true;
    } else {
      bool_CW = false;
    }

    if (bool_CW) {
      count++;
    } else {
      count--;
    }

    count = constrain(count, tempoMinimo, tempoMassimo);

    interval = count * 1000  ;
    Serial.print("Tempo di campionamento: ");
    Serial.print(count);
    Serial.println(" minuti");

    pinCLK_prev = pinCLK_value;
  }
}

void setup() {
  isConnected = connectToWiFi();

  pinMode(ledRosso, OUTPUT);
  pinMode(ledBlu, OUTPUT);
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);
  pinMode(sensore, INPUT);

  count = 1;
  bool_CW = false;
}

void loop() {
  checkEncoder();

  valoreSensore = analogRead(sensore);
  if (valoreSensore < dry) {
    digitalWrite(ledRosso, HIGH);
    digitalWrite(ledBlu, LOW);
    // Serial.println("Livello: DRY");
    strcpy(statoSensore, "DRY");
  } else if (valoreSensore >= dry && valoreSensore < very_wet) {
    digitalWrite(ledRosso, LOW);
    digitalWrite(ledBlu, LOW);
    // Serial.println("Livello: WET");
    strcpy(statoSensore, "WET");
  } else {
    digitalWrite(ledRosso, LOW);
    digitalWrite(ledBlu, HIGH);
    // Serial.println("Livello: VERY WET");
    strcpy(statoSensore, "VERY WET");
  }
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if(isConnected == 1){
      const size_t CAPACITY = JSON_OBJECT_SIZE(1);
      StaticJsonDocument<CAPACITY> doc;
      JsonObject object = doc.to<JsonObject>();
      object["statoSensore"] = statoSensore;
      object["valoreGrezzo"] = valoreSensore;
      object["pcktCounter"] = pcktCount;
      serializeJson(doc, Serial);
      Serial.println("");

      Udp.begin(8888);
      Udp.beginPacket(ipServer, 8888);
      serializeJson(doc, Udp);
      // sprintf(msg,"{\"statoSensore\":\"%s\", \"valoreGrezzo\":%d}" ,statoSensore, valoreSensore);
      // int msg_len = sizeof(msg)/sizeof(char);
      // Udp.write((uint8_t*)(msg),msg_len);
      Udp.endPacket();

      // Serial.println(msg);
      pcktCount++;
    }
  }
  
}
