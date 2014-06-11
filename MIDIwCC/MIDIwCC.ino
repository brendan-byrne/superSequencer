// ----------------------------------------------------------------------------------------------------------------
//    ____ _  _ ___  ____ ____    ____ ____ ____ _  _ ____ _  _ ____ ____    ____ _ ____ _  _ ___ ____ ____
//    [__  |  | |__] |___ |__/    [__  |___ |  | |  | |___ |\ | |    |___    |___ | | __ |__|  |  |___ |__/
//    ___] |__| |    |___ |  \    ___] |___ |_\| |__| |___ | \| |___ |___    |    | |__] |  |  |  |___ |  \
// ----------------------------------------------------------------------------------------------------------------                                                                                                    
// 

// ----- LIBRARIES -------------------------------------------------------------------------------------------------
#include <SPI.h>                        // Included in Arduino package. Needed to communicate with LED Matrix
#include <Adafruit_GFX.h>               // https://github.com/adafruit/Adafruit-GFX-Library
#include <Max72xxPanel.h>               // https://github.com/markruys/arduino-Max72xxPanel

// ----- VARIABLES -------------------------------------------------------------------------------------------------
// Pin Mapping
const int topSlideBank = A2;
const int botSlideBank = A0;                            // Connected to pin 3 of first CD4051
const int buttonBank = A1;                              // Connected to pin 3 of second CD4051
const int bitPins[] = {
  4, 5, 6};                     // Connected to pins 11, 10, and 9 of CD4051s. Pulldown resistors necessary.
const int matrixPin = 0;                             // Connected to pin 12 of Max7219 (CS). Also connect pin 1 of Teensy 2.0 (SCK) to pin 13 of Max2719 (CLK) and pin 2 of Teensy 2.0 (MOSI) to pin 1 of Max2719 (DIN)
const int dataPin = 13;                              // Connected to pin 14 of 74HC595
const int clockPin = 9;                              // Connected to pin 10 of 74HC595
const int latchPin = 10;                             // Connected to pin 11 of 74HC595
const int tempoPot = A3;                             // Connected to center pin rotary potentiometer
const int stepCountPot = A4;                            // Connected to center pin of slide potentiometer
const int runButton = 15;                            // Connected to tactile switch/pushbutton. Pulldown resistor necessary
const int revButton = 16;                            // Connected to tactile switch/pushbutton. Pulldown resistor necessary

// Slider Array Mapping + Value Holders
const int topSlideMux[] = {
  3, 7, 1, 5, 2, 4, 0, 6};
const int botSlideMux[] = {
  5, 7, 3, 1, 6, 0, 4, 2};
const int buttonMux[] = {
  1, 3, 7, 5, 2, 4, 0, 6};    // Awkward pin mapping to suit the PCB that was printed.
const int tolerance = 5;                            // Only accepted readings from potentiometers that are this much more or less than the previous reading
const int numChannels = 8;                           // The number of channels available on each multiplexer
int rawBotVals[numChannels];                       // Raw slider readings
int lastSlideVals[numChannels];                      // Stored value of last reading used to compare against new readings
int mappedSlideVals[numChannels];                    // Mapped value sent to allVals[]
int rawTopVals[numChannels];                         // Raw potentiometer readings
int lastPotVals[numChannels];                        // Stored value of last reading used to compare against new readings
int mappedPotVals[numChannels];                      // Mapped value sent to allVals[]
int topVals[8];                                     // All mapped analog readings stored in this array. Used to select from buttonCommands[] array
int botVals[8];    

// Button Management
const int numButtons = 2;                            // Number of pushbuttons directly wired to Teensy inputs
int buttonArray[] = {
  runButton, revButton};          // Array for running buttons through debounce for loop
boolean currentButtons[numButtons];                  // Current state of button presses
boolean lastButtons[numButtons];                     // Stored value of last reading to compare against new readings

// Shift Register and Max7219 Specific Variables
const int numberOfHorizontalDisplays = 1;            // Number of horizontal displays
const int numberOfVerticalDisplays = 2;              // Number of vertical displays
boolean registers[numChannels];                      // LED states stored within this array

// Step-Sequencer Variables
unsigned long changeTime;                            // Checks against TEMPO to see if long enough has passed to advance the sequencer one step
boolean runSeq = false;                              // Toggle sequencer on/off
boolean revSeq = false;                              // Toggle forward/reverse of sequence
float tempo;                                         // Time that passes between step changes
int stepPos;                                         // Stores current step of sequencer
int rawLength;                                       // Reading from potentiomer that controller length of sequence
int lastRawLength;                                   // Stored value of last reading to compare against new readings
int testReset;                                       // Resets conditional that turns button command off even if the sequence has stopped
int lastCommand;                                     // Stores value of last ON command. Used to turn off command at start of next step
int seqLength;                                       // Stores length of sequence

// Averaging Variables
const int numReadings = 15;                 //The number of readings taken that are later averaged together
int readingsTop[numChannels][numReadings];     //Two dimensional array for storing the readings to be averager per each analog channel
int readingsBot[numChannels][numReadings];
int totalTop[numChannels];                     //Running total to be averaged
int totalBot[numChannels];
int averageTop[numChannels];                   //Average value that will be used to do things with!
int averageBot[numChannels];
int tracker = 0;                              //Index to keep track of number of readings taken
int readingsTempo[numReadings];
int totalTempo;
int averageTempo;
int readingsLength[numReadings];
int totalLength;
int averageLength;
int lastBot[numChannels];
int lastTop[numChannels];

// led matrix calibration
int lowTop[] = {
  0, 73, 220, 366, 512, 658, 804, 950};
int highTop[] = {
  72, 219, 365, 511, 657, 803, 949, 1023};
int lowBot[] = {
  0, 102, 307, 512, 717, 922};
int highBot[] = {
  101, 306, 511, 716, 921, 1023};

// code variables
int selector = 0;

// Joy Commands
int buttonCommands[] = {
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10};           // Button commands to send to computer
int singleCommands[] = {
  0, 1, 2, 3, 4, 5, 8, 10, 12, 14};
int doubleCommands[] = {
  9, 11, 13, 15};
int doubleArray[] = {
  8, 10, 12, 14, 10, 12, 14, 8};

// ----- OBJECT DECLARATION ----------------------------------------------------------------------------------------
Max72xxPanel matrix = Max72xxPanel(matrixPin, numberOfHorizontalDisplays, numberOfVerticalDisplays);

// ----- SETUP -----------------------------------------------------------------------------------------------------
void setup() {
  for (int x=0; x<3; x++) pinMode(bitPins[x], OUTPUT);          //Initiate binary controllers as outputs
  pinMode(dataPin, OUTPUT);                                     // Set dataPin to output
  pinMode(clockPin, OUTPUT);                                    // Set clockPin to output
  pinMode(latchPin, OUTPUT);                                    // Set latchPin to output
  pinMode(runButton, INPUT);                                    // Set runButton as Input
  pinMode(revButton, INPUT);                                    // Set revButton as Input
  pinMode(tempoPot, INPUT);                                     // Set tempoPot as Input
  pinMode(stepCountPot, INPUT);                                    // Set lengthPot as Input
  pinMode(topSlideBank, INPUT);                                    // Set slideBank as Input (pin from multiplexer 1)
  pinMode(botSlideBank, INPUT);                                      // Set potBank as Input (pin from multiplexer 2)
  pinMode(buttonBank, INPUT);
  matrix.setIntensity(15);                                       // Set LED brightness of both matrix
  matrix.setRotation(0, 3);                                     // The first display is position upside down
  matrix.setRotation(1, 3);                                     // The same hold for the last display
}

// ----- LOOP ------------------------------------------------------------------------------------------------------
void loop() {
  clearMatrix();              // Clear LED Matrix
  slideReadings();            // Take readings from top and bottom slide banks
  buttonReadings();           // Take readings from start/stop and rev buttons
  tempoReading();             // Take reading from tempoPot
  lengthReading();            // Take readubg frin lengthPot
  valAssignment();            // Assign readings to LEDs and integers to be interpretted by sequencer

  // LED Steps + S
  if (runSeq == true) {
    if ((millis() - changeTime) > tempo) { 
      for (int x=7; x>=0; x--) registers[x] = LOW; 
      registers[stepPos] = HIGH;

      averageBot[stepPos] = map(averageBot[stepPos], 1023, 0, 0, 128);
      usbMIDI.sendControlChange(20, averageBot[stepPos], 1);
      
      switch(selector) {
      case 0: 
        midiMapA();
        break;
      }

      if (revSeq == false) ++stepPos;
      else if (revSeq == true) --stepPos;

      if (revSeq == false) {
        if (stepPos >= seqLength+1) stepPos = 0;
      }
      else if (revSeq == true) {
        if (stepPos <= -1) stepPos = seqLength;
      }
      changeTime = millis();
    }
    testReset = 1;
  }

  else if (testReset == 1) {
    switch(selector) {
    case 0: 
      midiMapB();
      break;
    }

  }  
 
  for (int x=0; x<numChannels; x++) {
    
    Serial.print(x);
    Serial.print("\t");
    Serial.println(averageBot[x]);
  }
 
  tracker += 1;                                                   //Advance index to next position
  if (tracker >= numReadings) tracker = 0; 
  writeRegisters();
  matrix.write();  
}
// ----- FUNCTIONS -------------------------------------------------------------------------------------------------
// MIDI
void midiMapA() {
  if (testReset == 1) {
    usbMIDI.sendNoteOff(lastCommand+2, 0, 1);
  }
  lastCommand = topVals[stepPos];  
  usbMIDI.sendNoteOn(lastCommand+50, 90, 1);
}

void midiMapB() {
  usbMIDI.sendNoteOff(lastCommand+2, 0, 1);
  testReset = 0;
}

// Read Multiplexer
int readMux(int readPin, int channel) {
  for (int x=0; x<3; x++) {
    digitalWrite(bitPins[x], bitRead(channel, x));  //bitRead: my new favorite function
  }                                                 //all pins on teensy below 11 are digital. this is a "rough" part of the code. could make it more specific or make separate functions for digi/analog readings.
  return analogRead(readPin);                         
}

// Write Values to Registers                                             //http://bildr.org/2011/02/74hc595/
void writeRegisters() {
  digitalWrite(latchPin, LOW);
  for(int i=7; i>=0; i--){
    digitalWrite(clockPin, LOW);
    digitalWrite(dataPin, registers[i]);
    digitalWrite(clockPin, HIGH);
  }
  digitalWrite(latchPin, HIGH);
}

// Clear Matrix
void clearMatrix() {
  for (int x=0; x<8; x++) {
    for (int y=0; y<16; y++) {
      matrix.drawPixel(x, y, LOW);
    }
  }
}

// Top Average
void avgTop(int readStep) {
  totalTop[readStep] = totalTop[readStep] - readingsTop[readStep][tracker];                   //Subtract last reading at this index. Otherwise the total would just keep getting bigger!           
  readingsTop[readStep][tracker] = readMux(topSlideBank, topSlideMux[readStep]);               //Read analog input 1-8 and assign to specific index
  totalTop[readStep] += readingsTop[readStep][tracker];                             //Add that index to the total
  averageTop[readStep] = (totalTop[readStep] / numReadings); 
}

// Bottom Average
void avgBot(int readStep) {
  totalBot[readStep] = totalBot[readStep] - readingsBot[readStep][tracker];                   //Subtract last reading at this index. Otherwise the total would just keep getting bigger!           
  readingsBot[readStep][tracker] = readMux(botSlideBank, botSlideMux[readStep]);               //Read analog input 1-8 and assign to specific index
  totalBot[readStep] += readingsBot[readStep][tracker];                             //Add that index to the total
  averageBot[readStep] = (totalBot[readStep] / numReadings); 
  averageBot[readStep] = map(averageBot[readStep], 0, 1023, 1023, 0);
}

// Both Averages
void slideReadings() {
  for (int x=0; x<numChannels; x++) {
    avgTop(x);
    avgBot(x);         
  }
}

void buttonReadings() {
  for (int x=0; x<2; x++) {
    currentButtons[x] = digitalRead(buttonArray[x]);                         // Read buttons
    if (currentButtons[0] == 1 && lastButtons[0] == 0) runSeq = !runSeq;   // Toggle runSeq (initally false)
    if (currentButtons[1] == 1 && lastButtons[1] == 0) revSeq = !revSeq;
    lastButtons[x] = currentButtons[x];                                    // Set lastButton to currentButton so that the if statement doesn't keep running as true
  }
}

void tempoReading() {
  int fastest = 1;
  int slowest = 200;
  totalTempo = totalTempo - readingsTempo[tracker];                   //Subtract last reading at this index. Otherwise the total would just keep getting bigger!           
  readingsTempo[tracker] = analogRead(tempoPot);               //Read analog input 1-8 and assign to specific index
  totalTempo += readingsTempo[tracker];                             //Add that index to the total
  averageTempo = (totalTempo / numReadings);  
  tempo = averageTempo;  
  tempo = (tempo-0) * (1.0-0.0) / (1023-0) + 0.0; 
  tempo = pow(tempo, 2);                                                     // Scale the range of readings. Feels better.
  tempo = int((tempo-0.0) * (slowest-fastest) / (1.0-0.0) + fastest);                      // Custom Map function for floats : (x-in_min) * (out_max-out_min) / (in_max-in_min) + out_min;
  tempo = map(tempo, fastest, slowest, slowest, fastest);
}

void lengthReading() {
  totalLength = totalLength - readingsLength[tracker];                   //Subtract last reading at this index. Otherwise the total would just keep getting bigger!           
  readingsLength[tracker] =   analogRead(stepCountPot);               //Read analog input 1-8 and assign to specific index
  totalLength += readingsLength[tracker];                             //Add that index to the total
  averageLength = (totalLength / numReadings);
  if (averageLength > lastRawLength + tolerance || averageLength < lastRawLength - tolerance) {   // If new reading is greater than or less than lastReading + tolerance
    lastRawLength = averageLength;                                                          // Set the last reading to the new reading
    seqLength = averageLength/128;                                                          // Map the reading to 0-7
  }  
}

void valAssignment() {
  for (int y=0; y<numChannels; y++) {
    for (int x=0; x<6; x++) {
      if (averageBot[y] > lowBot[x] && averageBot[y] < highBot[x]) {
        botVals[y] = x;
      }
      for (int i=5; i>=botVals[y]; i--) {
        matrix.drawPixel(y, i, HIGH);
      }
    }
    if (averageTop[y] > lastTop[y] + tolerance || averageTop[y] < lastTop[y] - tolerance) {
      for (int x=0; x<8; x++) {
        if (averageTop[y] > lowTop[x] && averageTop[y] < highTop[x]) {
          topVals[y] = x+8;
        }
      }
      lastTop[y] = averageTop[y];
    }
  matrix.drawPixel(y, topVals[y], HIGH);    
  }
}












