//  ============================================================
//
//  Program: ArdCore Clock Divider
//
//  Description: A template application for the Arduino Nano
//               based on code from 20 Objects AC01.
//               Given an incoming clock, print out 4 divisions,
//               based on the correspondent Potentiometers position.

//  Created:  30 August 2018  by Thopa
//
//  I/O Usage:
//    Knob 1: Clock divider for digital out 1, prints to window
//    Knob 2: Clock divider for digital out 2, prints to window
//    Knob 3: Clock divider for digital out 3, prints to window
//    Knob 4: Clock divider for digital out 4, prints to window

//    Digital Out 1: Divided clock display/trigger
//    Digital Out 2: Divided clock display/trigger
//    Digital Out 3: Divided clock display/trigger
//    Digital Out 4: Divided clock display/trigger
//    Clock In: External clock input
//    Reset In: External Reset on Pin 7

//  ============================================================
//
//  License:
//
//  This software is licensed under the Creative Commons
//  "Attribution-NonCommercial license. This license allows you
//  to tweak and build upon the code for non-commercial purposes,
//  without the requirement to license derivative works on the
//  same terms. If you wish to use this (or derived) work for
//  commercial work, please contact 20 Objects LLC at our website
//  (www.20objects.com).
//
//  For more information on the Creative Commons CC BY-NC license,
//  visit http://creativecommons.org/licenses/

//  ================= start of global section ==================

//  constants related to the Arduino Nano pin use
const int clkIn = 2;           // the digital (clock) input
const int digPin[4] = {3, 4, 5, 6};  // the digital output pins
const int trigTime = 25;       // 25 ms trigger timing



//  variables for interrupt handling of the clock input
volatile int clkState = LOW;
int clkDivide1 = 0;
int clkDivide2 = 0;
int clkDivide3 = 0;
int clkDivide4 = 0;

//  variables used to control the current DIO output states
int digState[4] = {LOW, LOW, LOW, LOW};        // start with both set low
unsigned long digMilli[4] = {0, 0, 0, 0};  // a place to store millis()


const int reset_Pin = 7;

//  ==================== start of setup() ======================

//  This setup routine should be used in any ArdCore sketch that
//  you choose to write; it sets up the pin usage, and sets up
//  initial state. Failure to properly set up the pin usage may
//  lead to damaging the Arduino hardware, or at the very least
//  cause your program to be unstable.

void setup()
{

  // if you need to send data back to your computer, you need
  // to open the serial device. Otherwise, comment this line out.
  Serial.begin(9600);

  // set up the digital (clock) input
  pinMode(clkIn, INPUT);

  // Set up the Reset for all clocks on Pin 7
  pinMode (reset_Pin, INPUT_PULLUP);

  // set up the digital outputs
  for (int i = 0; i < 4; i++) {
    pinMode(digPin[i], OUTPUT);
    digitalWrite(digPin[i], LOW);
  }

  // set up an interrupt handler for the clock in. If you
  // aren't going to use clock input, you should probably
  // comment out this call.
  // Note: Interrupt 0 is for pin 2 (clkIn)
  attachInterrupt(0, isr, RISING);
}


void loop()
{

  int i;


  // deal with possible reset

  if (digitalRead(reset_Pin) == 0) { 
    // This is the culprit of a buggy setting. Before it was set to 0 and when used with the eurorack it would give false readings, so, reset INPUT_PULLUP and state = 1 not 0
    //This is the line of code that might cause problems, on bread board i had to set it to 0, on previous eurorack module to 1. Look into different settings.
    //also this one = if (digitalRead(7) == 1) instead of digitalRead(reset_Pin)
    clkDivide1 = (analogRead(0) >> 6) + 1;
    clkDivide2 = (analogRead(1) >> 6) + 1;
    clkDivide3 = (analogRead(2) >> 6) + 1;
    clkDivide4 = (analogRead(3) >> 6) + 1;

  }

  // check to see if the clock as been set
  if (clkState == HIGH) {
    clkState = LOW;

    clkDivide1++;
    if (clkDivide1 > (analogRead(0) >> 6)) {

      //If you are careful to avoid sign extension, you can use the right-shift operator >>
      //as a way to divide by powers of 2. For example:
      //int x = 1000;
      //int y = x >> 3;   // integer division of 1000 by 8, causing y = 125.
      clkDivide1 = 0;
      digState[0] = HIGH;
      digMilli[0] = millis();
      digitalWrite(digPin[0], HIGH);
    }

    clkDivide2++;
    if (clkDivide2 > (analogRead(1) >> 6)) {
      clkDivide2 = 0;
      digState[1] = HIGH;
      digMilli[1] = millis();
      digitalWrite(digPin[1], HIGH);

    }

    clkDivide3++;
    if (clkDivide3 > (analogRead(2) >> 6)) {
      clkDivide3 = 0;
      digState[2] = HIGH;
      digMilli[2] = millis();
      digitalWrite(digPin[2], HIGH);

    }

    clkDivide4++;
    if (clkDivide4 > (analogRead(3) >> 6)) {
      clkDivide4 = 0;
      digState[3] = HIGH;
      digMilli[3] = millis();
      digitalWrite(digPin[3], HIGH);
    }
  }


  // do we have to turn off any of the digital outputs?
  for (int i = 0; i < 4; i++) {
    if ((digState[i] == HIGH) && (millis() - digMilli[i] > trigTime)) {
      digState[i] = LOW;
      digitalWrite(digPin[i], LOW);
    }
  }



  // print the analog input values
  Serial.print(analogRead(0));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(1));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(2));   // print the A2 input
  Serial.print('\t');            // print a tab character
  Serial.print(analogRead(3));   // print the A3 input
  Serial.println();              // line feed
}




//  =================== convenience routines ===================

//  These routines are some things you will need to use for
//  various functions of the hardware. Explanations are provided
//  to help you know when to use them.

//  isr() - quickly handle interrupts from the clock input
//  ------------------------------------------------------
void isr()
{
  // Note: you don't want to spend a lot of time here, because
  // it interrupts the activity of the rest of your program.
  // In most cases, you just want to set a variable and get
  // out.
  clkState = HIGH;
}



//  deJitter(int, int) - smooth jitter input
//  ----------------------------------------
int deJitter(int v, int test)
{
  // this routine just make sure we have a significant value
  // change before we bother implementing it. This is useful
  // for cleaning up jittery analog inputs.
  if (abs(v - test) > 8) {
    return v;
  }
  return test;
}


//  ===================== end of program =======================
