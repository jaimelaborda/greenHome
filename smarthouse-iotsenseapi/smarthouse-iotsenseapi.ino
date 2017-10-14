/* ************************************************* *
   ----- NODO IoTSense GREEN HOME HACKATON CAST ----
   ------------------ coded by ---------------------
   ------------- Iván Torres Jativa  ---------------
   --- https://github.com/jaimelaborda/greenHome ---
 * ************************************************* */

// SmartHouse core - Hackathon Cs 2017
// Makers Green - Iván Torres

#include <SPI.h>
#include <Wire.h>
#include <sensMotePro.h>
#include <WiFiEsp.h>
#include <WiFiEspClient.h>
#include <SensLED.h>
#include <DHT.h>
#include <BH1750.h>
#include <Servo.h>

// PLACA
SensMotePro SM;
SensLED dimm;  // Para los LED de la placa
const uint8_t wifiPowerPin = 1; // PIN power del modulo WIFI

// DHT HABITACION 2
int8_t h2, t2;
#define DHTPIN2 2
#define DHTTYPE2 DHT22
DHT dht2(DHTPIN2, DHTTYPE2);

// DHT HABITACION 3
int8_t h3, t3;
#define DHTPIN3 5
#define DHTTYPE3 DHT22
DHT dht3(DHTPIN3, DHTTYPE3);

// PIR
#define MOTION 13
int8_t motion;

// LIGHT METER
BH1750 lightMeter;
uint16_t lux;

// MAGNETIC
#define MAGNETIC 12
uint8_t window;

// PLANT
#define PLANT A3
uint8_t hum;

// SOLAR
#define SOLAR A4
uint16_t voltage;

// NFC
#define NFC 7
uint8_t nfc = 0;

//// ACTUADORES

// ALARMA
uint8_t robo = 0;

// BUZZER
#define BUZZER 22
uint8_t buzzer = 0;

// FAN
#define FAN 0
uint8_t fan = 0;

void setup() {

  // initialize serial for debugging
  Serial.begin(38400);

  analogReference(DEFAULT);

  Wire.begin(); // I2C (2 WIRE)
  lightMeter.begin();


  dht2.begin();
  dht3.begin();

  SM.setBoardPower(POWER_ON, 1000);
  delay(100);

  Serial.println(F(" \nInitializing..."));

  pinMode(MOTION, INPUT);
  pinMode(MAGNETIC, INPUT_PULLUP);
  pinMode(PLANT, INPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(FAN, OUTPUT);
  pinMode(NFC, INPUT);

  // initialize ESP module
  Serial.begin(115200);
  pinMode(wifiPowerPin, OUTPUT);
  SM.setSerial0Mux1();
  SM.setModulePower(wifiPowerPin, POWER_ON, 1500);

  WiFi.init(&Serial);
  delay(500);
  if (WiFi.status() != WL_NO_SHIELD) {
    char ssid[50] = "";
    char key[50] = "";
    WiFi.begin(ssid, key);
    dimm.setLed(LED_A, LED_ON);
  }
  SM.setSerial0Mux0();

  if (!dimm.initialize()) {
    Serial.println(F("Dimmer failed"));
  }
  Serial.println(F("Initialization done"));
}

void loop() {
  readRooms();
  readMotion();
  readLux();
  readWindow();
  readPlant();
  readVoltage();
  readNFC();

  alarm();

   sendData();
  delay(500);

}

void alarm() {
  if (!nfc && (window ))
  {
    robo = 1;
    digitalWrite(BUZZER, HIGH);
  }
  else
  {
    robo = 0;
    digitalWrite(BUZZER, LOW);
  }

}

void readNFC() {
  nfc = digitalRead(NFC);
}

void readVoltage() {
  voltage = analogRead(SOLAR);
}

void readPlant() {
  hum = analogRead(PLANT);
}

void readWindow() {
  if (digitalRead(MAGNETIC) == LOW)
  {
    window = 0;
  }
  else
  {
    window = 1;
  }
}

void readLux() {
  lux = lightMeter.readLightLevel();
  //Serial.println(lux);
}

void readRooms() {
  h2 = (int)dht2.readHumidity();
  t2 = (int)dht2.readTemperature();

  h3 = (int)dht3.readHumidity();
  t3 = (int)dht3.readTemperature();
}

void readMotion() {
  if (digitalRead(MOTION) == HIGH)
  {
    motion = 1;
  }
  else
  {
    motion = 0;
  }
}

// led 1 apagado 0 encendido
void sendData() {

  SM.setSerial0Mux1();
  char data[200];
  Serial.begin(115200);

  sprintf(data, "{\"node\":\"%s\", \"data\":\"TEMP2:%d|HUM2:%d|TEMP3:%d|HUM3:%d|PIR:%d|MAG:%d|LUX:%d|HUM:%d|ROBO:%d|SUN:%d|BZR:%d\"}", "HACKATHON2017_04", t2, h2, t3, h3, motion, window, lux, hum, robo, voltage, 0);
  WiFiEspClient client;
  if (client.connect("iotsens-inbox.grupogimeno.com", 80)) {
    dimm.setLed(LED_A, LED_FAST);
    client.println(F("POST /services/rawmessage?appName=HACKATHON2017&pass=XXXXXXXXXXXXXXX HTTP/1.1"));
    client.println(F("Host: iotsens-inbox.grupogimeno.com"));
    client.println(F("User-Agent: Arduino/1.0"));
    client.println(F("Connection: close"));
    client.println(F("Content-Type: application/json"));
    client.print(F("Content-Length: "));
    client.println(strlen(data));
    client.println();
    client.println(data);
  }

  SM.setSerial0Mux0();
  Serial.begin(38400);
  Serial.println("DATA SENT");
  dimm.setLed(LED_A, LED_OFF);

}
