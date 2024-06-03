#include "secrets.h"
#include <Servo.h>
#include <WiFiNINA.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>

// 1. SETUP
// SETUP PINS 
#define SS_PIN 10
#define RST_PIN 9
#define Buzzer 2
#define LED1 4
#define LED2 5

// LCD SCREEN SETUP
LiquidCrystal_I2C lcd(0x27, 16, 2);
byte lock = 0;

// STORE CREDENTIAL FOR WIFI NETWORK TO CONNECT TO
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

Servo servo;
MFRC522 rfid(SS_PIN, RST_PIN);
byte led1 = 0;
byte led2 = 0;
WiFiClient client;

//IFTTT SERVICE SETUP
char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME   = "/trigger/door_notification/with/key/ZvAxt_sFPoA_E-YXgLoRQ"; 

//https://maker.ifttt.com/trigger/door_notification/with/key/ZvAxt_sFPoA_E-YXgLoRQ


//INITIALIZE SETUP
void setup() {
  //CONNECT TO WIFI
  WiFi.begin(ssid, pass);

  //START SERIAL COMMUNICATION
  Serial.begin(9600);
  while (!Serial);
  
  //INITIALIZE LCD, SPI AND RFPID
  lcd.init();
  lcd.backlight();
  SPI.begin();
  rfid.PCD_Init();

  //SETTING PIN MODE
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(Buzzer, OUTPUT);

  //LOCK POSITION START
  servo.write(0);
  servo.attach(3);


  // IFTTT SERVER - connect to web server on port 80:
  if (client.connect(HOST_NAME, 80)) {
    // if connected:
    Serial.println("Connected to server");
  }
  else {// if not connected:
    Serial.println("connection failed");
  }

    Serial.println("SCAN YOUR CARD");
}


// MAIN LOOP 
void loop() {
  //DISPLAY START MESSAGE ON LCD
  lcd.setCursor(4, 0);
  lcd.print("Welcome!");
  lcd.setCursor(1, 1);
  lcd.print("Put your card");

  // CHECK AND READ CARD
  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  // CLEAR LCD AND START THE SCANNING PROCESS
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Scanning");

  // INDICATE THE CARD READ TO THE SERIAL MONITOR
  Serial.print("CARD READ:");
  tone(Buzzer, 700);
  delay(200);
  noTone(Buzzer);
  String ID = "";

  //READING AND DISPLAYING RFID ARD UID
  for (byte i = 0; i < rfid.uid.size; i++) {
    Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(rfid.uid.uidByte[i], HEX);
    ID.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    ID.concat(String(rfid.uid.uidByte[i], HEX));
    delay(300);
  }
  Serial.println();
  ID.toUpperCase();


  //AUTHORISED CARD UNLOCK
  if (ID.substring(1) == "B3 49 D4 24" && led1 == 0 ) {
    unlockSound();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is open");
    delay(1500);
    lcd.clear();

    digitalWrite(LED1, HIGH);
    servo.write(70);
    Serial.println("DOOR UNLOCKED");
    led1 = 1;
    sendNotification("DOOR UNLOCKED", "");

  //AUTHORISED CARD LOCK
  } else if (ID.substring(1) == "B3 49 D4 24" && led1 == 1 ) {
    lockSound();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Door is locked");
    delay(1500);
    lcd.clear();

    digitalWrite(LED1, LOW);
    Serial.println("DOOR LOCKED");
    servo.write(0);
    led1 = 0;
    sendNotification("DOOR LOCKED", "");

  //UNAUTHORISED CARD ACCESS DENIED
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wrong card!");
    delay(1500);
    lcd.clear();

    digitalWrite(LED1, LOW);
    servo.write(0);
    digitalWrite(LED2, HIGH);
    failSound();
    delay(1000);
    digitalWrite(LED2, LOW);
    Serial.print("ACCESS DENIED!");
    sendNotification("UNAUTHORISED CARD ACCESS ATTEMPT!", "");
  }
}

// METHOD FOR SENDING CUSTOM NOTIFICATION
void sendNotification(String value1, String value2) {
  if (client.connect(HOST_NAME, 80)) {
    client.println("GET " + PATH_NAME + value1 + value2 + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println();
    delay(10);
  }
}

//UNLOCK SOUND
void unlockSound() {
  tone(Buzzer, 700);
    delay(200);
    noTone(Buzzer);
    delay(150);
    tone(Buzzer, 700);
    delay(200);
    noTone(Buzzer);
    digitalWrite(LED1, HIGH);
    servo.write(0);
}

//LOCK SOUND
void lockSound() {
  tone(Buzzer, 700);
    delay(500);
    noTone(Buzzer);
}

// UNAUTHORISED CARD SOUND
void failSound() {
  tone(Buzzer, 200);
    delay(200);
    noTone(Buzzer);
    delay(150);
    tone(Buzzer, 200);
    delay(200);
    noTone(Buzzer);
}

