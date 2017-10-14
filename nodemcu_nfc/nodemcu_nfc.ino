/* ************************************************* *
   ----- NODO NFC GREEN HOME HACKATON CAST----------
   ------------------ coded by ---------------------
   ----------- Jaime Laborda Macario ---------------
   --- https://github.com/jaimelaborda/greenHome ---
 * ************************************************* */
 
#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "";
const char* password = "";

const char* APIAddress = "iotsens-inbox.grupogimeno.com"; // Your domain
const int httpPort = 80;

//VARIABLES NFC
byte nfc_id;

#define RST_PIN         D3//9          // Configurable, see typical pin layout above
#define SS_PIN          D4//10       // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void updateNFC(byte tsData) {
  char data[200];
  sprintf(data, "{\"node\":\"%s\", \"data\":\"NFC:%d\"}", "HACKATHON2017_04", tsData);

  WiFiClient client;
  Serial.println("Enviando datos...");
  if (client.connect(APIAddress, httpPort)) {
    client.println(F("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXXXXX HTTP/1.1"));
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

void setup() {
  Serial.begin(115200);
  delay(10);

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
  //Serial.println(readCard);

  updateNFC(nfc_id);

  //delay(1000); //Esperamos 2 seg
}




