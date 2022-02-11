/*
 * Using Arduino UNO
 * Pins Connections:
 * D0  Motor1_dir1
 * D1  Motor1_dir2
 * D2  Motor2_dir1
 * D3  Motor2_PWM
 * D4  Motor2_dir2
 * D5  Motor1_PWM
 * D6  Camer_servo signal
 * D7  CE   -> nRF
 * D8  CSN  -> nRF
 * D9  PUMP_PWM
 * D10 Nozzle servo
 * D11 MOSI -> nRF   // Reserved
 * D12 MISO -> nRF   // Reserved
 * D13 SCK  -> nRF   // Reserved
 * A0  ECHO_ultrasonic
 * A1  Trig_Ultrasonic
 * A2  LM35 Temp in
 * A3  PUMP_ON
 * A4  Unused
 * A5  Unused
 */
 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal.h>
#include <Servo.h>

// Motors Pins
#define M1_Dir2 0
#define M1_Dir1 1
#define M2_Dir1 2
#define M2_PWM  3
#define M2_Dir2 4
#define M1_PWM  5

#define CamServoPin 6

#define CE  7
#define CSN 8

#define PUMP_PWM 9
#define NozzleServoPin 10

#define ECHO_PIN A0
#define TRIG_PIN A1

#define TEMP_PIN A2

#define PUMP_ON_PIN A3

// nRF Communication data rate
#define DATARATE RF24_2MBPS

#define JOYSTICK_TOLERANCE 20

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
   * - Onboard Temperature frol LM35 Sensor
   */
  float temp = 0.0;
  float distance = 0.0;
};
Data_Package dataIn;

RF24 radio(CE, CSN);
const byte channel[][6] = {"MTE01", "MTE02"};

Servo camServo;
Servo nozzleServo;

// Ultrasonic variables
volatile uint32_t _start = 0, _end = 0;
bool gotDistance = false;

// Motors variables
int16_t R_motor_PWM, L_motor_PWM;

void setup() {
  // Set PORTB as Output
  DDRB =  0xFF;
  // Set PORTB pins to LOW
  PORTB = 0x00;

  // Set PORTD as Output
  DDRD =  0xFF;
  // Set PORTD pins to LOW
  PORTD = 0x00;

  // Set PORTC pins direction
  DDRC = 0b00000101;    // Set pins A0 and A2 as Input
  // Set PORTC pins to LOW
  PORTC = 0x00;

  // For Non-blocking code flow, pulseIn() will not be used
  configUltrasonic();


  // Configure Radio Communication
  radio.begin();
  radio.setChannel(125);
  radio.setPALevel(RF24_PA_MAX);
  radio.setRetries(4, 15);
  radio.setAutoAck(true);
  radio.enableDynamicPayloads();
  radio.enableAckPayload();
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1, addresses[0]);


  camServo.attach(CamServoPin);
  nozzleServo.attach(NozzleServoPin);
}


void loop() {
  /******   Collect Data   ******/
  for (int i = 0; i < 10; i ++){
	// Get average value of 10 readings
    dataOut.temp += analogRead(TEMP_PIN) * (500.0 / 1023.0);
  }
  
   // assign the distance value to the distance field in the dataOut struct
  runUltrasonic();

  /******   Transmit the Control Data   ******/
  delay(25);
  radio.stopListening();  // ready for transmission
  radio.write(&dataOut, sizeof(Data_Package));

  /******   Recieve Control Data from remote   ******/
  radio.startListening();
  // read from radio
  if (radio.available()) {
    radio.read(&dataIn, sizeof(Control_Package));

    // Map motors data accordingly
    _move_();

    // Move camera servo and nozzle servo relatively
    camServo.write(map(dataIn.R_PotX, 0, 1023, 1, 75));
    nozzleServo.write(map(dataIn.R_PotX, 0, 1023, 1, 100));

    // Control Pump
    if (!dataIn.tSwitch1 && !dataIn.tSwitch2){
      digitalWrite(PUMP_EN, HIGH);
      digitalWrite(PUMP_PWM, HIGH);  
    } else {
      digitalWrite(PUMP_EN, LOW);
      digitalWrite(PUMP_PWM, LOW);  
    }
  }
}


void configUltrasonic() {
  /*  
   *   To reach non-blocking code flow, pin change interrupt
   *   is used
   */
  PCICR  |= 0b00000010;   // Enable pin change interrupt on PORTC
  PCMSK1 = 0b00000001;    // Mask pins to trigger interrupt (A0 pin)
  sei();				  // Enable Golbal interrupt	
  start();
}

void runUltrasonic() {
  if (gotDistance) {
    // measure distance
    dataOut.distance = (_end - _start) / 57;
    delay(15);
    // get travel time again
    start();
  }
}

ISR(PCINT1_vect) {
  // Interrupt service routine
  switch(digitalRead(ECHO_PIN)){
    case HIGH:
      _start = micros();
      break;
    case LOW:
      _end = micros();
      gotDistance = true;
      break;
  }
}

void start() {
  gotDistance = false;
  // Trigger the pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);  
}


void _move_(){
  if (dataIn.L_PotX > MAX_TOL || dataIn.L_PotY > MAX_TOL 
      || dataIn.L_PotX < MIN_TOL || dataIn.L_PotY < MIN_TOL){

    /* Outside the deadzone */
    if (dataIn.L_PotX > MIN_TOL && dataIn.L_PotY > MAX_TOL 
        && dataIn.L_PotX < MAX_TOL){
      /* Forward */
      
      // set direction pins to Forward
      digitalWrite(M1_Dir1, HIGH);
      digitalWrite(M1_Dir2, LOW);
      digitalWrite(M2_Dir1, HIGH);
      digitalWrite(M2_Dir2, LOW);
      
      // both motors have speed Y
      R_motor_PWM = map(dataIn.L_PotY, MAX_TOL, 1023, 10, 255);
      L_motor_PWM = R_motor_PWM;
      analogWrite(M1_PWM, L_motor_PWM);
      analogWrite(M2_PWM, R_motor_PWM);
      
    } else if (dataIn.L_PotX > MIN_TOL && dataIn.L_PotY < MIN_TOL 
               && dataIn.L_PotX < MAX_TOL) {
      /* Backward */
      
      // set direction pins to Backward
      digitalWrite(M1_Dir1, LOW);
      digitalWrite(M1_Dir2, HIGH);
      digitalWrite(M2_Dir1, LOW);
      digitalWrite(M2_Dir2, HIGH);
      
      // both motors have speed Y (inverse)
      R_motor_PWM = map(dataIn.L_PotY, MIN_TOL, 0, 10, 255);
      L_motor_PWM = R_motor_PWM;
      analogWrite(M2_PWM, R_motor_PWM);
      analogWrite(M1_PWM, L_motor_PWM);
      
    } 
    else if (dataIn.L_PotY > MIN_TOL && dataIn.L_PotY < MAX_TOL 
               && dataIn.L_PotX < MIN_TOL) {
      /* Left */
      // left motor off
      digitalWrite(M1_Dir1, LOW);
      digitalWrite(M1_Dir2, LOW);
      analogWrite(M1_PWM, 0);
      // right motor forward with speed X (inverse)
      digitalWrite(M2_Dir1, HIGH);
      digitalWrite(M2_Dir2, LOW);
      R_motor_PWM = map(dataIn.L_PotX, MIN_TOL, 0, 10, 255);
      analogWrite(M2_PWM, R_motor_PWM);
    } else if (dataIn.L_PotY > MIN_TOL && dataIn.L_PotY < MAX_TOL 
               && dataIn.L_PotX > MAX_TOL) {
      /* Right */
      // right motor off
      digitalWrite(M2_Dir1, LOW);
      digitalWrite(M2_Dir2, LOW);
      analogWrite(M2_PWM, 0);
      
      // left motor Forward with speed x
      L_motor_PWM = map(dataIn.L_PotX, MAX_TOL, 1023, 10, 255);
      analogWrite(M1_PWM, L_motor_PWM);
    } else if (dataIn.L_PotY > MAX_TOL && dataIn.L_PotX > MAX_TOL) {
      /* Right forward */
      // Set both motors forward
      digitalWrite(M1_Dir1, HIGH);
      digitalWrite(M1_Dir2, LOW);
      digitalWrite(M2_Dir1, HIGH);
      digitalWrite(M2_Dir2, LOW);
      
      // Righ motor speed Y - X
      R_motor_PWM = map(max((dataIn.L_PotY - dataIn.L_PotX), 0), 0, 1023, 0, 255);
      analogWrite(M2_PWM, R_motor_PWM);
      
      // Left motor speed Y + X
      L_motor_PWM = map(min((dataIn.L_PotY + dataIn.L_PotX), 1023), 0, 1023, 0, 255);
      analogWrite(M1_PWM, L_motor_PWM);
      
    } else if (dataIn.L_PotY > MAX_TOL && dataIn.L_PotX < MIN_TOL){
      /* Left Forward */
      
      // Set both motors forward
      digitalWrite(M1_Dir1, HIGH);
      digitalWrite(M1_Dir2, LOW);
      digitalWrite(M2_Dir1, HIGH);
      digitalWrite(M2_Dir2, LOW);
      
      // Righ motor speed Y + X
      R_motor_PWM = map(dataIn.L_PotX, MIN_TOL, 0, MAX_TOL, 1023);
      R_motor_PWM = map(min((dataIn.L_PotY + R_motor_PWM), 1023), 0, 1023, 0, 255);
      analogWrite(M2_PWM, R_motor_PWM);
      
      // Left motor speed Y - X
      L_motor_PWM = map(dataIn.L_PotX, MIN_TOL, 0, MAX_TOL, 1023);
      L_motor_PWM = map(max((dataIn.L_PotY - L_motor_PWM), 0), 0, 1023, 0, 255);
      analogWrite(M1_PWM, L_motor_PWM);
    } else if (dataIn.L_PotY < MIN_TOL && dataIn.L_PotX > MAX_TOL) {
      /* Right backward */
      // Set both motors backward
      digitalWrite(M1_Dir1, LOW);
      digitalWrite(M1_Dir2, HIGH);
      digitalWrite(M2_Dir1, LOW);
      digitalWrite(M2_Dir2, HIGH);
      
      // Righ motor speed Y - X
      R_motor_PWM = map(dataIn.L_PotY, MIN_TOL, 0, MAX_TOL, 1023);
      R_motor_PWM = map(max((R_motor_PWM - dataIn.L_PotX), 0), 0, 1023, 0, 255);
      analogWrite(M2_PWM, R_motor_PWM);
      
      // Left motor speed Y + X
      L_motor_PWM = map(dataIn.L_PotY, MIN_TOL, 0, MAX_TOL, 1023);
      L_motor_PWM = map(min((L_motor_PWM + dataIn.L_PotX), 1023), 0, 1023, 0, 255);
      analogWrite(M1_PWM, L_motor_PWM);
      
    } else if (dataIn.L_PotY < MIN_TOL && dataIn.L_PotX < MIN_TOL) {
      /* Left backward */
      // Set both motors backward
      digitalWrite(M1_Dir1, LOW);
      digitalWrite(M1_Dir2, HIGH);
      digitalWrite(M2_Dir1, LOW);
      digitalWrite(M2_Dir2, HIGH);
      
      dataIn.L_PotY = map(dataIn.L_PotY, MIN_TOL, 0, MAX_TOL, 1023);
      dataIn.L_PotX = map(dataIn.L_PotX, MIN_TOL, 0, MAX_TOL, 1023);
      
      // Righ motor speed Y + X
      R_motor_PWM = map(min((dataIn.L_PotY + dataIn.L_PotX), 1023), 0, 1023, 0, 255);
      analogWrite(M2_PWM, R_motor_PWM);
      
      // Left motor speed Y - X
      L_motor_PWM = map(max((dataIn.L_PotY - dataIn.L_PotX), 0), 0, 1023, 0, 255);
      analogWrite(M1_PWM, L_motor_PWM);
    }
  }else {
    // Inside deadzone
    // stop
    digitalWrite(M1_Dir1, LOW);
    digitalWrite(M1_Dir2, LOW);
    digitalWrite(M2_Dir1, LOW);
    digitalWrite(M2_Dir2, LOW);
    R_motor_PWM = 0;
    L_motor_PWM = 0;
  }
}