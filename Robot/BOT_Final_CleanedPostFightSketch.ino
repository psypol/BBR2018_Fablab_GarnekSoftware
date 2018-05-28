/*
Exercise Monster Motor Mini Module
Uses Serial Monitor window to issue commands for controlling the DC motor
connected to the module
S = Stop | F = Forward | R = Reverse | C = Returns the current reading of the motors
Pxxx (P0 - P255) sets the PWM speed value | P? = Returns the current PWM value

//FRED NOTE:
// To receive a string of 5 elements:
// 0;0;0;0;0 (minimal extreme value)
// 1;255;1;255;1 (maximal extreme value)
// indexes: 
// 0       is direction of motor LEFT  [values 0 or 1]
// 1       is PWM of motor LEFT        [values 0 to 254]
// 2       is direction of motor RIGHT [values 0 or 1]
// 3       is PWM of motor RIGHT       [values 0 to 254]
// 4       is activeness of not of the 3rd engine (outside ring) [values 0 or 1]       

//FREDNOTE: FORWARD is the side with the driller 3rd engine

*/
const int SerialSpeed = 9600; //To set first the communication speed

//General constants for engines script
const int BRAKE = 0;
const int CW = 1;
const int CCW = 2;
const int CS_THRESHOLD = 15;
int timefromlastcommand=0;

//GENERAL MOTORS DEFINITION
int motors_Speed = 150;    //Default motor speed (applies to 2 motors if we don't interfere in LEFT/RIGHT Definitions
//Next lines applies by default this speed to both motors
int motor_Speed=motors_Speed;
int motor2_Speed=motors_Speed;

int motor_State = BRAKE;  // Current motor state
int motor2_State = BRAKE;  // Current motor(2) state (for motor RIGHT)
int mot_current = 0;      // Motor current (Fred: Not used for now)

//LEFT MOTOR Definitions
//int motor_Speed = 150;    //Uncomment if needed / Default motor speed
const int MOTOR_A1_PIN = 7;   //LEFT Motor control input pins
const int MOTOR_B1_PIN = 8;
const int PWM_MOTOR = 5;      // ~Motor PWM 
const int CURRENT_SENSE = A2; // Current sense pin
const int EN_PIN = A0;        // Enable/Diag pin

//RIGHT MOTOR Definitions
//int motor2_Speed = 150;     //Uncomment if needed / Default motor speed
const int MOTOR_A2_PIN = 2;   //RIGHT Motor(2) control input pins
const int MOTOR_B2_PIN = 4;
const int PWM_MOTOR2 = 3;      // ~RIGHT Motor(2) PWM 
const int CURRENT_SENSE2 = A3; // RIGHT Motor(2) Current sense pin
const int EN_PIN2 = A1;        // Enable/Diag pin (for motor RIGHT)

//
const int PIN_WEAPON = 9;        // Pn on which there is a relay

//VAR to retrieve command from serial
char readString[4];   // String array to hold PWM value typed in on keyboard

//From Cyryllo: RADIO receive
#include <VirtualWire.h>
#define led_pin 13
#define receive_pin 11

//===============================================================================
//  Initialization
//===============================================================================
void setup() {

// MOTOR1 (LEFT) - PINs DEF  
  pinMode(MOTOR_A1_PIN, OUTPUT);
  pinMode(MOTOR_B1_PIN, OUTPUT);
  pinMode(PWM_MOTOR, OUTPUT);
// MOTOR2 (RIGHT) - PINs DEF
  pinMode(MOTOR_A2_PIN, OUTPUT);
  pinMode(MOTOR_B2_PIN, OUTPUT);
  pinMode(PWM_MOTOR2, OUTPUT);
// WEAPON
  pinMode(PIN_WEAPON, OUTPUT);

  
// FRED: next is not needed for now
// Uncomment the next 2 lines to use the Enable pins to enable/disable the device.
// To monitor for fault conditions instead, they would be defined as inputs  
// pinMode(EN_PIN, OUTPUT);      
// digitalWrite(EN_PIN, HIGH);  
  
  Serial.begin(SerialSpeed);           // Initialize serial monitor
  Serial.println("Enter command:");    // Printout commands
  //Fred: old from example (serial commands) 
  /*  Serial.println("S = STOP | F = FOR/FOR | G = BAK/BAK | H = FOR/BAK | I = BAK/FOR | R = REVERSE");
  Serial.println("C = READ CURRENT | Pxxx = PWM SPEED (P000 - P254) | P? = CURRENT PWM"); */

//From Cyryllo: RADIO receive  
  vw_set_rx_pin(receive_pin);
  vw_setup(2000);   
  vw_rx_start(); // startujemy odbieranie danych (uruchamiamy)
  pinMode(led_pin, OUTPUT);

}

//===============================================================================
//                Main Loop (await commands from radio + apply on engines
//===============================================================================
void loop() 
{
  // Just loop while monitoring the serial port and then jump to DoSerial (FRED: To be changed to listen to RADIO commands incoming)
  //if(Serial.available())
  //{
    

    ////////////////////////////////////////////////////////////////////////////////////////
    //                    LOOP RADIO info received from Cyryllo
    ////////////////////////////////////////////////////////////////////////////////////////
    
    // częśc wymagana do poprawnego działania biblioteki
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;
    String wiadomosc ="";
     
    
    if (vw_get_message(buf, &buflen)) // jeśli odbierzemy dane
    {
        int i;
        
        timefromlastcommand=0;
        digitalWrite(led_pin, HIGH); //zapalamy LED
            
        for (i = 0; i < buflen; i++) // w pętli zczytujemy dane z odbiornika
        {
          wiadomosc+=char(buf[i]);
        }
        Serial.println(wiadomosc);
        digitalWrite(led_pin, LOW); // gasimy LED
        
        DoSerial(wiadomosc); //FRED: DoSerial is changed to process radio received commands
        
    }
    else
    {
      timefromlastcommand++; //Fred: increment the time since we had no command
      //Serial.print("NoRadio For:");
      //Serial.println(timefromlastcommand);
      delay(1);
      if(timefromlastcommand>=100)
      {
        wiadomosc="1;255;2;255;0";      
        DoSerial(wiadomosc); //FRED: If no command received : FORCE STOP
        Serial.println("FORCED STOP");
      }
    }

    
    delay(10);
  
       
  //}//End of the Serial await (from demo script)      |       FRED: To comment-out
} //End of MAIN LOOP

//===============================================================================
//  Subroutine to handle characters typed via Serial Monitor Window
//===============================================================================
void DoSerial(String wiadomoscfromradio)
{
  int index = 0; //Fred: used to collect incoming chars from serial
  int pwm_Value = 0;
  int pwm2_Value = 0;

  char ch; //Fred: moving here declaration

//1) Split 4 parameters (Engine1 direction / Speed / Engine2 direction / Speed)

char sz[] = "1;255;1;255;1"; //Fred: to prepare the biggest type of data the splitter will parse
String ReceivedThing = wiadomoscfromradio;
int Radioelementnumber=0;
String Motor1Left_Dir,Motor1Left_PWM,Motor2Right_Dir,Motor2Right_PWM,Weaponactivate;
Weaponactivate='0'; //By default weapon is off
int Motor1Left_FinalDir,Motor2Right_FinalDir;


    // Convert from String Object to String.
    char buf[sizeof(sz)];
    ReceivedThing.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    while ((str = strtok_r(p, ";", &p)) != NULL) // delimiter is the semicolon
      {
        //Debug printing
        /*Serial.print(str);
        Serial.print("/");*/

        switch (Radioelementnumber) //Fred: Once splitted put params in proper variable
        {
          case 0:
            Motor1Left_Dir=str;          
            break;

          case 1:
            Motor1Left_PWM=str;
            break;

          case 2:
            Motor2Right_Dir=str;
            break;

          case 3:
            Motor2Right_PWM=str;
            break;

          case 4:
            Weaponactivate=str;
            break;            

          default:
            break;          
        }

        Radioelementnumber++;
      }
  
  //Debug printing
  
  Serial.print(Motor1Left_Dir);
  Serial.print("*");
  Serial.print(Motor1Left_PWM);
  Serial.print("*");
  Serial.print(Motor2Right_Dir);
  Serial.print("*");
  Serial.print(Motor2Right_PWM);
  Serial.print("*");
  Serial.print(Weaponactivate);
  Serial.print("*");
  Serial.println("");
  

//2) Send to proper cases depending on parameters received
//Motor1Left_Dir/Motor1Left_PWM/Motor2Right_Dir/Motor2Right_PWM

//HERE ACTIVATE WEAPON
if(Weaponactivate=="0")
digitalWrite(PIN_WEAPON, LOW);

if(Weaponactivate=="1")
{
digitalWrite(PIN_WEAPON, HIGH);
Serial.println("KILL!");
}

//HERE IMPACT MOTORS DIRECTLY
Motor_Cmd(Motor1Left_Dir.toInt(), Motor1Left_PWM.toInt());
Motor2_Cmd(Motor2Right_Dir.toInt(), Motor2Right_PWM.toInt());




//FRED: Not needed for now for motor2 (maybe later?)
  int mot1_ADC = 0;
  float mot1_voltage = 0.0;


  
}

//===================================================================================
//======================== FUNCTIONS TO WRITE TO MOTORS =============================
//===================================================================================

//LEFT MOTOR
void Motor_Cmd(int direct, int pwm)     //Function that writes to the motors
{
  {
    if(direct == 1)    {
      digitalWrite(MOTOR_A1_PIN, LOW); 
      digitalWrite(MOTOR_B1_PIN, HIGH);
    }
    else if(direct == 2)    {
      digitalWrite(MOTOR_A1_PIN, HIGH);
      digitalWrite(MOTOR_B1_PIN, LOW);      
    }
    else    {
      digitalWrite(MOTOR_A1_PIN, LOW);
      digitalWrite(MOTOR_B1_PIN, LOW);            
    }
    analogWrite(PWM_MOTOR, pwm); 
  }
}  

//RIGHT MOTOR
void Motor2_Cmd(int direct, int pwm)     //Function that writes to the motors
{
  {
    if(direct == 1)    {
      digitalWrite(MOTOR_A2_PIN, LOW); 
      digitalWrite(MOTOR_B2_PIN, HIGH);
    }
    else if(direct == 2)    {
      digitalWrite(MOTOR_A2_PIN, HIGH);
      digitalWrite(MOTOR_B2_PIN, LOW);      
    }
    else    {
      digitalWrite(MOTOR_A2_PIN, LOW);
      digitalWrite(MOTOR_B2_PIN, LOW);            
    }
    analogWrite(PWM_MOTOR2, pwm); 
  }
}
  
