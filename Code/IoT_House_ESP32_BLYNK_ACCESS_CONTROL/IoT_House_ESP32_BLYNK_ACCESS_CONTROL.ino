/* Final Project Sensors and Interfacing
   Koen Verbeeck

   Important notes*:

   The RFID-RC522 Card Reader should be connected directly to the ESP-32's 3.3V port, external power sources will not work !!!!
   In case the RFID-RC522 Card Reader is stuck in Limbo, re-power the ESP32.
   
   De ESP32 en de RFID-RC522 Card Reader gebruiken het SPI protocol voor communicatie (de chip ondersteunt ook I2C en UART protocollen,
   maar deze worden niet ondersteunt door de library). The card reader en de kaart/badge communiceren via een frequentie van 13.56 MHz
   (ISO 14443A standard tags)

   Interrupt Pins op de ESP32 Dev Kit C V4: All GPIO Pins
   PWM Pins op de ESP32 Dev Kit C V4: All Digital In/Out ports

   PIR HC SR501 Motion Sensor status, 0 is geen detectie, 1 is detectie. De sensor zal op 1 blijven staan gedurende de ingestelde
   time delay na detectie.

   Geef de PIR sensor enkele seconden nadat er een switch tussen zon en maanstand gebeurt !!!!

   BLYNK: v0.6.1 is de laatste stabiele versie, NIET updaten naar 1.0.0 Beta, deze geeft problemen.
   5 seconds delay tussen BLYNK Push Notifications.
*/

#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h> // Library for BLYNK
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>
#include <MFRC522.h>
#include <SPI.h>
#include <Wire.h>

/*
   BLYNK
*/
// Own Blynk Server and port
char server[] = "192.168.10.122"; // RPi4B
#define port 8080 // Default port

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "********";

/*
   Wi-Fi
*/
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "********"; // Use the Orange modem Wi-Fi
char pass[] = "********";

/* OLED 128x64 Monochrone Screen
*/
#define SCREEN_WIDTH 128 // Breedte van het scherm
#define SCREEN_HEIGHT 64 // Hoogte van het scherm

// Initialize the OLED screen
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// BITMAP Koen_LEGO start-up screen
static const uint8_t image_data_KoenLego2[1024] = {
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xbf, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x3f, 0xf3, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x3f, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x3e, 0x7e, 0x7b, 0x3f, 0xff, 0xff,
  0xff, 0xfe, 0x03, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf2, 0x78, 0x18, 0x38, 0x1f, 0xff, 0xff,
  0xff, 0xff, 0xf3, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x71, 0x99, 0x98, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x73, 0x99, 0x99, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0xd7, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf2, 0x73, 0x90, 0x19, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0x97, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x33, 0x91, 0xf9, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0x87, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x13, 0x99, 0xf9, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0xc7, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x98, 0x18, 0x19, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf3, 0x88, 0x3c, 0x19, 0x9f, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xdf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xff, 0xbf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xef, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xff, 0xf8, 0xef, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf7, 0xc1, 0xff, 0xcf, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf8, 0x00, 0x1f, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfc, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfe, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0x00, 0x07, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xe7, 0xff,
  0xf0, 0x00, 0x8e, 0x00, 0xff, 0xff, 0xff, 0xfc, 0xf9, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xe7, 0xff,
  0x80, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xc3, 0xfc, 0xf9, 0xff, 0xf9, 0xff, 0xff, 0xff, 0xe7, 0xff,
  0x03, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x03, 0xfe, 0x73, 0xff, 0xfb, 0xff, 0xff, 0xff, 0xe7, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0xf0, 0x03, 0xfe, 0x73, 0x0e, 0x18, 0x3c, 0x3c, 0x3c, 0x66, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x0f, 0xe0, 0x01, 0xfe, 0x72, 0x06, 0x18, 0x18, 0x18, 0x18, 0x24, 0x7f,
  0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x01, 0xfe, 0x26, 0x66, 0x39, 0x99, 0x9b, 0x91, 0xe4, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x01, 0xff, 0x26, 0x06, 0x7b, 0x98, 0x10, 0x13, 0xe1, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0x26, 0x06, 0x7b, 0x90, 0x10, 0x13, 0xe1, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0xff, 0x06, 0x7e, 0x79, 0x99, 0xf3, 0xf3, 0xe4, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0xff, 0x8e, 0x7e, 0x79, 0x99, 0xf9, 0xf1, 0x24, 0x7f,
  0x00, 0x00, 0x00, 0x18, 0x03, 0xc0, 0x00, 0xff, 0x8f, 0x06, 0x70, 0x18, 0x18, 0x18, 0x26, 0x7f,
  0x00, 0x00, 0x00, 0x20, 0x01, 0xc6, 0x00, 0x7f, 0x9f, 0x8e, 0x7a, 0x7e, 0x3c, 0x3c, 0x67, 0x7f,
  0x20, 0x00, 0x00, 0x28, 0x01, 0xc6, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x60, 0x00, 0x00, 0x00, 0x00, 0x60, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xe3, 0x00, 0x40, 0x00, 0x00, 0xe0, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf3, 0x00, 0x00, 0x00, 0x0b, 0x10, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf0, 0x00, 0x00, 0x00, 0x0c, 0x10, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf8, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfc, 0x10, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xff, 0xe0, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xfb, 0xc0, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xf8, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xe0, 0x80, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0xc1, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x82, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x04, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void welcome_OLED_Koen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20); // x, y
  // Display static text
  display.println("Welcome,");
  display.println("Koen");
  display.display();
}

void welcome_OLED_Vero() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20); // x, y
  // Display static text
  display.println("Welcome,");
  display.println("Vero");
  display.display();
}

void denied_OLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20); // x, y
  // Display static text
  display.println("ALERT !!!");
  display.display();
}

void Present_Badge_OLED() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 20); // x, y
  // Display static text
  display.println("Present,");
  display.println("your badge");
  display.display();
}

/* RFID-RC522 Card Reader
*/
#define RST_PIN 17 // Reset Pin
#define SS_PIN 5 // SDA Pin

// Maak een virtueel object aan genaamd "mfrc522" en definieer de SDA en Reset Pin
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

/* SERVO Motor
*/
#define servoPin 13
Servo myservo; // Maak een virtueel object aan genaamd "myservo"

void open_close_Door() {
  myservo.write(0); // Open de deur
  delay(3000); // Wacht 3 seconden
  myservo.write(125); // Sluit de deur
  delay(300);
}

void open_Door() {
  myservo.write(0); // Open de deur
  Serial.println();
  Serial.println("Door Opened using BLYNK app");
}

void close_Door() {
  myservo.write(125); // Sluit de deur
  Serial.println();
  Serial.println("Door closed using BLYNK app");
}

// This function will open and close the door with the BLYNK app.
BLYNK_WRITE(V3)
{
  int pinValue = param.asInt();
  if (pinValue == 1)
  {
    open_Door();
  }
  else {
    close_Door();
  }
}

/* Red LEDs
*/
#define ledPin 32

/* Active Buzzer
*/
#define buzzerPin 12

// Functie om een trieste toon af te spelen en 3 LEDs te laten blinken
void sad_Tone_blink_LEDs() {
  for (int i = 0 ; i < 4 ; i++) {
    digitalWrite(buzzerPin, HIGH), digitalWrite(ledPin, HIGH);
    delay(200);
    digitalWrite(buzzerPin, LOW), digitalWrite(ledPin, LOW);
    delay(100);
  }
}

// Functie om een blije toon af te spelen
void happy_Tone() {
  for (int i = 0; i < 120; i++)
  {
    digitalWrite(buzzerPin, HIGH);
    delay(1);
    digitalWrite(buzzerPin, LOW);
    delay(1);
  }
}

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200); // Initieer de serieele monitor

  /*
     Wi-Fi
  */
  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  int wifi_ctr = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  /*
     BLYNK
  */
  // Initialize the BLYNK Client
  Blynk.begin(auth, ssid, pass, server, port);

  /* OLED 128x64 Monochrone Screen
  */
  // Initialiseer het OLED scherm op I2C address 0x3C
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  delay(2000); // Pause for 2 seconds

  // Clear the buffer.
  display.clearDisplay();

  // Draw Start-up screen bitmap on the screen
  // const tImage KoenLego2 = { image_data_KoenLego2, 128, 64, 8 }; >> info from bitmap (LCD Image Converter)
  display.drawBitmap(0, 0, image_data_KoenLego2, 128, 64, 1);
  display.display();

  /* RFID-RC522 Card Reader
  */
  SPI.begin(); // Initieer de SPI bus
  mfrc522.PCD_Init(); // Initieer de MFRC522 RFID Card Reader
  Serial.println("Please present your tag to the RFID tag reader...");
  Serial.println();

  /* SERVO Motor
  */
  myservo.attach(servoPin); // Initialize Servo Motor
  myservo.write(125); // Servo start positie

  /* Active buzzer
  */
  pinMode(buzzerPin, OUTPUT);

  /* Red LEDs
  */
  pinMode(ledPin, OUTPUT);

  // Pause for 2 seconds after set-up
  delay(5000); 
}



void loop() {
  // put your main code here, to run repeatedly:

  // Deze functie moete bovenaan staan, anders werkt het niet.
  Blynk.run();

  // Toon "Present, your badge" op het OLED scherm
  Present_Badge_OLED();

  // Wacht op een badge en lees deze in
  if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  // Lees de UID van de kaart
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  // Toon de UID op de serial monitor

  Serial.println(); // Print een lege lijn
  Serial.print("Uw UID is: ");
  String content;

  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }

  content.toUpperCase();
  if ((content.substring(1) == "C9 D1 82 A2") || (content.substring(1) == "47 E6 19 B2")) // Zet hier de UID(s) van de kaarten die je toegang wil verschaffen
    // In case of blue badge UID: C9 D1 82 A2 or blue badge UID: 47 E6 19 B2
  {
    Serial.println("\t >>> Your RFID tag is known");

    if (content.substring(1) == "C9 D1 82 A2") {
      // Blij gezicht + blije toon + bericht op Serial Monitor !!!
      Serial.print("You may enter, welcome Koen !!!");
      Serial.println();
      Blynk.notify("Koen has just entered the IoT House");
      welcome_OLED_Koen();
    }
    if (content.substring(1) == "47 E6 19 B2") {
      // Blij gezicht + blije toon + bericht op Serial Monitor !!!
      Serial.print("You may enter, welcome Vero !!!");
      Serial.println();
      Blynk.notify("Vero has just entered the IoT House");
      welcome_OLED_Vero();
    }
    // Deur gaat open
    happy_Tone();
    open_close_Door();
  }

  else {
    // Elke andere kaart waarvan de UID niet is geconfigureerd om toegang te verschaffen
    // Deur blijft toe + triest gezicht + trieste toon + bericht op Serial Monitor !!!
    Serial.println("\t >>> Your RFID tag is unknown");
    Serial.println("Access Denied !!!");
    Serial.println();
    Blynk.notify("IoT House: Alarm !!!! Unknown RFID tag detected");
    denied_OLED();
    sad_Tone_blink_LEDs();
  }
}
