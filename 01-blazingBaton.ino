/**
 * it seems to be better to use an 6N138 instead of 4N35 opto coupler
 * @see: http://sandsoftwaresound.net/wp-content/uploads/2016/05/schematic_midi_in.jpg
 *
 */
#include <Adafruit_NeoPixel.h>

#define PINLEDSTRIP 7 /** input pin Neopixel is attached to */
#define NUMPIXELS 16  /** number of neopixels in unit (1 chain) */
#define PIXELCHAINS 1 /** for having multiple ledstrips within one chain */


Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS * PIXELCHAINS, PINLEDSTRIP, NEO_GRB + NEO_KHZ800);

// those variables are for "05-midiEvents.ino"
unsigned long lastBarStartMicros = 0;     // microsecond of last bar first tick (@24 ppqm 1, 97, 193, ) to calculate tempo or tickWidth
unsigned long lastBarStartTickNumber = 0; // the most recent start tick of bar whichs millisecond is stored in lastBarStartMicros

const int countDownToQuarterNote = 64; /** end of countdown [quarter note] (should be a multiple of NUMPIXELS) */

/** TODO: remove unused color vars */
const uint32_t colorWhite = pixels.Color(255, 255, 255);
const uint32_t colorRed = pixels.Color(255, 0, 0);
const uint32_t colorGreen = pixels.Color(0, 255, 0);
const uint32_t colorBlue = pixels.Color(0, 0, 255);
const uint32_t colorCyan = pixels.Color(0, 255, 255);
const uint32_t colorMagenta = pixels.Color(255, 0, 255);
const uint32_t colorYellow = pixels.Color(255, 255, 0);

const uint32_t colorDefault = colorMagenta;
const uint32_t colorCountDown = colorBlue;
const uint32_t colorSectionIndex = colorCyan;
// const uint32_t colorNoSignal = colorRed;

const uint32_t colorDefaultForSourceSerial = colorMagenta;
const uint32_t colorCountDownForSourceSerial  = colorBlue;
const uint32_t colorSectionIndexForSourceSerial  = colorCyan;

const uint32_t colorDefaultForSourceRfHost = colorMagenta;
const uint32_t colorCountDownForSourceRfHost  = colorBlue;
const uint32_t colorSectionIndexForSourceRfHost  = colorYellow;

const uint32_t colorDefaultForSourceNone = colorRed;
const uint32_t colorCountDownForSourceNone  = colorWhite;
const uint32_t colorSectionIndexForSourceNone  = colorWhite;



/** some helper vars that does not change during runtime */
const int ppqn = 4;                                /** pulses per quarter note */
const int maxTicks = ppqn * countDownToQuarterNote; /** when to reset tick counter to zero */
const int countDownStartTick = ppqn * (countDownToQuarterNote - 4);
const int NUMSTATES = NUMPIXELS + 1;
const int numSections = countDownToQuarterNote / NUMPIXELS; /** amount of loops before restart */

/** some helper vars that do change during runtime */

uint32_t tickCounterBatonLoop = 0; // incremental counter since clock-start-event
int insideQuarterNoteCounter = 0;  /** on quarter note start */
int currentSection = 1;            /** loop index (starting from 1) */
int currentStepLedIndex = 0;       /** index of last highlighted led (starting from 0) */

// uint32_t idleColor = colorDefault;

uint32_t lastState[NUMSTATES];
uint32_t newState[NUMSTATES];


uint32_t getColorDefault() {
  if (currentDataSource == DATASOURCE_IS_RFHOST) {
    return colorDefaultForSourceRfHost;
  }
  if (currentDataSource == DATASOURCE_IS_SERIAL) {
    return colorDefaultForSourceSerial;
  }
  return colorDefaultForSourceNone;
}

uint32_t getColorCountDown() {
  if (currentDataSource == DATASOURCE_IS_RFHOST) {
    return colorCountDownForSourceRfHost;
  }
  if (currentDataSource == DATASOURCE_IS_SERIAL) {
    return colorCountDownForSourceSerial;
  }
  return colorCountDownForSourceNone;
}

uint32_t getColorSectionIndex() {
  if (currentDataSource == DATASOURCE_IS_RFHOST) {
    return colorSectionIndexForSourceRfHost;
  }
  if (currentDataSource == DATASOURCE_IS_SERIAL) {
    return colorSectionIndexForSourceSerial;
  }
  return colorSectionIndexForSourceNone;
}

void handleMidiEventStartBaton()
{
  tickCounterBatonLoop = tickCounter % maxTicks;
  insideQuarterNoteCounter = (tickCounterBatonLoop / ppqn) + 1;
  prepareNewLedStates();
  checkLedChange();
}

void handleMidiEventStopBaton()
{
  tickCounterBatonLoop = tickCounter % maxTicks;
  insideQuarterNoteCounter = (tickCounterBatonLoop / ppqn) + 1;
  prepareNewLedStates();
  checkLedChange();
}

// we cant use incremental operator as this script is used by host AND client
// sometims we (client) get the tickCounter set from outside
// so always refer to the tickCounter!
void handleMidiEventTickBaton()
{
  tickCounterBatonLoop = tickCounter % maxTicks;
  insideQuarterNoteCounter = (tickCounterBatonLoop / ppqn) + 1;
  currentSection = (tickCounterBatonLoop / (ppqn * NUMPIXELS)) + 1;
  prepareNewLedStates();
  checkLedChange();
}

/** this method does not trigger any led change but sets the stats in the newArray variable */
void prepareNewLedStates()
{
  if (clockRunning == false)
  {
    idleLedStates();
    return;
  }
  if (tickCounterBatonLoop < countDownStartTick)
  {
    standardLedStates();
    return;
  }
  countdownLedStates();
}

/**
 * increment colored leds by one on each quarter note change
 * and start from 1 again when we reach the last led
 */
void standardLedStates()
{
  currentStepLedIndex = 0;
  for (int i = 0; i < NUMPIXELS; i++)
  {
    newState[i] = 0;
    if (i + 1 <= insideQuarterNoteCounter % NUMPIXELS || insideQuarterNoteCounter % NUMPIXELS == 0)
    {
      newState[i] = getColorDefault();
      currentStepLedIndex = i;
    }
  }
  sectionIndexLedStates();
}

/**
 * perma lightning the last few leds depending on loop index before starting over again
 * but current step has a higher priority
 */
void sectionIndexLedStates()
{
  int ledIndexOfSection;
  for (int i = 0; i < currentSection; i++)
  {
    ledIndexOfSection = NUMPIXELS - numSections + i;
    if (ledIndexOfSection == currentStepLedIndex)
    {
      continue;
    }
    newState[ledIndexOfSection] = getColorSectionIndex();
  }
}

/**
 * clock is not running
 * alternating blink of 1st and 2nd led in quarter note interval
 * TODO: this does not work in case we do not receive clock ticks
 */
void idleLedStates()
{
  clearLedStates();
  if (insideQuarterNoteCounter % 2 == 0)
  {
    newState[0] = getColorDefault();
    return;
  }
  newState[1] = getColorDefault();
}

void checkLedChange()
{
  if (hasStateChanged() == false)
  {
    /** nothing to do without any changes within leds */
    return;
  }
  pixels.clear();
  for (int chainNum = 0; chainNum < NUMPIXELS; chainNum++)
  {
    for (int i = 0; i < NUMPIXELS; i++)
    {
      if (newState[i] == 0)
      {
        continue;
      }
      pixels.setPixelColor(chainNum * NUMPIXELS + i, newState[i]);
    }
  }
  pixels.setBrightness(newState[NUMPIXELS]);
  pixels.show();

  /** persist current led states in lastState array */
  rememberLastState();
}

bool hasStateChanged()
{
  for (int i = 0; i < NUMSTATES; i++)
  {
    if (lastState[i] != newState[i])
    {
      return true;
    }
  }
  return false;
}

/**
 * helper array to check if we need to apply any changes
 * too bad arduino does not support std::array<uint32_t,17> lastState = newState;
 */
void rememberLastState()
{
  for (int i = 0; i < NUMSTATES; i++)
  {
    lastState[i] = newState[i];
  }
}

/**
 * NOTE: led index arguments are starting from 1 (but array indices starts from zero)
 */
void batchColor(int ledIndexFrom, int ledIndexTo, uint32_t color)
{
  for (int i = ledIndexFrom; i <= ledIndexTo; i++)
  {
    newState[(i - 1)] = color;
  }
}

void clearLedStates()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    newState[i] = 0;
  }
}

/**
 * first <NUMPIXELS> values are led colors; last value is brightness
 */
void initStateArrays()
{
  for (int i = 0; i < NUMPIXELS; i++)
  {
    newState[i] = 0;
  }
  newState[NUMPIXELS] = 10;
  rememberLastState();
}

void setupBaton()
{
  initStateArrays();
  pixels.begin(); /** Initializes the NeoPixel library. */
}



/**
 * countdownLedStates()
 * this function handles the last 4 quarter notes of a 16 bar loop
 **********************************************
 *   3   X X X X  - - X X  X X - -  X X X X
 *   2   - - X X  X X - -  - - X X  X X - -
 *   1   - - - -  - - X X  X X - -  - - - -
 *  and  X X X X  X X X X  X X X X  X X X X
 **********************************************
 */
void countdownLedStates()
{
  const int flashDuration = 23; /** time in [ticks] before turning leds off */

  switch (tickCounterBatonLoop)
  {
  case countDownStartTick:
    clearLedStates();
    batchColor(1, 4, getColorCountDown());
    batchColor(7, 10, getColorCountDown());
    batchColor(13, 16, getColorCountDown());
    break;
  case countDownStartTick + ppqn:
    clearLedStates();
    batchColor(3, 6, getColorCountDown());
    batchColor(11, 14, getColorCountDown());
    break;
  case countDownStartTick + 2 * ppqn:
    clearLedStates();
    batchColor(7, 10, getColorCountDown());
    break;
  case countDownStartTick + 3 * ppqn:
    batchColor(1, 16, getColorCountDown());
    break;
  case countDownStartTick + flashDuration:
  case countDownStartTick + flashDuration + ppqn:
  case countDownStartTick + flashDuration + 2 * ppqn:
  case maxTicks - 1:
    clearLedStates();
    break;
  }
}
