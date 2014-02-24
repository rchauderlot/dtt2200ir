#include <IRremote.h>
#include <lcd.h>
#include <button.h>
#include <EEPROM.h>

//#define __DEBUG__

// PINTOUT ASSIGNMENT
// TO MAIN BOARD
int VOLUMEN_UP_DOWN     = 2;   
int VOLUMEN_INCREMENT   = 3;   
int VOLUMEN_CHIP_SELECT = 4; 
int VOL_UP_BUTTON       = 5;
int VOL_DOWN_BUTTON     = 6;
int MUTE_BUTTON         = 7;
// TO RELAY BOARD OR MAIN BOARD
int POWER_SWITCH_PIN    = -1;
// TO SENSOR OR MAIN BOARD
int IR_RECV_PIN         = 8;
// LCD BORAD
int LCD_UPDATE_PIN      = 10;
int LCD_CLOCK_PIN       = 11;
int LCD_SERIAL_PIN      = 12;
// INTERNAL PIN
int TEST_PIN            = 13;

// Buttons conf
int DEBOUNCE_DELAY       = 20;
// LCD conf
int digit_num = 4;
char muteMessage[]  = " off";
char clearMessage[] = "    ";
unsigned long messageDuration = 3000;
unsigned long ledDuration = 100;

// IR Code Table
unsigned long VOL_UP_CODE   = 0x68733A46;
unsigned long VOL_DOWN_CODE = 0x83B19366;
unsigned long MUTE_CODE     = 0x6C838E06;
unsigned long POWER_CODE    = 0xF4BA2988;
unsigned long IR_CODES[]    = {VOL_UP_CODE, VOL_DOWN_CODE, MUTE_CODE, 0};


int     volume_min = 1;
int     volume_max = 100;
long    volumeMemAddress = 10;
long    muteMemAddress   = 11;

// Internal status
int     volume = 0;
boolean mute   = true;
decode_results results;
decode_results last_results;
unsigned long timerMillis;
unsigned long ledTimerMillis;
int     lastMemVolume = 0;
boolean lastMemMute   = 0;
int     lastRheostateVolume = 0;



// Devices
IRrecv irrecv(IR_RECV_PIN);
Lcd lcd(LCD_UPDATE_PIN,LCD_CLOCK_PIN,LCD_SERIAL_PIN);
Button volUpButton(VOL_UP_BUTTON, volUpButtonPressed, DEBOUNCE_DELAY);
Button volDownButton(VOL_DOWN_BUTTON, volDownButtonPressed, DEBOUNCE_DELAY);
Button muteButton(MUTE_BUTTON, muteButtonPressed, DEBOUNCE_DELAY);

void setup()
{
//  if (POWER_SWITCH_PIN >= 0) {
//    pinMode(POWER_SWITCH_PIN   , OUTPUT);
//  }

#ifdef __DEBUG__  
  Serial.begin(9600);
  Serial.println("DTT2200IR");
#endif
  restoreStatus();
  initRheostate();
  initLED();
  irrecv.enableIRIn(); // Start the receiver
  sendStatusToLCD();
}


/**
 * Actions
 */
void upDetected () {
  if (mute && volume >= volume_min) {
    mute = false;
  } else {
    if (volume < volume_max) {
      volume ++;
      mute = false;
    }
  }
  statusChanged();
}

void downDetected () {
  if (mute && volume >= volume_min) {
    mute = false;
  } else {
    if (volume > volume_min) {
      volume --;
    } else {
      volume = volume_min - 1;
      mute = true;
    }
  }
  statusChanged();
}

void muteDetected () {
  mute = !mute;
  if (volume < volume_min) {
    volume = volume_min;
  }
  statusChanged();
}

void statusChanged() {
  sendStatusToRheostate();
  sendStatusToLCD();
  lightLED();
  timerStart();
}


/**
 * LCD functions
 */
void sendStatusToLCD() {
  if (mute) {
#ifdef __DEBUG__
    Serial.println("mute");
#endif
    lcd.sendCharString(muteMessage, true);
  } else {
#ifdef __DEBUG__
    Serial.println(volume);
#endif
    lcd.sendDigit(volume, digit_num, true);
  }
}

void clearLCD () {
  lcd.sendCharString(clearMessage);
}


/**
 * Timer functions
 */
void timerStart() {
  timerMillis = millis() + messageDuration;
}

void timeOut() {
  if (!mute) {
    clearLCD();
  }
  saveStatus();
}

void timerUpdate() {
  if (millis() > timerMillis && timerMillis > 0) {
    timerMillis = 0;
    timeOut();
  }
}

/**
 * Buttons functions
 */
void volUpButtonPressed() {
#ifdef __DEBUG__
  Serial.println("up pressed");
#endif
  upDetected();
}

void volDownButtonPressed() {
#ifdef __DEBUG__
  Serial.println("down pressed");
#endif
  downDetected();
}

void muteButtonPressed() {
#ifdef __DEBUG__
  Serial.println("mute pressed");
#endif
  muteDetected();
}


/**
 * Digital rheostate functions
 */
void initRheostate() {
    if (VOLUMEN_UP_DOWN >= 0) {
    pinMode(VOLUMEN_UP_DOWN    , OUTPUT);   
  }
  if (VOLUMEN_INCREMENT >= 0) {
    pinMode(VOLUMEN_INCREMENT  , OUTPUT);   
  }
  if (VOLUMEN_CHIP_SELECT >= 0) {
    pinMode(VOLUMEN_CHIP_SELECT, OUTPUT);
  }
  digitalWrite(VOLUMEN_CHIP_SELECT, LOW);
  for (int i=0; i < 100; i++) {
      sendDownToRheostate();
  }
  lastRheostateVolume = 0;
  sendStatusToRheostate();
}

void sendUpToRheostate() {
  digitalWrite(VOLUMEN_INCREMENT, HIGH);
  digitalWrite(VOLUMEN_UP_DOWN, HIGH);
  digitalWrite(VOLUMEN_INCREMENT, LOW);
}

void sendDownToRheostate() {
  digitalWrite(VOLUMEN_INCREMENT, HIGH);
  digitalWrite(VOLUMEN_UP_DOWN, LOW);
  digitalWrite(VOLUMEN_INCREMENT, LOW);
}

void sendStatusToRheostate() {
  int volumeToSend = volume;
  if (mute) {
    volumeToSend = 0;
  }
  if (lastRheostateVolume > volumeToSend) {
    int steps = lastRheostateVolume - volumeToSend;
    for (int i = 0; i < steps; i++) {
      sendDownToRheostate();
    }
  } else if (lastRheostateVolume < volumeToSend) {
    int steps = volumeToSend - lastRheostateVolume;
    for (int i = 0; i < steps; i++) {
      sendUpToRheostate();
    }
  }
  lastRheostateVolume = volumeToSend;
}

/**
 * LED functions
 */
void initLED () {
  if (TEST_PIN >= 0) {
    pinMode(TEST_PIN           , OUTPUT);
  }
}

void lightLED () {
  digitalWrite(TEST_PIN, HIGH);
  ledTimerMillis = millis() + ledDuration;
}

void turnOffLED () {
  digitalWrite(TEST_PIN, LOW);
}

void updateLED () {
  if (millis() > ledTimerMillis && ledTimerMillis > 0) {
    ledTimerMillis = 0;
    turnOffLED();
  }
}

/**
 * Persistence functions
 */
void saveStatus () {
  if (volume != lastMemVolume || mute != lastMemMute) {
#ifdef __DEBUG__  
  Serial.println("Saving new status");
#endif
    EEPROM.write(volumeMemAddress, volume);
    EEPROM.write(muteMemAddress, mute);
    lastMemVolume = volume;
    lastMemMute = mute;    
  }
}

void restoreStatus () {
    int storedVolume = EEPROM.read(volumeMemAddress);
    int storedMute = EEPROM.read(muteMemAddress);
    if (storedVolume > volume_max || storedVolume < volume_min - 1 || storedMute > 1 || storedMute < 0 ) {
#ifdef __DEBUG__  
      Serial.println("Status not restored properly defaulting to muted");
#endif
      volume = 0;
      mute = 1;
    } else {
#ifdef __DEBUG__  
      Serial.println("Status restored");
#endif
      volume = storedVolume;
      mute = storedMute;
    }
    lastMemVolume = volume;
    lastMemMute = mute;    
}



/**
 * Main loop
 */
void loop() {
  timerUpdate();
  updateLED();
  volUpButton.update();
  volDownButton.update();
  muteButton.update();
  if (irrecv.decode(&results)) {
    if (last_results.decode_type == results.decode_type && last_results.value == results.value) {

#ifdef __DEBUG__  
      Serial.println(results.value, HEX);
#endif
      if (last_results.value == VOL_UP_CODE) { // UP DETECTED
        upDetected();
      }
      if (last_results.value == VOL_DOWN_CODE) { // DOWN_DETECTED
        downDetected();
      }
      if (last_results.value == MUTE_CODE) { // MUTE DETECTED
        muteDetected();
      }
//      if (last_results.value == POWER_CODE) { // POWER_DETECTED
//      }
    }
    last_results = results;
    irrecv.resume(); // Receive the next value
  }
}

