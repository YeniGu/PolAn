Programming in Arduino IDE

https://www.arduino.cc/en/software/

> programming is also possible in PlatformIO and VS Code, as you like

additional libraries

preferences (https://github.com/vshymanskyy/Preferences)
update (https://github.com/arduino-libraries/Arduino_ESP32_OTA)

Arduino settings

> see screenshot

to do's

device matrix
simulate test data
normalise intensitites
- two steps
  1. create difference ligth - dark > lock-in amplifier behavior
  2. normalise sample values to the reference diode value
calculating Stokes vectors from intensities
Poincare sphere
light source managament (switching red/yellow/green LED)
data export as CSV files
program security (protect upload and settings by password)

changes

05.08.26
- change webpages from strings to littleFS
- better editing of webpages and scripts
- images can be stored in littleFS and loaded into webpages

02.09.26
- improved file upload and OTA programming

12.09.26
- added Poincare sphere to the html
  - drawing actual vector
  - drawing trace of old vectors
  - rotating sphere by mouse or touch
  - simulated stokes data

> do not render images or charts at ESP32, send data to webpage and render charts and poincare sphere with JS in browser

13.09.26
- translated all the comments to English
