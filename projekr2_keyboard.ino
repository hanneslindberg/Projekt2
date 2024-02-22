#include <Wire.h>
#include <U8glib.h>
#include <RtcDS3231.h>
#include <Keyboard.h>

#define SwitchMenuPin 8 // Definiera knappens pin för att växla mellan visning av tid och temperatur
const int buttonPins[] = {12, 11, 10, 9, 8}; // knapparnas pins
const int numButtons = 5; // antal knappar

char buttonChars[] = {'0', '9', '8', '7'}; // tecken som skrivs ut av knapparna

int previousButtonState[numButtons] = {HIGH}; // hålle tidigare knapptryckningsstatus

RtcDS3231<TwoWire> Rtc(Wire); // skapar en rtc insats
int hour;  // skapa variablar av tid, datum och temperatur
int minute;
int day;
int month;
int temp;

U8GLIB_SSD1306_128X64_2X oled(U8G_I2C_OPT_NO_ACK); // skapa en oled insats


void setup() {
  Keyboard.begin(); // aktivera tangentbordsfunktionen
  
  Serial.begin(9600); // seriell komunikation
  pinMode(SwitchMenuPin, INPUT_PULLUP); // kolla pinstatusen på knappen till oleden med pullup motstånd
  
  Wire.begin(); // starta I2C kommunikation
  oled.setFont(u8g_font_helvB10); // ange teckensnitt för oleden
  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__); // hämta datum och tid vid kompilering
  Rtc.SetDateTime(compiled); // ange datum och tid för rtc:n

  for (int i = 0; i < numButtons; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP); // ange de andra knapparnas pins med pullup motstånd
  }
}

void loop() {
  //loopa genom varje knapp och hantera knapptryck
  for (int i = 0; i < numButtons; i++) {
    int buttonState = digitalRead(buttonPins[i]);

    if (buttonState == LOW && previousButtonState[i] == HIGH) { // detta betyder att knappen nu har annorlunda status än den hade innan, alltså är den nedtryckt
      char key = buttonChars[i];
      Keyboard.press(key); // säg åt tangentbordet att trycka på knappen som motsavrar knappen
      delay(50);
    } else if (buttonState == HIGH && previousButtonState[i] == LOW) {
      char key = buttonChars[i];
      Keyboard.release(key); // släpp tangenten
      delay(50);
    }

    previousButtonState[i] = buttonState; // säg att de knappen va innan är den nu så att if-satsen ska känna igen att knappen trycks ner
  }
  
  // uppdatera rtc och läs av temperaturen om knapp 5 är tryckt
  updateRTC();

  if (digitalRead(buttonPins[4]) == LOW) { // visa tid och datum, men temperatur om knapp 5 är nertryckt
    updateOledSecond("Temp: " + String (temp) + " C");
  } else {
    updateOled("Time:  " + String(hour) + ":" + String(minute), "Date:  " + String(day) + " / " + String(month));
  }  
}


void updateRTC() { // funktion för att se tid, datum och temperatur på rtc:n
  RtcDateTime now = Rtc.GetDateTime();
  hour = now.Hour();
  minute = now.Minute();
  day = now.Day();
  month = now.Month();

  RtcTemperature rtcTemp = Rtc.GetTemperature();
  temp = rtcTemp.AsFloatDegC(); // omvandla till celcius
}

void updateOled(String text, String date) { // funktion för att visa tid och datum på oleden
  oled.firstPage();
  do {
    oled.drawStr(17, 44, text.c_str());
    oled.drawStr(17, 30, date.c_str());
  } while  (oled.nextPage());
}


void updateOledSecond(String temp) { // funktion för att visa temperaturen i rummet
 oled.firstPage();
  do {
    oled.drawStr(20, 35, temp.c_str());
  }while (oled.nextPage());
}
