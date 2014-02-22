#include <IRremote.h>


// PINTOUT ASSIGNMENT
// TO MAIN BOARD
int VOLUMEN_UP_DOWN = 2;   
int VOLUMEN_INCREMENT = 3;   
int VOLUMEN_CHIP_SELECT = 4; 
int VOL_UP_BUTTON = 5;
int VOL_DOWN_BUTTON = 6;
int MUTE_BUTTON = 7;
// TO RELAY BOARD OR MAIN BOARD
int POWER_SWITCH_PIN = 8;
// TO SENSOR OR MAIN BOARD
int IR_RECV_PIN = 9;
// LCD BORAD
int LCD_UPDATE_PIN = 10;
int LCD_CLOCK_PIN  = 11;
int LCD_SERIAL_PIN = 12;
// INTERNAL PIN
int TEST_PIN = 13;


int volume = 0;
boolean mute = 0;


unsigned long VOL_UP_CODE   = 0x68733A46;
unsigned long VOL_DOWN_CODE = 0x83B19366;
unsigned long POWER_CODE    = 0xF4BA2988;

decode_results results;
decode_results last_results;

IRrecv irrecv(IR_RECV_PIN);

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
  if (VOL_UP_BUTTON >= 0) {
    pinMode(VOL_UP_BUTTON      , INPUT);
  }
  if (VOL_DOWN_BUTTON >= 0) {
    pinMode(VOL_DOWN_BUTTON    , INPUT);
  }
  if (MUTE_BUTTON >= 0) {
    pinMode(MUTE_BUTTON        , INPUT);
  }
  if (POWER_SWITCH_PIN >= 0) {
    pinMode(POWER_SWITCH_PIN   , OUTPUT);
  }
  if (LCD_UPDATE_PIN >= 0) {
    pinMode(LCD_UPDATE_PIN     , OUTPUT);
  }
  if (LCD_CLOCK_PIN >= 0) {
    pinMode(LCD_CLOCK_PIN      , OUTPUT);
  }
  if (LCD_SERIAL_PIN >= 0) {
    pinMode(LCD_SERIAL_PIN     , OUTPUT);
  }
  if (TEST_PIN >= 0) {
    pinMode(TEST_PIN           , OUTPUT);
  }

  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}






void upDetected () {
   volume ++;
}

void downDetected () {
  volume --;
}

void muteDetected () {
  mute != mute;
}

//- (void)powerDetected () {
//  
//}

//- (void)sendProgressToLCD(int progress) {
//  
//}

void loop() {
  if (irrecv.decode(&results)) {
    if (last_results.decode_type == results.decode_type && last_results.value == results.value) {
      Serial.println(results.value, HEX);

      if (last_results.value == VOL_UP_CODE) { // UP DETECTED
        digitalWrite(TEST_PIN, HIGH);
      }
      if (last_results.value == VOL_DOWN_CODE) { // DOWN_DETECTED
        digitalWrite(TEST_PIN, HIGH);
      }
      delay(100);
      digitalWrite(TEST_PIN, LOW);
    }
    last_results = results;
    irrecv.resume(); // Receive the next value
  }
}

