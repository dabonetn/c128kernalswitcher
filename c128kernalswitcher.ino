#include <EEPROM.h>

// C64 Kernel Switcher and Restore Key Reset/Selector
// version 1.1 -- 26-March-2019
// By Adrian Black

// restore key mod: https://www.breadbox64.com/blog/c64-restore-mod/
//
// C128 Changes by Mark Ormond aka dabone
// 06-22-2020
//


const int PowerLED = 4;      // Output Power LED
const int PowerLEDAlt = 13;  // Output Power LED (onboard LED)
const int A15 = 5;           // Output EPROM Address Line 13
const int A14 = 6;           // Output EPROM Address Line 14
const int ResetLine = 7;     // Output to /RESET line
const int EXROMLine = 3;     // Output the /EXROM line
const int RestoreKey = 8;    // Input Restore key
const int U35 = 9;           // 128 Mode Kernal Line

int RestoreDelay = 2000;  // 2000ms delay for restore key
const int FlashSpeed = 75; // LED Flash delay

const unsigned long repeatdelay = 500; // used for debouncing
 
int CurrentROM; // which rom is select (0-3)
int debouncecounter = 0;       // how many times we have seen new value (for debounce)
int debouncereading;
int debounce_count;
int RestoreHeld;
unsigned long TimeHeld; // amount of time Resotre is held down

int buttonDuration = 0; // for keeping track of how long restore is held down
boolean buttonHeld = 0; // for keeping track when you are holding down 
boolean Released = 0; // Keeping track when the restore key is released
boolean holdingRestore = 0; // Keeping track if you are holding restore
boolean resetSystem = 0; // keep track wheter to reset

int buttonInput; // used to return if restore is held
unsigned long time; //used to keep track of millis output
unsigned long htime; //used to keep track of millis output
unsigned long btime; //used to keep track of bounce millis output

void setup() {
  pinMode(PowerLED, OUTPUT);
  pinMode(PowerLEDAlt, OUTPUT);
  pinMode(A15, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(U35, OUTPUT);
  pinMode(ResetLine, INPUT);
  pinMode(EXROMLine, INPUT);
  pinMode(RestoreKey, INPUT);

  digitalWrite(PowerLED, HIGH); // turn on the power LED

  digitalWrite(ResetLine, LOW); // keep the system reset
  pinMode(ResetLine, OUTPUT); // switch reset line to OUTPUT so it can hold it low
  digitalWrite(ResetLine, LOW); // keep the system reset

  CurrentROM = EEPROM.read(1);
  SetSlot(CurrentROM);
  delay(200);
  pinMode(ResetLine, INPUT); // set the reset pin back to high impedance which releases the INPUT line
  delay(1000); // wait 1000ms 
  FlashLED(CurrentROM);  // flash the power LED to show the current state
  
  // all set!
}

void loop() {
  buttonInput = readButton(); delay(500);
  time = millis(); // load the number of milliseconds the arduino has been running into variable time
  if (buttonInput == 1) {
    if (!buttonHeld) {
      htime = time; TimeHeld = 0; buttonHeld = 1; } //restore button is pushed
    else { 
      TimeHeld = time - htime; } // button is being held down, keep track of total time held.
  }
  if (buttonInput == 0) {
    if (buttonHeld) {
      Released = 1; buttonHeld = 0; htime = millis(); TimeHeld = 0; //restore button not being held anymore
    } 
  }
  
  if (TimeHeld > RestoreDelay && !Released) { // do this when the time the button is held is longer than the delay and the button is released
    htime = millis();
    if (holdingRestore == 0) { FlashLED(CurrentROM); holdingRestore = 1; resetSystem = 1; } // first time this is run, so flash the LED with current slot and reset time held. Set the holding restore variable.
    else {
      if (CurrentROM < 3) { CurrentROM++; SaveSlot(CurrentROM); } // or you've already been holding restore, so increment the current ROM slot otherwise reset it to 0
      else { CurrentROM = 0; SaveSlot(CurrentROM); }
      if (TimeHeld > RestoreDelay) { TimeHeld = 0;}  // reset the time held
      FlashLED(CurrentROM); //flash the LED
    }
  }
  
  if (Released) {
    //if time held greater than restore delay, reset the system, set the current rom slot, reselt the time held and holding restore
    if (resetSystem) { // on do this if the reset system has been set above
      htime = millis();
      resetSystem = 0;
      holdingRestore = 0;
      Released = 0;
      digitalWrite(ResetLine, LOW); // keep the system reset
      digitalWrite(EXROMLine, LOW); // keep the EXROM line low
      pinMode(ResetLine, OUTPUT);
      pinMode(EXROMLine, OUTPUT);
      digitalWrite(ResetLine, LOW); // keep the system reset
      digitalWrite(EXROMLine, LOW); // keep the EXROM line low
      delay(50); // wait 50ms
      SetSlot(CurrentROM); // select the appropriate kernal ROM
      delay(200); // wait 200ms before releasing RESET line
      pinMode(ResetLine, INPUT); // set the reset pin back to high impedance so computer boots
      delay(300); // wait 300ms before releasing EXROM line
      pinMode(EXROMLine, INPUT); // set the reset pin back to high impedance so computer boots
    } else { //otherwise do nothing
      htime = millis(); Released = 0; resetSystem = 0; holdingRestore = 0;
    }
  }
// finished with loop  
}

int readButton() {
 if (!digitalRead(RestoreKey) && (millis() - btime >= repeatdelay)) {
  for(int i = 0; i < 10; i++)
    {
      debouncereading = !digitalRead(RestoreKey);

      if(!debouncereading && debouncecounter > 0)
      {
        debouncecounter--;
      }
      if(debouncereading)
      {
        debouncecounter++; 
      }
      // If the Input has shown the same value for long enough let's switch it
      if(debouncecounter >= debounce_count)
      {
        btime = millis();
        debouncecounter = 0;
        RestoreHeld = 1;
      }
    delay (10); // wait 10ms
    }
   } else {
    RestoreHeld = 0;
   }
return RestoreHeld;
}


void SaveSlot(int CurrentRomSlot) {
  // Save Current ROM selection (0-3) into EPROM
  EEPROM.write(1,CurrentRomSlot);
}

void FlashLED(int flashcount) {
    // Flash the LED to represent which ROM slot is selected
    switch (flashcount) {
    case 0:
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      break;
    case 1:
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      break;
    case 2:
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      break;
    case 3:
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      delay(FlashSpeed);
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      break;
    default:
      digitalWrite(PowerLED, LOW);
      digitalWrite(PowerLEDAlt, LOW);
      delay(FlashSpeed);
      digitalWrite(PowerLED, HIGH);
      digitalWrite(PowerLEDAlt, HIGH);
      break;
  }
}

void SetSlot(int DesiredRomSlot) {
    // Select the actual ROM slot being used
    switch (DesiredRomSlot) {
    //Stock-Kernal0
    case 0:
      digitalWrite(A15, LOW);
      digitalWrite(A14, LOW);
      digitalWrite(U35, LOW);
      break;
    //Kernal1
    case 1:
      digitalWrite(A15, HIGH);
      digitalWrite(A14, LOW);
      digitalWrite(U35, HIGH);
      break;
    //Kernal2
    case 2:
      digitalWrite(A15, LOW);
      digitalWrite(A14, HIGH);
      digitalWrite(U35, HIGH);
      break;
    //Kernal3  
    case 3:
      digitalWrite(A15, HIGH);
      digitalWrite(A14, HIGH);
      digitalWrite(U35, HIGH);
      break;
    default:
      digitalWrite(A15, LOW);
      digitalWrite(A14, LOW);
      digitalWrite(U35, LOW);
      break;
  }
}
