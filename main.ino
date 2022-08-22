// all imported libraries
// <LIB.h> is an official imported library
// "LIB.h" is an locally imported library, custom made or modified.
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Relay.h"
#include <Button.h>
#include <Adafruit_GFX.h>    
#include <Adafruit_ST7735.h> 
#include <SPI.h>
#include <Thread.h>
#include <EEPROM.h>

#define TFT_CS    8
#define TFT_RST   9  
#define TFT_DC    10 

#define TFT_SCLK 11   
#define TFT_MOSI 12   

#define WIDTH    160
#define HEIGHT   128

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// All Relay classes & pins associated.
Relay p1 = Relay(2);
Relay p2 = Relay(3);
Relay v1 = Relay(4);
Relay v2 = Relay(5);
Relay v3 = Relay(6);

// GPIO where the DS18B20 is/are connected to
#define DS18B20PIN_ALL    7

Button button_left(A0, 300);
Button button_mid(A3, 300);
Button button_right(A5, 300);

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(DS18B20PIN_ALL);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// temp sensors uniques addresses
const uint8_t sensorsIds[6][8] = {
  {40,44,227,72,246,77,60,238}, // T1
  {40,102,105,72,246,00,60,48}, // T2
  {40,185,221,72,246,36,60,122},// T3
  {40,237,204,72,246,237,60,63},// T4
  {40,143,110,72,246,105,60,54},// T5
  {40,84,194,72,246,217,60,26}  // RT
};

// temp sensors associated local values
float sensorsTemperature[6] = {DEVICE_DISCONNECTED_C,DEVICE_DISCONNECTED_C,DEVICE_DISCONNECTED_C,DEVICE_DISCONNECTED_C,DEVICE_DISCONNECTED_C,DEVICE_DISCONNECTED_C};

int T2_MAX;
int T3_MAX;
int RT_MAX;

Thread thread1 = Thread();
Thread thread2 = Thread();
Thread thread3 = Thread();

long lastScreenUpdate;

int eeAddr = 0;

void setup() {
  
  // Enable reading serial logs through USB cable and Arduino desktop app
  Serial.begin(9600);
  Serial.println(F("starting..."));
  
  tft.initR(INITR_BLACKTAB);  
  tft.setRotation(1);
  tft.fillScreen(ST7735_BLACK); 
 
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(2);
  centeredString("R-Heating", 105, HEIGHT/2.2);
  tft.setTextSize(1);
  tft.setCursor(WIDTH-70, HEIGHT-15);
  tft.print(F("by barbierb"));
  delay(1000);
  tft.fillScreen(ST7735_BLACK); 
  
  Serial.println(F("TFT screen... OK"));
  
  pinMode(LED_BUILTIN, OUTPUT);

  tft.setCursor(2, 10);
  tft.print(F("Buttons:"));
  button_left.begin();
  button_mid.begin();
  button_right.begin();

  tft.print(F(" L="));
  tft.print(button_left.read());
  tft.print(F(" M="));
  tft.print(button_mid.read());
  tft.print(F(" R="));
  tft.print(button_right.read());
  
  tft.setCursor(2, 22);
  tft.print(F("EEPROM: "));
  EEPROM.get(eeAddr, T2_MAX);
  EEPROM.get(eeAddr+sizeof(int), T3_MAX);
  EEPROM.get(eeAddr+sizeof(int)*2, RT_MAX);
  if(T2_MAX == -1)
    T2_MAX = 60;
  if(T3_MAX == -1)
    T3_MAX = 40;
  if(RT_MAX == -1)
    RT_MAX = 20;

  Serial.print(F("T2_MAX="));
  Serial.print(T2_MAX);
  tft.print(F("T2_MAX="));
  tft.print(T2_MAX);
  Serial.print(F("T3_MAX="));
  Serial.println(T3_MAX);
  tft.print(F(" T3_MAX="));
  tft.print(T3_MAX);
  Serial.print(F("RT_MAX="));
  Serial.println(RT_MAX);
  tft.print(F(" RT_MAX="));
  tft.print(RT_MAX);
  

  tft.setCursor(2, 40);
  tft.print(F("TEMP SENSORS:"));
  sensors.requestTemperatures();

  for(int i = 0; i < (sizeof(sensorsIds) / sizeof(sensorsIds[0])); i++) {
    float temp = sensors.getTempC(sensorsIds[i]);

    if(i==0)
      tft.print(F(" T1="));
    else if(i==1)
      tft.print(F(" T2="));
    else if(i==2)
      tft.print(F(" T3="));
    else if(i==3)
      tft.print(F(" T4="));
    else if(i==4)
      tft.print(F(" T5="));
    else if(i==5)
      tft.print(F(" RT="));

    if(temp == DEVICE_DISCONNECTED_C) {
      tft.print(F("DISCONNECTED"));
      while(1);
    }
    tft.print(F("OK"));
  }

  updateTemp(nullptr);
  delay(1000);
  
  tft.fillScreen(ST7735_BLACK); 

  thread1.onRun(updateTemp);
  thread1.setInterval(1000);
  thread2.onRun(updateLogic);
  thread2.setInterval(1000);
  thread3.onRun(updateScreen);
  thread3.setInterval(500);
}

int menu = 0;

void loop() {
  if(thread1.shouldRun())
    thread1.run();
  if(thread2.shouldRun())
    thread2.run();
  if(thread3.shouldRun())
    thread3.run();

  int left = button_left.read();
  int mid = button_mid.read();
  int right = button_right.read();
  Serial.print(left);
  Serial.print(mid);
  Serial.print(right);
  Serial.println();

  if(menu > 0 && millis() > lastScreenUpdate + 15000) {
      tft.fillScreen(ST7735_BLACK);
      menu = 0;
      thread3.enabled = true;
      return;
  }

  if(menu == 0 && mid == 0) {
    menu = 1;
    thread3.enabled = false;
    tft.fillScreen(ST7735_BLACK); 
    lastScreenUpdate = millis(); 
    updateScreen();
    
  } else if(menu == 1) {

    if(left == 0) {
      RT_MAX-=1;
      EEPROM.put(eeAddr+sizeof(int)*2, RT_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    }else if(right == 0) {
      RT_MAX+=1;
      EEPROM.put(eeAddr+sizeof(int)*2, RT_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    } else if(mid == 0) {
      tft.fillScreen(ST7735_BLACK); 
      menu = 2;
      lastScreenUpdate = millis(); 
      updateScreen();
    }

    
  } else if(menu == 2) {
    if(left == 0) {
      T2_MAX-=1;
      EEPROM.put(eeAddr, T2_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    }else if(right == 0){
      T2_MAX+=1;
      EEPROM.put(eeAddr, T2_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    } else if(mid == 0) {
      tft.fillScreen(ST7735_BLACK); 
      menu = 3;
      lastScreenUpdate = millis(); 
      updateScreen();
    }
  } else if(menu == 3) {
    if(left == 0) {
      T3_MAX-=1;
      EEPROM.put(eeAddr+sizeof(int), T3_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    }else if(right == 0){
      T3_MAX+=1;
      EEPROM.put(eeAddr+sizeof(int), T3_MAX);
      lastScreenUpdate = millis(); 
      updateScreen();
    } else if(mid == 0) {
      tft.fillScreen(ST7735_BLACK);
      menu = 0;
      thread3.enabled = true;
    }
  }
  delay(10);
}


void updateScreen() {
  
  if(menu == 0) {
    
    tft.setTextSize(1);
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setCursor(3, 10);
    tft.print(F("HT SOL:"));
    tft.setCursor(3, 30);
    tft.print(F("HT STO:"));
    tft.setCursor(3, 50);
    tft.print(F("LT STO:"));
    tft.setCursor(3, 70);
    tft.print(F("LT SOL:"));
    tft.setCursor(3, 90);
    tft.print(F("UnderF:"));
    tft.setCursor(3, 110);
    tft.print(F("RoomT\367:"));
    
    
    float t1 = sensorsTemperature[0];
    float t2 = sensorsTemperature[1];
    float t3 = sensorsTemperature[2];
    float t4 = sensorsTemperature[3];
    float t5 = sensorsTemperature[4];
    float rt = sensorsTemperature[5];

    
    char buffer_temp[6];
    char str_temp[6];
    
    dtostrf(t1, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 10);
    tft.print(buffer_temp);
    
    dtostrf(t2, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 30);
    tft.print(buffer_temp);
    
    dtostrf(t3, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 50);
    tft.print(buffer_temp);
    
    dtostrf(t4, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 70);
    tft.print(buffer_temp);
    
    dtostrf(t5, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 90);
    tft.print(buffer_temp);
    
    dtostrf(rt, 4, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    tft.setCursor(50, 110);
    tft.print(buffer_temp);

    tft.drawLine(92, 0, 92, HEIGHT, ST7735_WHITE);

    tft.setCursor(102, 10);
    tft.print(F("P HT: "));
    tft.setCursor(102, 35);
    tft.print(F("P LT: "));
    tft.setCursor(102, 60);
    tft.print(F("V1 LT: "));
    tft.setCursor(102, 85);
    tft.print(F("V2 LT: "));
    tft.setCursor(102, 110);
    tft.print(F("V3 HT: "));

    
    if(p1.getState() == OFF) {
      tft.fillCircle(150, 12, 5, ST77XX_BLUE);
    } else {
      tft.fillCircle(150, 12, 5, ST77XX_GREEN);
    }
    if(p2.getState() == OFF) {
      tft.fillCircle(150, 12+(25*1), 5, ST77XX_BLUE);
    } else {
      tft.fillCircle(150, 12+(25*1), 5, ST77XX_GREEN);
    }
    if(v1.getState() == OFF) {
      tft.fillCircle(150, 12+(25*2), 5, ST77XX_BLUE);
    } else {
      tft.fillCircle(150, 12+(25*2), 5, ST77XX_GREEN);
    }
    if(v2.getState() == OFF) {
      tft.fillCircle(150, 12+(25*3), 5, ST77XX_BLUE);
    } else {
      tft.fillCircle(150, 12+(25*3), 5, ST77XX_GREEN);
    }
    if(v3.getState() == OFF) {
      tft.fillCircle(150, 12+(25*4), 5, ST77XX_BLUE);
    } else {
      tft.fillCircle(150, 12+(25*4), 5, ST77XX_GREEN);
    }
   
    
  } else if(menu == 1) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setTextSize(1);
    tft.setCursor(30, 20);
    tft.print(F("Room Temp limit:"));
    
    char buffer_temp[6];
    char str_temp[6];
    dtostrf(RT_MAX, 3, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    
    tft.setTextSize(3);
    tft.setCursor(30, 55);
    tft.print(buffer_temp);

    tft.setTextSize(1);
    tft.setCursor(10, 115);
    tft.print(F("-1"));
    
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setCursor(75, 115);
    tft.print(F("OK"));
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setCursor(125, 115);
    tft.print(F("+1"));
    
  } else if(menu == 2) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setTextSize(1);
    tft.setCursor(30, 20);
    tft.print(F("HT storage limit:"));
    
    char buffer_temp[6];
    char str_temp[6];
    dtostrf(T2_MAX, 3, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    
    tft.setTextSize(3);
    tft.setCursor(30, 55);
    tft.print(buffer_temp);

    tft.setTextSize(1);
    tft.setCursor(10, 115);
    tft.print(F("-1"));
    
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setCursor(75, 115);
    tft.print(F("OK"));
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setCursor(125, 115);
    tft.print(F("+1"));
    
  } else if(menu == 3) {
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setTextSize(1);
    tft.setCursor(30, 20);
    tft.print(F("LT storage limit:"));
    
    char buffer_temp[6];
    char str_temp[6];
    dtostrf(T3_MAX, 3, 1, str_temp);
    sprintf(buffer_temp,"%s\367C", str_temp);
    
    tft.setTextSize(3);
    tft.setCursor(30, 55);
    tft.print(buffer_temp);

    tft.setTextSize(1);
    tft.setCursor(10, 115);
    tft.print(F("-1"));
    
    tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
    tft.setCursor(75, 115);
    tft.print(F("OK"));
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    
    tft.setCursor(125, 115);
    tft.print(F("+1"));
  }
  
}

bool updateTemp(void*) {
  
  sensors.requestTemperatures();

  for(int i = 0; i < (sizeof(sensorsIds) / sizeof(sensorsIds[0])); i++) {
    
    float temp = sensors.getTempC(sensorsIds[i]);
    sensorsTemperature[i] = temp;
    
    Serial.print(i);
    Serial.print("=");
    Serial.print(temp);
    Serial.print(" ");

    if(temp == DEVICE_DISCONNECTED_C) {
      
       Serial.print(F("Error: Could not read temperature data for sensor "));
       printAddress(sensorsIds[i]);
       continue;
    }
  }
  Serial.println();
}

/**
 * Function used by the logicalTimer every x ms
 * updating all the relays according to the locally stored temperatures
 */
void updateLogic() {

  // be aware that this may has to be changed because indexes of this array 
  // may not be associated to the real T(X) temperature sensor.
  float t1 = sensorsTemperature[0];
  float t2 = sensorsTemperature[1];
  float t3 = sensorsTemperature[2];
  float t4 = sensorsTemperature[3];
  float t5 = sensorsTemperature[4];
  float rt = sensorsTemperature[5];

  if(  t1 == DEVICE_DISCONNECTED_C || t2 == DEVICE_DISCONNECTED_C 
    || t3 == DEVICE_DISCONNECTED_C || t4 == DEVICE_DISCONNECTED_C 
    || t5 == DEVICE_DISCONNECTED_C ) {
    p1.turnOff();
    p2.turnOff();
    v1.turnOff();
    v2.turnOff();
    v3.turnOff();
    Serial.println(F("One of the temperature sensor is not working, closing all relays and stopping program"));
    while(1);
  }

  logicHT();
  logicLT();
}

void logicHT() {

  float t1 = sensorsTemperature[0];
  float t2 = sensorsTemperature[1];
  float t3 = sensorsTemperature[2];
  float rt = sensorsTemperature[5];
  
  if(t2 > T2_MAX && t3 > T3_MAX) { 
    
    p1.turnOff();
    v3.turnOff();
    return;
    
  }
  
  if(t1 > t2 && t2 < T2_MAX) {
    
    v3.turnOff();    // default state is HT SOLAR to HT STORAGE
    p1.turnOn();     // activates pump
    return;
    
  }

  if(t1 > t3 && t3 < T3_MAX) {
    
    p1.turnOn();      // activates pump
    v3.turnOn();      // activated state where HT SOLAR to LT STORAGE
    return;
    
  }
}

void logicLT() {
  
  float t3 = sensorsTemperature[2];
  float t4 = sensorsTemperature[3];
  float t5 = sensorsTemperature[4];
  float rt = sensorsTemperature[5];

  if(t4 > t5 && rt < RT_MAX) { 

    v1.turnOff();
    v2.turnOn();
    p2.turnOn();
    return;
  }
  
  if(t3 < T3_MAX && t4 > t3 && t4 < 100 /* T4_MAX */) { 

    v1.turnOn();
    v2.turnOn();
    p2.turnOn();
    return;
  }

  if(t3 > t5 && rt < RT_MAX) { 

    v1.turnOff();
    v2.turnOff();
    p2.turnOn();
    return;
  }
   
  v1.turnOff();
  v2.turnOff();
  p2.turnOff();
  
}

/**
 * function to print a device address
 */
void printAddress(const uint8_t* deviceAddress) {
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) 
      Serial.print("0");
    Serial.print(deviceAddress[i]);
    Serial.print(",");
  
  }
}

void centeredString(const char *buf, int x, int y)
{
    int16_t x1, y1;
    uint16_t w, h;
    tft.getTextBounds(buf, x, y, &x1, &y1, &w, &h); //calc width of new string
    tft.setCursor(x - w / 2, y);
    tft.print(buf);
}
