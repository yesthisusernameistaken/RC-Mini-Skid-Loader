/* 

Takes PPM input and controls a servo

*/
#include <PPMReader.h>
#include <Servo.h>


//--------------------------------------------------
//Motor stuff
int M1_PHASE = 5;
int M1_DIRECTION = 6;
int M2_DIRECTION = 9;
int M2_PHASE = 8;
int Speed_A = 0;          // Speed of motor
int Speed_B = 0;
// For PPM values, the difference between is the deadband
int lowerLimit = 1450;
int upperLimit = 1550; 

//Battery sensor
#define Battery_Sensor_Pin  A6        // Connected to the Battery input over 1/11 voltage divider. 
#define Battery_Sensor_Factor  11.6   // Multiply the reading with this value and you will find the battery voltage in mV  
#define Cell_Min_Voltage 3200         // Minimum 3.2V per cell when the motors off.
uint16_t Battery_Voltage_mV = 0;      // The battery voltage in mV
uint8_t Battery_Cell_Count = 1;       // Detected battery cell count (1 or 2)
#define CHECK_PERIOD 1000L            // Check the battery level in every 1000ms
unsigned long last_check_time = 0L ;  
volatile boolean motors_off = 1;
//--------------------------------------------------



Servo myservo;  // create servo object to control a servo

// Initialize a PPMReader on digital pin 2 with 6 expected channels. Receiver used JMT RX2A PPM FS-RX2A Pro Receiver Mini RX for Flysky
int interruptPin = 2; int channelAmount = 6;
PPMReader ppm(interruptPin, channelAmount);

int pos = 0;    // variable to store the servo position

void setup() {
  myservo.attach(A4);  // attaches the servo on pin 9 to the servo object
  
//--------------------------------------------------
 //Motor stuff 
  pinMode (M2_DIRECTION, OUTPUT); pinMode (M2_PHASE, OUTPUT); pinMode (M1_PHASE, OUTPUT); pinMode (M1_DIRECTION, OUTPUT);

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
//--------------------------------------------------
}

void loop() {

  unsigned long PPM_Channel_1 = ppm.latestValidChannelValue(1, 1500);//A
  unsigned long PPM_Channel_2 = ppm.latestValidChannelValue(2, 1500);//E
  unsigned long PPM_Channel_3 = ppm.latestValidChannelValue(3, 1500);//T
  unsigned long PPM_Channel_4 = ppm.latestValidChannelValue(4, 1500);//R
  unsigned long PPM_Channel_5 = ppm.latestValidChannelValue(5, 1500);
  unsigned long PPM_Channel_6 = ppm.latestValidChannelValue(6, 1500);

  myservo.write(PPM_Channel_2);

  //int RMapped = map(PPM_Channel_1, 1000, 2000, 0, 255);
  //Speed_A = RMapped;
  //digitalWrite(M1_PHASE, HIGH);
  //analogWrite(M1_DIRECTION, Speed_A);



 
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

    //Write speed to motors
    //analogWrite(M1_DIRECTION, Speed_A);
    //analogWrite(M2_DIRECTION, Speed_B);










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
                  /*LED_ON
                  delay(100);
                  LED_OFF
                  delay(100);
                  LED_ON
                  delay(100);
                  LED_OFF
                  delay(1000);*/}
               }
            }   
       }



  
}
