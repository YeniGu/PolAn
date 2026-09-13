/*
 * PolTest ESP32 Control
 * Setup
 */

// +++ Init Variables and Arrays +++

void init_prog() {
  startup = TSTARTUP;


  

  ccal.read = true;
  ccal.write = false;

//  prefs_getdata();  //  Starting values Range, Mode
}

// +++ Startup and Cooldown +++

void cooldown_timer() {
  if (startup > 0) {  // 5 seconds start
    startup--;
    if (startup == 0) {
      setup_wifi();  // Wifi settings and first connection
    }
  }  

}

