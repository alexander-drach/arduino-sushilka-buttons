#define DHTPIN 8 // пин датчика влажности/температуры
#define DHTTYPE DHT11 // тип датчика DHT 11

#include <GyverButton.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "DHT.h"
#define DHTPIN 8                   // пин для сигнала поступающего с датчика

GButton btnLow(2);                 // LOW
GButton btnMid(3);                 // MID
GButton btnHigh(4);                // HIGH
GButton btnPlus(9);                // +
GButton btnMinus(10);              // -
GButton btnStart(11);              // start
GButton btnStop(12);               // stop

DHT dht(DHTPIN, DHTTYPE);          // Инициация датчика влажности/температуры

int count = 0;                     // значение счетчика
int temp;
int cooler;
int time = 0;

byte degree[8] =                    // кодируем символ градуса
{
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000,
};    
 
LiquidCrystal_I2C lcd(0x27, 20, 4);   // Задаем адрес и размерность дисплея

void setup() {
  Serial.begin(9600);

  dht.begin();

  lcd.init();                       // Инициализация lcd             
  lcd.backlight();                  // Включаем подсветку
  lcd.createChar(1, degree);        // Создаем символ под номером 
  
  lcd.setCursor(0, 0);              // Устанавливаем курсор в начало 1 строки
  lcd.print("Program:");           // Выводим текст "Programm"
  lcd.setCursor(0, 2);              // Устанавливаем курсор в начало 1 строки
  lcd.print("Temp =     \1C ");     // Выводим текст, \1 - значок градуса
  lcd.setCursor(0, 3);              // Устанавливаем курсор в начало 1 строки
  lcd.print("Hum  =      % ");      // Выводим текст
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Set level");           // Выводим дефолтное название программы - LOW

  lcd.setCursor(0, 1);              // Устанавливаем курсор в начало 2 строкe, 0 символ
  lcd.print("Time: ");              // Выводим дефолтное название программы - LOW
  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
  lcd.setCursor(9, 1);              // Устанавливаем курсор в начало 2 строкe, 8 символ
  lcd.print("min");
}

void loop() {
  // Считывание данных температуры занимает около 250 milliseconds!
  // Показания датчика также могут быть "устаревшими" на 2 секунды (это очень медленный датчик)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();  

  if (!digitalRead(2)) {            // LOW
    String str = "LOW        ";
    temp = 0;
    cooler = 80;
    setSettingsProgram(str);
  } 
  
  if (!digitalRead(3)) {            // MIDDLE
    String str = "MIDDLE";
    temp = 40;
    cooler = 170;
    setSettingsProgram(str);
  }
  
  if (!digitalRead(4)) {            // HIGH
    String str = "HIGH     ";
    temp = 60;
    cooler = 255;
    setSettingsProgram(str);
  }

  if (!digitalRead(9)) {
    setTimePlus();    
  }

  if (!digitalRead(10)) {
    setTimeMinus();    
  }

  if (!digitalRead(11)) { // запуск программы
    startProgram();   
  }

  if (!digitalRead(12)) { // остановка программы
    stopProgramm(); 
  }
  
  outPutTempHum(t, h);
}

void setSettingsProgram(String str) {
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print(str);                   // Выводим название программы
}

void outPutTempHum(float t, float h) {
  lcd.setCursor(7, 2);              // Устанавливаем курсор в начало 3 строкe, 8 символ
  lcd.print(t,1);                   // Выводим значение температуры
  lcd.setCursor(7, 3);              // Устанавливаем курсор в начало 4 строкe, 8 символ
  lcd.print(h,1);                   // Выводим значение влажности
}

void setTimePlus() {
  time = time + 5;
  delay(500);

  if (time > 60) {
    time = 60;
  }

  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
}

void setTimeMinus() {
  time = time - 5;
  delay(500);

  if (time < 10) {
    time = 10;
  }

  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
}

void startProgram() {
  Serial.println("start");
  startTimer();
  analogWrite(5, cooler);
}

void stopProgramm() {
  Serial.println("стоп");
  time = 0;
  temp = 0;
  cooler = 0;
  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
  analogWrite(5, 0);
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Set level");           // Выводим дефолтное название программы - LOW
}

void startTimer() {
  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
}