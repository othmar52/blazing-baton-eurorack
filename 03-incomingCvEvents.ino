
/**
 * Beatstep PRO clock is LOW during stop and goes HIGH ~500 MICROseconds after reset goes high
 * 
 * Keystep PRO clock is HIGH during stop
 *   when pressing start we have an additional RISE on the clock input (without going LOW before...)
 */

const int CLOCK_INPUT_PIN = 2; // Input signal pin, must be usable for interrupts
const int RESET_INPUT_PIN = 3; // Reset signal pin, must be usable for interrupts


// ===========================================================================

const int PPQN = 4;

void setupCvInputs() {
  // Interrupts
  pinMode(CLOCK_INPUT_PIN, INPUT);
  pinMode(RESET_INPUT_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INPUT_PIN), isrClock, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET_INPUT_PIN), isrReset, CHANGE);
}

void handleIncomingClockCvRising() {
  // do stuff thats only blazing baton related
  handleMidiEventTickBaton();

  tickCounter++;
  handleEventTickModClock(HIGH);
}

void handleIncomingClockCvFalling() {
  handleEventTickModClock(LOW);
}

void handleIncomingResetCvRising() {
  clockRunning = true;
  tickCounter = 0;
  startModClock();

  // do stuff thats only blazing baton related
  handleMidiEventStartBaton();
}

void handleIncomingResetCvFalling() {
  clockRunning = false;
  tickCounter = 0;
  stopModClock();
  handleMidiEventStopBaton();
}

void isrClock() {
  // shouldn't this be inverted??
  if (digitalRead(CLOCK_INPUT_PIN) == LOW) {
    handleIncomingClockCvRising();
    return;
  }
  handleIncomingClockCvFalling();
}

void isrReset() {
  // shouldn't this be inverted??
  if (digitalRead(RESET_INPUT_PIN) == LOW) {
    handleIncomingResetCvRising();
    return;
  }
  handleIncomingResetCvFalling();
}
