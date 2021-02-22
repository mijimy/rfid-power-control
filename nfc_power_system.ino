

#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SS_PIN 10
#define RST_PIN 9

#define buzzer 3
#define power 4


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

byte key_0[] = {0x33, 0xD2, 0xEE, 0x0C};
byte key_1[] = {0x4c, 0x17, 0x96, 0x21};

bool power_on = 0; // power on status
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[4];

void setup() {
  pinMode(buzzer, OUTPUT);
  pinMode(power, OUTPUT);
  digitalWrite(power, 1);
  Serial.begin(9600);
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds
  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  tone(buzzer, 1000); // Send 1KHz sound signal...
  delay(200);        // ...for 1 sec
  noTone(buzzer);     // Stop sound...
}

void loop() {
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  if (power_on == 0)
  {
    display.setCursor(0, 0);
    display.println("Scan card");
    display.println(" For Power");
    display.display();      // Show initial text
    delay(500);
  }
  else
  {
    display.setCursor(0, 0);
    display.println("Power ON");
    display.println("*********");
    display.println("Swipe card");
    display.println("to OFF");
    display.display();      // Show initial text
    delay(500);
  }

  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }


  Serial.println(F("card has been detected."));

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
  }
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  Serial.print(F("In dec: "));
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();
  bool pass = 0;
  if (key_0[0] == nuidPICC[0] ||    // checking key 0
      key_0[1] == nuidPICC[1] ||
      key_0[2] == nuidPICC[2] ||
      key_0[3] == nuidPICC[3] ) {
    pass = 1;
  }
  else if (key_1[0] == nuidPICC[0] ||    // checking key 1
           key_1[1] == nuidPICC[1] ||
           key_1[2] == nuidPICC[2] ||
           key_1[3] == nuidPICC[3] ) {
    pass = 1;
  }



  if (pass == 1) // correct card
  {
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(500);
    noTone(buzzer);     // Stop sound...
    if (power_on == 0)
    { Serial.println(F("powering on"));
      power_on = 1;
       digitalWrite(power, 0);
      display.setTextSize(2); // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
      display.clearDisplay();
      display.setCursor(5, 0);
      display.println("Power ON ");
      display.display();      // Show initial text
      delay(2000);
    }
    else
    { Serial.println(F("powering off"));
      power_on = 0;
       digitalWrite(power, 1);
      display.setTextSize(2); // Draw 2X-scale text
      display.setTextColor(SSD1306_WHITE);
      display.clearDisplay();
      display.setCursor(5, 0);
      display.println("Power OFF ");
      display.display();      // Show initial text
      delay(2000);
    }

  }

  else
  {
    Serial.println(F("wrong card"));
    display.setTextSize(2); // Draw 2X-scale text
    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    display.setCursor(5, 0);
    display.println("Wrong card");
    display.println("**********");
    display.display();      // Show initial text
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(200);
    noTone(buzzer);     // Stop sound...
    delay(200);
    tone(buzzer, 1000); // Send 1KHz sound signal...
    delay(200);
    noTone(buzzer);     // Stop sound...
    delay(200);
  }

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte * buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
