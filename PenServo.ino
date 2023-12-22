#include <Servo.h>
#include <EEPROM.h>

//Sketch to decode the Air/Spindle pin output from standard GRBL and use it to control a servo to raise/lower a pen. 
//There are forks of GRBL which directly control a servo for the Z axis, but they seemed to have comprimises in terms of timing or control
//Using a second arduino might seem like overkill, but many of us have one knocking about in a drawer somewhere.

// EEPROM Addresses for UP and DOWN values
#define DOWN_VAL 0
#define UP_VAL 1

//Pin to write Servo signal to
#define SERVO_PIN 9

// Pin to watch for pen state. Low is pen up, High is pen down
#define INPUT_PIN 13

Servo penServo;  // create servo object to control a servo

byte down=EEPROM.read(DOWN_VAL);
byte up  =EEPROM.read(UP_VAL);

//Track current state
bool penDown=false;

String command;

void setup() {

  pinMode(INPUT_PIN, INPUT);
  
  Serial.begin(9600);
  penServo.attach(SERVO_PIN);

  // Do bounds-checking and update defaults if defaults are out of range
  if(down > 180){
    EEPROM.write(DOWN_VAL, 0);
    down=0;
  }
  if(up > 180){
    EEPROM.write(UP_VAL, 180);
    up=180;
  }

  command = "";

  penServo.write(down);
  delay(250);
  penServo.write(up);

  Serial.println("Running. Range between 1 and 180. Commands are u=<number> and d=<number> to set servo positions for up and down. '?' for current status");
  
}

void printStatus(){
  Serial.print("Up: ");
  Serial.println(up);
  
  Serial.print("Down: ");
  Serial.println(down);
  
  Serial.print("PenDown: ");
  Serial.println(penDown?"Yarp":"Narp");
}

void processCommand(String command){
  if(command.startsWith("u=")){
    String valueString = command.substring(2);
    int value=valueString.toInt();
    if(value > 0 && value <=181){
      EEPROM.write(UP_VAL, value);
      up=value;
      Serial.println("Up value stored and updated");
    }else{
      Serial.println("Value not understood: " + valueString);
    }    
  }
  else if(command.startsWith("d=")){
    String valueString = command.substring(2);
    int value=valueString.toInt();
    if(value > 0 && value <=181){
      EEPROM.write(DOWN_VAL, value);
      down=value;
      Serial.println("Down value stored and updated");
    }else{
      Serial.println("Value not understood: " + valueString);
    }    
  }
  else if(command.startsWith("?")){
    printStatus();
  }
  else {
    Serial.println("Command not understood: " + command);
  }
}

void checkSerial(){
    if (Serial.available())  {
    char c = Serial.read();  //gets one byte from serial buffer
    if (c == '\n') {  //looks for end of data packet marker
     processCommand(command);
     command="";
    } else if (c == '\r') {
      //Skip LF if client sends CR LF
    }
    else {
      command +=c;
    }
  }
}


void loop() {

  checkSerial();

  //High is pen-down
  if(digitalRead(INPUT_PIN) == HIGH){
    if(penDown==false){
      penServo.write(down);
      penDown=true;
      Serial.println("Pen Down");
    }
  }else{
    if(penDown==true){
      penServo.write(up);
      penDown=false;
      Serial.println("Pen Up");
    }
  }
}
