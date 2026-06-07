#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// -------- LCD ----------
LiquidCrystal_I2C lcd(0x27, 20, 4);

// -------- Servo ----------
Servo lockServo;
int lockPos = 0;
int unlockPos = 90;

// -------- Keypad ----------
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {9,8,7,6};
byte colPins[COLS] = {5,4,3,2};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// -------- Password ----------
String password = "1234";
String input = "";

// -------- State ----------
bool unlocked = false;
unsigned long unlockTime = 0;

// -------- Hall Sensor ----------
#define HALL_PIN 11
bool hallIgnore = false;

// -------- Output Signal Pin ----------
#define WRONG_SIGNAL_PIN A0

// -------- Extra Unlock Input Pins ----------
#define UNLOCK_PIN_13 13
#define UNLOCK_PIN_A0 A0

int abcStep = 0;

// ----------------------------
void setup() {

  lcd.init();
  lcd.backlight();

  pinMode(HALL_PIN, INPUT_PULLUP);

  // Pin 13 and A0 are normally HIGH because of INPUT_PULLUP
  pinMode(UNLOCK_PIN_13, INPUT_PULLUP);
  pinMode(UNLOCK_PIN_A0, INPUT_PULLUP);

  // Pin 12 is normally always HIGH
  pinMode(WRONG_SIGNAL_PIN, OUTPUT);
  digitalWrite(WRONG_SIGNAL_PIN, LOW);

  lockServo.attach(10);
  lockDoor();

  showScreen("Door Locked");
}

// ----------------------------
void loop() {

  char key = keypad.getKey();

  // ================= PIN 13 LOW UNLOCK =================
  if (digitalRead(UNLOCK_PIN_13) == LOW) {
    doUnlock("PIN 13 UNLOCK");
  }

  // ================= A0 LOW UNLOCK =================
  else if (digitalRead(UNLOCK_PIN_A0) == LOW) {
    doUnlock("A0 UNLOCK");
  }

  // ================= KEYPAD =================
  if(key){

    if(key >= '0' && key <= '9'){
      input += key;

      lcd.setCursor(0,3);
      lcd.print("Pass: ");

      for(int i = 0; i < input.length(); i++) {
        lcd.print("*");
      }
    }

    if(key == '#'){
      checkPassword();
    }

    if(key == '*'){
      input = "";
      showScreen("Door Locked");
    }

    handleABC(key);
  }

  // -------- ignore timer ----------
  if(unlocked && (millis() - unlockTime > 5000)){
    hallIgnore = false;
  }

  // -------- HALL SENSOR ----------
  if(unlocked && !hallIgnore){

    if(digitalRead(HALL_PIN) == LOW){

      delay(2000);

      lockDoor();
      showScreen("Door Locked");
    }
  }
}

// ----------------------------
void doUnlock(String message){

  unlockDoor();

  unlocked = true;
  unlockTime = millis();
  hallIgnore = true;

  showScreen(message);
}

// ----------------------------
void wrongPasswordSignal(){

  digitalWrite(WRONG_SIGNAL_PIN, HIGH);
  delay(10);
  digitalWrite(WRONG_SIGNAL_PIN, LOW);
}

// ----------------------------
void handleABC(char key){

  if(key == 'A') {
    abcStep = 1;
  }
  else if(key == 'B' && abcStep == 1) {
    abcStep = 2;
  }
  else if(key == 'C' && abcStep == 2){

    abcStep = 0;
    changePassword();
  }
  else{
    abcStep = 0;
  }
}

// ----------------------------
void checkPassword(){

  if(input == password){

    doUnlock("Door Unlocked");

  } else {

    wrongPasswordSignal();
    showScreen("Wrong Password");
    delay(1500);
    showScreen("Door Locked");
  }

  input = "";
}

// ----------------------------
void changePassword(){

  String oldP = "", newP = "", confirmP = "";

  lcd.clear();
  lcd.print("Old Pass:");

  while(oldP.length() < 4){
    char k = keypad.getKey();
    if(k && isDigit(k)){
      oldP += k;
      lcd.print("*");
    }
  }

  if(oldP != password){

    wrongPasswordSignal();

    lcd.clear();
    lcd.print("Wrong Old Pass");
    delay(1500);
    showScreen("Door Locked");
    return;
  }

  lcd.clear();
  lcd.print("New Pass:");

  while(newP.length() < 4){
    char k = keypad.getKey();
    if(k && isDigit(k)){
      newP += k;
      lcd.print("*");
    }
  }

  lcd.clear();
  lcd.print("Confirm:");

  while(confirmP.length() < 4){
    char k = keypad.getKey();
    if(k && isDigit(k)){
      confirmP += k;
      lcd.print("*");
    }
  }

  if(newP == confirmP){
    password = newP;

    lcd.clear();
    lcd.print("Saved!");
    delay(1500);
  } else {

    wrongPasswordSignal();

    lcd.clear();
    lcd.print("Not Match");
    delay(1500);
  }

  showScreen("Door Locked");
}

// ----------------------------
// LCD FUNCTION
// ----------------------------
void showScreen(String status){

  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("  BRAC UNIVERSITY  ");

  lcd.setCursor(0,1);
  lcd.print("   CSE DEPARTMENT  ");

  lcd.setCursor(0,2);

  String line = status;
  while(line.length() < 20) {
    line += " ";
  }
  lcd.print(line);

  lcd.setCursor(0,3);
  lcd.print("Pass: ");
}

// ----------------------------
void lockDoor(){
  lockServo.write(lockPos);
  unlocked = false;
}

// ----------------------------
void unlockDoor(){
  lockServo.write(unlockPos);
}