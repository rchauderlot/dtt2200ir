#include <IRremote.h>
#include <lcd.h>
#include <button.h>

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
char muteMessage[]  = "MUTE";
char clearMessage[] = "    ";
unsigned long long messageDuration = 3*60;

// IR Code Table
unsigned long VOL_UP_CODE   = 0x68733A46;
unsigned long VOL_DOWN_CODE = 0x83B19366;
unsigned long MUTE_CODE     = 0x6C838E06;
unsigned long POWER_CODE    = 0xF4BA2988;
unsigned long IR_CODES[]    = {VOL_UP_CODE, VOL_DOWN_CODE, MUTE_CODE, 0};


// Internal status
int volume = 0;
boolean mute = 0;
decode_results results;
decode_results last_results;
unsigned long timerMillis;

// Devices
IRrecv irrecv(IR_RECV_PIN);
Lcd lcd(LCD_UPDATE_PIN,LCD_CLOCK_PIN,LCD_SERIAL_PIN);
Button volUpButton(VOL_UP_BUTTON, volUpButtonPressed, DEBOUNCE_DELAY);
Button volDownButton(VOL_DOWN_BUTTON, volDownButtonPressed, DEBOUNCE_DELAY);
Button muteButton(MUTE_BUTTON, muteButtonPressed, DEBOUNCE_DELAY);

void setup()
{
  if (VOLUMEN_UP_DOWN >= 0) {
    pinMode(VOLUMEN_UP_DOWN    , OUTPUT);   
  }
  if (VOLUMEN_INCREMENT >= 0) {
    pinMode(VOLUMEN_INCREMENT  , OUTPUT);   
  }
  if (VOLUMEN_CHIP_SELECT >= 0) {
    pinMode(VOLUMEN_CHIP_SELECT, OUTPUT);
  }
  if (POWER_SWITCH_PIN >= 0) {
    pinMode(POWER_SWITCH_PIN   , OUTPUT);
  }
  if (TEST_PIN >= 0) {
    pinMode(TEST_PIN           , OUTPUT);
  }

  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}


/**
 * Actions
 */
void upDetected () {
  if (mute) {
    mute = false;
  } else {
    volume ++;
  }
  sendProgressToLCD();
}

void downDetected () {
  if (mute) {
    mute = false;
  } else {
    volume --;
  }
}

void muteDetected () {
  mute != mute;
  sendProgressToLCD();
}

/**
 * LCD functions
 */
void sendProgressToLCD() {
  if (mute) {
    lcd.sendCharString(muteMessage);
  } else {
    lcd.sendDigit(volume, digit_num);
    timerStart();
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
  clearLCD();
}

void timerUpdate() {
  if (timerMillis > millis() / 1000) {
    timerMilis = 0;
    timeOut();
  }
}

/**
 * Buttons functions
 */
void volUpButtonPressed() {
  upDetected();
}

void volDownButtonPressed() {
  downDetected();
}

void muteButtonPressed() {
  muteDetected();
}


/**
 * Main loop
 */
void loop() {
  timerUpdate();
  volUpButton.update();
  volDownButton.update();
  muteButton.update();
  if (irrecv.decode(&results)) {
    if (last_results.decode_type == results.decode_type && last_results.value == results.value) {
      Serial.println(results.value, HEX);

      if (last_results.value == VOL_UP_CODE) { // UP DETECTED
        digitalWrite(TEST_PIN, HIGH);
        upDetected();
      }
      if (last_results.value == VOL_DOWN_CODE) { // DOWN_DETECTED
        digitalWrite(TEST_PIN, HIGH);
        downDetected();
      }
      if (last_results.value == MUTE_CODE) { // MUTE DETECTED
        digitalWrite(TEST_PIN, HIGH);
        muteDetected();
      }
//      if (last_results.value == POWER_CODE) { // POWER_DETECTED
//        digitalWrite(TEST_PIN, HIGH);
//      }
      delay(100);
      digitalWrite(TEST_PIN, LOW);
    }
    last_results = results;
    irrecv.resume(); // Receive the next value
  }
}

