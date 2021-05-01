/// Title: Ardrando
// VERSION:  3.1

//CLARK where is variable s defined? it should be a random roll.
boolean DEBUG = 0;
boolean NOTEDEBUG = 0;
boolean KEYDEBUG = 0 ; // this gives details about all the keys pressed at any given moment
#include <Wire.h>
//-------------------- Ins and outs -------------------------
// ---------------------- ANALOG

#define  clock_in A1 // analog clock input
#define  SDA_pin  A4 // non-negotiable, see pinout
#define  SCL_pin  A5 // non-negotiable, see pinout
//     analog inputs for pots 1-4      Shift      |   Normal
//                                   -------------|---------------
#define  pot1  A3   //              quant scale   |   Random lock
#define  pot2  A6   //                  bpm       |   Random Scale
#define  pot3  A7   //              note length   |   Sequence length

// ---------------------- DIGITAL
#define gateOutPin 5 // uses PWM out
#define led_red 2
#define led_blue 3
#define led_green 4
#define seqPin 6
#define MCP4725 0x60              //MCP4725 address as 0x61 Change yours accordingly; see http://henrysbench.capnfatz.com/henrys-bench/arduino-projects-tips-and-more/arduino-quick-tip-find-your-i2c-address/

//These are the valid values for scales
// paste0(sort(c(c((0:8 * 12) + 0), c((0:8 * 12) + 5))), collapse=", ")
//octaves
uint8_t scale1[9]  = {0, 12, 24, 36, 48, 60, 72, 84, 96};
uint8_t scale2[18] =   {0, 7, 12, 19, 24, 31, 36, 43, 48,
                        //octaves and fifths
                        55, 60, 67, 72, 79, 84, 91, 96, 103
                       };
//ionian
uint8_t scale3[36] =   {0, 2, 4, 5, 7, 9, 11, 12, 14, 16, 17, 19, 21, 23, 24, 26, 28, 29, 31, 33, 35, 36, 38, 40, 41, 43, 45, 47, 48, 50, 52, 53, 55, 57, 59, 60};
//dorian
uint8_t scale4[36] =   {0, 2, 3, 5, 7, 9, 10, 12, 14, 15, 17, 19, 21, 22, 24, 26, 27, 29, 31, 33, 34, 36, 38, 39, 41, 43, 45, 46, 48, 50, 51, 53, 55, 57, 58, 60};
//phrygian
uint8_t scale5[36] =   {0, 1, 3, 5, 7, 8, 10, 12, 13, 15, 17, 19, 20, 22, 24, 25, 27, 29, 31, 32, 34, 36, 37, 39, 41, 43, 44, 46, 48, 49, 51, 53, 55, 56, 58, 60};
//lydian
uint8_t scale6[36] =   {0, 2, 4, 6, 7, 9, 11, 12, 14, 16, 18, 19, 21, 23, 24, 26, 28, 30, 31, 33, 35, 36, 38, 40, 42, 43, 45, 47, 48, 50, 52, 54, 55, 57, 59, 60};
//myxolydian
uint8_t scale7[36] =   {0, 2, 4, 5, 7, 9, 10, 12, 14, 16, 17, 19, 21, 22, 24, 26, 28, 29, 31, 33, 34, 36, 38, 40, 41, 43, 45, 46, 48, 50, 52, 53, 55, 57, 58, 60};
//aeolian
uint8_t scale8[36] =   {0, 2, 3, 5, 7, 8, 10, 12, 14, 15, 17, 19, 20, 22, 24, 26, 27, 29, 31, 32, 34, 36, 38, 39, 41, 43, 44, 46, 48, 50, 51, 53, 55, 56, 58, 60};
//locrian
uint8_t scale9[36] =   {0, 1, 3, 5, 6, 8, 10, 12, 13, 15, 17, 18, 20, 22, 24, 25, 27, 29, 30, 32, 34, 36, 37, 39, 41, 42, 44, 46, 48, 49, 51, 53, 54, 56, 58, 60};
//chromatic
uint8_t scale10[128] =   {1, 2, 3, 4, 5, 6, 7, 8, 9,
                          10, 11, 12, 13, 14, 15, 16, 17, 18,
                          19, 20, 21, 22, 23, 24, 25, 26, 27,
                          28, 29, 30, 31, 32, 33, 34, 35, 36,
                          37, 38, 39, 40, 41, 42, 43, 44, 45,
                          46, 47, 48, 49, 50, 51, 52, 53, 54,
                          55, 56, 57, 58, 59, 60, 61, 62, 63,
                          64, 65, 66, 67, 68, 69, 70, 71, 72,
                          73, 74, 75, 76, 77, 78, 79, 80, 81,
                          82, 83, 84, 85, 86, 87, 88, 89, 90,
                          91, 92, 93, 94, 95, 96, 97, 98, 99,
                          100, 101, 102, 103, 104, 105, 106, 107, 108,
                          109, 110, 111, 112, 113, 114, 115, 116, 117,
                          118, 119, 120, 121, 122, 123, 124, 125, 126,
                          127, 128
                         };

// digital input for shift button
int shift = 7;

// needed for toggle cause it is conductive
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;    // the debounce time; increase if the output flickers


//-------------------- Midi/key defaults  -------------------------

int vel = 87; // default velocity
int notes = 128;

byte buffer[3];    // for dac

// -------------  Variables for the sequencer -------------------

// CLARK WTF are these sequencer steps?
//uint8_t def_sequencer_steps[16] = {60, 67, 72, 79, 84, 79, 72, 67, 60, 55, 48, 43, 36, 43, 48, 55};
//I THINK WE NEED TO TURN THIS ARRAY LENTH INTO A VARIABLE AND ADJUST IT BASED ON CHOSEN SCALE
uint8_t def_sequencer_steps[36] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36};
uint8_t sequencer_steps[36] ;
float sequencer_lens[36];
uint8_t seq_counter;
uint8_t seq_length;
boolean seq_running ;
boolean gate_is_high ;
uint8_t thisscale[128];
uint8_t thisscaleid = 10;
uint8_t thisscalelen = 128;

int prev_note;
int base;

// ------------------                   Clock stuff
// These keep track of the toggle state, debounced
int internal_clock ; // int, ext midi, ext cv
int this_clock_state ;
int last_clock_state ; // int, ext midi, ext cv, save this for debouncing

//      internal clock
unsigned long time_next_note;  //needs to be unsigned for millis() comparison
unsigned long time_prev_note;
unsigned long time_to_restart;
unsigned long time_to_reset_seq;
unsigned long time_to_reset_bpm;
boolean shift_already = false;
boolean already_reset_seq = false;
boolean already_reset_bpm = false;
int time_between_quarter_notes;
int bpm = 120;



// For Turing Functions
//CLARK where are these used? NOWHERE.
int random_scale;
int random_amt;
int random_rest;

// function declarations
void sequencer_mode();
void edit_mode();
void update_key(int i, int val);
void noteOn(int pitch, int velocity);
void noteOff(int pitch, int velocity);

void process_shift_change();
void setSequence();
void toggleSequencer();
void toggleClock();
void success_leds();
void waiting_leds();
void set_time_between_quarter_notes(float bpm);
void update_param(int button, int param = 0);

int process_pot(int a_input);
float noteToVolt(int note);
void read_seq_and_params();
void vpoOut(unsigned int pitch);
void gateOut(unsigned int state);

void check_pots();

// restart function
void(* resetFunc) (void) = 0;

void setup() {
  Wire.begin();                    //Begins the I2C communication
  time_prev_note = 0;
  pinMode(led_red, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(gateOutPin, OUTPUT);

  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
  pinMode(pot3, INPUT);
  pinMode(seqPin, INPUT_PULLUP);
  pinMode(shift, INPUT_PULLUP);

  digitalWrite(led_red, LOW);
  digitalWrite(led_blue, LOW);
  digitalWrite(led_green, LOW);
  digitalWrite(gateOutPin, LOW);
  internal_clock =  digitalRead(seqPin);
  last_clock_state = internal_clock ;

  // -------------  Variables for the sequencer -------------------
  // these are in the setup() because we want to be able to reset the seqeunce when the unit is reset
  for (int k = 0; k < 36; k++) {
    sequencer_steps[k] = def_sequencer_steps[k];
    
    //CLARK .5 sequencer lens?
    sequencer_lens[k] = .5;
  }
  for (int k = 0; k < thisscalelen; k++) {
    thisscale[k] = scale10[k];
  }
  seq_counter = 0;
  seq_length = 16;
  seq_running = false;
  gate_is_high = false;
  int prev_note;

  vel = 87; // default velocity
  notes = 128;

  delay(50);

  if (DEBUG) {
    Serial.begin(9600);
    Serial.print("setup done\n");
  } else {
    Serial.begin(31250);
  }
  success_leds();

}


void loop() {
  // base note to start on.
  base = (12 * 3);
  // check_pots()
  // start by checking if the shift key is pressed.  If so ignore any note changes
  // note shift is connected as a pull up, so activated means 0, deactivated means 1
  int this_clock_state = digitalRead(seqPin);
  if (this_clock_state != last_clock_state) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (this_clock_state != internal_clock) { // still
      if (!internal_clock) {
        digitalWrite(led_blue, HIGH);
        delay(100);
        digitalWrite(led_blue, LOW);
        delay(50);
        digitalWrite(led_red, HIGH);
        delay(100);
        digitalWrite(led_red, LOW);
        delay(50);
        digitalWrite(led_green, HIGH);
        delay(100);
        digitalWrite(led_green, LOW);
        delay(50);
        digitalWrite(led_red, HIGH);
        delay(100);
        digitalWrite(led_red, LOW);
        internal_clock = 1;
      } else {
        internal_clock = 0;
      }
    }
  }

  last_clock_state = this_clock_state;
  // pause sequencer if you need to
  if (digitalRead(shift) == 0 ) {
    digitalWrite(led_green, HIGH);
    process_shift_change();
    seq_running = false;
  } else {
    digitalWrite(led_green, LOW);
    seq_running = true;
    shift_already = false;
    already_reset_bpm = false;
    already_reset_seq = false;
  }

  if (seq_running) {
    sequencer_mode();
  } else {
    // otheriwse, if shift key is not pressed,
    digitalWrite(led_blue, LOW);
  }
}

/* -------------------------------    Main Logic     --------------------------- */
// one of the three following modes is run each time the main loop executes:
// playing the keyboard, running the sequencer, and editing stuff


void check_note_length_int() {
  if (gate_is_high) {
    if (millis() > (time_prev_note - (sequencer_lens[seq_counter] * time_between_quarter_notes))) {
      noteOff(prev_note, 0);
    }
  }
}
void check_note_length_ext() {
  if (gate_is_high) {
    if (millis() > (time_prev_note - (sequencer_lens[seq_counter] * time_between_quarter_notes))) {
      noteOff(prev_note, 0);
    }
  }
}
void sequencer_mode() {

  // first, check for BPM changes
  // next, checks clock mode
  // next, checks if enouggh time has ellapsed between notes

  set_time_between_quarter_notes(bpm);
  // internal
  if (internal_clock) {
    time_prev_note = millis();

    if (millis() > time_next_note) {

      play_sequence();
      time_next_note = time_prev_note + time_between_quarter_notes;
    } else {
      check_note_length_int();
    }
    //   external cv
  } else {
    // check for a CV pulse. USA A VOLTAGE DIVIDER TO ATTENUATE INPUT!!!
    // also, ignore long pulses:  for instance, STARTUP by music thing modular outputs clock triggers for 10MS
    // so for safety lets wait for a 5x that (100)

    if (analogRead(clock_in) > 400) {

      if ((millis() - time_prev_note)  > 100) {

        time_prev_note = millis();
        play_sequence();
      }
    } else {
      check_note_length_ext();
    }
  }
}


//-------------------------------   Functions  ---------------------------------
int scale_centered_sequence_note(int thisnote, int scaling, int half_scaling) {
  // this scales the seqeunced centered around the middle of the keyboard, key 64
  // for instace, scale to 1/3 of total notes(42) and the sequence is  48 60 72
  // centered around 1 (really 64) this is 48/64 60/64, 72/64
  //                                     .75       .9375      1.125
  // scaled to 42's center: 21           .75*21    .9375*21   1.125*21
  //                                   15.75     19.6875     23.625
  //but thats not centered around the middle of the keyboard, so
  // we add the difference between new middle and old middle: 64 - 21 = 43
  // New sequence is                    58.75    62.6875     66.625
  return (ceil(float(thisnote / 64.0) * half_scaling + (64 - half_scaling)));

}

int quantize_to_scale(int thisnote, int thisscalelen) {
  //Quantize if neccesary: check if the note is in the selected scale.
  // If not, increment or decrement closer to center
  int quantized = 0;
  for (int q = 0; q < thisscalelen; q++) {
    if (abs(thisscale[q] - thisnote) < abs(quantized - thisnote)) {
      quantized = thisscale[q];
    }
  }
  return (quantized);
}
void play_sequence() {
  //CLARK scaling is defined by pot2
  int scaling =  map(process_pot(pot2), 0, 1023, 1, 128);
  int half_scaling =  ceil(float(scaling / 2.0));
  if (DEBUG) {
    if (seq_counter == 0) {
      for (int i = 0 ; i < seq_length; i++) {
        Serial.print(sequencer_steps[i]);
        Serial.print(" ");
      }
      Serial.print(" || ");
      for (int i = 0 ; i < seq_length; i++) {
        Serial.print(scale_centered_sequence_note(sequencer_steps[i], scaling, half_scaling));
        Serial.print(" ");
      }
      Serial.print(" || ");
      for (int i = 0 ; i < seq_length; i++) {
        Serial.print(quantize_to_scale(scale_centered_sequence_note(sequencer_steps[i], scaling, half_scaling), thisscalelen));
        Serial.print(" ");
      }
      Serial.print(thisscaleid);
      Serial.println(" ");
    }
  }
  seq_length = round(map(process_pot(pot3), 0, 1023, 1, 16));
  int thisnote = sequencer_steps[seq_counter];
  int thisscalednote = scale_centered_sequence_note(thisnote, scaling, half_scaling);
  int quantized = quantize_to_scale(thisscalednote, thisscalelen);
  // kill previous note
  if (thisnote == 0) {
    // this is how we play rests
    gateOut(0);
  } else {
    noteOff(prev_note, 0);
    noteOn(quantized, vel);
  }

  // increment, resetting counter if need be
  seq_counter = seq_counter + 1 ;
  if (seq_counter >= seq_length) {
    seq_counter = 0;
  }

  //  turing bit
  random_amt = map(process_pot(pot1), 0, 1023, 100, 0);
  // decide change note or not

  //CLARK dice roll based on pot value
  //What's "scaling" variable doing here?
  //CLARK rscale is the randomizer also, play with the following conditional statement:

 // OLD VERSION:  if ( random_amt  > random(100)) {
  if ( random_amt  > random(50)) {
    int base = ceil(scaling / 2.0);
    int rscale = random(scaling);
// new note is centered around middle
    int new_note = thisnote - base + rscale;
    if (DEBUG) {
      Serial.print("old step ");

      Serial.print(sequencer_steps[seq_counter]);
      Serial.print("; new step ");
      Serial.print(new_note);
      Serial.print("; base ");
      Serial.print(base);
      Serial.print("; rand ");
      Serial.println(rscale);
    }
    sequencer_steps[seq_counter] = constrain(new_note, 1, 128);
  }
}


void set_time_between_quarter_notes(float bpm) {
  float bps = 60 / bpm ;                    // beats per second (2 @120)
  time_between_quarter_notes = bps * 1000 ; // milliseconds per beat
}


int process_pot(int a_input) {
  int thisval = analogRead(a_input);
  return (thisval);
}


///////////////////////////    Functions for CV   /////////////////
int divider = 38;
float noteToVolt(int note) {
  // returns value 0-4096 to sent to DAC
  // note to freq
  // https://en.wikipedia.org/wiki/Piano_key_frequencies
  float freq = pow(2, ((note - 49) / 12.0)) * 440 ;
  int constr_note = constrain(note, 36, 97);
  int mvolts = map(constr_note, 36, 97, 0, 5000) ; // start at A220
  float volts = (mvolts / 1000.0);
  return (volts);
}

void gateOut(unsigned int state) {
  if (state == 1) {
    digitalWrite(gateOutPin, HIGH);
  } else {
    digitalWrite(gateOutPin, LOW);
  }
}

void vpoOut(unsigned int pitch) {
  buffer[0] = 0b01000000;            //Sets the buffer0 with control byte (010-Sets in Write mode)
  //Serial.print("ADC:");
  //Serial.println(adc);
  int adc =  map(noteToVolt(pitch) * 1000, 0, 5000, 0, 4096);
  buffer[1] = adc >> 4;              //Puts the most significant bit values
  buffer[2] = adc << 4;              //Puts the Least significant bit values
  Wire.beginTransmission(MCP4725);         //Joins I2C bus with MCP4725 with 0x61 address

  Wire.write(buffer[0]);            //Sends the control byte to I2C
  Wire.write(buffer[1]);            //Sends the MSB to I2C
  Wire.write(buffer[2]);            //Sends the LSB to I2C

  Wire.endTransmission();           //Ends the transmission
}

///////////////////////////    Functions for MIDI /////////////////


void noteOn(int pitch, int velocity) {
  digitalWrite(led_red, HIGH);
  vpoOut(pitch);
  gateOut( 1 );
  gate_is_high  = true;
  if (NOTEDEBUG) {
    Serial.print("on ");
    Serial.print(pitch);
    Serial.print("\n");
  }

  prev_note = pitch;
}

void noteOff(int pitch, int velocity) {
  digitalWrite(led_red, LOW);
  gateOut(0);
  gate_is_high = false;
  if (NOTEDEBUG) {
    Serial.print("off ");
    Serial.print(pitch);
    Serial.print("\n");
  }
}


///////////////////////////////////   Functions for Sequencer, ETC ////////////
void process_shift_change() {
  // After 4 seconds, clear the sequence to all Cs
  // After 8, reset the arduino
  seq_running = false;
  if (!shift_already) {

    shift_already = true;
    // restart if held for 8 seconds
    time_to_restart = millis() + (8 * 1000) ;
    time_to_reset_seq = millis() + (4 * 1000) ;
    time_to_reset_bpm = millis() + (2 * 1000) ;
// CLARK    trigger_new_seq = millis() + 100;
  } else {
    if (time_to_restart < millis()) {
      for (int i = 0; i < 4; i ++) {
        digitalWrite(led_red, HIGH);
        digitalWrite(led_green, HIGH);
        digitalWrite(led_blue, HIGH);
        delay(200);
        digitalWrite(led_red, LOW);
        digitalWrite(led_green, LOW);
        digitalWrite(led_blue, LOW);
        delay(100);
      }
      resetFunc();
    } else if (time_to_reset_bpm < millis() and not already_reset_bpm)  {
      bpm = map(process_pot(pot2), 0, 1023, 10, 400);
      already_reset_bpm = true;
      digitalWrite(led_blue, HIGH);
      delay(100);
      digitalWrite(led_blue, LOW);
      delay(100);
      digitalWrite(led_blue, HIGH);
      delay(100);
      digitalWrite(led_blue, LOW);
      thisscaleid = map(process_pot(pot1), 0, 1023, 1, 8);
      if (thisscaleid == 1) {
        thisscalelen = 9 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale1[s];
        }
      } else if (thisscaleid == 2) {
        thisscalelen = 18 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale2[s];
        }
      } else if (thisscaleid == 3) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale3[s];
        }
      } else if (thisscaleid == 4) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale4[s];
        }
      } else if (thisscaleid == 5) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale5[s];
        }
      } else if (thisscaleid == 6) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale6[s];
        }
      } else if (thisscaleid == 7) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale7[s];
        }
      } else if (thisscaleid == 8) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale8[s];
        }
      } else if (thisscaleid == 9) {
        thisscalelen = 36 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale9[s];
        }
      } else {
        thisscalelen = 128 ;
        for (int s = 0; s < thisscalelen; s++) {
          thisscale[s] = scale10[s];
        }
      }
    } else if (time_to_reset_seq < millis() and not already_reset_seq) {
      digitalWrite(led_red, HIGH);
      delay(100);
      digitalWrite(led_red, LOW);
      delay(100);
      digitalWrite(led_red, HIGH);
      delay(100);
      digitalWrite(led_red, LOW);
      already_reset_seq = true;
      for (int k = 0; k < seq_length; k++ ) {
        sequencer_steps[k] = 60;
        sequencer_lens[k] = .5;
      }
    }
  }
}


void success_leds() {
  for (int i = 0; i < 4 ; i++) {
    digitalWrite(led_green, HIGH);
    delay(50);
    digitalWrite(led_green, LOW);
    delay(100);
  }
}



void toggleSequencer() {
  seq_running = !seq_running;
}


void check_pots() {
  Serial.print("1: ");
  Serial.print(process_pot(pot1));
  Serial.print(";  2: ");
  Serial.print(process_pot(pot2));
  Serial.print(":  3: . ");
  Serial.println(process_pot(pot3));

}
void check_seq() {
  for (int i = 0; i < seq_length; i++ ) {
    Serial.print(sequencer_steps[i]);
    Serial.print("\t");
  }
  Serial.println();
}
