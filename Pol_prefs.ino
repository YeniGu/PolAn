/*
 * PolTest ESP32 Control
 * Preferences store/load from flash
 */

#include <Preferences.h>

Preferences prefs;

// Write settings, only changed ones

void prefs_setcal() {
  prefs.begin("data", false);  // Open Namespace, read and write, do not CLEAR!
  if (prefs.getFloat("scal0", 1.0)      != ccal.scal[0])   prefs.putFloat("scal0", ccal.scal[0]);
  if (prefs.getFloat("scal1", 1.0)      != ccal.scal[1])   prefs.putFloat("scal1", ccal.scal[1]);
  if (prefs.getFloat("scal2", 1.0)      != ccal.scal[2])   prefs.putFloat("scal2", ccal.scal[2]);
  if (prefs.getFloat("scal3", 1.0)      != ccal.scal[3])   prefs.putFloat("scal3", ccal.scal[3]);
  if (prefs.getFloat("scal4", 1.0)      != ccal.scal[4])   prefs.putFloat("scal4", ccal.scal[4]);
  if (prefs.getFloat("scal5", 1.0)      != ccal.scal[5])   prefs.putFloat("scal5", ccal.scal[5]);
  // if (prefs.getInt("range", 0)       != (int)cset.rset)  prefs.putInt("range", (int)cset.rset);
  // if (prefs.getInt("mode", 0)        != (int)cset.mode)  prefs.putInt("mode", (int)cset.mode);
  // if (prefs.getBool("auto", false)   != cset.aut)    prefs.putBool("auto", cset.aut);
  // if (prefs.getBool("log", false)    != cset.log)    prefs.putBool("log", cset.log);
  // if (prefs.getBool("bi", false)     != cset.bi)     prefs.putBool("bi", cset.bi);
  // if (prefs.getBool("filter", false) != cset.filter) prefs.putBool("filter", cset.filter);
  // if (prefs.getBool("slow", false)   != cset.slow)   prefs.putBool("slow", cset.slow);
  prefs.end();  //  Close Namespace
}

// Read settings

void prefs_getcal() {
  prefs.begin("data", true);  //  Open Namespace, read only
  ccal.scal[0] = prefs.getFloat("scal0", ccal.scal[0]);
  ccal.scal[1] = prefs.getFloat("scal1", ccal.scal[1]);
  ccal.scal[2] = prefs.getFloat("scal2", ccal.scal[2]);
  ccal.scal[3] = prefs.getFloat("scal3", ccal.scal[3]);
  ccal.scal[4] = prefs.getFloat("scal4", ccal.scal[4]);
  ccal.scal[5] = prefs.getFloat("scal5", ccal.scal[5]);
  // cset.rset   = (Erange)prefs.getInt("range", (int)cset.rset);
  // cset.mode   = (Emode)prefs.getInt("mode", (int)cset.mode);
  // cset.aut    = prefs.getBool("auto", cset.aut);
  // cset.log    = prefs.getBool("log", cset.log);
  // cset.bi     = prefs.getBool("bi", cset.bi);
  // cset.filter = prefs.getBool("filter", cset.filter);
  // cset.slow   = prefs.getBool("slow", cset.slow);
  prefs.end();  // Close Namespace
}

void prefs_setwifi() {
  prefs.begin("wifi", false);  // Open Namespace
  prefs.clear();  //  Delete entries
  prefs.putBool("en", wifiset.en);
  prefs.putBool("ap", wifiset.ap);
  prefs.putString("ssid", wifiset.ssid);
  prefs.putString("pwd", wifiset.pwd);
  prefs.putString("host", wifiset.host);
  prefs.putString("ssid", wifiset.ssid);
  prefs.putUInt("statip", (uint32_t)wifiset.statip);
  prefs.putUInt("mask", (uint32_t)wifiset.mask);
  prefs.putUInt("dns1", (uint32_t)wifiset.dns1);
  prefs.putUInt("dns2", (uint32_t)wifiset.dns2);
  prefs.putUInt("gateway", (uint32_t)wifiset.gateway);
  prefs.end();  // Close Namespace
}

void prefs_getwifi() {
  prefs.begin("wifi", true);  // Open Namespace, read only
  wifiset.en      = prefs.getBool("en", wifiset.en);
  wifiset.ap      = prefs.getBool("ap", wifiset.ap);
  wifiset.ssid    = prefs.getString("ssid", wifiset.ssid);
  wifiset.pwd     = prefs.getString("pwd", wifiset.pwd);
  wifiset.host    = prefs.getString("host", wifiset.host);
  wifiset.ssid    = prefs.getString("ssid", wifiset.ssid);
  wifiset.statip  = prefs.getUInt("statip", (uint32_t)wifiset.statip);
  wifiset.mask    = prefs.getUInt("mask", (uint32_t)wifiset.mask);
  wifiset.dns1    = prefs.getUInt("dns1", (uint32_t)wifiset.dns1);
  wifiset.dns2    = prefs.getUInt("dns2", (uint32_t)wifiset.dns2);
  wifiset.gateway = prefs.getUInt("gateway", (uint32_t)wifiset.gateway);
  prefs.end();  // Close Namespace
}
