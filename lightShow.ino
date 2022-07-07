
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

// color setup
#define BASE_BRIGHTNESS 10
uint8_t BASE_RED = 0;
uint8_t BASE_BLUE = 0;
uint8_t BASE_GREEN = 20;
uint8_t cutColor(String color, uint8_t value){
  uint8_t v = value % 256;
  if(color == "green"){
    return v > BASE_GREEN ? v : BASE_GREEN;
  } else if(color == "red"){
    return v > BASE_RED ? v : BASE_RED;
  } else if(color == "blue"){
    return v > BASE_BLUE ? v : BASE_BLUE;
  } else {
    return v > BASE_BRIGHTNESS ? v : BASE_BRIGHTNESS;
  }
}

// hardcoded sinus wave, discretized values stored in sinusgamma
uint8_t sinusgamma[256];
void calcSinusGamma(uint8_t *arr){
  for(int i = 0; i < 256; i ++){
    arr[i] = cutColor("base", strip.gamma8(strip.sine8(i)));
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
void sine_traverse(unsigned long t, uint8_t * state){
  for(int i = 0; i < LED_COUNT; i++){
     uint8_t t_mod = (i + t) % 256;
     state[i] = sinusgamma[t_mod];
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

  calcSinusGamma(sinusgamma);

  //Serial.begin(9600);
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(BRIGHTNESS);
}

#define T_PROG 15
unsigned long tProg = 0;
bool run_prog = false;

#define T_SETUP 7
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

      sine_traverse(tProg, pred);
  }
  if(run_setup && dtSetup > T_SETUP){
      // update time
      tSetup++;
      tSetupLastTime = millis();
      // update state
      sine_traverse(tProg, pred_t);
      bool done = build_state_linear(pred_t, pgreen_t, pblue_t,
                        pred, pgreen, pblue, 1);
      if(done){
        run_setup = false;
        run_prog = true;
      }
  }

  set_pixels(pred, pgreen, pblue);  

}
