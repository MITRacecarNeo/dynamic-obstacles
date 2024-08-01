/*
Interrupts from timer 1 are used to schedule and deliver the sensor
trigger pulse.
The duration of the sensor echo signal is measured by generating an
external interrupt ervery time the echo signal changes state.
*/
#include <TimerOne.h> 
#include <Stepper.h>
//#include "HC_SR04.h"

const int trigPin = 9;
const int echoPin = 10;

const int pulsesPerRev = 400;
const int revs = 1;

const float revsPerSec = 1.0;
const float pulsesPerSec = pulsesPerRev * revsPerSec;

const int pulseWidthMicros = 20;
const int pulseDelayMicros = (int) (1000000 / pulsesPerSec) - pulseWidthMicros;

float duration, distance; //length of the sound wave and how far away the object is


//HC_SR04 sensor(TRIG_PIN, ECHO_PIN, ECHO_INT);

//stepper library initialization from pins 8 through 11
//Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);
// defines pins for stepper motor
#define stepPin 10
#define dirPin 16
#define stepPin2 8
#define dirPin2 9
#define stepPin3 6
#define dirPin3 7
 
//var for stop sign
bool stopSignVisible = false;
//Beam break sensor pin (for now)
volatile bool sensorHigh = false; //ISR so it's volatile
volatile bool gate = false; //current state of the gate
bool motorStopped = false; //state of motor

void setup() {

  pinMode(trigPin, OUTPUT);  
	pinMode(echoPin, INPUT);  
  pinMode(dirPin, OUTPUT);
  pinMode(step2Pin, OUTPUT);
  pinMode(dir2Pin, OUTPUT);
  pinMode(step2Pin, OUTPUT);
  pinMode(dir3Pin, OUTPUT);
  pinMode(step3Pin, OUTPUT);
  //speed of the motor
  //Serial port in case we need to debug
  Serial.begin(9600);
}

void loop() {
  digitalWrite(trigPin, LOW);  
	delayMicroseconds(2); //2 microseconds delay just to make sure that the pin in low first.
	digitalWrite(trigPin, HIGH);  
	delayMicroseconds(10); //10 microseconds delay, which sends out an 8 cycle sonic burst from the transmitter, which then bounces of an object and hits the receiver(Which is connected to the Echo Pin). 
	digitalWrite(trigPin, LOW); 

    if (stopSignVisible) {

      Serial.println("Hiding stop sign and opening gate...");

      //mechanism to rotate
      digitalWrite(dirPin,HIGH); // Enables the motor to move in a particular direction
      // Rotate 180 degs clockwise (Opening Gate)
      for(int x = 0; x < revs * pulsesPerRev * 0.5; x++) { 
        digitalWrite(stepPin,HIGH); 
        delayMicroseconds(pulseWidthMicros);    // by changing this time delay between the steps we can change the rotation speed
        digitalWrite(stepPin,LOW); 
        delayMicroseconds(pulseDelayMicros); 
        }
      stopSignVisible = false;
      gate = false;

    } else {

      //Show the stop sign and close the gate
      Serial.println("Showing stop sign and closing gate...");
      digitalWrite(dirPin,LOW); //Changes the rotations direction
  // Rotate 180 degs anti-clockwise (Closing gate)
      for(int x = 0; x < revs * pulsesPerRev * 0.5; x++) {
        digitalWrite(stepPin,HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin,LOW);
        delayMicroseconds(pulseDelayMicros);
      }
      stopSignVisible = true;
      gate = true;
    }
    
    delay(10000); //change every 5-10 seconds. 10 for now
  }
}
