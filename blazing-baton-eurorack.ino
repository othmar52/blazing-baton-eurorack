/*
 * TODO: cleanup 
 */

#define DEBUG 0

// those are needed in shared files from client
#define DATASOURCE_NONE 0
#define DATASOURCE_IS_SERIAL 1
#define DATASOURCE_IS_RFHOST 2

uint8_t currentDataSource = DATASOURCE_IS_SERIAL;
unsigned long lastMidiEventFromSerial = 0;

long tickCounter = 0; // incremental counter since clock-start-event
bool clockRunning = true;

void setup()
{
  Serial.begin(31250); // MIDI baudrate
  Serial.begin(115200);
  setupCvInputs();
  setupBaton();
  setupModClockOutputs();
}

void loop()
{
  // loopCvClockReset();
}

void debug(String Msg)
{
  if (DEBUG)
  {
    Serial.println(Msg);
  }
}
