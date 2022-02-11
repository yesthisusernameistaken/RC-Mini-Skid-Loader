
/* 
Varies the speed of two motors from 0 to 100%
Written by Vincent Y
*/


int motora1 = 5;
int motora2 = 6;

int motorb1 = 9;
int motorb2 = 8;

int speeda = 0;    // Speed of motor
int speedb = 0;
 
int speedchange = 5;    // 


void setup(){
  pinMode (motorb1, OUTPUT);
  pinMode (motorb2, OUTPUT);
  pinMode (motora1, OUTPUT);
  pinMode (motora2, OUTPUT);

}
void loop(){
  //In order to change motor direction, change motora1 and motorb2 to high or low
  digitalWrite (motora1, HIGH);
  analogWrite (motora2, speeda);
  analogWrite (motorb1, speeda);
  digitalWrite (motorb2, LOW);

  
  //analogWrite(PWM_A, speeda);           //Both are told to go at the same speed
  //analogWrite(PWM_B, speeda);  
  
   speeda = speeda + speedchange;      //Increase speed
  
  if (speeda == 0 || speeda == 255)     //Once the speed reaches 225 or 0 it changes direction
  {
    speedchange = -speedchange ; 
  }     
     
  delay(100); //Wait a bit for motor to speed up
  
 
}
