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
#include <PubSubClient.h>

// Objeto servo
Servo servo;

const char* ssid = "";
const char* password = "";

const char* APIAddress = "iotsens-inbox.grupogimeno.com"; // IoTSense address
const int httpPort = 80;

const char* mqtt_server = "m20.cloudmqtt.com"; //MQTT Server

WiFiClient espClient;
PubSubClient client(espClient);

//VARIABLES NFC
byte nfc_id;
bool presence_58 = 0, presence_150 = 0, presence = 0;

//VARIABLES DHT11
int pinDHT11 = D2;
SimpleDHT11 dht11;
byte temp = 0;
byte hum = 0;

// VARIABLES LED
bool led1 = 0, led2 = 0;

// NFC reader pinout
#define RST_PIN         D3 //was pin9          
#define SS_PIN          D4 //was pin10       

#define PRESENCE_PIN D8

#define SERVO_PIN D1
#define SERVO_UP 180
#define SERVO_DOWN 90

//ACTUADORES
#define LED_SALON_1 D0 // LED Tocho
#define LED_CUARTO_2 10  // Tira arriba dormitorio
//#define LED_COCINA_3 10 // Tira abajo

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void updateAPI(byte nfc, byte temp, byte hum) {
  char data[200];
  sprintf(data, "{\"node\":\"%s\", \"data\":\"NFC:%d|TEMP1:%d|HUM1:%d|SRV:%d|LED1:%d|LED2:%d\"}", "HACKATHON2017_04", nfc, temp, hum, (int)presence, (int)led1, (int)led2);

  WiFiClient client;
  Serial.println("Enviando datos...");
  if (client.connect(APIAddress, httpPort)) {
    client.println("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXX HTTP/1.1");
    client.println("Host: iotsens-inbox.grupogimeno.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(strlen(data));
    client.println();
    client.println(data);

    if (client.connected()) {
      Serial.println("Dato enviado correctamente: " + String(data));
      Serial.println();
    }
  }
}

void initializeAPI(void) {
  char data[200];
  sprintf(data, "{\"node\":\"%s\", \"data\":\"NFC:%d|SRV:%d|LED1:%d|LED2:%d\"}", "HACKATHON2017_04", 0, 0, 0, 0);

  WiFiClient client;
  Serial.println("Enviando datos...");
  if (client.connect(APIAddress, httpPort)) {
    client.println("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXX HTTP/1.1");
    client.println("Host: iotsens-inbox.grupogimeno.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
    client.println(strlen(data));
    client.println();
    client.println(data);

    if (client.connected()) {
      Serial.println("Dato enviado correctamente: " + String(data));
      Serial.println();
    }
  }
}


void updateAPILed() {
  char data[200];
  sprintf(data, "{\"node\":\"%s\", \"data\":\"LED1:%d|LED2:%d\"}", "HACKATHON2017_04", (int)led1, (int)led2, hum, (int)presence);

  WiFiClient client;
  Serial.println("Enviando datos...");
  if (client.connect(APIAddress, httpPort)) {
    client.println("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXX HTTP/1.1");
    client.println("Host: iotsens-inbox.grupogimeno.com");
    client.println("User-Agent: Arduino/1.0");
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.print("Content-Length: ");
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
  pinMode(LED_SALON_1, OUTPUT);
  pinMode(LED_CUARTO_2, OUTPUT);
  //pinMode(LED_COCINA_3, OUTPUT);

  // Cocina siempre encendida debido a error UI
  //digitalWrite(LED_COCINA_3, LOW);
  digitalWrite(LED_SALON_1, LOW); //Default OFF
  digitalWrite(LED_CUARTO_2, LOW); //Default OFF

  //Objeto servo puerta
  servo.attach(SERVO_PIN);

  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522

  // - Conectar a la red WiFi -----------------------
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
  Serial.print("WiFi connected on IP:");
  Serial.println(WiFi.localIP());
  // -----------------------------------------------

  // Connect to MQTT server
  client.setServer(mqtt_server, 12444);
  client.setCallback(callback);

  // HACER POST INICIALIZACIÓN A 0
  initializeAPI();
}

// MQTT CALLBACK -> executed when message received to the topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Toggle LED_SALON if a 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(LED_SALON_1, !led1);   // Turn the LED on (Note that LOW is the voltage level
    led1 = !led1; //Update variable
  }
  // toggle LED_CUARTO if a 2 was received as first character
  if ((char)payload[0] == '2') {
    digitalWrite(LED_CUARTO_2, !led2);
    led2 = !led2; //Update variable
  }

  // Si recibo MQTT, envío la info actualizada de los LEDS por POST
  updateAPILed();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", "MQTTuser", "MQTTpassword")) {
      Serial.println("connected");
      client.subscribe("greenhome");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {
  // MANEJO DE LOOP MQTT ASÍNCRONO
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(500); //Ejecuta el loop cada 0.5 segundos
  
  //LEER DATOS NFC
  // Look for new cards
  while ( ! mfrc522.PICC_IsNewCardPresent()) { //Si NO hay nueva NFC
    Serial.print(".");
    return; //Sale del void loop()
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

  presence = presence_58 || presence_150; // Hay presencia si hay algún usuario o los dos (OR)

  digitalWrite(PRESENCE_PIN, presence); //Se lo paso a Ivan
  
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
  } else {
    Serial.println("NO HAY nadie en casa");
    servo.write(SERVO_DOWN);
    nfc_id = 0;
  }
  //Serial.println(readCard);

  //Samplear Temp & Hum
  SampleDHT11();

  updateAPI(nfc_id, temp, hum); //Solo subo datos cuando trigger NFC

  //delay(1000); //Esperamos 2 seg
}




