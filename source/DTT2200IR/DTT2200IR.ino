#define LCD_TM1637
//#define LCD_74HC595
#define IR_RECEIVER

#include <button.h>
#include <EEPROM.h>
#ifdef LCD_74HC595
#include <lcd.h>
#endif
#ifdef LCD_TM1637
#include <TM1637.h>
#endif
#ifdef IR_RECEIVER
#include <IRremote.h>
#endif

#define __DEBUG__


#define SERIAL_BAUDRATE 9600

// PINTOUT ASSIGNMENT
// TO MAIN BOARD
int VOLUMEN_UP_DOWN     = 6;   
int VOLUMEN_INCREMENT   = 3;   
int VOLUMEN_CHIP_SELECT = -1; 
int VOL_UP_BUTTON       = 8;
int VOL_DOWN_BUTTON     = -1;
int MUTE_BUTTON         = -1;
// TO RELAY BOARD OR MAIN BOARD
int POWER_SWITCH_PIN    = -1;
// TO SENSOR OR MAIN BOARD
int IR_RECV_PIN         = 7;
// LCD BORAD
int LCD_UPDATE_PIN      = -1;
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
unsigned long VOL_UP_CODE   = 0xB1641F6E; //0x68733A46;
unsigned long VOL_DOWN_CODE = 0xCCA2788E; //0x83B19366;
unsigned long MUTE_CODE     = 0x6C319E4A; //0x2340B922;
unsigned long POWER_CODE    = 0x3DAB0EB0; //0xF4BA2988;
//unsigned long IR_CODES[]    = {VOL_UP_CODE, VOL_DOWN_CODE, MUTE_CODE, 0};


int     volume_min = 1;
int     volume_max = 100;
long    volumeMemAddress = 10;
long    muteMemAddress   = 11;

// Internal status
int     volume = 0;
boolean mute   = true;
#ifdef IR_RECEIVER
decode_results results;
decode_results last_results;
#endif
unsigned long timerMillis;
unsigned long ledTimerMillis;
int     lastMemVolume = 0;
boolean lastMemMute   = 0;
int     lastRheostateVolume = 0;
int     rheostateChipSelect = LOW;
unsigned long rheostatePulseWidth = 2;
unsigned long rheostateDelayBetweenPulses = 2;


// Devices
#ifdef IR_RECEIVER
IRrecv irrecv(IR_RECV_PIN);
#endif
#ifdef LCD_74HC595
Lcd lcd(LCD_UPDATE_PIN,LCD_CLOCK_PIN,LCD_SERIAL_PIN);
#endif
#ifdef LCD_TM1637
TM1637 lcd(LCD_CLOCK_PIN, LCD_SERIAL_PIN);
#endif

Button volUpButton(VOL_UP_BUTTON, volUpButtonPressed, DEBOUNCE_DELAY);
Button volDownButton(VOL_DOWN_BUTTON, volDownButtonPressed, DEBOUNCE_DELAY);
Button muteButton(MUTE_BUTTON, muteButtonPressed, DEBOUNCE_DELAY);

void setup()
{
//  if (POWER_SWITCH_PIN >= 0) {
//    pinMode(POWER_SWITCH_PIN   , OUTPUT);
//  }

#ifdef __DEBUG__  
  Serial.begin(SERIAL_BAUDRATE);
  Serial.println("DTT2200IR");
#endif
  restoreStatus();
  initRheostate();
  initLED();
#ifdef IR_RECEIVER
  irrecv.enableIRIn(); // Start the receiver
#endif
  initLCD();
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
 
void initLCD() {

#ifdef LCD_TM1637
  lcd.init();
  lcd.set(BRIGHT_TYPICAL);//BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
#endif
  sendStatusToLCD();
  timerStart();
}
 
void sendStatusToLCD() {
  if (mute) {
#ifdef __DEBUG__
    Serial.println("mute");
#endif
#ifdef LCD_74HC595
  lcd.sendCharString(muteMessage, true);
#endif
#ifdef LCD_TM1637
 lcd.display(0,0x10);
 lcd.display(1,0x00); 
 lcd.display(2,0x0F);
 lcd.display(3,0x0F);
#endif


  } else {
#ifdef __DEBUG__
    Serial.println(volume);
#endif

#ifdef LCD_74HC595
  lcd.sendDigit(volume, digit_num, true);
#endif
#ifdef LCD_TM1637
  int vol=volume;
  for (int i=0; i< 4; i++) {
    if (vol==0 && i > 0) {
      lcd.display(3-i, 0x10);
    } else {
      lcd.display(3-i,vol % 10);
      vol=vol/10;
    }
 }
#endif

  }
}

void clearLCD () {
#ifdef LCD_74HC595
  lcd.sendCharString(clearMessage);
#endif
#ifdef LCD_TM1637
  lcd.clearDisplay();
#endif
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
  digitalWrite(VOLUMEN_CHIP_SELECT, rheostateChipSelect);
  for (int i=0; i < volume_max; i++) {
    if (i > 0 && rheostateDelayBetweenPulses > 0) {
      delay(rheostateDelayBetweenPulses);
    }  
    sendDownToRheostate();  
  }
  lastRheostateVolume = 0;
  sendStatusToRheostate();
}

void sendUpToRheostate() {
#ifdef __DEBUG__  
  Serial.println("----Send up to rheostate");
#endif

  digitalWrite(VOLUMEN_INCREMENT, HIGH);
  digitalWrite(VOLUMEN_UP_DOWN, HIGH);
  if (rheostatePulseWidth > 0) {
    delay(rheostatePulseWidth);
  }
  digitalWrite(VOLUMEN_INCREMENT, LOW);
}

void sendDownToRheostate() {
#ifdef __DEBUG__  
  Serial.println("----Send down to rheostate");
#endif
  digitalWrite(VOLUMEN_INCREMENT, HIGH);
  digitalWrite(VOLUMEN_UP_DOWN, LOW);
    if (rheostatePulseWidth > 0) {
    delay(rheostatePulseWidth);
  }
  digitalWrite(VOLUMEN_INCREMENT, LOW);
}

void sendStatusToRheostate() {
#ifdef __DEBUG__  
  Serial.println("--Start send to rheostate");
#endif
  int volumeToSend = volume;
  if (mute) {
    volumeToSend = 0;
    for (int i = 0; i < volume_max; i++) {
      if (i > 0 && rheostateDelayBetweenPulses > 0) {
       delay(rheostateDelayBetweenPulses);
      }
      sendDownToRheostate();
    }
  } else {
    if (lastRheostateVolume > volumeToSend) {
      int steps = lastRheostateVolume - volumeToSend;
      for (int i = 0; i < steps; i++) {
        if (i > 0 && rheostateDelayBetweenPulses > 0) {
         delay(rheostateDelayBetweenPulses);
        }        
        sendDownToRheostate(); 
      }
    } else if (lastRheostateVolume < volumeToSend) {
      int steps = volumeToSend - lastRheostateVolume;
      for (int i = 0; i < steps; i++) {
      if (i > 0 && rheostateDelayBetweenPulses > 0) {
       delay(rheostateDelayBetweenPulses);
      }
        sendUpToRheostate();
      }
    }
  }
  lastRheostateVolume = volumeToSend;
#ifdef __DEBUG__  
  Serial.println("--End send to rheostate");
#endif

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
//#ifdef LCD_TM1637
//  lcd.point(POINT_ON);
//#endif
  ledTimerMillis = millis() + ledDuration;
}

void turnOffLED () {
  digitalWrite(TEST_PIN, LOW);
//#ifdef LCD_TM1637
//  lcd.point(POINT_OFF);
//#endif
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
#ifdef IR_RECEIVER
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
#endif
}

