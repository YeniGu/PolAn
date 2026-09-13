/*
 * PolTest ESP32 Control
 * ADS1220 Data Aquisition
 */

// ADS1220 control bytes

#define CADS_MUX03     0x20    // MUX 0010 : AIN0 pos, AIN3 neg
#define CADS_MUX13     0x40    // MUX 0100 : AIN1 pos, AIN3 neg
#define CADS_MUX23     0x50    // MUX 0101 : AIN2 pos, AIN3 neg
#define CADS_MUXZERO   0xE0    // MUX 1110 : pos+neg at AVDD/2 = offset
#define CADS_GAIN8     0x06    // GAIN 011 : 8 = -256..256mV
#define CADS_GAIN16    0x08    // GAIN 100 : 16 = -128..128mV
#define CADS_GAIN32    0x0A    // GAIN 101 : 32 = -64..64mV
#define CADS_GAIN64    0x0C    // GAIN 110 : 64 = -32..32mV
#define CADS_GAIN128   0x0E    // GAIN 111 : 128 = -16..16mV
                               // BYPASS off

#define CADS_DR20      0x00   // Data Rate 000 : 20 sps
#define CADS_DR175     0x60   // Data Rate 011 : 175 sps
#define CADS_DR330     0x80   // Data Rate 100 : 330 sps
#define CADS_DR600     0xA0   // Data Rate 101 : 600 sps
#define CADS_DR1000    0xC0   // Data Rate 110 : 1000 sps
#define CADS_MODE      0x00   // MODE 00 : normal
#define CADS_CONSIN    0x00   // Conv Mode 0 : single shot
#define CADS_CONCON    0x04   // Conv Mode 4 : continuous used for 50 Hz filter
        // Temp Sens 0
        // Curr Source 0 : off

#define CADS_VREF      0x00   // VREF 00 : 2048mV Ref
#define CADS_Fnone     0x00   // FILT 00 : kein, 01 : 50/60Hz
#define CADS_F50       0x10   // FILT 10 : 50Hz, 11 : 60Hz
        // PSW 0
        // Curr set 000 : off

#define CADS_DRDY      0x02   // DRDY Mode 1 : Data ready an DOUT
#define CADS_START     0x08   // Start Command
#define CADS_RESET     0x06
#define CADS_PDOWN     0x02   // Power down
#define CADS_RDATA     0x10
#define CADS_WREG      0x40   // write register
#define CADS_RREG      0x20   // read register
#define CADS_REG0      0x00   // MUX:GAIN:BYPASS
#define CADS_REG1      0x01   // DR:MODE:CONV:TS:BCS
#define CADS_REG2      0x02   // VREF:FILT:PSW:IDAC
#define CADS_REG3      0x03   // IMUX:DRDY    


// +++ Init ADS and arrays +++

void init_ads() {
  for (int k = 0; k < CHANMAX; k++) {
    chan[k].light = 0.0;
    chan[k].dark = 0.0;
    chan[k].signal = 0.0;
    chan[k].mean = 0.0;
  }

  for (int k = 0; k < CHANMAX; k++) {
    ccal.scal[k] = 1.0;  // standard calibration 1µA/mV
  }
  prefs_getcal();  // calibration from flash

  hspi_enable_csA();  // init ADS A
  ads_reset();  // reset ADS
  ads_set_config();
//  ads_startconv();  // start first sample
  hspi_disable_csA();  
  delayMicroseconds(200);

  hspi_enable_csB();  // init ADS B
  ads_reset();  // reset ADS
  ads_set_config();
//  ads_startconv();  // start first sample
  hspi_disable_csB();  
}


// +++ ADS1220 Reset +++

void ads_reset() {
  hspi_transfer_byte(CADS_RESET);
  delayMicroseconds(200);  // min 175µs wait
}  

// +++ ADS1220 Powerdown +++

void ads_powerdown() {
  hspi_transfer_byte(CADS_PDOWN);
  delayMicroseconds(200);  // min 175µs wait
}  

// +++ ADS1220 Start Conversion +++

void ads_startconv() {
  hspi_transfer_byte(CADS_START); 
}  

// +++ ADS1220 Write Register +++

void ads_write_register(byte reg, byte data) {
  byte buf[2];

  buf[0] = CADS_WREG | (reg << 2);  // write register
  buf[1] = data;  // config register
  hspi_transfer(buf, 2);  // write command
}

// +++ ADS1220 Set Config +++

void ads_set_config() {
  ads_write_register(CADS_REG0, CADS_MUX03 | CADS_GAIN16);    // MUX:GAIN:BYPASS
  ads_write_register(CADS_REG1, CADS_DR600 | CADS_MODE | CADS_CONSIN);    // DR:MODE:CONV:TS:BCS
  ads_write_register(CADS_REG2, CADS_VREF | CADS_Fnone);    // VREF:FILT:PSW:IDAC
  ads_write_register(CADS_REG3, CADS_DRDY);    // IMUX:DRDY
}     

// +++ ADS1220 Set Channel +++

void ads_set_channel(int chan) {
  if (chan == 1) {
    ads_write_register(CADS_REG0, CADS_MUX13 | CADS_GAIN16);    // MUX:GAIN:BYPASS
  } else if (chan == 2) {
    ads_write_register(CADS_REG0, CADS_MUX23 | CADS_GAIN16);    // MUX:GAIN:BYPASS
  } else {
    ads_write_register(CADS_REG0, CADS_MUX03 | CADS_GAIN16);    // MUX:GAIN:BYPASS
  }
}     

// +++ ADS1220 Read Data 24 bit +++

int32_t ads_readdata() {
  byte buf[4];

  buf[0] = 0x00;
  buf[1] = 0x00;
  buf[2] = 0x00;
  buf[3] = 0x00;  // byte 4 dummy for line high, see datasheet 8.5.6
  hspi_transfer(buf, 4);

  int32_t vval = (uint32_t)buf[0] << 16 | (uint32_t)buf[1] << 8 |(uint32_t)buf[2];
  if (buf[0] & 0x80) {  // if negativ
    vval |= 0xFF000000;  // extend neg
  }
  return vval;
}     

// +++ ADS1220 Sample 2 x 3 Channels +++

void ads_sample(int state) {
  for (int ch = 0; ch < 3; ch++) {
    hspi_enable_csA();  // start sample ADS A
    ads_set_channel(ch);
    ads_startconv();
    hspi_disable_csA();

    hspi_enable_csB();  // start sample ADS B
    ads_set_channel(ch);
    ads_startconv();
    hspi_disable_csB();

    bool readyA = false;
    bool readyB = false;

    for (int k = 0; k < 20; k++) {
      if (!readyA) {
        hspi_enable_csA();
        if (hspi_dataready()) readyA = true;  // check DRDY A
        hspi_disable_csA();
      }
      if (!readyB) {
        hspi_enable_csB();
        if (hspi_dataready()) readyB = true;  // check DRDY B
        hspi_disable_csB();
      }
      if (readyA && readyB) break;  // booth finished
      delayMicroseconds(100);
    }

    chan[ch].ok = readyA;
    if (readyA) {
      hspi_enable_csA();
      chan[ch].adc = ads_readdata();  // read sample value A
      chan[ch].count++;
      hspi_disable_csA();
    }

    chan[ch + 3].ok = readyB;
    if (readyB) {
      hspi_enable_csB();
      chan[ch + 3].adc = ads_readdata();  // read sample value B
      chan[ch + 3].count++;
      hspi_disable_csB();
    }
  }

  float vval;
  for (int ch = 0; ch < 5; ch++) {
    if (chan[ch].ok) {
      vval = (float)chan[ch].adc * ccal.scal[ch] / 65536.0;  // adc to pd current
    } else {
      vval = 0.0;
    }
    if (state == 0) {
      chan[ch].dark = vval;  // dark current
    } else {
      chan[ch].light = vval;  // light current
      vval = vval - chan[ch].dark;  // diff signal
      chan[ch].signal = vval;  // single shot signal
      chan[ch].mean = (chan[ch].mean * 15 + vval) / 16;
    }

  }
}     

// +++ Errors AVR +++

// void get_errors(int data) {
//   if ((data & (1 << 0)) != 0) {  // Error Overcurrent, only set, not cleared
//     error.iover = CD_ERR;
//   }
//   if ((data & (1 << 1)) != 0) {  // Error Overvoltage, only set, not cleared
//     error.uover = CD_ERR;
//   }
// }

// +++ Data MinMax +++

// float maxOfArray(const float *array, const int lenght) {
//   float max = array[0];      
//   for (int k = 1; k < lenght; k++) {
//     if (array[k] > max) {
//       max = array[k];
//     }
//   }
//   return max;
// }

// float minOfArray(const float *array, const int lenght) {
//   float min = array[0];      
//   for (int k = 1; k < lenght; k++) {
//     if (array[k] < min) {
//       min = array[k];
//     }
//   }
//   return min;
// }

// void data_minmax() {
//   current.min5 = minOfArray(minmax.min, MINMAX_LEN);
//   current.max5 = maxOfArray(minmax.max, MINMAX_LEN);
// }

// +++ Data CSV +++

void data_store() {
  int ts;
  int dp;
  unsigned long newtime = millis();
  if ((newtime - datatime >= 250) || (newtime < datatime)) {  // every 1/4 sec
    datatime = newtime;  

    dp = dcsv.dpos;
    dp = (dp + 1) % DAT_LEN;
    dcsv.dpos = dp;
    dcsv.count++;  // data counter
    dattim[dp].count  = dcsv.count;
    dattim[dp].msec   = newtime;
    for (int ch = 0; ch < CHANMAX; ch++) {
      datval[ch][dp].signal = chan[ch].signal;     
      datval[ch][dp].mean   = chan[ch].mean;     
    }
  }
}

// +++ Largest Absolute Value from Array

float abs_max(float array[], int len) {
  float vmax = 0.0;
  for (int k = 0; k < len; k++){
    if (abs(array[k]) > vmax) {
      vmax = abs(array[k]);
    }
  }
  return vmax;
}

// +++ Max Value from Array

float max(float array[], int len) {
  float vmax = -10000.0;
  for (int k = 0; k < len; k++){
    if (array[k] > vmax) {
      vmax = array[k];
    }
  }
  return vmax;
}

// +++ Min Value from Array

float min(float array[], int len) {
  float vmin = 10000.0;
  for (int k = 0; k < len; k++){
    if (array[k] < vmin) {
      vmin = array[k];
    }
  }
  return vmin;
}
