

const int CLOCK_INPUT = 2; // Input signal pin, must be usable for interrupts
const int RESET_INPUT = 3; // Reset signal pin, must be usable for interrupts

// ===========================================================================

long clockTickCount = -1; // Input clock counter, -1 in order to go to 0 no the first pulse

const int PPQN = 4;

unsigned long lastBarStartMilliSecond = 0;
unsigned long currentMilliSecond = 0;
float bpm = .0;
volatile bool clockTickSignal = false; // Clock signal digital reading, set in the clock ISR
volatile bool clockFlag = false; // Clock signal change flag, set in the clock ISR
volatile bool resetFlag = false; // Reset flag, set in the reset ISR



void setupCvClockReset() {
  // Interrupts
  pinMode(CLOCK_INPUT, INPUT);
  pinMode(RESET_INPUT, INPUT);
  attachInterrupt(digitalPinToInterrupt(CLOCK_INPUT), isrClock, CHANGE);
  attachInterrupt(digitalPinToInterrupt(RESET_INPUT), isrReset, RISING);
}

void loopCvClockReset() {
  if (clockFlag == false) {
    return;
  }
  
  // Clock signal changed
  clockFlag = false;
    
  if (clockTickSignal == false) {
    return;
  }
  currentMilliSecond = micros();
  // Clock rising, update counter
  clockTickCount++;
  handleMidiEventTick();
  if (resetFlag == true) {
    resetFlag = false;
    clockTickCount = 0;
    handleMidiEventStart();
    lastBarStartMilliSecond = currentMilliSecond;
  }

  if (clockTickCount % PPQN == 0) {
    bpm = 60000000 / (currentMilliSecond - lastBarStartMilliSecond);
    lastBarStartMilliSecond = currentMilliSecond;
      
    if (DEBUG) {
      Serial.print("Counter changed: ");
      Serial.print(clockTickCount);
      Serial.print("  BPM: ");
      Serial.println(bpm);
    }
  }
}


void isrClock() {
  clockTickSignal = (digitalRead(CLOCK_INPUT) == HIGH);
  clockFlag = true;
}

void isrReset() {
  resetFlag = true;
}

void handleMidiEventTick()
{
  tickCounter++;
  // do stuff thats only blazing baton related
  handleMidiEventTickBaton();
  if (tickCounter == 1)
  {
    lastBarStartMicros = micros();
  }
}


void handleMidiEventStart()
{
  clockRunning = true;
  tickCounter = 0;

  // do stuff thats only blazing baton related
  handleMidiEventStartBaton();

}

void handleMidiEventStop()
{
  clockRunning = false;
  tickCounter = 0;

  // do stuff thats only blazing baton related
  handleMidiEventStopBaton();
}
