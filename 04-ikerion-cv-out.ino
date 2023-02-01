/**
 *  
 *  this fixes a personal issue with the ikerion clock divider
 *  https://www.modulargrid.net/e/other-unknown-clock-divider--
 *  
 *  to get it in sync with my masterclock i need to
 *   -) invert the reset signal
 *   -) supress the very first gate pulse after start
 */ 

#define IKERION_CLOCK_PIN A4
#define IKERION_RESET_PIN A5
#define IKERION_CLOCK_PIN2 A3


bool slowGateFlag = false;
void setupIkerionClock() {
  pinMode(IKERION_CLOCK_PIN, OUTPUT);
  pinMode(IKERION_RESET_PIN, OUTPUT);
  pinMode(IKERION_CLOCK_PIN2, OUTPUT);
  digitalWrite(IKERION_RESET_PIN, HIGH);
}


void handleMidiEventTickIkerionClock(bool highLow) {
  if (tickCounter == 0) {
    // do not send out very first tick;
    return;
  }
  digitalWrite(IKERION_CLOCK_PIN, highLow);
  if ((tickCounter+1) % 16 == 0 && slowGateFlag == false) {
    digitalWrite(IKERION_CLOCK_PIN2, highLow);
    slowGateFlag = true;
  }
  if (slowGateFlag == true && !highLow) {
    digitalWrite(IKERION_CLOCK_PIN2, LOW);
    slowGateFlag = false;
  }
}
void handleMidiEventStartIkerionClock() {
  digitalWrite(IKERION_RESET_PIN, LOW);
}
void handleMidiEventStopIkerionClock() {
  digitalWrite(IKERION_RESET_PIN, HIGH);
}
