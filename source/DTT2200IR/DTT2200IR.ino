#include <IRremote.h>

int IR_RECV_PIN = 11;
int TEST_PIN = 13;
int VOL_UP_PIN = 1;
int VOL_DOWN_PIN = 2;
//int POWER_PIN = 3;


int LCD_UPDATE_PIN = 4;
int LCD_CLOCK_PIN  = 5;
int LCD_SERIAL_PIN = 8;


int max_bar_segments = 10;
int volume_value = 0;
int volume_max = 10;
int volume_min = 0;
int volume_increment = 1;
int mute_value = 0;


unsigned long VOL_UP_CODE   = 0x68733A46;
unsigned long VOL_DOWN_CODE = 0x83B19366;
//unsigned long POWER_CODE    = 0xF4BA2988;

decode_results results;
decode_results last_results;

IRrecv irrecv(IR_RECV_PIN);

void setup()
{
  pinMode(TEST_PIN, OUTPUT);
  pinMode(VOL_UP_PIN, OUTPUT);
  pinMode(VOL_DOWN_PIN, OUTPUT);
  
  pinMode(LCD_UPDATE_PIN, OUTPUT);
  pinMode(LCD_CLOCK_PIN   , OUTPUT);
  pinMode(LCD_SERIAL_PIN    , OUTPUT);
  
//  pinMode(POWER_PIN, OUTPUT);
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}



void loop() {
  if (irrecv.decode(&results)) {
    if (last_results.decode_type == results.decode_type && last_results.value == results.value) {
      Serial.println(results.value, HEX);

      if (last_results.value == VOL_UP_CODE) { // UP DETECTED

        digitalWrite(TEST_PIN, HIGH);
      }
      if (last_results.value == VOL_DOWN_CODE) { // DOWN_DETECTED
        sendUpToDigitalPotentiometer();
        digitalWrite(TEST_PIN, HIGH);
      }
      delay(100);
      digitalWrite(TEST_PIN, LOW);
      digitalWrite(VOL_UP_PIN, LOW);
      digitalWrite(VOL_DOWN_PIN, LOW);
    }
    last_results = results;
    irrecv.resume(); // Receive the next value
  }
}


- (void)sendUpToDigitalPotentiometer () {
  digitalWrite(VOL_UP_PIN, HIGH);
  digitalWrite(VOL_DOWN_PIN, HIGH);
}

- (void)sendDownToDigitalPotentiometer () {

  digitalWrite(VOL_DOWN_PIN, HIGH);
}



- (void)sendProgressToLCD(int progress) {
  digitalWrite(LCD_UPDATE_PIN, LOW);
  for (int i = 1; i <= max_bar_segments; i++) {
    if (i <= progress) {
      digitalWrite(LCD_CLOCK_PIN, LOW);
      digitalWrite(LCD_SERIAL_PIN, HIGH);
      digitalWrite(LCD_CLOCK_PIN, HIGH);

    } else {
      digitalWrite(LCD_CLOCK_PIN, LOW);
      digitalWrite(LCD_SERIAL_PIN, LOW);
      digitalWrite(LCD_CLOCK_PIN, HIGH);
    }
  }
  digitalWrite(LCD_UPDATE_PIN, HIGH);  
}
