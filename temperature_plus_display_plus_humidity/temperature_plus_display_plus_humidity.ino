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

int katodPins[] = {A1, A2, A3, A4};

int segmentsPins[] = {5, 6, 7, 8, 9, 10, 11, 12}; 

int temperature_pin = 2;

int counter = 0;

float mode = MODE_TEMPERATURE;

int current_value = 0;

int seg[10][8] = {
  {1, 1, 1, 1, 1, 1, 0, 0}, //Цифра 0
  {0, 1, 1, 0, 0, 0, 0, 0}, //Цифра 1
  {1, 1, 0, 1, 1, 0, 1, 0}, //Цифра 2
  {1, 1, 1, 1, 0, 0, 1, 0}, //Цифра 3
  {0, 1, 1, 0, 0, 1, 1, 0}, //Цифра 4
  {1, 0, 1, 1, 0, 1, 1, 0}, //Цифра 5
  {1, 0, 1, 1, 1, 1, 1, 0}, //Цифра 6
  {1, 1, 1, 0, 0, 0, 0, 0}, //Цифра 7
  {1, 1, 1, 1, 1, 1, 1, 0}, //Цифра 8
  {1, 1, 1, 1, 0, 1, 1, 0}  //Цифра 9
};

int c[] = {1, 0, 0, 1, 1, 1, 0, 0};

decode_results ir_code;

float getTemperature() {
  float t = dht.readTemperature();
  return isnan(t) ? 0 : t;
}

float getHumidity() {
  float h = dht.readHumidity();
  return isnan(h) ? 0 : h;
}

void drawDigit(int digit, int pos = 2, boolean dp = false) {
  // set values
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentsPins[i], seg[digit][i]);
  }
  // stobe to write the digit
  digitalWrite(katodPins[pos], LOW);
  delay(1); 
  digitalWrite(katodPins[pos], HIGH);
}
void drawNumber(int number, int pos = 2) {
  if (number > 0) {
    drawDigit(number % 10, pos);
    drawNumber(number / 10, pos - 1);
  }
}

void drawLetter(int* letter_matrix, int pos) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(segmentsPins[i], letter_matrix[i]);
  }
  // stobe to write the digit
  digitalWrite(katodPins[pos], LOW);
  delay(1); 
  digitalWrite(katodPins[pos], HIGH);
}

int getValue() {
  if (mode == MODE_TEMPERATURE) return (int)getTemperature();
  if (mode == MODE_HUMIDITY) return (int)getHumidity();
  return 0;
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  for (int i = 0; i < 4; i++) {
    pinMode(katodPins[i], OUTPUT);
    digitalWrite(katodPins[i], HIGH);
  }
  for (int i = 0; i < 8; i++) pinMode(segmentsPins[i], OUTPUT);
  pinMode(temperature_pin, INPUT);
  irrecv.enableIRIn();
}

void loop() {
  if (counter % 5000 == 0) {
    counter = 0;
    current_value = getValue();
  }
  drawNumber(current_value);
  if (mode == MODE_TEMPERATURE) {
    drawLetter(c, 3);
  }
  if (irrecv.decode(&ir_code)) {
    if (ir_code.value != 0xFFFFFFFF) {
      mode = ir_code.value;
      current_value = getValue(); 
    }
    irrecv.resume();
  }
  counter++;
}
