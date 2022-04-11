/* https://wavedrom.com/

{signal: [
  {name: 'TRIG_PIN', wave: '030...30...30', data: ['10uS','10uS','10uS']},
  {name: 'SONIC_BURST', wave: '0..4.0..4.0..', data: ['8 x 25uS','8 x 25uS']},
  {name: 'ECHO_PIN', wave: '0.....50...50', data: ['Data','Data']}
]}

*/

#include <Arduino.h>

#define TX_ONLY // tx mode only (ignore echo)
#define TRIG_PIN 18 // trigger pin
#define ECHO_PIN 5 // echo pin
#define SOUND_SPEED 0.034 // sound speed in cm/uS

#ifndef TX_ONLY
long duration;
float distanceCm;
float distanceInch;
#endif

void setup() {
  Serial.begin(115200); 
  pinMode(TRIG_PIN, OUTPUT); 
  pinMode(ECHO_PIN, INPUT); 
}

void loop() {
  // clear TRIG_PIN
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // set TRIG_PIN for 10uS
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
#ifndef TX_ONLY
  // read ECHO_PIN with travelling time in uS
  duration = pulseIn(ECHO_PIN, HIGH);
  
  // compute distance
  distanceCm = duration * SOUND_SPEED/2;
  
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
#endif

  delayMicroseconds(300);
}

