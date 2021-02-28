// === Alexander Morgan ====
// Final project, Option #1&2
//==============Cayenne===============
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP32.h>
int ONOFF ;
// WiFi network info.

char *ssid = "YOUR SSID";
char *wifiPassword = "YOUR WIFI PASSWORD";

// Cayenne authentication info. This should be obtained from the Cayenne Dashboard. 
//Replace with your MQTT  USERNAME, PASSWORD, and CLIENT_ID
char username[] = "CAYENNE USERNAME";  
char password[] = "CAYENNE PASSWORD";   
char clientID[] = "CAYENNE CLIENTID";   

//==================== LCD ====================
#include <Wire.h>  //lcd
#include <LiquidCrystal_I2C.h> //lcd
LiquidCrystal_I2C lcd(0x27,16,2); //set the LCD address to 0x3F for a 16 chars and 2-line display
// if it does not work then try 0x3F, if both addresses do not work then run the scan code below

const int bzr=32;      // GPIO32 to connect the Buzzer
const int PIR = 5; //Sensor for side street
int PirValue;

const int red_LED1  = 14;   // The red LED1 is wired to ESP32 board pin GPIO14
const int yellow_LED1  =12;   // The yellow LED1 is wired to ESP32 board pin GPIO12
const int green_LED1 = 13; // The green LED1 is wired to ESP32 board pin GPIO13
const int Xw_LED1 = 23;

const int red_LED2  = 27;   // The red LED2 is wired to Mega board pin GPIO25
const int yellow_LED2  = 26;   // The yellow LED2 is wired to Mega board pin GPIO 26
const int green_LED2 = 25; // The green LED2 is wired to Mega board pin GPIO 27
const int Xw_LED2 = 4;

int Xw_value1; //value to hold cross walk values
const int Xw_button1 = 18; //Cross Walk button 1
int Xw_value2;
const int Xw_button2 = 19; //Cross walk button 2

TaskHandle_t Task1; //Task handles to handle tasks on seperate cores
TaskHandle_t Task2;

void setup() {
  Serial.begin(115200);
  pinMode(Xw_button1, INPUT_PULLUP); // 0=pressed, 1 = unpressed button
  pinMode(Xw_button2, INPUT_PULLUP);
  // This is here to force the ESP32 to reset the WiFi and initialize correctly.
  
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());

  WiFi.disconnect(true);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(1000);
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode());
  // End silly stuff !!! =======================

  // Connect to provided SSID and PASSWORD
  WiFi.begin(ssid, password);
  //=============== Setting ESP32 for Cayenne ==============
  Cayenne.begin(username, password, clientID, ssid, wifiPassword);
  
  pinMode(PIR, INPUT); //0=No Activity, 1=ActivityD

  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("= Alex Morgan =");
  Serial.println("= Alex Morgan =");
  delay(2500);
  pinMode(bzr,OUTPUT);
  
  pinMode(red_LED1, OUTPUT);  // initialize digital pin 14 (Red LED1) as an output.
  pinMode(yellow_LED1, OUTPUT);  // initialize digital pin12 (yellow LED1) as an output.
  pinMode(green_LED1, OUTPUT);    // initialize digital pin 13 (green LED1) as an output.
  pinMode(Xw_LED1, OUTPUT);

  pinMode(red_LED2, OUTPUT);  // initialize digital pin 25(Red LED2) as an output.
  pinMode(yellow_LED2, OUTPUT); // initialize digital pin 26 (yellow LED2) as an output.
  pinMode(green_LED2, OUTPUT); // initialize digital pin 27 (green LED2) as an output.
  pinMode(Xw_LED2, OUTPUT);
  digitalWrite(Xw_LED1, LOW);
  digitalWrite(Xw_LED2, LOW);
  Xw_value1 = 1;
  Xw_value2 = 1;

  //create a task that will be executed in the XwTask() function, 
  //with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    XwTask,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 

  //create a task that will be executed in the LED_LCD_task() function, 
  //with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    LED_LCD_task,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
  delay(500);
}

void loop() {
}

void XwTask(void * pvParameters) { //core 0 task for taking data from buttons and Cayenne site
    Serial.print("XwTask running on core ");
    Serial.println(xPortGetCoreID());
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    while (true) {
        Cayenne.loop(); // Calling Cayenne =========
        Serial.print("ONOFF = ");
        Serial.println(ONOFF);
        Xw_value1 = digitalRead(Xw_button1);
        Xw_value2 = digitalRead(Xw_button2);
        Serial.print("PIR = ");
        Serial.println(digitalRead(PIR));
        if (Xw_value1 <= 0) { //these statements allow button2 to be press while button1/Xw1 is in action
            while (Xw_value1 <= 0) {
                Xw_value1 -= 1;
                Xw_value2 = digitalRead(Xw_button2);
                Cayenne.loop();
                if ( ONOFF == 1) { //** I've place these if statements all over to interrupt every action if 
                                   //**Emergency button is triggered
                    break;
                }
                if (Xw_value2 <= 0) {
                    while (Xw_value2 <= 0) {
                        Xw_value2 -= 1;
                        Cayenne.loop();
                        if ( ONOFF == 1) { //**
                            break;
                        }
                        delay(50);
                    }
                }
                delay(50);
            }
        }
        if (Xw_value2 <= 0) { //these statements allow button1 to be press while button2/Xw2 is in action
            while (Xw_value2 <= 0) {
                Xw_value2 -= 1;
                Xw_value1 = digitalRead(Xw_button1);
                Cayenne.loop();
                if ( ONOFF == 1) { //**
                    break;
                }
                if (Xw_value1 <= 0) {
                    while (Xw_value1 <= 0) {
                        Xw_value1 -= 1;
                        Cayenne.loop();
                        if ( ONOFF == 1) { //**
                            break;
                        }
                        delay(50);
                    }
                }
                delay(50);
            }
        }
        delay(50);
    }
}

void LED_LCD_task(void * pvParameters) { //Core 1 task for changing lights depending on action
    Serial.print("LED_LCD_task running on core ");
    Serial.println(xPortGetCoreID());
    Serial.println(uxTaskGetStackHighWaterMark(NULL));
    LcdNoWalk();
    while (true) {
        digitalWrite(green_LED1, HIGH);
        digitalWrite(yellow_LED1, LOW);
        digitalWrite(red_LED1, LOW);
        digitalWrite(green_LED2, LOW);
        digitalWrite(yellow_LED2, LOW);
        digitalWrite(red_LED2, HIGH);
        PirValue = digitalRead(PIR);
        if (ONOFF == 1) {
          delay(250);
          Emergency();
        }
        else if (Xw_value1 <= 0) {
            LcdWalk1();
        }
        else if (Xw_value2 <= 0) {
            delay(2000);
            LcdWalk2();
        }
        else if (PirValue == 1) {
            delay(2000);
            LED2();
        }
        delay(100);
    }
}

void LcdWalk1() { //allows Xw1 to walk along with street 1(main street), counts down walk time
    for (int i=10; i>= 0; i--){
        if ( ONOFF == 1) { //**
          break;
        }
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  == Walk1 ==  ");
        Serial.print("== Walk1 == ");
        Serial.println(i);
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(i);
        lcd.print("       ");
        digitalWrite(Xw_LED1, HIGH);
        digitalWrite(bzr, HIGH);
        delay(500);
        if ( ONOFF == 1) { //**
          break;
        }
        digitalWrite(Xw_LED1, LOW);
        digitalWrite(bzr, LOW);
        delay(500);
    }
    Xw_value1 = 1;
    LcdNoWalk();
    digitalWrite(Xw_LED1, LOW);
    digitalWrite(bzr, LOW);
}

void LcdWalk2() { //allows Xw2 to walk along with street 2(side street), counts down walk time
    digitalWrite(green_LED1, LOW);
    digitalWrite(yellow_LED1, HIGH);
    digitalWrite(red_LED1, LOW);
    delay(1000);
    digitalWrite(green_LED1, LOW);
    digitalWrite(yellow_LED1, LOW);
    digitalWrite(red_LED1, HIGH);
    delay(1000);
    for (int i=10; i>= 0; i--){ //walk counter
        if ( ONOFF == 1) { //**
          break;
        }
        digitalWrite(green_LED2, HIGH);
        digitalWrite(yellow_LED2, LOW);
        digitalWrite(red_LED2, LOW);
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("  == Walk2 ==  ");
        Serial.print("== Walk2 == ");
        Serial.println(i);
        lcd.setCursor(0, 1);
        lcd.print("      ");
        lcd.print(i);
        lcd.print("       ");
        digitalWrite(Xw_LED2, HIGH);
        digitalWrite(bzr, HIGH);
        delay(500);
        if ( ONOFF == 1) { //**
          break;
        }
        digitalWrite(Xw_LED2, LOW);
        digitalWrite(bzr, LOW);
        delay(500);
    }
    Xw_value2 = 1;
    LcdNoWalk();
    digitalWrite(Xw_LED2, LOW);
    digitalWrite(bzr, LOW);
    digitalWrite(green_LED2, LOW);
    digitalWrite(yellow_LED2, HIGH);
    digitalWrite(red_LED2, LOW);
    delay(1000);
    digitalWrite(green_LED2, LOW);
    digitalWrite(yellow_LED2, LOW);
    digitalWrite(red_LED2, HIGH);
    delay(1000);
}

void LcdNoWalk() { //function to stop cross walk
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("= Do Not Walk =");
    Serial.println("= Do Not Walk =");
}

void LED2() { //function to change LED1s(street 1) to red and LED2s(side street) to green, then back to red
  for (int i=1; i>=1; i--){ //this for statement only runs once
                            //but I need a for loop in case emergency button is pressed
    if ( ONOFF == 1) { //**
      break;
    }
    digitalWrite(green_LED1, LOW);
    digitalWrite(yellow_LED1 , HIGH);
    digitalWrite(red_LED1, LOW);
    delay(1000);
    if ( ONOFF == 1) { //**
      break;
    }
    digitalWrite(green_LED1, LOW);
    digitalWrite(yellow_LED1 , LOW);
    digitalWrite(red_LED1, HIGH);
    delay(1000);
    if ( ONOFF == 1) { //**
      break;
    }
    digitalWrite(green_LED2, HIGH);
    digitalWrite(yellow_LED2 , LOW);
    digitalWrite(red_LED2, LOW);
    delay(2500);
    if ( ONOFF == 1) { //**
      break;
    }
    delay(2500);
  }
  digitalWrite(green_LED1, LOW);
  digitalWrite(yellow_LED1, LOW);
  digitalWrite(red_LED1, HIGH);
  digitalWrite(green_LED2, LOW);
  digitalWrite(yellow_LED2 , HIGH);
  digitalWrite(red_LED2, LOW);
  delay(1000);
  digitalWrite(green_LED2, LOW);
  digitalWrite(yellow_LED2 , LOW);
  digitalWrite(red_LED2, HIGH);
  delay(1000);
  PirValue = 0;
}

void Emergency() { //stops all traffic, then flashes blue and red lights for emergency vehicles
                   //as long as the emergency button is pressed
  LcdNoWalk();
  Serial.println("= Emergency =");
  digitalWrite(green_LED1, LOW);
  digitalWrite(yellow_LED1, HIGH);
  digitalWrite(red_LED1,LOW);
  delay(1000);
  digitalWrite(green_LED1, LOW);
  digitalWrite(yellow_LED1, LOW);
  digitalWrite(red_LED1,HIGH);
  delay(1000);
  while ( ONOFF == 1) {
    digitalWrite(Xw_LED1, LOW);
    digitalWrite(Xw_LED2, HIGH);
    digitalWrite(red_LED1, HIGH);
    digitalWrite(red_LED2, LOW);
    delay(500);
    digitalWrite(Xw_LED1, HIGH);
    digitalWrite(Xw_LED2, LOW);
    digitalWrite(red_LED1, LOW);
    digitalWrite(red_LED2, HIGH);
    delay(500);
  }
  digitalWrite(Xw_LED1, LOW);
  digitalWrite(Xw_LED2, LOW);
  digitalWrite(red_LED1, LOW);
  digitalWrite(red_LED2, LOW);
}

CAYENNE_IN(1) {
  // Using channel (1) (you can use any other channel but it must match the widget 
  //you will be creating for it. 
  //As shown in the example below for channel (1) widget
  ONOFF = getValue.asInt();
}
