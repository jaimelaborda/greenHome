/* ************************************************* *
   ----- NODO NFC GREEN HOME HACKATON CAST----------
   ------------------ coded by ---------------------
   ----------- Jaime Laborda Macario ---------------
   --- https://github.com/jaimelaborda/greenHome ---
 * ************************************************* */
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SimpleDHT.h>
#include <Servo.h> 

Servo servo; 
const char* ssid = "";
const char* password = "";

const char* APIAddress = "iotsens-inbox.grupogimeno.com"; // Your domain
const int httpPort = 80;

//VARIABLES NFC
byte nfc_id;
bool presence_58 = 0, presence_150 = 0, presence = 0;

//VARIABLES DHT11
int pinDHT11 = D2;
SimpleDHT11 dht11;
byte temp = 0;
byte hum = 0;

#define RST_PIN         D3//9          // Configurable, see typical pin layout above
#define SS_PIN          D4//10       // Configurable, see typical pin layout above

#define PRESENCE_PIN D8

#define SERVO_PIN D1
#define SERVO_UP 180
#define SERVO_DOWN 90

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void updateAPI(byte nfc, byte temp, byte hum) {
  char data[200];
  sprintf(data, "{\"node\":\"%s\", \"data\":\"NFC:%d|TEMP1:%d|HUM1:%d|SRV:%d\"}", "HACKATHON2017_04", nfc, temp, hum, (int)presence);

  WiFiClient client;
  Serial.println("Enviando datos...");
  if (client.connect(APIAddress, httpPort)) {
    client.println(F("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXXXXXXXXX HTTP/1.1"));
    client.println(F("Host: iotsens-inbox.grupogimeno.com"));
    client.println(F("User-Agent: Arduino/1.0"));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: "));
    client.println(strlen(data));
    client.println();
    client.println(data);

    if (client.connected()) {
      Serial.println("Dato enviado correctamente: " + String(data));
      Serial.println();
    }
  }
}

void SampleDHT11 (void) {
  if (dht11.read(pinDHT11, &temp, &hum, NULL)) {
    Serial.print("Read DHT11 failed.");
    return;
  } else {
    Serial.print("Sample OK: ");
    Serial.print((int)temp); Serial.print(" *C, ");
    Serial.print((int)hum); Serial.println(" %");

  }
}

void setup() {
  Serial.begin(115200);
  delay(10);

  pinMode(PRESENCE_PIN, OUTPUT);
  servo.attach(SERVO_PIN); 

  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522

  // Conectar a la red WiFi
  Serial.println();
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) { //Espera hasta que nos conectemos
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {

  //LEER DATOS NFC

  // Look for new cards
  Serial.print("Esperando NFC");
  while ( ! mfrc522.PICC_IsNewCardPresent()) { //Si NO hay nueva NFC
    Serial.print(".");
    delay(500);
  }
  Serial.println();

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  //mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
  byte nfc_id = mfrc522.uid.uidByte[0];

  // DETECTAR PRESENCIA
  if (nfc_id == 58) presence_58 = !presence_58;
  if (nfc_id == 150) presence_150 = !presence_150;

  presence = presence_58 || presence_150;

  digitalWrite(PRESENCE_PIN, presence);
  // ----
  Serial.println("Usuario autenticado: " + String(nfc_id));
  if (presence_58) {
    Serial.println("User 58 DENTRO");
  } else {
    Serial.println("User 58 FUERA");
  }

  if (presence_150) {
    Serial.println("User 150 DENTRO");
  } else {
    Serial.println("User 150 FUERA");
  }

  if (presence) {
    Serial.println("HAY alguien en casa");
    servo.write(SERVO_UP);
  }else{
    Serial.println("NO HAY nadie en casa");
    servo.write(SERVO_DOWN);
  }
  //Serial.println(readCard);

  //Samplear Temp & Hum
  SampleDHT11();

  updateAPI(nfc_id, temp, hum);

  //delay(1000); //Esperamos 2 seg
}




