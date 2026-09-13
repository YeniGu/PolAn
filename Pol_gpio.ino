/*
 * PolTest ESP32 Control
 * IOs, Buttons and LED
 */

// Pins Test

 #define TPIN_1      4  // input, pullups
 #define TPIN_2      5
 #define TPIN_3      6
 #define TPIN_4      7

 #define LS_RD       40  // output light sources
 #define LS_YE       41
 #define LS_GN       42



// +++ Init GPIO +++

void init_gpio() {
  pinMode(TPIN_1, INPUT_PULLUP);  // test pin inputs, pullups
  pinMode(TPIN_2, INPUT_PULLUP);
  pinMode(TPIN_3, INPUT_PULLUP);
  pinMode(TPIN_4, INPUT_PULLUP);

  pinMode(LED_1, OUTPUT);  // test LED 1
  digitalWrite(LED_1, HIGH);  // LED 1 on
  pinMode(LED_2, OUTPUT);  // test LED 2
  digitalWrite(LED_2, LOW);  // LED 2 off

  pinMode(LS_RD, OUTPUT);  // light source red
  digitalWrite(LS_RD, LOW);  
  pinMode(LS_YE, OUTPUT);  // light source yellow
  digitalWrite(LS_YE, LOW);  
  pinMode(LS_GN, OUTPUT);  // light source green
  digitalWrite(LS_GN, LOW);  

  rgbLedWrite(RGB_BUILTIN, 0, 0, 0);
}

// +++ Read buttons +++

void update_gpio() {



}

// +++ Button handler +++

void update_buttons() {



}

// +++ WS2812 Neopixel LED +++

void update_wsled(int val) {
  if (wifiset.ota) {
    rgbLedWrite(RGB_BUILTIN, 0, val, 0); // green > Upload
  } else {
    if (wifiset.con) {
      if (wifiset.ap) {
        rgbLedWrite(RGB_BUILTIN, val, 0, val); // magenta
      } else {
        rgbLedWrite(RGB_BUILTIN, 0, 0, val); // blue      
      }
    }
  }
}

// +++ Light Sources on/off+++

void set_lightsource(int state) {
  if (state == 0) {  
    digitalWrite(LS_RD, LOW);  // dark 
    digitalWrite(LS_YE, LOW);  // dark 
    digitalWrite(LS_GN, LOW);  // dark 
  } else {
    digitalWrite(LS_RD, HIGH);  // light
    digitalWrite(LS_YE, HIGH);  // light
    digitalWrite(LS_GN, HIGH);  // light
  }
}

