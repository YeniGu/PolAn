/*
 * PolTest ESP32 Control
 * Wifi
 */

//#include <WiFi.h>       // >> must go right at the beginning, for WifiEvent
#include <WebServer.h>  // standard library
#include <Update.h>
#include <ESPmDNS.h>

#include <LittleFS.h>
#include "Pol_html.h"   // .h file that stores your html page code

#define USE_INTRANET  // comment out when used as AP

#define LOCAL_SSID "your-ssid"  // Module as station in the local Wlan
#define LOCAL_PASS "your-pw"
#define STA_NAME   "esp32-logc"

#define AP_SSID "esp32-logc-ap"  // Module reachable as AP, comment out USE_INTRANET
#define AP_PASS "pass2esp"

WebServer server(80);  // Create HTML webserver

File fsUploadFile;  // LittleFS

// +++ Wifi Event Handler +++

void WiFiEvent(WiFiEvent_t event) {
  switch(event) {
    case ARDUINO_EVENT_WIFI_AP_START:
      Serial.println("AP start");
      WiFi.softAPsetHostname(AP_SSID);  //set ap hostname here
      break;
    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("STA start");
      WiFi.setHostname(wifiset.host.c_str());  //set sta hostname here
      break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("STA connected");
      break;
    case ARDUINO_EVENT_WIFI_AP_GOT_IP6:
      //both interfaces get the same event
      break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("STA got IP");
      wifiSTAconnect();
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      wifiSTAdisconnect();
      break;
    default:
      break;
  }
}

// +++ Init Filesys for HTML pages +++

void init_filesys() {
  // if (!LittleFS.begin(true)) {
  //   Serial.println("LittleFS init failed");
  // }
  if (!LittleFS.begin(false)) {
    Serial.println("LittleFS Mount fehlgeschlagen. Starte Formatierung...");
    if (LittleFS.format()) {
      Serial.println("Formatierung erfolgreich");
      if (LittleFS.begin(false)) {
        Serial.println("LittleFS erfolgreich gemountet");
        return;
      }
    }
    Serial.println("LittleFS init failed");
  } else {
    Serial.println("LittleFS erfolgreich gemountet");
  }
}

// +++ Init Wifi, do not start yet +++

void init_wifi() {
  wifiset.en   = true;
  wifiset.ssid = String(LOCAL_SSID);
  wifiset.pwd  = String(LOCAL_PASS);
  wifiset.host = String(STA_NAME);
  wifiset.statip = IPAddress(0, 0, 0, 0);
  wifiset.mask = IPAddress(0, 0, 0, 0);
  wifiset.dns1 = IPAddress(0, 0, 0, 0);
  wifiset.dns2 = IPAddress(0, 0, 0, 0);
  wifiset.gateway = IPAddress(0, 0, 0, 0);

  prefs_getwifi();  // Read Wifi settings from flash

  WiFi.disconnect(true);
  wifiset.con = false;
  wifiset.ota = false;
}

// +++ Start Wifi +++

void setup_wifi() {
  if (wifiset.en) {
    WiFi.onEvent(WiFiEvent);
    if (wifiset.ap || (wifiset.ssid.length() == 0)) {   // AP Mode
      WiFi.mode(WIFI_MODE_AP);
      IPAddress local_ip(192, 168, 178, 1);
      IPAddress local_gateway(192, 168, 178, 254);
      IPAddress local_mask(255, 255, 255, 0);
      WiFi.softAPConfig(local_ip, local_gateway, local_mask);
      WiFi.softAP(AP_SSID, AP_PASS);
      wifiset.ip = WiFi.softAPIP();

    } else {  // STA Mode
      WiFi.mode(WIFI_MODE_STA);  // new, station only, no AP
      if (wifiset.statip > 0) {  // if Static IP
        WiFi.config(wifiset.statip, wifiset.gateway, wifiset.mask, wifiset.dns1, wifiset.dns2);  // Static IP config
      }
      WiFi.setHostname(STA_NAME);  // before begin
      WiFi.begin(wifiset.ssid, wifiset.pwd);  // Connect to local Wlan
    }

    server.on("/favicon.ico", HTTP_GET, Send204_empty);  // catch favicon request
    server.on("/update", HTTP_GET, []() {
      server.send(200, "text/html", update_html_fallback);
    });
    server.on("/json", HTTP_GET, Sendjson_all);  // callback for JSON data request
    server.on("/csv", HTTP_GET, Sendcsv_all);  // callback for CSV data files
    server.on("/settings", HTTP_POST, Getsettings_all);  // Callback for Settings > POST from FORM
    server.on("/upload", HTTP_POST, handleUploadResponse, handleUploadStream);

    server.serveStatic("/", LittleFS, "/");

    server.onNotFound([]() {
      server.sendHeader("Location", "/index.html", true);  // redirect to index.html
      server.send(302, "text/plain", "");
    });

    const char* headerkeys[] = {"Content-Length"};  // store file-size for OTA progress
    server.collectHeaders(headerkeys, 1);

    server.begin();
  }
}

// +++ Wifi reset + restart +++

void reset_wifi() {
  WiFi.disconnect(true);
  wifiset.con = false;
  server.stop();
  wifiset.resetcnt = 40;  // Reset after 10 sec
}

// +++ Wifi connected +++

void wifiSTAconnect() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  long rssi = WiFi.RSSI();
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  MDNS.begin(wifiset.host);
  wifiset.con = true;
  wifiset.cdcon = ERR_CDOWN; 
  wifiset.ip   = WiFi.localIP();
  wifiset.mac  = WiFi.macAddress();
  wifiset.rssi = WiFi.RSSI();
  wifiset.resetcnt = 0;  // clear reset counter
  wifiset.retrycnt = 0;  // clear retry counter
}

// +++ Wifi disconnected +++

void wifiSTAdisconnect() {
  Serial.println("Wifi disconnected");

  if (wifiset.con) { // only if previously connected
    MDNS.end();
    wifiset.cddis = ERR_CDOWN;
  }
  wifiset.con = false;

  wifiset.retrycnt++;
  if (wifiset.retrycnt >= 15) {
    wifiset.ap = true;  // Switch to AP
    reset_wifi();
  }
}

// +++ Check webserver 4x per sec +++

void check_webserver() {
  server.handleClient();

  if (wifiset.resetcnt > 0) {
    wifiset.resetcnt--;
    if (wifiset.resetcnt == 0) {
      Serial.println("Wifi reset");
      setup_wifi();
    }
  }
  wifiset.rssi = WiFi.RSSI();
}

// +++ Send webpage from html.h +++

// void Sendhtml_index() {
//   unsigned long otime = micros();
//   String tval;
//   String tpage = String(header_html);
//   if (server.args() > 0) {
//     if (server.argName(0) == "query") {  // // first Arg query
//       tval = server.arg(0);
//       if (tval == "chart") {
//         tpage += String(radio_css);
//         tpage += String(navbar_html);
//         tpage += String(chart_html);
//         // dcsv[0].oldcount = 0;  // reset position data for chart
//         // dcsv[1].oldcount = 0;
//         // dcsv[2].oldcount = 0;
//       } else if (tval == "data") {
//         tpage += String(button_css);
//         tpage += String(navbar_html);
//         tpage += String(data_html);
//       } else if (tval == "setup") {
//         tpage += String(submit_css);
//         tpage += String(navbar_html);
//         tpage += String(setup_html);
//       } else if (tval == "system") {
//         tpage += String(system_css);
//         tpage += String(navbar_html);
//         tpage += String(system_html);
//       } else if (tval == "update") {
//         tpage += String(submit_css);
//         tpage += String(upload_css);
//         tpage += String(navbar_html);
//         tpage += String(update_html);  
//       } else {  // ArgVal not found or "live" > main page
//         tpage += String(live_css);
//         tpage += String(navbar_html);
//         tpage += String(live_html);
//       }
//     } else {  // Arg is not query > main page
//       tpage += String(live_css);
//       tpage += String(navbar_html);
//       tpage += String(live_html);
//     }
//   } else {  // no Args > main page
//     tpage += String(live_css);
//     tpage += String(navbar_html);
//     tpage += String(live_html);
//   }
// //  Serial.print("sending page "); Serial.print(tval); Serial.print(" : size "); Serial.print(tpage.length()); Serial.print(" : us "); Serial.println(micros() - otime);

//   server.sendHeader("Connection", "close");
//   server.send(200, "text/html", tpage);
// };

void Send204_empty() {
  // server.sendHeader("Connection", "close");
  // server.send(204, "text/plain", "");  // // Response 204 No Content
  server.send(204);  // // Response 204 No Content
};

// +++ OTA + File Update Handler +++

void handleUploadResponse() {
  server.sendHeader("Connection", "close");
  if (wifiset.ota) {
    if (Update.hasError()) {
      server.send(500, "text/plain", "OTA FAIL");
    } else {
      server.send(200, "text/plain", "OTA OK");
      delay(500);
      ESP.restart(); // Reboot only on firmware update
    }
    wifiset.ota = false;
  } else {
    server.send(200, "text/plain", "File Saved");
  }
}

// +++ OTA + File Uploader +++

void handleUploadStream() {
  static int oldprog = -5;
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {
    oldprog = -5;
    if (upload.filename.endsWith(".bin")) {  // Firmware-Upload to Flash
      wifiset.ota = true;
      size_t fileSize = server.header("Content-Length").toInt();
      size_t updateSize = (fileSize > 0) ? fileSize : UPDATE_SIZE_UNKNOWN;

      if (!Update.begin(updateSize, U_FLASH)) {
//        display_draw_update(OTA_ERROR);
      } else {
//        display_draw_update(OTA_START);
      }
    } else {  // HTML, CSS... Upload to LittleFS
      wifiset.ota = false;
      String filename = upload.filename;
      if (!filename.startsWith("/")) filename = "/" + filename;
      fsUploadFile = LittleFS.open(filename, "w");
    }

  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (wifiset.ota) {
      // Firmware flashen
      if (Update.write(upload.buf, upload.currentSize) == upload.currentSize) {
        size_t total_file_size = server.header("Content-Length").toInt();
        if (total_file_size > 0) {
          int newprog = (upload.totalSize * 100) / total_file_size;
          if (newprog >= oldprog + 5) {
//            display_draw_update(OTA_WRITE, newprog);
            update_wsled(newprog);
            oldprog = newprog;
          }
        }
      }
    } else {
      // Write to file
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    }

  } else if (upload.status == UPLOAD_FILE_END) {
    if (wifiset.ota) {
      if (Update.end(true)) {
//        display_draw_update(OTA_END);
      } else {
//        display_draw_update(OTA_ERROR);
      }
    } else {
      if (fsUploadFile) {
        fsUploadFile.close();
      }
    }
    oldprog = -5;
  }
}

// +++ Send data in JSON +++

void Sendjson_all() {
  update_wsled(RGB_FULL);

  String tjson;
  String tval;
  bool jsonok = false;
  unsigned long otime = micros();

  if (server.args() > 0) {
    if (server.argName(0) == "query") {  // first Arg query
      tval = server.arg(0);
      if (tval == "live") {  // Data page "live"
        tjson.reserve(512);
        tjson =  "{\"rssi\":"; tjson += String(wifiset.rssi);  // always send RSSI, dummy otherwise causes JSON parser error
        tjson += ",\"count\":";    tjson += String(chan[0].count);
        tjson += ",\"channels\":[";
        for (int i = 0; i < 6; i++) {
          if (i > 0) tjson += ",";  // set colon between values        
          tjson += "{\"mean\":";   tjson += String(chan[i].mean, 3);
          tjson += ",\"signal\":"; tjson += String(chan[i].signal, 3);
          tjson += ",\"light\":";  tjson += String(chan[i].light, 3);
          tjson += ",\"dark\":";   tjson += String(chan[i].dark, 3);
          tjson += "}";
        }
        tjson += "]}";
        jsonok = true;

      } else if (tval == "chart") {  // Data Page "chart"
        tjson =  "{\"rssi\":"; tjson += String(wifiset.rssi);  // always send RSSI, dummy otherwise causes JSON parser error
        if (dcsv.count != dcsv.oldcount) {  // Pos data changed, reset or not all sent
          String tcount = "";
          String ttime = "";
          String ch_mean[CHANMAX];
          String ch_signal[CHANMAX];          
          tcount.reserve(256); ttime.reserve(256);
          for (int ch = 0; ch < CHANMAX; ch++) {
            ch_mean[ch].reserve(256); 
            ch_signal[ch].reserve(256);
          }
          int n = 0;
          int pos = (dcsv.dpos + 1) % DAT_LEN;  // oldest value in the ring buffer

          for (int k = 0; k < DAT_LEN; k++) {  // go through ring buffer
            unsigned long dval = dattim[pos].count;
            if (dval > dcsv.oldcount) {  // new values
              dcsv.oldcount = dval;  // remember last sent data
              n++;  // number of data points
              tcount += String(dval);
              ttime  += String(dattim[pos].msec / 1000);
              for (int ch = 0; ch < CHANMAX; ch++) {
                ch_mean[ch]   += String(datval[ch][pos].mean, 3);
                ch_signal[ch] += String(datval[ch][pos].signal, 3);
              }
              if (dval >= dcsv.count) {  // newest value, end
                break;
              }
              if (n >= 20) {  // send max 20 data points at once
                break;
              }
              tcount += ",";
              ttime += ",";
              for (int ch = 0; ch < CHANMAX; ch++) {
                ch_mean[ch] += ",";
                ch_signal[ch] += ",";
              }
            }
            pos = (pos + 1) % DAT_LEN;  // go through ring buffer
          }
          tjson.reserve(2048);
          tjson += ",\"count\":["; tjson += tcount;
          tjson += "],\"time\":[";  tjson += ttime;
          tjson += "],\"channels\":[";
          for (int ch = 0; ch < CHANMAX; ch++) {
            tjson += "{\"mean\":[";   tjson += ch_mean[ch];
            tjson += "],\"signal\":["; tjson += ch_signal[ch];
            tjson += "]}";
            if (ch < CHANMAX - 1) tjson += ",";
          }
          tjson += "]"; // end of channels array
        }

        tjson += "}"; // close JSON
        jsonok = true;

      } else if (tval == "setup") {  // Data page "setup"
        tjson.reserve(512);
        tjson =  "{\"wifi\":{";
        tjson += "\"host\":\""      ; tjson += wifiset.host;
        tjson += "\",\"ssid\":\""   ; tjson += wifiset.ssid;
        tjson += "\",\"pwd\":\""    ; tjson += wifiset.pwd;
        tjson += "\",\"statip\":\"" ; tjson += wifiset.statip.toString();
        tjson += "\",\"mask\":\""   ; tjson += wifiset.mask.toString();
        tjson += "\",\"dns1\":\""   ; tjson += wifiset.dns1.toString();
        tjson += "\",\"dns2\":\""   ; tjson += wifiset.dns2.toString();
        tjson += "\",\"gateway\":\""; tjson += wifiset.gateway.toString();
        tjson += "\"},\"cal\":{";
        tjson += "\"s0\":"       ; tjson += String(ccal.scal[0]);
        tjson += ",\"s1\":"      ; tjson += String(ccal.scal[1]);
        tjson += ",\"s2\":"      ; tjson += String(ccal.scal[2]);
        tjson += ",\"s3\":"      ; tjson += String(ccal.scal[3]);
        tjson += ",\"s4\":"      ; tjson += String(ccal.scal[4]);
        tjson += ",\"s5\":"      ; tjson += String(ccal.scal[5]);
        tjson += "},\"test\":{";
        tjson += "\"t1\":"       ; tjson += String(0);
        tjson += ",\"t2\":"      ; tjson += String(0);
        tjson += ",\"t3\":"      ; tjson += String(0);
        tjson += ",\"t4\":"      ; tjson += String(0);
        tjson += ",\"t5\":"      ; tjson += String(0);
        tjson += "}}";  // ca. 300 Byte
        jsonok = true;

      } else if (tval == "system") {  // Data page "system"
        tjson.reserve(512);
        tjson =  "{\"system\":{";
        tjson += "\"ip\":\""        ; tjson += wifiset.ip.toString();
        tjson += "\",\"host\":\""   ; tjson += wifiset.host;
        tjson += "\",\"rssi\":"     ; tjson += String(wifiset.rssi);
        tjson += ",\"mac\":\""      ; tjson += wifiset.mac;
        tjson += "\",\"frequ\":"      ; tjson += String(getCpuFrequencyMhz());
        tjson += ",\"hfree\":"      ; tjson += String(ESP.getFreeHeap());
        tjson += ",\"hsize\":"      ; tjson += String(ESP.getHeapSize());
        tjson += ",\"uptime\":"     ; tjson += String(millis() / 1000);
        tjson += ",\"version\":\""  ; tjson += VERSION;
        tjson += "\",\"build\":\""  ; tjson += BUILDTIME;
        tjson += "\"}}";  // ca. 300 Byte
        jsonok = true;
      }
    }
  }
  if (jsonok) {  // Send JSON
//    Serial.print("Json size "); Serial.print(tjson.length()); Serial.print(" : us "); Serial.println(micros() - otime);

    server.send(200, "application/json", tjson);
  } else {
    server.send(204);  // no Args, stay on page
  }  
  update_wsled(RGB_LOW);
}

// +++ Send data as CSV +++

void Sendcsv_all() {
  update_wsled(RGB_FULL);

  String tcsv;
  String tval;
  String tfile;
  int k, pos, cs;
  bool csvok = false;
  if (server.args() > 0) {
    if (server.argName(0) == "query") {  // first Arg query
      tcsv.reserve(32768);
      tcsv = "Log Current Data\n";
      String thead = "Time; Current; Peak; Min; Max\nsec; mA; mA; mA; mA\n";     
      tval = server.arg(0);
      if (tval.startsWith("curr")) {  // data
        if (tval.charAt(4) == '1') {  // counts from zero
          tfile = "\"logcurr_1sec.csv\"";  // individual filename for header, in quotes
          tcsv += "LogC 1sec\n\n";
          cs = 0;
        } else if (tval.charAt(4) == '2') {
          tfile = "\"logcurr_15sec.csv\"";  // individual filename for header, in quotes
          tcsv += "LogC 15sec\n\n";
          cs = 1;
        } else if (tval.charAt(4) == '3') {
          tfile = "\"logcurr_5min.csv\"";  // individual filename for header, in quotes
          tcsv += "LogC 5min\n\n";
          cs = 2;
        }      
        tcsv += thead;  // Table header
        // pos = (dcsv[cs].dpos + 1) % DAT_LEN;  // oldest value in the ring buffer
        // for (k = 0; k < DAT_LEN; k++) {  // go through ring buffer
        //   if (dattim[cs][pos].count > 0) {  // if valid entry
        //     tcsv += String(double(dattim[cs][pos].msec) / 1000, 1); tcsv += "; ";     
        //     tcsv += String(datval[cs][pos].mean, 3); tcsv += "; ";     
        //     tcsv += String(datval[cs][pos].peak, 3); tcsv += "; ";     
        //     tcsv += String(datval[cs][pos].min , 3); tcsv += "; ";     
        //     tcsv += String(datval[cs][pos].max , 3); tcsv += "\n";     
        //   }
        //   pos = (pos + 1) % DAT_LEN;  // go through ring buffer
        // }
        csvok = true;
      }
    }
  }
  if (csvok) {  // Send CSV
//    Serial.print("CSV "); Serial.print(tfile); Serial.print(" : "); Serial.println(tcsv.length());

    server.sendHeader("Content-Type", "text/csv");
    server.sendHeader("Content-Disposition", String("attachment; filename=") + tfile);
    server.sendHeader("Connection", "close");
    server.send(200, "text/csv", tcsv);  // send CSV 
  } else {
    server.send(204);  // no args, stay on page
  }  
  update_wsled(RGB_LOW);
}

// +++ Apply setup +++

void Getsettings_all() {
  update_wsled(RGB_FULL);

  String tname;
  String tval;

  String ssid;
  String pwd;
  IPAddress statip(0, 0, 0, 0);
  IPAddress mask(0, 0, 0, 0);
  IPAddress dns1(0, 0, 0, 0);
  IPAddress dns2(0, 0, 0, 0);
  IPAddress gateway(0, 0, 0, 0);
  IPAddress brokerip(0, 0, 0, 0);
  int  port;

  if (server.args() > 0) {
    if (server.argName(0) == "query") {  // first Arg query
      tval = server.arg(0);
      if (tval == "wifi") {  // Wifi Settings
        for (int i = 1; i < server.args(); i++) {  // go through remaining Args
          tname = server.argName(i);
          tval = server.arg(i);

          // Serial.print("Wifi: ");
          // Serial.print(tname);
          // Serial.print(": ");
          // Serial.println(tval);

          if (tname == "host") {
            if ((tval.length() > 3) && (tval.length() <= 32)) {
              wifiset.host = tval;
            }  
          } else if (tname == "ssid") {
            if ((tval.length() > 0) && (tval.length() <= 32)) {
              ssid = tval;
            }  
          } else if (tname == "pwd") {
            if ((tval.length() > 7) && (tval.length() <= 32)) {
              pwd = tval;
            }  
          } else if (tname == "statip") {
            if ((tval.length() > 6) && (tval.length() <= 15)) {
              statip.fromString(tval);
            }  
          } else if (tname == "mask") {
            if ((tval.length() > 6) && (tval.length() <= 15)) {
              mask.fromString(tval);
            }  
          } else if (tname == "dns1") {
            if ((tval.length() > 6) && (tval.length() <= 15)) {
              dns1.fromString(tval);
            }  
          } else if (tname == "dns2") {
            if ((tval.length() > 6) && (tval.length() <= 15)) {
              dns2.fromString(tval);
            }  
          } else if (tname == "gateway") {
            if ((tval.length() > 6) && (tval.length() <= 15)) {
              gateway.fromString(tval);
            }  
          }
        }
        if ((ssid.length() > 0) && (pwd.length() > 7)) {  // if SSID and PWD were sent
          wifiset.ssid = ssid;
          wifiset.pwd = pwd;
          wifiset.ap = false;  // STA mode after reset
        }
        if (statip > 0) {  // if Static IP was sent
          wifiset.statip = statip;
          wifiset.mask = mask;
          wifiset.dns1 = dns1;
          wifiset.dns2 = dns2;
          wifiset.gateway = gateway;
        } else {
          wifiset.statip = IPAddress(0, 0, 0, 0);
          wifiset.mask = IPAddress(0, 0, 0, 0);
          wifiset.dns1 = IPAddress(0, 0, 0, 0);
          wifiset.dns2 = IPAddress(0, 0, 0, 0);
          wifiset.gateway = IPAddress(0, 0, 0, 0);
        }
        prefs_setwifi();  // Write Wifi settings to flash
        reset_wifi();  // Restart Wifi

      } else if (tval == "cal") {  // Cal settings
        for (int i = 1; i < server.args(); i++) {  // go through remaining args
          tname = server.argName(i);
          tval = server.arg(i);

          // Serial.print("Cal: ");
          // Serial.print(tname);
          // Serial.print(": ");
          // Serial.println(tval);

          if (tname == "s0") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[0] = tval.toFloat();
            }  
          } else if (tname == "s1") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[1] = tval.toFloat();
            }  
          } else if (tname == "s2") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[2] = tval.toFloat();
            }  
          } else if (tname == "s3") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[3] = tval.toFloat();
            }  
          } else if (tname == "s4") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[4] = tval.toFloat();
            }  
          } else if (tname == "s5") {
            if ((tval.length() > 0) && (tval.length() <= 8)) {
              ccal.scal[5] = tval.toFloat();
            }  
          }
        }
        prefs_setcal();  // Write Cal settings to flash
        ccal.write = true;

      // } else if (tval == "inst") {  // Instrument settings
      //   for (int i = 1; i < server.args(); i++) {  // go through remaining Args
      //     tname = server.argName(i);
      //     tval = server.arg(i);
      //     if (tname == "scal1") {
      //       if ((tval.length() > 0) && (tval.length() <= 4)) {
      //         ccal.s1 = tval.toInt();
      //       }  
      //     } else if (tname == "scal2") {
      //       if ((tval.length() > 0) && (tval.length() <= 4)) {
      //         ccal.s2 = tval.toInt();
      //       }  
      //     } else if (tname == "scal3") {
      //       if ((tval.length() > 0) && (tval.length() <= 4)) {
      //         ccal.s3 = tval.toInt();
      //       }  
      //     }
      //   }
      //   prefs_setdata();  // Write Inst settings to flash
      //   ccal.write = true;  // Send Cal to AVR on next cycle
      }
    }
  }
  server.send(204);  // stay on page
  update_wsled(RGB_LOW);
}

