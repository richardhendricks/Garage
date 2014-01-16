// Garage Door open sensor node
// This BTLE sensor node will wake up, run a test to detect
// if the garage door is open, then transmit the status for a
// short while before going back to sleep.

// Copyright 2014 by Richard Hendricks
// Licensed as Create Commons Attribution ShareAlike 3.0 
// (see http://creativecommons.org/licenses/by-sa/3.0/legalcode)
// LED ambient light detector providedd by
// http://playground.arduino.cc/Learning/LEDSensor

#include <RFduinoBLE.h>

const unsigned int ILLUM_LED_P =  1;         //This LED lights up the garage door, current limit to 5mA
const unsigned int DETECT_LED_P = 2;         //This is the anode side of the detect LED
const unsigned int DETECT_LED_N = 3;         //This is the cathode side of the detect LED

const unsigned int CAP_CHARGE_DELAY = 200000;   //Delay waiting for diode to charge in reverse bias in microseconds
const unsigned int MAX_LOOP = 20000;         //How many times to run through the loop waiting for the diode to discharge

const unsigned int MAX_MEASURE = 1;          //How many times to measure the LED discharge

const unsigned int TRANSMIT_TIME = 5;        //How long to transmit/receive data in seconds
const unsigned int SLEEP_TIME = 5;           //How long to enter low-power mode in seconds

const unsigned int INTERVAL = 200;           //Interval time between BTLE advertisements in milliseconds

void setup() {
  pinMode( ILLUM_LED_P, OUTPUT );
  digitalWrite( ILLUM_LED_P, LOW );  
  
  RFduinoBLE.deviceName = "Garage";
  RFduinoBLE.advertisementData = " Detector";
  RFduinoBLE.advertisementInterval = INTERVAL;
}

int measure_brightness(){
  int i=0;
  // Apply a reverse voltage to the diode, charging it up as a capacitor
  /////RAH NEED TO MAKE SURE THIS WILL WORK THE SAME ON RFDUINO
  /////RAH Make it take in P/N as variables, allow for multiple detectors?
  pinMode( DETECT_LED_N, OUTPUT ); 
  pinMode( DETECT_LED_P, OUTPUT );
  digitalWrite( DETECT_LED_N, HIGH );
  digitalWrite( DETECT_LED_P, LOW );
  
  // Original code didn't have a delay here, just putting in a few hundred us to make sure
  // the capacitor is charged up
  
  delayMicroseconds( CAP_CHARGE_DELAY );
  
  // Change the cathode GPIO to an input to measure how long it takes to discharge and measure 0,
  // make sure internal pullup resistor is NOT enabled.
  
  pinMode( DETECT_LED_N, INPUT ); 
  digitalWrite( DETECT_LED_N, LOW );
  
  // Count how long it takes the diode to bleed back down to a logic zero
  for( i=0; i< MAX_LOOP; i++ )
  {
    if( 0 == digitalRead(DETECT_LED_N) )
      { break; }
  }
  return i;
}

// time is in seconds, minimum is going to be 1
void sendData( int data, unsigned int time )
{
  unsigned int k;

  if( 0 == time )
  { time = 1; }
  
  RFduinoBLE.begin();  
  for( k=0; k<time; k++ )
  {
    RFduinoBLE.sendInt( data );
    RFduino_ULPDelay( SECONDS(1) );
  }
  RFduinoBLE.end();
}

void loop() {
  unsigned int j=0;
  
/////RAH Does the loop destroy these variables?  would prefer they not be destroyed each time
/////RAH through the loop so that command codes can modify them.
  unsigned int max_measure=MAX_MEASURE;
  unsigned int transmit_time=TRANSMIT_TIME;
  unsigned int sleep_time=SLEEP_TIME;
  int total_delay=0;
  
  //Turn on Illuminate LED
  digitalWrite( ILLUM_LED_P, HIGH );  
  
  //Measure Detect LED
  for( j = 0; j < max_measure; j++ )
  {
    total_delay+=measure_brightness();
  }
  
  //Calculate average delay
  total_delay = total_delay/max_measure;
  
  //Turn off Illuminate LED  
  digitalWrite( ILLUM_LED_P, LOW );  
  
  //Transmit for 5 seconds
  sendData( total_delay, transmit_time );
  
  /////RAH TBD
  //Listen for any command packets - verify command packets
  //Update any internal variables - make sure they are valid first

  //Sleep for 5 seconds
  RFduino_ULPDelay( SECONDS(sleep_time) );

}
