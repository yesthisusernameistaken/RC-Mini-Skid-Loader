
#include <PPMReader.h>

/* 
Written by Vincent Y
Alternative code for https://github.com/flytron/TinyDrive
Using RX2A PPM FS-RX2A Pro Receiver Mini RX for Flysky receiver

Flashing the Tinydrive:
Select Tools>Board>Arduino Pro or Pro mini
Processor>ATmega328P (5v, 16Mhz)
-Arduino Firmware Upload Process:
Turn off the TX
Unplug the battery
Press the Upload button of Arduino IDE and wait for compiling
Plug the battery immediately(in 2 seconds) when you see "Uploading..." message on the display
FTDI LEDs will start blinking and you will see "Done Uploading" message in a few seconds.

To do: 
-Change pins to work with the code below
https://github.com/flytron/TinyDrive/blob/main/TinyDrive_v1/TinyDrive_v1.ino
-Add servo support
Can't user servo library as it uses timers and dissables pin 8 and 9
Also causing issues with timer
-Add LED support
-Fix issue with directions not being correct when reversing then turning
-Detect if valid PPM signal or not, stop it from misbehaving when the TX is not on


Done:
-Add reverse 10.02.2022
-Rename motor pins 10.02.2022
-Test out with expected channel set to 8 11.02.2022
This is based on the receiver used and supported PPM
-Add batterty code sensor and test 12.02.2022

*/


//Specific for the Tinydrive
//#define SBUS_ENABLE_PIN A3  // HIGH: Enables the XOR gate to invert the SBUS signal to Standard Serial.  LOW: Standard Serial Input mode. 
#define STATUS_LED 13         // On Board LED pin

// Simplified Status LED Modes 
#define LED_TOGGLE digitalWrite(STATUS_LED,!digitalRead(STATUS_LED));
#define LED_ON digitalWrite(STATUS_LED,HIGH); 
#define LED_OFF digitalWrite(STATUS_LED,LOW);

//Servos
#define SERVO_1 A4          // CH4 Servo PWM signal output.
//#define SERVO_2 A5          // CH5 Servo PWM signal output.
#define HEADLIGHT_PIN 3     // D3(PWM) pin connected to the Base pin of an NPN transistor. [LED+] has a serial 10 Ohm resistor to limit the current. Use AnalogWrite(0-255) to control the light intensity.

//Battery sensor
#define Battery_Sensor_Pin  A6        // Connected to the Battery input over 1/11 voltage divider. 
#define Battery_Sensor_Factor  11.6   // Multiply the reading with this value and you will find the battery voltage in mV  
#define Cell_Min_Voltage 3200         // Minimum 3.2V per cell when the motors off.
uint16_t Battery_Voltage_mV = 0;      // The battery voltage in mV
uint8_t Battery_Cell_Count = 1;       // Detected battery cell count (1 or 2)
#define CHECK_PERIOD 1000L            // Check the battery level in every 1000ms
unsigned long last_check_time = 0L ;  
volatile boolean motors_off = 1;

//Motor stuff
int M1_PHASE = 5;
int M1_DIRECTION = 6;
int M2_DIRECTION = 9;
int M2_PHASE = 8;
int Speed_A = 0;          // Speed of motor
int Speed_B = 0;

// For PPM values, the difference between is the deadband
int lowerLimit = 1420;
int upperLimit = 1520; 

// Initialize a PPMReader on digital pin 2 with 6 expected channels.
// Receiver used JMT RX2A PPM FS-RX2A Pro Receiver Mini RX for Flysky
int interruptPin = 2;
int channelAmount = 6;
PPMReader ppm(interruptPin, channelAmount);



void setup(){

  //Battery setup
  analogReference(INTERNAL);  // Enable internal 1.1v analog reference for the battery voltage readings
  delay(200);                 // Wait 200mS on the startup for the voltage stabilisation. 
  Battery_Voltage_mV =  analogRead(Battery_Sensor_Pin) * Battery_Sensor_Factor; // Read battery voltage in mV
  delay(100);
  // The first read may be faulty due to the ADC referance change on the startup. The next read will be fine.  
  Battery_Voltage_mV =  analogRead(Battery_Sensor_Pin) * Battery_Sensor_Factor; // Read battery voltage in mV
   if (Battery_Voltage_mV>6200)       {
        Battery_Cell_Count = 2;  // Detect 2S liPo if >6.2v
      }else{ 
        Battery_Cell_Count = 1;}  // Run on 1S LiPo mode for 1S LiPo or 3-4x AA Batteries(<=6V) 
  last_check_time = millis (); // set the first battery check time.
      
  //Motor stuff 
  pinMode (M2_DIRECTION, OUTPUT); pinMode (M2_PHASE, OUTPUT); pinMode (M1_PHASE, OUTPUT); pinMode (M1_DIRECTION, OUTPUT);
  pinMode(HEADLIGHT_PIN, OUTPUT); digitalWrite(HEADLIGHT_PIN,LOW);
  
  //The two servos
  pinMode(SERVO_1, OUTPUT); digitalWrite(SERVO_1,LOW);
  //pinMode(SERVO_2, OUTPUT); digitalWrite(SERVO_2,LOW);
  
  //Serial.begin(115200);
  delay(200);
  //march();   // Play The Imperial March 
  pinMode(STATUS_LED, OUTPUT);  // initialize the LED pin as an output.
  LED_ON; 
}

void loop(){
 
  unsigned long PPM_Channel_1 = ppm.latestValidChannelValue(1, 1500);//A
  unsigned long PPM_Channel_2 = ppm.latestValidChannelValue(2, 1500);//E
  unsigned long PPM_Channel_3 = ppm.latestValidChannelValue(3, 1500);//T
  unsigned long PPM_Channel_4 = ppm.latestValidChannelValue(4, 1500);//R
  unsigned long PPM_Channel_5 = ppm.latestValidChannelValue(5, 1500);
  unsigned long PPM_Channel_6 = ppm.latestValidChannelValue(6, 1500);

// Forward. reverse
  if (PPM_Channel_3 > upperLimit) {
    //Stick is pushed upwards, go forward
    Speed_A = map(PPM_Channel_3, upperLimit, 2000, 0, 255);
    Speed_B = Speed_A; 
    digitalWrite(M1_PHASE, LOW); digitalWrite(M2_PHASE, LOW);
  }else if (PPM_Channel_3 < lowerLimit){
    //Stick being pulled down, go backwards
    Speed_A = map(PPM_Channel_3, lowerLimit, 1000, 0, 255);
    Speed_B = Speed_A; 
    digitalWrite(M1_PHASE, HIGH); digitalWrite(M2_PHASE, HIGH);
  }else{
    //Stick in middle position. stop all motors
    Speed_A = 0;
    Speed_B = Speed_A;
  }

  // Steeting 
  if (PPM_Channel_1 < lowerLimit) {
    // Convert the declining X-axis readings from 470 to 0 into increasing 0 to 255 value
    int RMapped = map(PPM_Channel_1, lowerLimit, 1000, 0, 255);
    // Move to left - decrease left motor speed, increase right motor speed
    Speed_A = Speed_A - RMapped;
    Speed_B = Speed_B + RMapped;
    // Confine the range from 0 to 255
    if (Speed_A < 0) {
      Speed_A = 0;    }
    if (Speed_B > 255) {
      Speed_B = 255;    }
  }
  if (PPM_Channel_1 > upperLimit) {
    // Convert the increasing X-axis readings from 550 to 1023 into 0 to 255 value
    int xMapped = map(PPM_Channel_1, 1500, 2000, 0, 255);
    // Move right - decrease right motor speed, increase left motor speed
    Speed_A = Speed_A + xMapped;
    Speed_B = Speed_B - xMapped;
    // Confine the range from 0 to 255
    if (Speed_A > 255) {
      Speed_A = 255;    }
    if (Speed_B < 0) {
      Speed_B = 0;    }
  }

    //Write speed to motors
    analogWrite(M1_DIRECTION, Speed_A);
    analogWrite(M2_DIRECTION, Speed_B);


  digitalWrite(SERVO_1, HIGH);
  delayMicroseconds((PPM_Channel_2/2) + 1000);
  digitalWrite(SERVO_1, LOW);
  





  //Serial.print(String(Speed_A) + " " + String(Speed_B));
  //Serial.println();
  //delay(100); //Wait a bit for motor to speed up







  //Battery check routine
  if ((Speed_A< 10) && (Speed_B < 10)){ //central dead zone
          motors_off = 1;
          //m_throttle = 0;
         // m_direction = 0;        
        }else{
        motors_off = 0;}
       
  // Check for the battery voltage 
   if ((millis() - last_check_time) >= CHECK_PERIOD){
        last_check_time = millis ();
        if (motors_off){ // Measure the battery when the motors off. 
            Battery_Voltage_mV = analogRead(Battery_Sensor_Pin) * Battery_Sensor_Factor; // in mV
            if (Battery_Voltage_mV< (Battery_Cell_Count* Cell_Min_Voltage)) // stop working below the minimum cell voltage and blink the LED. 
               {  while(1){ // Infinite loop 2 short blink 1 second OFF
                  LED_ON
                  delay(100);
                  LED_OFF
                  delay(100);
                  LED_ON
                  delay(100);
                  LED_OFF
                  delay(1000);}
               }
            }   
       }
 
}
