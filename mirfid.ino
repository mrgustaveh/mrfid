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

bool onetimeFlag = false;

void setup() {
  Lcd.begin(LCD_COLS, LCD_ROWS);
  Lcd.blink();
  Lcd.clear();

  Serial.begin(9600);

  SPI.begin();
  RfId.PCD_Init();
  Serial.println("\nRFID ready");
}

void loop() {
  
}
