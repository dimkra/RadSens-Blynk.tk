//RsdSens4
// Инициализируем библиотеки
#define BLYNK_PRINT Serial
//#define BLYNK_TEMPLATE_ID "TMPL4mPNlWztI"
//#define BLYNK_TEMPLATE_NAME "RadSens"
#define BLYNK_AUTH_TOKEN "9V3d75MV9WoS96JqYYWoDxIMOql5IzKu"
#include <Wire.h>
#include <CG_RadSens.h>
#include <GyverOLED.h>
//#include <BlynkMultiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#define ADC_pin A0 // задаём значение пина АЦП
#define buz_pin 14 // Задаём значения пина для пищалки
#define OLED_NO_PRINT
// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Keenetic-3468";
char pass[] = "Cwq4512%";
char auth[] = BLYNK_AUTH_TOKEN;
//GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;        // с буфером
//GyverOLED<SSD1306_128x64, OLED_NO_BUFFER> oled;     // без буфера
//GyverOLED<SSH1106_128x64> oled;                     // только программный буфер
GyverOLED<SSD1306_128x64, OLED_BUFFER> oled;  // Инициализируем 1.3" OLED-экран
CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS); // Инициализируем RadSens

uint16_t ADC; // Переменная для значений АЦП
uint32_t timer_cnt; // Таймер для измерений дозиметра
uint32_t timer_bat; // Таймер для измерения заряда батареи
uint32_t timer_imp; // Таймер опроса импульсов для пьезоизлучателя
uint32_t timer_key;
uint32_t pulsesPrev; // Число импульсов за предыдущую итерацию
float cpulses;
float pP;
float pT;
float din;
const int buttonPin = 12;
boolean  buttonState = false;
int light =1; //световая индикация вкл по умолчанию
int sound = 0; //звуковая индикация выкл по умолчанию
boolean press_flag = false; //признак нажатия кнопки  
boolean long_press_flag = false; //признак долгого нажатия кнопки
unsigned long last_press = 0;   

//Функция аудиоприветствия
void hello() {
  for (int i = 1; i < 5; i++) {
    //tone(buz_pin, i * 1000);
    delay(100);
  }
  //tone(buz_pin, 0);
  delay(100);
  oled.setScale(2);
  oled.setCursor(10, 3);
  oled.print("RadSens 5");
  oled.update();  
  delay(3000);
  oled.clear(); 
}

//Функция, которая создаёт "трески" пьезоизлучателя при появлении импульсов
// Функция, описывающая время и частоту пищания пьезоизлучателя
void beep() {     
  tone(buz_pin, 3500);
  delay(13);
  tone(buz_pin, 0);
  delay(40);
}

//Функция предупреждения при превышении порога излучения
void warning() {
  for (int i = 0; i < 3; i++) {
    tone(buz_pin, 1500);
    delay(250);
    tone(buz_pin, 0);
    delay(250);    
  }
}

//рисует значек динамика
void dinamik() {
  oled.rect(94, 3, 96, 6, OLED_FILL);
  oled.line(97, 2, 99, 0, OLED_FILL);
  oled.line(97, 7, 99, 9, OLED_FILL);
  oled.line(100, 0, 100, 9, OLED_FILL);
}

//стирает значек динамика
void nodinamik() {
  oled.clear(94, 0, 100, 9);
}

void drawlight() {
  oled.circle(82, 5, 4, OLED_STROKE);
  oled.circle(82, 5, 1, OLED_FILL);
  radSens.setLedState(1);
}

void nolight() {
  oled.clear(77, 0, 88, 11);
  radSens.setLedState(0);
}

void rad_sign(int is) {
  oled.line(6+is, 0, 13+is, 1, OLED_FILL);
  oled.line(7+is, 1, 12+is, 1, OLED_FILL);
  oled.line(8+is, 2, 11+is, 2, OLED_FILL);
  oled.line(9+is, 3, 10+is, 3, OLED_FILL);
  oled.line(9+is, 5, 10+is, 5, OLED_FILL);
  oled.line(8+is, 6, 11+is, 6, OLED_FILL);
  oled.line(8+is, 7, 11+is, 7, OLED_FILL);
  oled.line(9+is, 8, 10+is, 8, OLED_FILL);
  oled.line(3+is, 9, 7+is, 9, OLED_FILL);
  oled.line(12+is, 9, 16+is, 9, OLED_FILL);
  oled.line(4+is, 10, 7+is, 10, OLED_FILL);
  oled.line(12+is, 10, 15+is, 10, OLED_FILL);
  oled.line(5+is, 11, 7+is, 11, OLED_FILL);
  oled.line(12+is, 11, 14+is, 11, OLED_FILL);
  oled.line(6+is, 12, 7+is, 12, OLED_FILL);
  oled.line(12+is, 12, 13+is, 12, OLED_FILL);
  oled.dot(7+is, 12, OLED_FILL);
  oled.dot(12+is, 12, OLED_FILL);
  oled.line(0, 14, 127, 14, OLED_FILL);
}

void setup() {
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass, "blynk.tk", 8080); 
  Wire.begin();
  oled.init(); // Инициализируем OLED в коде
  oled.clear(); 
  oled.update();  
  pinMode(ADC_pin, OUTPUT); // Инициализируем АЦП как получатель данных
  hello();  // Приветствуем пищанием  
  oled.update();  // Обновляем экран
  pulsesPrev = radSens.getNumberOfPulses(); // Записываем значение для предотвращения серии тресков на старте
  radSens.setSensitivity(580); //Установка чувствительности ссчетчика (105 для SBM20) первая проба 500
  pinMode(buttonPin, INPUT); //Вход кнопки
  oled.setContrast(127); //Яркость дисплея
  light=radSens.getLedState();
  if (light==1) {drawlight();} else { nolight();}
  oled.line(0, 14, 127, 14, OLED_FILL);
 }


BLYNK_WRITE(V3) // this command is listening when something is written to V1
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if (pinValue == 1){
   // do something when button is pressed;
  } else if (pinValue == 0) {
   // do something when button is released;
  }
  light=pinValue;
}

BLYNK_WRITE(V4) // this command is listening when something is written to V1
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  if (pinValue == 1){
   // do something when button is pressed;
  } else if (pinValue == 0) {
   // do something when button is released;
  }
  sound=pinValue;
}



void loop() {
  Blynk.run();
   // Раз в 250 мс происходит опрос счётчика импульсов для создания тресков, если число импульсов за 250 мс превысит 5, раздастся предупреждение
  if (millis() - timer_imp > 250) {  
    timer_imp = millis();
    int pulses = radSens.getNumberOfPulses();
    if (pulses - pulsesPrev > 5 ) {
      pulsesPrev = pulses;
      if (sound ==1) {warning();}
        rad_sign(0); //рисует знак радиации
       }
       else {
      //oled.clear(3, 0, 48, 13); //стирает знак радиации
      }
    if (pulses > pulsesPrev) {
      for (int i = 0; i < (pulses - pulsesPrev); i++) {
        if (sound ==1) {beep();}
      }
      pulsesPrev = pulses;
    }
  }
  // Снимаем показания с дозиметра и выводим их на экран
  if (millis() - timer_cnt > 2000) { 
    timer_cnt = millis();
    pT=radSens.getNumberOfPulses();
    cpulses=(pT-pP)/2;
    pP=pT;
    char buf1[50];
    char buf2[50];
    char buf3[50];
    din=radSens.getRadIntensyDynamic();
    if (din<20) {
      oled.clear(3, 0, 48, 13);} else {
      if (din<50) {
        rad_sign(0);
        oled.clear(19, 0, 48, 13);} else {
          if (din<100) {
            rad_sign(0);
            rad_sign(16);
            oled.clear(35, 0, 48, 13);} else {
              rad_sign(0);
              rad_sign(16);
              rad_sign(32);
            }
          }
        }
    if (din<1000) {
    sprintf(buf1, "%.1fмкР/ч  ", din); // Собираем строку с показаниями динамической интенсивности
    } else {
      sprintf(buf1, "%.3fмР/ч  ", din/1000); // Собираем строку с показаниями динамической интенсивности
    }
    sprintf(buf2, "Ст: %.1fмкР/ч ", radSens.getRadIntensyStatic()); // Собираем строку с показаниями средней интенсивности за период работы
    sprintf(buf3, "%.1fcps ", cpulses);
    
    //BLYNK_WRITE(V3);
    //BLYNK_WRITE(V4);

    Blynk.virtualWrite(V0,cpulses);
    Blynk.virtualWrite(V1,din);
    Blynk.virtualWrite(V2,radSens.getRadIntensyStatic());
    Blynk.virtualWrite(V3,light);
    Blynk.virtualWrite(V4,sound);
    //---Дин---
    oled.setCursorXY(0, 37);
    oled.setScale(2);
    oled.print(buf1);
    //---Стат---
    oled.setCursor(0, 7);
    oled.setScale(1);
    oled.print(buf2);
    //---CPS---
    oled.setCursor(0, 2);
    oled.setScale(2);
    oled.print(buf3);
  }
  // Считываем показание с АЦП, рисуем батарею и создаём индикацию заряда, показания АЦП вы можете подстроить под своё удобство
  if (millis() - timer_bat > 5000) { 
    timer_bat = millis();
    ADC = analogRead(ADC_pin); 
    oled.rect(110, 0, 124, 9, OLED_STROKE); 
    oled.rect(125, 3, 126, 6, OLED_FILL);
    if (ADC >= 350) {
      oled.rect(112, 2, 114, 7, OLED_FILL);
      oled.rect(116, 2, 118, 7, OLED_FILL);
      oled.rect(120, 2, 122, 7, OLED_FILL);
    }
    if (ADC < 350 && ADC >= 335) {
      oled.rect(112, 2, 114, 7, OLED_FILL);
      oled.rect(116, 2, 118, 7, OLED_FILL);
    }
    if (ADC < 335 && ADC >= 320) {
      oled.rect(112, 2, 114, 7, OLED_FILL);
    }
    if (ADC < 320){
      oled.rect(110, 0, 124, 9, OLED_STROKE);
      oled.rect(125, 3, 126, 6, OLED_FILL);
    }
  }
  
  buttonState = digitalRead(buttonPin);
  if (buttonState == true  && press_flag == false && millis() - last_press > 100) {
    press_flag = !press_flag;
    last_press = millis();
  }
  if (buttonState == true && press_flag == true && millis() - last_press > 1000) {
    long_press_flag = !long_press_flag;
    last_press = millis();
    Serial.println("долгое нажатие ");
    if (sound==0) {
      sound=1;
      dinamik();
    } else {
      sound=0;
      nodinamik();
    }
  }
  if (buttonState == false && press_flag == true && long_press_flag == true) {
    press_flag = !press_flag;            
    long_press_flag = !long_press_flag;  
  }
   if (buttonState == false && press_flag == true && long_press_flag == false) {
    press_flag = !press_flag;  
    Serial.println("короткое нажатие ");
    if (light==1) {
      light=0;
      nolight();
    } else {
      light=1;
      drawlight();
    }
    Serial.println(analogRead(ADC_pin));
  }
  if (light==1) {drawlight();} else {nolight();}
  if (sound==1) {dinamik();} else {nodinamik();}
  oled.update(); // Обновляем экран в конце цикла
}
