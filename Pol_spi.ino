/*
 * PolTest ESP32 Control
 * SPI Protocol
 */

#include <SPI.h>

#define HSCK  12
#define HMOSI 11
#define HMISO 13
#define HCSA  10  // ADS A select
#define HCSB  9   // ADS B select

#define HSPI_CLK  1000000  // 1 MHz bus speed

#define BUFF_MAX  256

SPIClass hspi(HSPI);

// +++ Init SPI for AVR +++

void init_hspi() {
  pinMode(HCSA, OUTPUT);
  digitalWrite(HCSA, HIGH);
  pinMode(HCSB, OUTPUT);
  digitalWrite(HCSB, HIGH);

  hspi.begin(HSCK, HMISO, HMOSI, -1);
}

void hspi_enable_csA() {
  digitalWrite(HCSA, LOW);  // enable CS
  delayMicroseconds(10);
}

void hspi_disable_csA() {
  delayMicroseconds(10);
  digitalWrite(HCSA, HIGH);  // disable CS
}

void hspi_enable_csB() {
  digitalWrite(HCSB, LOW);  // enable CS
  delayMicroseconds(10);
}

void hspi_disable_csB() {
  delayMicroseconds(10);
  digitalWrite(HCSB, HIGH);  // disable CS
}

bool hspi_dataready() {
  return digitalRead(HMISO) == LOW;
}

// +++ Fast shift byte +++

byte hspi_transfer_byte(byte data) {
  hspi.beginTransaction(SPISettings(HSPI_CLK, MSBFIRST, SPI_MODE0));
//  digitalWrite(HCS, LOW);
  byte rx = hspi.transfer(data);
//  digitalWrite(HCS, HIGH);
  hspi.endTransaction();
  return rx;
}

// +++ Transfer data +++

void hspi_transfer(byte spi_buf[], int len) {
  if (len >= BUFF_MAX) {
    len = BUFF_MAX;
  }
  hspi.beginTransaction(SPISettings(HSPI_CLK, MSBFIRST, SPI_MODE0));
//  digitalWrite(HCS, LOW);  // enable CS
  for (int k = 0; k < len; k++) {
    spi_buf[k] = hspi.transfer(spi_buf[k]);
  }
  hspi.endTransaction();
//  digitalWrite(HCS, HIGH);  // disable CS
}


