/*
 * PolTest ESP32 Control
 * ESP32-S3-WROOM-1 Modul 2 x USB-C
 * compile with ESP32 S3 Dev Module
*/

#include <WiFi.h>          // Wifi Event needs to be before Main
#include "Pol_config.h"    // Configuration



void setup(void) {
  init_prog();

  delay(100);

  init_uart();
  init_gpio();

  init_filesys();

  init_hspi();
  init_ads();

//  init_display();
  init_wifi();  // Wifi just init, not start yet
  delay(50);

  oldtime = millis();
}

// +++ Mainloop +++ Tasks 10 msec +++

void loop(void) {
  unsigned long newtime = millis();
  delay(1);

  if ((newtime - oldtime) >= 10) {  // each 10 msec, millis overflow every 50 days
    oldtime = newtime;  

    if (taskcnt == 0) {  // 1 sec "running"
      digitalWrite(LED_1, HIGH);  // LED 1 on
    } else if (taskcnt == 8) {
      digitalWrite(LED_1, LOW);  // LED 1 off
    }

    int task = taskcnt % 4;  // loop 40 msec
    if (task == 0) {
      ads_sample(0);  // sample channels dark

    } else if (task == 1) {  
      set_lightsource(1);  // light source on

    } else if (task == 2) {  
      ads_sample(1);  // sample channels light

    } else if (task == 3) {  
      set_lightsource(0);  // light source off
      data_store();  // store data for chart and csv
    }

    if (taskcnt % 2 == 1) {  // every 20 msec
      update_gpio();  
      update_buttons();
      check_webserver();
    }

    taskcnt += 1;
    if (taskcnt >= 100) {  // 100 x 10 msec = 1 sec
      cooldown_timer(); 
      uart_handle();
      update_wsled(RGB_LOW);
      taskcnt = 0;
    }
  }
}
