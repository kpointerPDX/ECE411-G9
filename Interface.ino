#if (defined(ESP32))
// Declarations necessary for ESP32
const int buttonPins[] = {33, 27, 12, 19}; // Order: Up, Enter, Down, Back


#include <LiquidCrystal.h>
const int rs = 21, en = 17, d4 = 16, d5 = 14, d6 = 32, d7 = 15;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
const bool i2c = false;
#else
// Declarations for Arduino Uno R3
// *** ADD Declarations for Arduino Uno R3 *** //
const int buttonPins[] = {4, 6, 7, 5}; // Order: Up, Enter, Down, Back


#include <LiquidCrustal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
const bool i2c = true;
#endif

const int HOME_SCREEN = 0, SCAN_PRINT = 1, ADD_PRINT = 2, REMOVE_PRINT = 3, ENROLLED_PRINTS = 4;


int state, ix, stack_pointer, print_id;
int stack[10][2];


const char HOME_TEXT[][2][32] = {
  {"WELCOME", "U ^ D v"},
  {"SCAN PRINT", "U ^ D v E >"},
  {"ADD PRINT", "U ^ D v E >"},
  {"REMOVE PRINT", "U ^ D v E >"},
};


const char SCAN_TEXT[][2][32] = {
  {"SCAN FINGER", "B <"},
};


const char ADD_TEXT[][2][32] = {
  {"ADD A PRINT", "E > B <"},
  {"CONFIRM?", "E > B <"},
};


const char REMOVE_TEXT[][2][32] = {
  {"REMOVE A PRINT", "E > B <"},
  {"CONFIRM?", "E > B <"},
};


char PRINTS_TEXT[][2][32] = {
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
void printMenu(const char menu[2][32]) {
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

    /*
     * Digital read for active high input on ESP32
     *
    val += digitalRead(buttonPins[i]);
    /**/

    /*
     * Digital read for active low input on Uno R3
     * 
     */
    val += (digitalRead(buttonPins[i])) ? 0 : 1;
    /**/
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


void setup() {
  // put your setup code here, to run once:

  // Set pins to input
  for (int i = 0; i < sizeof(buttonPins) / sizeof(int); i++) {
    pinMode(buttonPins[i], INPUT);
  }

  // Start LCD and initialize variables
  if (i2c) {
    lcd.backlight();
  }
  
  lcd.begin(16, 2);
  stack_pointer = 0;
  state = HOME_SCREEN;
  ix = 0;
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
            ix = ix % (sizeof(HOME_TEXT) / 64);
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
            ix += (sizeof(HOME_TEXT) / 64) - 1;
            ix = ix % (sizeof(HOME_TEXT) / 64);
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
            ix = ix % (sizeof(SCAN_TEXT) / 64);
          } else if (buttonVal == 4) {
            //Enter
            // *** ADD Code to scan the fingerprint *** //
          } else if (buttonVal == 8) {
            //Up
            ix += (sizeof(SCAN_TEXT) / 64) - 1;
            ix = ix % (sizeof(SCAN_TEXT) / 64);
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
              // Mark the selected print as enrolled
              strcpy(PRINTS_TEXT[print_id][1], "Enrolled");
              
              // *** ADD Code to enroll a fingerprint *** //

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
            ix = ix % (sizeof(PRINTS_TEXT) / 64);
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
            ix += (sizeof(PRINTS_TEXT) / 64) - 1;
            ix = ix % (sizeof(PRINTS_TEXT) / 64);
          }

          // Wait until all buttons are released
          while (getButtons());
          break;
        }
      }
      
      break;
  }
}
