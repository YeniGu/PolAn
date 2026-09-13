/*
 * PolTest ESP32 Control
 * Uart
 */




// +++ Init uart +++

void init_uart() {
  Serial.begin(115200);
  Serial.println(VERSION);
  Serial.println(BUILDTIME);

}

// +++ Uart handler +++

void uart_handle() {
  Serial.print(chan[0].count);
  Serial.print("   ");
  for (int k = 0; k < CHANMAX; k++) {
    Serial.print(k);
    Serial.print(": ");
    Serial.print(chan[k].signal, 3);
    Serial.print("   ");
  }
  Serial.println();
}