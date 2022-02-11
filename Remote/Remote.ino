/*
 * Using Arduino NANO
 * Pins Connections:
 * D0 Not Connected
 * D1  LED
 * D2  Not Connected
 * D3  LCD_D7
 * D4  LCD_D6
 * D5  LCD_D5
 * D6  LCD_D4
 * D7  EN
 * D8  RS
 * D9  CE   -> nRF
 * D10 CSN  -> nRF
 * D11 MOSI -> nRF   // Reserved
 * D12 MISO -> nRF   // Reserved
 * D13 SCK  -> nRF   // Reserved
 * A0  Left Joystick btn
 * A1  Left joystick Y
 * A2  Left joystick X
 * A3  Left Toggle switch
 * A4  Right Toggle switch
 * A5  Right Joystick btn
 * A6  Right joystick Y
 * A7  Right joystick X
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>

#define WARNING_LED 1   // LED for warnings

/* LCD PINS */
#define RS 3
#define EN 4
#define D4 5
#define D5 6
#define D6 7
#define D7 8

/* nRF PINS */
#define CE 9
#define CSN 10

/* Left Joystick pins */
#define Ljoy_btn_PIN    A0
#define Ljoy_Y_PIN      A1
#define Ljoy_X_PIN      A2

/* Toggle Switches*/
#define toggSwitch1_PIN A3
#define toggSwitch2_PIN A4

/* Right Joystick pins */
#define Rjoy_btn_PIN    A5
#define Rjoy_Y_PIN      A6
#define Rjoy_X_PIN      A7


/*  Macros  */
#define MIN_DIST 10
#define MAX_TEMP 50

/****  instantiating objects  ****/
#define DATARATE RF24_2MBPS
RF24 radio(CE, CSN); // CE, CSN
const byte channel[][6] = {"MTE01", "MTE02"};


LiquidCrystal lcd(RS, EN, D4, D5, D6, D7);
///////////////////////////////////

/*******    Initializatons & Declerations      *******/
struct Control_Package {
  /* 
   *  Data  to be sent to the robot
   *  to control motors / pump / servos
  */
  int16_t R_PotX;
  int16_t R_PotY;
  byte R_Button;
  int16_t L_PotX;
  int16_t L_PotY;
  byte L_Button;
  byte tSwitch1;
  byte tSwitch2;
};
Control_Package dataOut;

 
struct Data_Package {
  /*
   * Data to be recieved from the robot
   * - Distance from ultrasonic sensor
   * - Onboard Temperature from LM35 Sensor
   */
  float temp = 0.0;
  int distance = 0;
};
Data_Package dataIn;

long _time = millis();

void setup() {
  DDRD  = 0xFF;           // Set PORTD Pins as Output
  PORTD = 0;              // Set PORTD Pins as LOW

  DDRB |= 1;              // Set Pin 0 in PORTB as Output

  DDRC = 0x00;            // Set PORTC Pins as INPUT
  PORTC |= 0b00100001;    // Activate Pullup resistor on pins 0 and 5 in PORTC
  
  
  lcd.begin(16, 2);       // Operate LCD

  // Configure nRF Radio Communication
  radio.begin();
  radio.setChannel(125);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(4, 15);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(channel[0]);
  radio.openReadingPipe(1, channel[1]);
}


void loop() {
  /******   Recieve Data from robot   ******/
  radio.startListening(); // ready for recieving data
  
  /******   Display data on LCD   ******/
  if (radio.available()) {
    while (radio.available()) {
      // read from radio
      radio.read(&dataIn, sizeof(Data_Package));

      if (millis() - _time >= 1000){
        LCD.setCursor(0, 0);
        LCD.print("Temp: ");
        LCD.print(dataIn.temp);
        LCD.println(" C         ");
        LCD.setCursor(0, 1);
        LCD.print("Dist: ");
        LCD.print(dataIn.distance);
        LCD.println(" cm        ");
        _time = millis();
      }
      
      if (dataIn.distance < 10){
        digitalWrite(WARNING_LED, HIGH);
      } else {
        digitalWrite(WARNING_LED, LOW);
      }
    }
  }
  
  delay(25);
  
  /******   Collect Control Data   ******/
  dataOut.R_PotX = analogRead(Rjoy_X_PIN);
  dataOut.R_PotY = map(analogRead(Rjoy_Y_PIN), 0, 1023, 1023, 0);
  dataOut.R_Button = digitalRead(Rjoy_btn_PIN);
  dataOut.L_PotX = map(analogRead(Ljoy_X_PIN), 0, 1023, 1023, 0);
  dataOut.L_PotY = analogRead(Ljoy_Y_PIN);
  dataOut.L_Button = digitalRead(Ljoy_btn_PIN);
  dataOut.tSwitch1 = digitalRead(toggSwitch1_PIN);
  dataOut.tSwitch2 = digitalRead(toggSwitch2_PIN);
  
  /******   Transmit the Control Data   ******/
  radio.stopListening();  // ready for transmission
  // sending the data to the other controller
  radio.write(&dataOut, sizeof(Control_Package));
}
