/**
 *  
 *  this replaces the very first clock pulse with a reset pulse
 *  https://www.modulargrid.net/e/other-unknown-clock-divider--
 *  
 *  the modified clock is compatible to modules:
 *   malekko varigate 4+
 *   shakmat time wizard
 *   ikerion clock divider
 *  
 *  to get it in sync with my masterclock we need to
 *   -) reset to LOW at FALLING edge of very first clock pulse
 *   -) supress the very first clock pulse after start
 *   
 *  additionaly we create another clock with division 1/16
 *  this clock output has also the very first clock pulse supressed
 */ 

#define MOD_CLOCK_PIN A4
#define MOD_RESET_PIN A5
#define MOD_CLOCK_PIN2 A3


bool slowGateFlag = false;
void setupModClockOutputs() {
  pinMode(MOD_CLOCK_PIN, OUTPUT);
  pinMode(MOD_RESET_PIN, OUTPUT);
  pinMode(MOD_CLOCK_PIN2, OUTPUT);
  digitalWrite(MOD_RESET_PIN, HIGH);
}


void handleEventTickModClock(bool highLow) {
  if (tickCounter == 1) {
    // do not send out very first tick but replace it with a single reset pulse
    digitalWrite(MOD_RESET_PIN, highLow);
    return;
  }

  // all other clock pulses should be a simple copy of the incoming clock pulses
  digitalWrite(MOD_CLOCK_PIN, highLow);

  // additional slow clock with division 1/16
  if ((tickCounter-1) % 16 == 0 && slowGateFlag == false) {
    digitalWrite(MOD_CLOCK_PIN2, highLow);
    slowGateFlag = true;
  }
  if (slowGateFlag == true && !highLow) {
    digitalWrite(MOD_CLOCK_PIN2, LOW);
    slowGateFlag = false;
  }
}
void startModClock() {
  digitalWrite(MOD_RESET_PIN, LOW);
  digitalWrite(MOD_CLOCK_PIN, LOW);
  digitalWrite(MOD_CLOCK_PIN2, LOW);
}
void stopModClock() {
  digitalWrite(MOD_RESET_PIN, LOW);
  digitalWrite(MOD_CLOCK_PIN, LOW);
  digitalWrite(MOD_CLOCK_PIN2, LOW);
}
