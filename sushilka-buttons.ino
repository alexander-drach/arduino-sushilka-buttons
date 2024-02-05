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

void setup() {
  Serial.begin(9600);

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

  lcd.setCursor(0, 1);                // Устанавливаем курсор в начало 2 строкe, 0 символ
  lcd.print("Time: ");                // Выводим дефолтное название программы - LOW
  lcd.setCursor(6, 1);                // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
  lcd.setCursor(9, 1);                // Устанавливаем курсор в начало 2 строкe, 8 символ
  lcd.print("min");
}

void loop() {
  // Считывание данных температуры занимает около 250 milliseconds!
  // Показания датчика также могут быть "устаревшими" на 2 секунды (это очень медленный датчик)
  float h = dht.readHumidity();
  // Read temperature as Celsius
  float t = dht.readTemperature();  

  if (!digitalRead(2)) {              // LOW
    //stopProgrammOnBtn();
    str = "LOW        ";
    temp = 0;
    cooler = 80;
    setSettingsProgram(str);
  } 
  
  if (!digitalRead(3)) {              // MIDDLE
    //stopProgrammOnBtn();
    str = "MIDDLE   ";
    temp = 40;
    cooler = 170;
    setSettingsProgram(str);
  }
  
  if (!digitalRead(4)) {              // HIGH
    //stopProgrammOnBtn();
    str = "HIGH     ";
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

  if (!digitalRead(11)) {            // запуск программы
    startProgram();    
  }

  if (!digitalRead(12)) {            // остановка программы
    stopProgramm(str); 
  }

  outPutTempHum(t, h);

  if (play) {
    checkTemp(t, temp);
  }
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
  time = time + 5;
  delay(500);

  if (time > 60) {
    time = 60;
  }

  lcd.setCursor(6, 1);               // прибавляем время
  lcd.print(time);
}

void setTimeMinus() {
  time = time - 5;
  delay(500);

  if (time < 10) {
    time = 10;
  }

  lcd.setCursor(6, 1);              // уменьшаем время
  lcd.print(time);
}

void startProgram() {
  play = true;
  heating();
  startTimer();
  analogWrite(5, cooler);
}

void stopProgramm(String str) {
  play = false;
  cooling();
  time = 0;
  temp = 0;
  cooler = 0;
  lcd.setCursor(6, 1);              // Устанавливаем курсор в начало 2 строкe, 6 символ
  lcd.print(time);
  //Serial.println(str);
  //analogWrite(5, 255);            // включаем вентилятор на полную мощность перед выключением на 1 мин
  //delay(60000);  
  analogWrite(5, 0);                // выключаем вентилятор
  buzOn();
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Set level");           // Выводим дефолтное название программы - Set level
}

void stopProgrammOnBtn() {
  play = false;
  time = 0;
  temp = 0;
  cooler = 0;
  analogWrite(5, 0);                // выключаем вентилятор
  lcd.setCursor(9, 0);              // Устанавливаем курсор в начало 1 строкe, 10 символ
  lcd.print("Stop");                // Выводим дефолтное надпись - Stop
  delay(500);
  buzOn();
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

void heating() {                    // нагрев тены
  digitalWrite(6, HIGH);
}

void cooling() {                    // выключение тены
  digitalWrite(6, LOW);
}

void checkTemp(int t, int temp) {   // проверка температуры с заданным значением
  if (t >= temp) {
    cooling();
  }
  if (t <= (temp - 4)) {
    heating();
  }
}