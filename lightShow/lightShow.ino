
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

#define MAX_BRIGHTNESS 200
#define BRIGHTNESS 130 

#define LED_PIN     6
#define LED_COUNT  127
#define LED_MIDDLE 69
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);

// hardcoded sinus wave, discretized values stored in sinusgamma
uint8_t sinusgamma[256];
void sinusGamma(uint8_t *arr){
  for(int i = 0; i < 256; i ++){
    arr[i] = strip.gamma8(strip.sine8(i));
  }
}
uint8_t diracgamma[256];
void diracDelta(uint8_t *arr, uint8_t s){
  uint8_t temp = 0;
  for(int i = 0; i < 256; i ++){
    if(i > LED_MIDDLE + s || i < LED_MIDDLE - s){
      temp = 0;
    } else {
      temp = 255;
    }
    arr[i] = strip.gamma8(temp);
  }
}

// set color evenly #todo refactor
void setEvenColor(uint8_t red, uint8_t green, uint8_t blue){
  for(int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, red, green, blue);
  }
  strip.show(); 
}

// set pixel values
void set_pixels(uint8_t * r, uint8_t * g, uint8_t * b){
    for(int i = 0; i < LED_COUNT; i++){
      strip.setPixelColor(i, r[i], g[i], b[i]);
    }
    strip.show();
}

// increase state with roc at index, cap at target
void capped_increase(uint8_t * state, uint8_t * target, uint8_t i, uint8_t roc){
  state[i] += roc;
  // handle overshoot
  if(state[i] > target[i]){
    state[i] = target[i];
  }
}

// decrease state with roc at index, cap at target
void capped_decrease(uint8_t * state, uint8_t * target, uint8_t i, uint8_t roc){
  state[i] -= roc;
  // handle undershoot
  if(state[i] < target[i]){
    state[i] = target[i];
  }
}

// linearly builds state one step until target is reached
int build_state_linear(uint8_t * targetR, uint8_t * targetG, uint8_t * targetB,
                        uint8_t * stateR, uint8_t * stateG, uint8_t * stateB,
                        uint8_t rateOfChange){
  bool reachedTarget = true;
  for(int i = 0; i < LED_COUNT; i++){
    // increase mode
    if(stateR[i] < targetR[i]){
      reachedTarget = false;
      capped_increase(stateR, targetR, i, rateOfChange);
    }
    if(stateG[i] < targetG[i]){
      reachedTarget = false;
      capped_increase(stateG, targetG, i, rateOfChange);
    }
    if(stateB[i] < targetB[i]){
      reachedTarget = false;
      capped_increase(stateB, targetB, i, rateOfChange);
    }
    // decrease mode
    if(stateR[i] > targetR[i]){
      reachedTarget = false;
      capped_decrease(stateR, targetR, i, rateOfChange);
    }
    if(stateG[i] > targetG[i]){
      reachedTarget = false;
      capped_decrease(stateG, targetG, i, rateOfChange);
    }
    if(stateB[i] > targetB[i]){
      reachedTarget = false;
      capped_decrease(stateB, targetB, i, rateOfChange);
    }
  }
  return reachedTarget;
}

// sine values at time t to state
void sine_traverse(unsigned long t, uint8_t * state, double factor){
  for(int i = 0; i < LED_COUNT; i++){
     uint8_t t_mod = (i + t) % 256;
     state[i] = sinusgamma[t_mod] * factor;
  }
}

// dirac delta traversing from the middle outwards
void dirac_traverse_outwards(unsigned long t, uint8_t * state){
  for(int i = 0; i < LED_MIDDLE; i++){
     uint8_t t_mod = (i + t) % 256;
     state[i] = diracgamma[t_mod];
  }
  for(int i = 0; i < LED_COUNT-LED_MIDDLE; i++){
     uint8_t t_mod = (LED_MIDDLE + i - t) % 256;
     state[i+LED_MIDDLE] = diracgamma[t_mod];
  }
  // remove the first 11 nodes to have equal on and off
  for(int i = 0; i < 11; i++){
    state[i] = 0;
  }
}

// enforces minimum brightness
void enforce_base_color(uint8_t * state, uint8_t c){
  for(int i = 0; i < LED_COUNT; i++){
    if(state[i] < c){
      state[i] = c;
    }
  }
}

// time 
unsigned long tProgLastTime;
unsigned long tSetupLastTime;

// pixels
uint8_t pred[LED_COUNT];
uint8_t pgreen[LED_COUNT];
uint8_t pblue[LED_COUNT];

uint8_t pred_t[LED_COUNT];
uint8_t pgreen_t[LED_COUNT];
uint8_t pblue_t[LED_COUNT];

// base colors
uint8_t BASE_RED = 15;
uint8_t BASE_BLUE = 0;
uint8_t BASE_GREEN = 3;

void setup() {
  // init time
  tProgLastTime = millis();
  tSetupLastTime = millis();

  // init pixels
  for(int i = 0; i < LED_COUNT; i++){
    // actuals
    pred[i] = 0;
    pgreen[i] = 0;
    pblue[i] = 0;
    // temps
    pred_t[i] = 0;
    pgreen_t[i] = 0;
    pblue_t[i] = 0;
  }

  sinusGamma(sinusgamma);
  diracDelta(diracgamma, 3);
  //Serial.begin(9600);
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
}

#define T_PROG 15
unsigned long tProg = 0;
bool run_prog = false;

#define T_SETUP 8
unsigned long tSetup = 0;
bool run_setup = true;

void loop() {
  // update time 
  unsigned long now = millis();
  unsigned long dtProg = now - tProgLastTime;
  unsigned long dtSetup = now - tSetupLastTime;
  if(run_prog && dtProg > T_PROG){
      tProg++;
      tProgLastTime = millis();

      //dirac_traverse_outwards(tProg, pred, 0);
      //dirac_traverse_outwards(tProg, pblue, 0);

      sine_traverse(tProg, pred, 1); 
      sine_traverse(tProg, pgreen, 0.2);
      sine_traverse(tProg, pblue, 0);

      enforce_base_color(pred, BASE_RED);
      enforce_base_color(pgreen, BASE_GREEN);
      enforce_base_color(pblue, BASE_BLUE);
      
  }
  if(run_setup && dtSetup > T_SETUP){
      // update time
      tSetup++;
      tSetupLastTime = millis();
      // update state
      //dirac_traverse_outwards(tProg, pred_t, 0);
      //dirac_traverse_outwards(tProg, pblue_t, 0);
      sine_traverse(tProg, pred_t, 1);
      sine_traverse(tProg, pgreen_t, 0.2);
      sine_traverse(tProg, pblue_t, 0);
      bool done = build_state_linear(pred_t, pgreen_t, pblue_t,
                        pred, pgreen, pblue, 1);
      if(done){
        run_setup = false;
        run_prog = true;
      }
  }

  set_pixels(pred, pgreen, pblue);  

}
