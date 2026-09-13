/*
 * PolTest ESP32 Control
 * Configuration
*/


#define VERSION     "PolTest ESP32-S3 2026"
#define BUILDTIME   __DATE__ " " __TIME__

//#define LED_BL      2  // LED blue onboard
#define LED_1       1
#define LED_2       2

#define TSTARTUP    5  // 5 sec

#define CH_P0       0
#define CH_P90      1
#define CH_P45      2
#define CH_CIRC     3
#define CH_REF      4
#define CH_6        5

#define CHANMAX     6

#define DAT_LEN     1000

#define OTA_NONE    0
#define OTA_START   1
#define OTA_WRITE   2
#define OTA_END     3
#define OTA_ERROR   9

#define ERR_CDOWN   20  // Error cooldown 2sec

#define RGB_LOW     8
#define RGB_FULL    255


// global variables

int taskcnt = 0;
int dcnt;
int chartset = 0;
unsigned long oldtime;
unsigned long datatime;
int startup;



struct Tchan {
  int   count;
  int32_t  adc;  // 24 bit
  bool  ok;
  float light;
  float dark;
  float signal;
  float mean;
};
Tchan chan[CHANMAX];  // channel samples

struct Tdtim {
  unsigned long count;
  unsigned long msec;
};
Tdtim dattim[DAT_LEN];

struct Tdata {
  float signal;
  float mean;
};
Tdata datval[CHANMAX][DAT_LEN];

struct Tdcsv {
  unsigned long oldcount;
  unsigned long count;
  int  dpos;
  int  icnt;
};
Tdcsv dcsv;

struct Tcal {
  bool read;
  bool write;
  float scal[CHANMAX];
};
Tcal ccal;

struct Terror {
  int  wdt;
  int  adt;
  int  buf;
  int  uover;
  int  iover;
};
Terror error;

struct Twifi {  
  String ssid;
  String pwd;
  String host;
  String mac;
  IPAddress ip;
  IPAddress statip;
  IPAddress mask;
  IPAddress dns1;
  IPAddress dns2;
  IPAddress gateway;
  bool en;
  bool ap;
  int  rssi;
  int  retrycnt;
  int  resetcnt;
  bool con;
  bool ota;
  int  cdcon;
  int  cddis;
};
Twifi wifiset;



