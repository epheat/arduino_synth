//Evan Heaton
//A-S 385 : Circuits and Bits
//11/29/16
//Square wave and Trianglemod wave synthesizer

#include <Wire.h>
#include <Adafruit_MCP4725.h>

Adafruit_MCP4725 dac;

void setup(void) {
  Serial.begin(9600);         //Baud rate = 9600
  
  pinMode(3, INPUT);          //Set pin 3 to input (momentary switch)
  pinMode(2, INPUT);          //Set pin 2 to input (toggle switch, 0=square 1=trianglemod)
  
  pinMode(A2, OUTPUT);        //Set A2 and A3 as Outputs to make them our GND and Vcc,
  pinMode(A3, OUTPUT);        //which will power the MCP4725
  digitalWrite(A2, LOW);      //Set A2 as GND
  digitalWrite(A3, HIGH);     //Set A3 as Vcc

  // For Adafruit MCP4725A1 the address is 0x62 (default) or 0x63 (ADDR pin tied to VCC)
  // For MCP4725A0 the address is 0x60 or 0x61
  // For MCP4725A2 the address is 0x64 or 0x65
  dac.begin(0x60);      
}

int pot = 0;
int pot2 = 0;
int pot_pin = A0;
int pot2_pin = A1;

void loop(void) {
  bool triangle_not_square = (digitalRead(2) == HIGH) ;
  if (triangle_not_square) {
    trianglemod_loop();
  } else {
    square_loop();
  }
}

int square_UP_timer = 50;
int square_DOWN_timer = 50;
int square_countdown = 50;
bool square_state = true;

void square_loop() {
  if (square_countdown <= 0) {
    if (square_state) {
      if (digitalRead(3) == HIGH) {
        dac.setVoltage(0, false);
      }
      
      square_countdown = square_DOWN_timer;
      square_state = false;
    } else {
      
      if (digitalRead(3) == HIGH) {
        dac.setVoltage(4095, false);
      }
      square_countdown = square_UP_timer;
      square_state = true;
    }
  }
  
  pot = analogRead(pot_pin);
  pot2 = analogRead(pot2_pin); // max 682
  int square_period = 15 + (double)(pot/20);
  square_UP_timer = square_period * (double)(pot2+86)/860; // keep the UP and DOWN timers at least 10% of the period
  square_DOWN_timer = square_period - square_UP_timer;
  square_countdown--;

}



bool read_this_time = true;
bool rising = true;
int voltage = 0;
int trianglemod_period_min = 31;
int trianglemod_period = 31;
int trianglemod_UP_timer = 30;
int trianglemod_DOWN_timer = 1;
int volt_max = 4096;

void trianglemod_loop() {
  //compute changes in voltage
  if (rising) {
    voltage += volt_max/trianglemod_UP_timer;
  } else {
    voltage -= volt_max/trianglemod_DOWN_timer;
  }
  
  //compute changes in rising
  if (voltage >= volt_max) {
    rising = false;
    voltage = volt_max-volt_max/trianglemod_DOWN_timer;
    read_this_time = true;
  } else if (voltage <= 0) {
    rising = true;
    voltage = volt_max/trianglemod_UP_timer;
  }
  
  if (digitalRead(3) == HIGH)
    dac.setVoltage(voltage, false);
  else
    dac.setVoltage(0, false);
  
  //compute changes in UP and DOWN timers from knobby bits (on down edges only)
  if (read_this_time) {
    pot = analogRead(pot_pin);
    pot2 = analogRead(pot2_pin); // max 682
    
    trianglemod_period = trianglemod_period_min + pot/10;
    trianglemod_UP_timer = trianglemod_period * (double)(pot2+86)/860;
    trianglemod_DOWN_timer = trianglemod_period - trianglemod_UP_timer;
    
    read_this_time = false;
  } 
}
