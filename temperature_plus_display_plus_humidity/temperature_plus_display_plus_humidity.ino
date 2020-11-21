#include <ir_Lego_PF_BitStreamEncoder.h>
#include <IRremote.h>

#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 2
#define IRPIN 3
#define MODE_TEMPERATURE 0xFF30CF
#define MODE_HUMIDITY 0xFF18E7

DHT dht(DHTPIN, DHT11);

IRrecv irrecv(IRPIN);

// output pins for display catods
int catodPins[] = {A1, A2, A3, A4};

// output pins for display number renderers
int segmentsPins[] = {5, 6, 7, 8, 9, 10, 11, 12}; 

// counter to update the value
int counter = 0;

// display mode (currently only 2 - temperature and humidity)
float mode = MODE_TEMPERATURE;

// current value to display
int current_value = 0;

// digits renderer
int seg[10][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0}, // digit 0
  {0, 1, 1, 0, 0, 0, 0, 0}, // digit 1
  {1, 1, 0, 1, 1, 0, 1, 0}, // digit 2
  {1, 1, 1, 1, 0, 0, 1, 0}, // digit 3
  {0, 1, 1, 0, 0, 1, 1, 0}, // digit 4
  {1, 0, 1, 1, 0, 1, 1, 0}, // digit 5
  {1, 0, 1, 1, 1, 1, 1, 0}, // digit 6
  {1, 1, 1, 0, 0, 0, 0, 0}, // digit 7
  {1, 1, 1, 1, 1, 1, 1, 0}, // digit 8
  {1, 1, 1, 1, 0, 1, 1, 0}  // digit 9
};

// letter C
int c[] = {1, 0, 0, 1, 1, 1, 0, 0};

// result of IR decoding
decode_results ir_code;

/**
 * @return {float} temperature value
 */
float getTemperature() {
  float t = dht.readTemperature();
  return isnan(t) ? 0 : t;
}

/**
 * @return {float} humidity value
 */
float getHumidity() {
  float h = dht.readHumidity();
  return isnan(h) ? 0 : h;
}

/**
 * Draws the digit on the display
 * {int} digit - 0 to 9 digit
 * {int} pos - position of the digit on the display, 0 to 3
 */
void drawDigit(int digit, int pos = 2) {
  // set values
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentsPins[i], seg[digit][i]);
  }
  // stobe to write the digit
  digitalWrite(catodPins[pos], LOW);
  delay(1); 
  digitalWrite(catodPins[pos], HIGH);
}

/**
 * Draw a number
 * {int} number - number to  draw (3 digits max)
 * {int} pos - position to start drawing from. Used for selfcall
 */
void drawNumber(int number, int pos = 2) {
  if (number > 0) {
    drawDigit(number % 10, pos);
    drawNumber(number / 10, pos - 1);
  }
}

/**
 * Draw a letter form the digit template
 * {int} letter_template - array of boolean
 * {int} pos - position on the display (0 to 3)
 */
void drawLetter(int* letter_template, int pos) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentsPins[i], letter_template[i]);
  }
  // stobe to write the digit
  digitalWrite(catodPins[pos], LOW);
  delay(1); 
  digitalWrite(catodPins[pos], HIGH);
}

/**
 * Returns a value based on current mode
 * @return {int}
 */
int getValue() {
  if (mode == MODE_TEMPERATURE) return (int)getTemperature();
  if (mode == MODE_HUMIDITY) return (int)getHumidity();
  return 0;
}

/**
 * Standard setup function
 */
void setup() {
  // start DHT sensor
  dht.begin();

  // initalize display catod pins
  for (int i = 0; i < 4; i++) {
    pinMode(catodPins[i], OUTPUT);
    digitalWrite(catodPins[i], HIGH);
  }

  // initalize display digit pins
  for (int i = 0; i < 8; i++) pinMode(segmentsPins[i], OUTPUT);

  // initialize DHTP data pin
  pinMode(DHTPIN, INPUT);

  // start IR sensor
  irrecv.enableIRIn();

  // set the initial value
  current_value = getValue();
}

/**
 * Standard loop function
 */
void loop() {
  // update value each 5000th cycle
  if (counter % 5000 == 0) {
    counter = 0;
    current_value = getValue();
  }

  // draw a value on the display
  drawNumber(current_value);

  // draw a letter C if the temperature is displayed
  if (mode == MODE_TEMPERATURE) {
    drawLetter(c, 3);
  }

  // listen IR remote sygnal
  if (irrecv.decode(&ir_code)) {
    // if the remote button isn't pressed and holded - change mode and update value
    if (ir_code.value != 0xFFFFFFFF) {
      mode = ir_code.value;
      current_value = getValue(); 
    }

    // resume listening to IR remote sygnal
    irrecv.resume();
  }

  counter++;
}
