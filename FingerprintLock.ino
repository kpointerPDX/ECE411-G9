#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Declarations for Arduino Uno R3
const int buttonPins[] = {4, 6, 7, 5}; // Order: Up, Enter, Down, Back
const int triggerPin = 8;
const int HOME_SCREEN = 0, SCAN_PRINT = 1, ADD_PRINT = 2, REMOVE_PRINT = 3, ENROLLED_PRINTS = 4;
const int PAUSE_S = 1000;
const int PAUSE_M = 2000;
const int PAUSE_L = 3000;
const int PAUSE_DOOR = 5000;

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint reader = Adafruit_Fingerprint(&mySerial);

int state, ix, stack_pointer, print_id;
int stack[10][2];

const char HOME_TEXT[][2][16] = {
  {"WELCOME",     "U ^ D v"},
  {"SCAN PRINT",  "U ^ D v E >"},
  {"ADD PRINT",   "U ^ D v E >"},
  {"REMOVE PRINT","U ^ D v E >"},
};

const char SCAN_TEXT[][2][16] = {
  {"SCAN FINGER", "B <"},
};

const char ADD_TEXT[][2][16] = {
  {"ADD A PRINT", "E > B <"},
  {"CONFIRM?", "E > B <"},
};


const char REMOVE_TEXT[][2][16] = {
  {"REMOVE A PRINT", "E > B <"},
  {"CONFIRM?", "E > B <"},
};


char PRINTS_TEXT[][2][16] = {
  {"1", ""},
  {"2", ""},
  {"3", ""},
  {"4", ""},
  {"5", ""},
  {"6", ""},
  {"7", ""},
  {"8", ""},
  {"9", ""},
  {"10", ""},
};


// Print two lines from a menu to LCD
void printMenu(const char menu[2][16]) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menu[0]);
  lcd.setCursor(0, 1);
  lcd.print(menu[1]);

  return;
}


// Check input buttons and store their value in an integer. Each bit represents value of an individual button
int getButtons() {
  int val = 0;
  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    val = val << 1;
    val += (digitalRead(buttonPins[i])) ? 0 : 1;
  }

  return val;
}


// Check if exactly one button is pressed.
bool oneButton(int buttonVal) {
  while (buttonVal != 0) {
    if (buttonVal % 2 == 1) {
      break;
    }
    
    buttonVal = buttonVal >> 1;
  }

  return buttonVal == 1;
}

void triggerLock(int ms) {
  digitalWrite(triggerPin, HIGH-);
  delay(ms);
  digitalWrite(triggerPin, LOW);
}


void setup() {
  // put your setup code here, to run once:

  // Set pins to input
  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i], INPUT);
  }
  pinMode(triggerPin, OUTPUT);

  // Start LCD and initialize variables
  lcd.init();                           // initialize the lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);

  // initialize fingerprint reader
  Serial.begin(9600);
  delay(100);

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("initializing");
  lcd.setCursor(0,1);
  lcd.print("reader...");
  delay(200);
  
  reader.begin(57600);
  bool lookingForReader = true;
  while (lookingForReader) {
    if (reader.verifyPassword()) {
//      Serial.println("reader found");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Reader found!");
      reader.getParameters();
      lookingForReader = false;
      delay(1000);
    } else {
//      Serial.println("no reader found");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Reader not");
      lcd.setCursor(0,1);
      lcd.print("found! :(");
      delay(1000);
    }
  }
  
  stack_pointer = 0;
  state = HOME_SCREEN;
  ix = 0;
}

uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

void loop() {
  // put your main code here, to run repeatedly:
  int buttonVal = 0;
  
  switch (state) {
    case HOME_SCREEN:
    // Home Screen Menu
      printMenu(HOME_TEXT[ix]);

      while (true) {
        buttonVal = getButtons();

        if (oneButton(buttonVal)) {
          if (buttonVal == 1) {
            //Back
          } else if (buttonVal == 2) {
            //Down
            ix += 1;
            ix = ix % (sizeof(HOME_TEXT) / 32);
          } else if (buttonVal == 4) {
            //Enter
            // Add state to stack and increment stack pointer
            stack[stack_pointer][0] = state;
            stack[stack_pointer][1] = ix;
            stack_pointer++;

            // Set new state
            state = ix;
            ix = 0;
          } else if (buttonVal == 8) {
            //Up
            ix += (sizeof(HOME_TEXT) / 32) - 1;
            ix = ix % (sizeof(HOME_TEXT) / 32);
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }
      
      break;

      
    case SCAN_PRINT:
    // Scan Finger menu
      printMenu(SCAN_TEXT[ix]);

      while (true) {
        buttonVal = getButtons();

        if (oneButton(buttonVal)) {
          if (buttonVal == 1) {
            //Back
            // Pop most recent value off the stack and decrement stack pointer
            state = stack[stack_pointer - 1][0];
            ix = stack[stack_pointer - 1][1];
            stack_pointer--;
          } else if (buttonVal == 2) {
            //Down
            ix += 1;
            ix = ix % (sizeof(SCAN_TEXT) / 32);
          } else if (buttonVal == 4) {
            //Enter
            // *** ADD Code to scan the fingerprint *** //
            if (getFingerprintID()) {
              triggerLock(PAUSE_DOOR);
            }
          } else if (buttonVal == 8) {
            //Up
            ix += (sizeof(SCAN_TEXT) / 32) - 1;
            ix = ix % (sizeof(SCAN_TEXT) / 32);
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }
      
      break;

      
    case ADD_PRINT:
    // Add fingerprint menu
      printMenu(ADD_TEXT[ix]);

      while (true) {
        buttonVal = getButtons();

        if (oneButton(buttonVal)) {
          if (buttonVal == 1) {
            //Back
            // Pop most recent value off the stack and decrement stack pointer
            state = stack[stack_pointer - 1][0];
            ix = stack[stack_pointer - 1][1];
            stack_pointer--;
          } else if (buttonVal == 2) {
            //Down
          } else if (buttonVal == 4) {
            //Enter
            if (ix == 0) {
              // Store state on stack and increment stack pointer
              stack[stack_pointer][0] = state;
              stack[stack_pointer][1] = ix;
              stack_pointer++;

              // Look at the fingerprint slots
              state = ENROLLED_PRINTS;
            } else if (ix == 1) {
              // *** ADD Code to enroll a fingerprint *** //
              if (getFingerprintEnroll(print_id)) {
                // Mark the selected print as enrolled
                strcpy(PRINTS_TEXT[print_id][1], "Enrolled");  
              }

              // Reset stack and return to home screen
              stack_pointer = 0;
              state = HOME_SCREEN;
              ix = 0;
            }
          } else if (buttonVal == 8) {
            //Up
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }
      
      break;

    case REMOVE_PRINT:
    // Remove fingerprint menu
      printMenu(REMOVE_TEXT[ix]);

      while (true) {
        buttonVal = getButtons();

        if (oneButton(buttonVal)) {
          if (buttonVal == 1) {
            //Back
            // Pop most recent value off the stack and decrement stack pointer
            state = stack[stack_pointer - 1][0];
            ix = stack[stack_pointer - 1][1];
            stack_pointer--;
          } else if (buttonVal == 2) {
            //Down
          } else if (buttonVal == 4) {
            //Enter
            if (ix == 0) {
              // Store state on stack and increment stack pointer
              stack[stack_pointer][0] = state;
              stack[stack_pointer][1] = ix;
              stack_pointer++;

              // Look at the fingerprint slots
              state = ENROLLED_PRINTS;
            } else if (ix == 1) {
              // Mark the selected print as no longer enrolled
              strcpy(PRINTS_TEXT[print_id][1], "");
              
              // *** ADD Code to delete a fingerprint *** //

              // Reset stack and return to home screen
              stack_pointer = 0;
              state = HOME_SCREEN;
              ix = 0;
            }
          } else if (buttonVal == 8) {
            //Up
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }
      
      break;

     case ENROLLED_PRINTS:
     // View fingerprint slots. Used in both add and remove fingerprint operations
      printMenu(PRINTS_TEXT[ix]);

      while (true) {
        buttonVal = getButtons();

        if (oneButton(buttonVal)) {
          if (buttonVal == 1) {
            //Back
            // Pop most recent state off the stack and decrement stack pointer
            state = stack[stack_pointer - 1][0];
            ix = stack[stack_pointer - 1][1];
            stack_pointer--;
          } else if (buttonVal == 2) {
            //Down
            ix += 1;
            ix = ix % (sizeof(PRINTS_TEXT) / 32);
          } else if (buttonVal == 4) {
            //Enter
            // Store selected print id and go to next step of Add/Remove operation after storing state on stack
            print_id = ix;

            stack[stack_pointer][0] = state;
            stack[stack_pointer][1] = ix;
            stack_pointer++;
            
            state = stack[stack_pointer - 2][0];
            ix = 1;
          } else if (buttonVal == 8) {
            //Up
            ix += (sizeof(PRINTS_TEXT) / 32) - 1;
            ix = ix % (sizeof(PRINTS_TEXT) / 32);
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }    
  break;
  }
}

uint8_t getFingerprintID() {
  uint8_t p = reader.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("DETECTED..."));
      delay(PAUSE_M);
//      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println("No finger detected");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      return false;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
      return false;
    default:
//      Serial.println("Unknown error");
      return false;
  }

  // OK success!

  p = reader.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      return false;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      return false;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      return false;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      return false;
    default:
//      Serial.println("Unknown error");
      return false;
  }

  // OK converted!
  p = reader.fingerSearch();
  if (p == FINGERPRINT_OK) {
//    Serial.println("Found a print match!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("MATCHED"));
    String fingerStr = String("FINGERPRINT #");
    fingerStr.concat(reader.fingerID);
    lcd.setCursor(0,1);
    lcd.print(fingerStr);
    delay(PAUSE_L);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("COMMS ERROR!"));
    delay(PAUSE_M);
    return false;
  } else if (p == FINGERPRINT_NOTFOUND) {
//    Serial.println("Did not find a match");
    return p;
  } else {
//    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("UNKNOWN ERROR!"));
    delay(PAUSE_M);
    return false;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(reader.fingerID);
  Serial.print(" with confidence of "); Serial.println(reader.confidence);
//  lcd.clear();
//  lcd.setCursor(0,0);
//  String fingerStr = String("FINGERPRINT #");
//  fingerStr.concat(reader.fingerID);
//  lcd.print(fingerStr);
//  lcd.setCursor(0,1);
//  lcd.print(F("DETECTED!"));
  if (reader.confidence > 80) {
    return true;
  } else {
    return false;
  }
}

uint8_t getFingerprintEnroll(int id) {
  int p = -1;
//  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("WAITING..."));
  while (p != FINGERPRINT_OK) {
    p = reader.getImage();
    switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("CAPTURED..."));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.println(".");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("..."));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COMMS ERROR!"));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("IMAGING ERROR!"));
      delay(PAUSE_M);
      break;
    default:
//      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("UNKNOWN ERROR!"));
      delay(PAUSE_M);
      break;
    }
  }

  // OK success!

  p = reader.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("CONVERTED!"));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("IMAGE TOO"));
      lcd.setCursor(0,1);
      lcd.print(F("MESSY!"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COMMS ERROR!"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COULD NOT READ"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COULD NOT READ"));
      delay(PAUSE_M);
      return p;
    default:
//      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("UNKNOWN ERROR!"));
      delay(PAUSE_M);
      return p;
  }

//  Serial.println("Remove finger");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("REMOVE FINGER"));
  delay(2500);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = reader.getImage();
  }
//  Serial.print("ID "); Serial.println(id);
  p = -1;
//  Serial.println("Place same finger again");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("PLACE SAME"));
  lcd.setCursor(0,1);
  lcd.print(F("FINGER..."));
  delay(PAUSE_L);
  while (p != FINGERPRINT_OK) {
    p = reader.getImage();
    switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image taken");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("CAPTURED..."));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_NOFINGER:
//      Serial.print(".");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("..."));
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COMMS ERROR!"));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_IMAGEFAIL:
//      Serial.println("Imaging error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("IMAGING ERROR!"));
      delay(PAUSE_M);
      break;
    default:
//      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("UNKNOWN ERROR!"));
      delay(PAUSE_M);
      break;
    }
  }

  // OK success!

  p = reader.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
//      Serial.println("Image converted");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("CONVERTED!"));
      delay(PAUSE_M);
      break;
    case FINGERPRINT_IMAGEMESS:
//      Serial.println("Image too messy");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("IMAGE TOO"));
      lcd.setCursor(0,1);
      lcd.print(F("MESSY!"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
//      Serial.println("Communication error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COMMS ERROR!"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_FEATUREFAIL:
//      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COULD NOT READ"));
      delay(PAUSE_M);
      return p;
    case FINGERPRINT_INVALIDIMAGE:
//      Serial.println("Could not find fingerprint features");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("COULD NOT READ"));
      delay(PAUSE_M);
      return p;
    default:
//      Serial.println("Unknown error");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(F("UNKNOWN ERROR!"));
      delay(PAUSE_M);
      return p;
  }

  // OK converted!
//  Serial.print("Creating model for #");  Serial.println(id);

  p = reader.createModel();
  if (p == FINGERPRINT_OK) {
//    Serial.println("Prints matched!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("MATCH FOUND!"));
    delay(PAUSE_M);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("COMMS ERROR!"));
    delay(PAUSE_M);
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
//    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("MISMATCH!"));
    delay(PAUSE_M);
    return false;
  } else {
//    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("UNKNOWN ERROR!"));
    delay(PAUSE_M);
    return p;
  }

//  Serial.print("ID "); Serial.println(id);
  p = reader.storeModel(id+1);
  if (p == FINGERPRINT_OK) {
//    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0,0);
    String storeStr = String("FINGERPRINT #");
    storeStr.concat(id+1);
    lcd.print(storeStr);
    lcd.setCursor(0,1);
    lcd.print(F("STORED!"));
    delay(PAUSE_L);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
//    Serial.println("Communication error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("COMMS ERROR!"));
    delay(PAUSE_L);
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
//    Serial.println("Could not store in that location");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("BAD LOCATION!"));
    delay(PAUSE_L);
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
//    Serial.println("Error writing to flash");
    lcd.setCursor(0,0);
    lcd.print(F("WRITE ERROR!"));
    delay(PAUSE_L);
    return p;
  } else {
//    Serial.println("Unknown error");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(F("UNKNOWN ERROR!"));
    delay(PAUSE_L);
    return p;
  }

  return true;
}
