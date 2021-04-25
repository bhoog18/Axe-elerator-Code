#include <Wire.h>
#include <LiquidCrystal.h>

#define pwm 11
#define dir 12
#define pwm2 44
#define dir2 35
#define encoderB 1
#define encoderA 2
#define ENC_COUNT_REV 374
#define seatMotor 3
#define armStopSensor 20
#define startButton 22
#define eStopButton 19
#define seq2Button 21
#define seatMotor2 46
#define floorMotor1a 31
#define floorMotor1b 33
#define motor1enable1 45
#define seatInput1a 52
#define seatInput1b 53
#define seatInput2a 38
#define seatInput2b 39

#define floorMotor2a 49
#define floorMotor2b 50
#define motor2enable1 13

#define armStopSensor2 16

#define DIR_CW HIGH
#define DIR_CCW LOW

#define TOTAL_CYCLES 8
#define PULSE_WIDTH 50 

LiquidCrystal lcd(8, 9, 4, 5, 6, 7); // select the pins used on the LCD panel

int speed = 100; 
int timeDelay = 200;
bool rotateSeats = true;

// Pulse count from encoder
volatile long encoder_value_A = 0;
volatile long encoder_value_B = 0;

// One-second interval for measurements
int interval = 1000;

// Counters for milliseconds during interval
long previousMillis = 0;
long currentMillis = 0;

// Variable for RPM measuerment
int rpm = 0;
 
// Variable for PWM motor speed output
int motorPwm = 0;

// track cycles
int cycle_count = 0;

int motorDirection = LOW;
int targetPulseCount = PULSE_WIDTH;
int rideStarted = 0;
int rideComplete = 0;
int rideDirection = 1;
int eStopped = 0;

void setup() {

  // put your setup code here, to run once:

  // Setup Serial Monitor
  Serial.begin(9600);

  // Set PWM and DIR connections as outputs
  pinMode(pwm,OUTPUT);
  pinMode(dir,OUTPUT);
  pinMode(pwm2,OUTPUT);
  pinMode(dir2,OUTPUT);
  pinMode(motor1enable1, OUTPUT);
  pinMode(floorMotor1a, OUTPUT);
  pinMode(floorMotor1b, OUTPUT);

  pinMode(seatMotor, OUTPUT);
  pinMode(seatMotor2, OUTPUT);

  // Set encoder as input with internal pullup  
  pinMode(encoderA, INPUT_PULLUP);
  pinMode(encoderB, INPUT_PULLUP);

    pinMode(startButton, INPUT_PULLUP);
    pinMode(eStopButton, INPUT_PULLUP);
    pinMode(seq2Button, INPUT_PULLUP);


 // Attach interrupts 
 attachInterrupt(digitalPinToInterrupt(encoderA), encoder_channel_a, RISING);
 //attachInterrupt(digitalPinToInterrupt(encoderB), encoder_channel_b, RISING);

  delay(1000);
  // Stop motor
  moveMotor(DIR_CCW, 0);
  Serial.print("Starting cycles...  ");
  Serial.println(TOTAL_CYCLES);
  moveFloorMotor(HIGH, 50);
  delay(1000);
  
 lcd.begin(16, 2); // start the library
 lcd.setCursor(0,0); // set the LCD cursor position
 lcd.print("Welcome to Ax"); // print a simple message on the LCD
 delay(2000);
  moveFloorMotor(HIGH, 0);
 lcd.setCursor(0,1);
 lcd.print("Are you ready?");
 delay(2000);
 lcd.setCursor(0,1);
 lcd.print("Starting in 3...");
 delay(1000);
 lcd.setCursor(0,1);
 lcd.print("Starting in 2...");
 delay(1000);
 lcd.setCursor(0,1);
 lcd.print("Starting in 1...");
 delay(1000);
 lcd.setCursor(0,0);
 lcd.print("Ax in Progress            ");
 lcd.setCursor(0,1);
 lcd.print("                      ");


 // Floor hobby motors
// void setspeed(int val) {//////////////////////////////////
  //analogWrite(motor1enable1, val);
 //}
// void setdir(bool dir) {
  //digitalWrite(floorMotor1a, dir);
  //digitalWrite(floorMotor1b, dir);/////////////////////////////
 //}
 
 // Setup initial values for timer
 previousMillis = millis();
 return;
 Wire.begin(20);
 Wire.onReceive(dataReceive);
}

void moveFloorMotor(bool mDir, int mSpeed) {
  analogWrite(motor1enable1, 0);
  analogWrite(motor2enable1, 0);
  delay(100);
  digitalWrite(floorMotor1a, mDir);
  digitalWrite(floorMotor1b, !mDir);  
  digitalWrite(floorMotor2a, mDir);
  digitalWrite(floorMotor2b, !mDir);  
  analogWrite(motor1enable1, mSpeed);
  analogWrite(motor2enable1, mSpeed);
}

void resetRide() {
  Serial.println("RESETTING RIDE");
  // Move motor at speed of 40 in CW direction
  moveMotor(DIR_CW, 40); 

  // Variable to track if sensor 1 is triggered
  bool sensorFound = false;

  // Variable to track if sensor 2 is triggered
  bool sensor2Found = false;

  bool sensorMalfunction = false;
  // This loop will run as long as sensor 1 or sensor 2 is not triggered
  // Once both sensors are triggered, the while loop will exit
  while ( (not sensorFound || not sensor2Found) and not sensorMalfunction)
  {
    // is sensor one triggered?
    // if so, set sensorFound variable to true
    if (digitalRead(armStopSensor) == LOW)
    {
      sensorFound = true;
      // Stops arm 1 motor by setting speed to 0
      moveSingleMotor(DIR_CW, 0, 1);
    }
    // is sensor two triggered?
    // if so, set sensor2FOund variable to true
    if (digitalRead(armStopSensor2) == LOW)
    {
      sensor2Found = true;
      // Stops arm 2 motor by setting speed to 0
      moveSingleMotor(DIR_CW, 0, 2);
    }
    if (encoder_value_A > 40) {
      sensorMalfunction = true;
      moveMotor(DIR_CW, 0);
      lcd.setCursor(0,1);
      lcd.print("SENSOR MALFUNCTION");
    }
    delay(10);
  }

/*  while (encoder_value_B > 0) {
    delay(1); 
  }*/

  moveFloorMotor(LOW, 50);
  delay(3000);
  moveFloorMotor(LOW, 0);
  // resets rideStarted varialbe to 0
  rideStarted = 0;
}

void moveMotor(int mDir, int mSpeed) {
  digitalWrite(dir,mDir);
  delay(1);
  digitalWrite(dir2,mDir);
  motorDirection = mDir;
  Serial.print("CHANGE DIR: ");
  Serial.println(mDir);
  analogWrite(pwm,mSpeed);  
  delay(1);
  analogWrite(pwm2,mSpeed);  
  Serial.print("CHANGE SPEED: ");
  Serial.println(mSpeed);
}

void moveSingleMotor(int mDir, int mSpeed, int mNum) {
  // if mNum is 1, we control motor arm 1; if mNum is 2, we control motor arm 2
  if (mNum == 1) {
    digitalWrite(dir,mDir);
    analogWrite(pwm,mSpeed);  
  }
  else if (mNum == 2) {
    digitalWrite(dir2,mDir);
    analogWrite(pwm2,mSpeed);  
  }
}

float get_rpm(int encValue) {
  return (float)(encValue * 60 / ENC_COUNT_REV);
}

void print_stats() {
  Serial.print("ENCODER A: ");
  Serial.println(encoder_value_A);
  //Serial.print("ENCODER A RPM: ");
  //Serial.println(get_rpm(encoder_value_A));
  Serial.print("ENCODER B: ");
  Serial.println(encoder_value_B);
  //Serial.print("ENCODER B RPM: ");
  //Serial.println(get_rpm(encoder_value_B));  
  Serial.print("TPC: ");
  Serial.println(targetPulseCount);
}

void swingArmDown() {
  

  print_stats();
  moveMotor(DIR_CW, speed);
  while (encoder_value_A < targetPulseCount) {
    delay(1);
  }
  
  moveMotor(DIR_CW, 0);
  delay(timeDelay);

  print_stats();
  moveMotor(DIR_CCW, speed);

  while (encoder_value_B < targetPulseCount) {
    delay(1);
  }

  moveMotor(DIR_CCW, 0);
  delay(timeDelay);

  cycle_count--;
  Serial.print("Cycle Count: ");
  Serial.println(cycle_count);
  lcd.setCursor(0,1);
  lcd.print("Cycle Count: ");
  lcd.setCursor(13,1);
  lcd.print(cycle_count);

  targetPulseCount -= PULSE_WIDTH;

  if (cycle_count == 2 and rotateSeats) {
    moveSeatMotor(LOW, 0);
  }  
}

void swingArmUp() {

  print_stats();
  moveMotor(DIR_CW, speed);
  while (encoder_value_A < targetPulseCount) {
    delay(1);
  }
  
  moveMotor(DIR_CW, 0);
  delay(timeDelay);

  print_stats();
  moveMotor(DIR_CCW, speed);

  while (encoder_value_B < targetPulseCount) {
    delay(1);
  }

  moveMotor(DIR_CCW, 0);
  delay(timeDelay);

  cycle_count++;
  Serial.print("Cycle Count: ");
  Serial.println(cycle_count);
  lcd.setCursor(0,1);
  lcd.print("Cycle Count: ");
  lcd.setCursor(13,1);
  lcd.print(cycle_count);

  targetPulseCount += PULSE_WIDTH;

  if (cycle_count == 2 and rotateSeats) {
    // Start seat motor
    moveSeatMotor(HIGH, 150);
  }

}

void moveSeatMotor(bool mDir, int mSpeed) {
  analogWrite(seatMotor, 0);
  analogWrite(seatMotor2, 0);
  delay(100);
  digitalWrite(seatInput1a, mDir);
  digitalWrite(seatInput1b, !mDir);  
  digitalWrite(seatInput2a, mDir);
  digitalWrite(seatInput2b, !mDir);  
  analogWrite(seatMotor, mSpeed);
  analogWrite(seatMotor2, mSpeed);
}


void moveSeatMotorOld(int mOn, int mSpeed) {
     digitalWrite(seatMotor, mOn);
     delay(100);
     digitalWrite(seatMotor2, mOn);
     delay(100);
     analogWrite(seatMotor, mSpeed);
     delay(100);
     analogWrite(seatMotor2, mSpeed);
}


void loop() {

  if (digitalRead(eStopButton) == LOW) {
    Serial.println("E STOPPING");
    eStopped = 1;
    delay(10);
  }
  if (eStopped and (rideComplete == 0)) {
    // Stop Arm
    moveMotor(DIR_CW, 20);
    // Stop SeatMotor
    moveSeatMotor(LOW, 0);
    delay(10);
    rideComplete = 1;
    resetRide();
    delay(1000);
    eStopped = 0;
    return;
  }
  
  if (rideComplete)
  {
    if (digitalRead(startButton) == LOW) {
      rotateSeats = true;
      cycle_count = 0;
      motorDirection = LOW;
      targetPulseCount = PULSE_WIDTH;
      rideStarted = 0;
      rideComplete = 0;
      rideDirection = 1;
      encoder_value_A = 0;
      encoder_value_B = 0;

      moveFloorMotor(HIGH, 50);
      delay(3000);
      moveFloorMotor(HIGH, 0);
    }

    if (digitalRead(seq2Button) == LOW) {
      rotateSeats = false;
      cycle_count = 0;
      motorDirection = LOW;
      targetPulseCount = PULSE_WIDTH;
      rideStarted = 0;
      rideComplete = 0;
      rideDirection = 1;
      encoder_value_A = 0;
      encoder_value_B = 0;

      moveFloorMotor(HIGH, 50);
      delay(3000);
      moveFloorMotor(HIGH, 0);
    }
    
    return;
  }

    if (rideDirection == 1) {
      rideStarted = 1;
      if (cycle_count >= TOTAL_CYCLES) {
        rideDirection = 0;
        cycle_count--;
        targetPulseCount -= PULSE_WIDTH;
        lcd.setCursor(13,1);
        lcd.print("        ");

      } else {
        swingArmUp();
      }
    }

    if (rideDirection == 0) {
      if (cycle_count <= 0) {
        rideComplete = 1;
        resetRide();
      } else {
        swingArmDown();
      }
    }
  
  return;

}

void encoder_channel_a() {
  //if (rideDirection == 1) {
    if (motorDirection == DIR_CW) {
      encoder_value_A++;
      encoder_value_B--;
    } else {
      encoder_value_A--;
      encoder_value_B++;    
    }
  /*} else {
    if (motorDirection == DIR_CCW) {
      encoder_value_A++;
      encoder_value_B--;
    } else {
      encoder_value_A--;
      encoder_value_B++;    
    }    
  }*/
}

void encoder_channel_b() {
  encoder_value_B++;
  //encoder_value_A--;
}

void dataReceive(int data_length) {
  int i = 0;
  char data[10] = {};
  while (Wire.available()) {
    data[i] = Wire.read();
    i++;
  }
  String dataString = String(data);
  Serial.println("Got data");
}
