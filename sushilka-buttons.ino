#define DHTPIN 8                      // пин датчика влажности/температуры
#define DHTTYPE DHT11                 // тип датчика DHT 11
#define DHTPIN 8                      // пин для сигнала поступающего с датчика
#define Buz 7                         // пин для зуммера - пищалки

#include <GyverButton.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

GButton btnLow(2);                    // prog LOW
GButton btnMid(3);                    // prog MID
GButton btnHigh(4);                   // prog HIGH
GButton btnPlus(9);                   // btn +
GButton btnMinus(10);                 // btn -
GButton btnStart(11);                 // btn start
GButton btnStop(12);                  // btn stop

DHT dht(DHTPIN, DHTTYPE);             // Инициация датчика влажности/температуры

int count = 0;                        // значение счетчика
int temp;
int cooler;
int time = 0;
int timeInSec = 0;
int countdownTime = time * 60; // время обратного отсчета в секундах
boolean play = false;
boolean pressBtnLevel = false;

String str;                           // название программы

byte degree[8] =                      // кодируем символ градуса
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

unsigned long startTime = 0;
unsigned long currentTime = 0;
unsigned long elapsedTime = 0;
unsigned long lastPrintTime = 0;
unsigned long countdownDuration = 5 * 60 * 1000; // 5 минут в миллисекундах
char lcdLine[20];

void setup() {
  Serial.begin(9600);

  startTime = millis();
  lastPrintTime = startTime;

  dht.begin();

  pinMode(Buz, OUTPUT);               // пин зуммера - пищалки
  pinMode(6, OUTPUT);                 // пин тены

  lcd.init();                         // Инициализация lcd             
  lcd.backlight();                    // Включаем подсветку
  lcd.createChar(1, degree);          // Создаем символ под номером 
  
  lcd.setCursor(0, 0);                // Устанавливаем курсор в начало 1 строки
  lcd.print("Program:");              // Выводим текст "Programm"
  lcd.setCursor(0, 2);                // Устанавливаем курсор в начало 2 строки
  lcd.print("Temp =     \1C ");       // Выводим текст, \1 - значок градуса
  lcd.setCursor(0, 3);                // Устанавливаем курсор в начало 4 строки
  lcd.print("Hum  =      % ");        // Выводим текст
  lcd.setCursor(9, 0);                // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Set level");             // Выводим дефолтное название программы - Set level

  outPutTime(0);
}

int getDeltaTimeInSec() {
  int currentTime = millis();
  int previous = startTime;
  int delta = currentTime - previous;
  if (delta >= 1000) {
    startTime = currentTime;
    return delta / 1000;
  }

  return 0;
}

void loop() {

  // Считывание данных температуры занимает около 250 milliseconds!
  // Показания датчика также могут быть "устаревшими" на 2 секунды (это очень медленный датчик)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {  // Проверяем, успешно ли прошло считывание данных
    outPutError();
    stopProgramm(str);
    buzOnLong();    
  }

  if (!play) {
    if (!digitalRead(2)) {              // LOW
      pressBtnLevel = true;
      str = "LOW        ";
      temp = 5;
      cooler = 80;
      setSettingsProgram(str);
    } 
    
    if (!digitalRead(3)) {              // MIDDLE
      pressBtnLevel = true;
      str = "MIDDLE   ";
      temp = 40;
      cooler = 170;
      setSettingsProgram(str);
    }
    
    if (!digitalRead(4)) {              // HIGH
      pressBtnLevel = true;
      str = "HIGH     ";
      temp = 60;
      cooler = 255;
      setSettingsProgram(str);
    }

    if (!digitalRead(9)) {             // увеличиваем время
      setTimePlus();
      outPutTime(time*60);
    }

    if (!digitalRead(10)) {            // уменьшаем время
      setTimeMinus();
      outPutTime(time*60); 
    }

    if (!digitalRead(11)) {            // запуск программы
      startProgram();    
    }
  }

  if (!digitalRead(12)) {            // остановка программы
    stopProgramm(str);
  }

  outPutTempHum(t, h);

  if (play) {
    checkTemp(t, temp);

    timeInSec -= getDeltaTimeInSec();
    outPutTime(timeInSec);
    if (timeInSec <= 0) {
      stopProgramm(str);
    }
  } else {    
  }
}

void outPutTime(int secs) {
  sprintf(lcdLine, "Time: %02d:%02d", secs/3600, ((secs % 3600) / 60));
  lcd.setCursor(0, 1);
  lcd.print(lcdLine);
}

void outPutEndTime() {
  sprintf(lcdLine, "Time: END      ");
  lcd.setCursor(0, 1);
  lcd.print(lcdLine);
  return;
}

void outPutError() {
  sprintf(lcdLine, "Error   ");
  lcd.setCursor(7, 2);
  lcd.print(lcdLine);
  lcd.setCursor(7, 3);
  lcd.print(lcdLine);
}

void setSettingsProgram(String str) {
  lcd.setCursor(9, 0);               // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print(str);                    // Выводим название программы
}

void outPutTempHum(float t, float h) {
  lcd.setCursor(7, 2);               // Устанавливаем курсор в начало 3 строкe, 8 символ
  lcd.print(t,1);                    // Выводим значение температуры
  lcd.setCursor(7, 3);               // Устанавливаем курсор в начало 4 строкe, 8 символ
  lcd.print(h,1);                    // Выводим значение влажности
}

void setTimePlus() {
  time = time + 10;
  delay(300);

  if (time > 300) {
    time = 300;
  }
}

void setTimeMinus() {
  time = time - 10;
  delay(300);

  if (time < 10) {
    time = 0;
  }
}

void startProgram() {
  if (temp > 0 && pressBtnLevel && time > 0) {
    timeInSec = time * 60;
    play = true;
    heating();
    startTimer();
    analogWrite(5, cooler);
  }  
}

void stopProgramm(String str) {
  play = false;
  cooling();
  time = 0;
  countdownTime = 0;
  temp = 0;
  cooler = 0;
  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
  analogWrite(5, 0);                // выключаем вентилятор
  buzOn();
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Set level");           // Выводим дефолтное название программы - Set level
  outPutEndTime();
}

void startTimer() {
  lcd.setCursor(6, 1);              // рисуем стартовое значение таймера
  lcd.print(time);
}

void buzOn() {                      // вывод звукового сигнала
  for (int i=0; i<3; i++){
      digitalWrite(Buz, HIGH); 
      delay(600);
      digitalWrite(Buz, LOW);
      delay(400);
  }
}

void buzOnLong() {                      // вывод звукового сигнала
  for (int i=0; i < 15; i++){
      digitalWrite(Buz, HIGH); 
      delay(600);
      digitalWrite(Buz, LOW);
      delay(400);
  }
}

void heating() {                    // нагрев тены
  if (temp > 0 && pressBtnLevel && play && time > 0) {
    digitalWrite(6, HIGH);
  }  
}

void cooling() {                    // выключение тены
  digitalWrite(6, LOW);
}

void checkTemp(int t, int temp) {   // проверка температуры с заданным значением
  if (t >= temp) {
    cooling();
  }
  if (t <= (temp - 2)) {
    heating();
  }
}