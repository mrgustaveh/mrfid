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

#define ONE_TIME_FLAG false

LiquidCrystal Lcd(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
MFRC522 RfId(RF_SS_PIN, RF_RST_PIN);
MFRC522::MIFARE_Key RfKey;

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
  
}
