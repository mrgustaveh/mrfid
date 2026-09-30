#include <LiquidCrystal.h>
#include <SPI.h>
#include <MFRC522.h>

#define LCD_RS_PIN 9
#define LCD_RW_PIN 10
#define LCD_ENABLE_PIN 11
#define LCD_D0 2
#define LCD_D1 3
#define LCD_D2 4
#define LCD_D3 5

#define LCD_COLS 16
#define LCD_ROWS 2

#define RF_SS_PIN 7
#define RF_RST_PIN 8

LiquidCrystal Lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
MFRC522 RfId(RF_SS_PIN, RF_RST_PIN);
MFRC522::MIFARE_Key RfKey;

bool ONE_TIME_FLAG {false};

void setup() {
  Lcd.begin(LCD_COLS, LCD_ROWS);
  Lcd.display();
  Lcd.clear();
  
  Serial.begin(9600);

  SPI.begin();
  RfId.PCD_Init();
  Serial.println("\nRFID ready");

  for(byte i = 0; i < 6; i++) {
    RfKey.keyByte[i] = 0xFF;
  }
  
  Lcd.clear();
  Lcd.setCursor(0, 2);
  Lcd.print("PLACE CARD");
  Lcd.setCursor(0, 3);
  Lcd.print("ON READER");

  delay(1000);
}

bool isWriteableBlock(byte block){
  if(block == 0) return false;

  if((block + 1) % 4 == 0) return false;

  return true;
}

void loop() {
  byte writeBuffer[18] {0};
  byte readBuffer[16] {0};
  byte sizeofWriteBuffer {sizeof(writeBuffer)};
  byte bytesToRead, bytesToWrite, sector, blockNumber, trailerBlock;
  MFRC522::StatusCode status;

  if(ONE_TIME_FLAG == false){
    Lcd.clear();
    Lcd.setCursor(0, 2);
    Lcd.print("PLACE CARD");
    Lcd.setCursor(0, 3);
    Lcd.print("ON READER");

    ONE_TIME_FLAG = true;
  };

  if(!RfId.PICC_IsNewCardPresent() || !RfId.PICC_ReadCardSerial()) return;

  ONE_TIME_FLAG = false;

  Lcd.clear();
  Lcd.setCursor(0, 2);
  Lcd.print("READING CARD");
  Lcd.setCursor(0, 3);
  Lcd.print("WAIT...");
  delay(1000); // 1s

  Serial.print("CARD UID");
  Lcd.clear();
  Lcd.setCursor(0, 2);
  Lcd.print("CARD UID");
  Lcd.setCursor(0, 1);
    
  for (byte i = 0; i < RfId.uid.size; i++) {
    Serial.print(RfId.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(RfId.uid.uidByte[i], HEX);
    Lcd.print(RfId.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Lcd.print(RfId.uid.uidByte[i], HEX);
  };

  Serial.println();

  // flush serial input
  while (Serial.available()) {
    Serial.read();
  };

  do {
    Lcd.clear();
    Lcd.home();
    Lcd.print("ENTER BLOCK NO.");
    Serial.println("Enter Block Number (0 - 63)");

    while (!Serial.available());
    
    blockNumber = Serial.parseInt();
    Serial.read();

    if(!isWriteableBlock(blockNumber)){
      Serial.println("cannot write on this block");
      Lcd.setCursor(0, 1);
      Lcd.println("INVALID BLOCK");
      delay(1000);
    } else {
      Lcd.setCursor(0, 1);
      Lcd.print(blockNumber);
      delay(1000);
      break;
    };
  } while (1);

  while (Serial.available()) {
    Serial.read();
  };

   do {
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("ENTER BYTES NO.");
    
    Serial.println("Enter the number of bytes to write (1-16): ");
    while (!Serial.available());

    bytesToRead = Serial.parseInt();
    Serial.read();

    if (bytesToRead < 1 || bytesToRead > 16) {
      Serial.println("\nInvalid byte count.");
      Lcd.setCursor(0, 1);
      Lcd.print("Invalid bytes no");
      delay(1000);
    } else {
      Lcd.setCursor(0, 1);
      Lcd.print(bytesToRead);
      delay(1000);
      break;
    };
  } while (1);


  while (Serial.available()) {
    Serial.read();
  };

  do {
    Serial.setTimeout(60000);
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("Enter Data : ");
    Serial.print("Enter your Data : ");

    while (Serial.available()) { Serial.read(); };

    memset(readBuffer, 0x00, 16);
    bytesToRead = Serial.readBytes(readBuffer, bytesToRead);
    readBuffer[bytesToRead] = '\0';

    if (bytesToRead > 0) {
      Lcd.setCursor(0, 1);
      Lcd.print((char *)readBuffer);
      delay(1000);
      break;
    } else {
      Serial.println("\nNo Data Received.");
    };
  } while (1);

  // authenticate block
  sector = blockNumber / 4;
  trailerBlock = sector * 4 + 3;

  status = RfId.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &RfKey, &(RfId.uid));

  if (status != MFRC522::STATUS_OK) {
    Serial.print("Authentication failed: ");
    Serial.println(RfId.GetStatusCodeName(status));
    RfId.PICC_HaltA();
    RfId.PCD_StopCrypto1();
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("AUTH FAILED");
    Lcd.setCursor(0, 1);
    Lcd.print(RfId.GetStatusCodeName(status));
    delay(1000);
    return;
  } else {
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("AUTH");
    Lcd.setCursor(0, 1);
    Lcd.print("SUCCESSFUL");
    delay(3000);
  }

  memset(writeBuffer, 0x00, sizeofWriteBuffer);

  status = RfId.MIFARE_Read(blockNumber, writeBuffer, &sizeofWriteBuffer);
  
  if (status == MFRC522::STATUS_OK) {
    memcpy(writeBuffer, readBuffer, bytesToRead);
    status = RfId.MIFARE_Write(blockNumber, writeBuffer, 16);

    if (status == MFRC522::STATUS_OK) {
      Serial.println("Data written successfully.");
      Lcd.clear();
      Lcd.setCursor(0, 0);
      Lcd.print("DATA WRITTEN");
      delay(3000);
    } else {
      Serial.print("Write failed: ");
      Serial.println(RfId.GetStatusCodeName(status));
      Lcd.clear();
      Lcd.setCursor(0, 0);
      Lcd.print("WRITE FAILED");
      Lcd.print(RfId.GetStatusCodeName(status));
      delay(3000);
    };
  } else {
    Serial.print("Read Failed: ");
    Serial.println(RfId.GetStatusCodeName(status));
    Lcd.clear();
    Lcd.setCursor(0, 0);
    Lcd.print("READ FAILED");
    Lcd.setCursor(0, 1);
    Lcd.print(RfId.GetStatusCodeName(status));
    delay(3000);
    return;
  };

  Serial.print("\ncard data:\n");
  RfId.PICC_DumpToSerial(&(RfId.uid));
  RfId.PCD_StopCrypto1();

  Lcd.clear();
  Lcd.setCursor(0, 0);
  Lcd.print("REMOVE CARD");
  delay(2000);
};