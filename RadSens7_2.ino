#include <Arduino.h>
#include <FileData.h>
#include <LittleFS.h>
struct Data {
  uint8_t wifi_e = 0;
  int sound_e = 0;
  int scr_e = 1;
  float s_f_c_e=0.36;
  float m_v_c_e=0.15;
  float k_c_e=6.15;
};
Data mydata;
FileData data(&LittleFS, "/data.dat", 'B', &mydata, sizeof(mydata));

#define BLYNK_AUTH_TOKEN ""
#include <Wire.h>
#include <CG_RadSens.h>
#include <BlynkSimpleEsp8266.h>
#include <ESP8266WiFi.h>
#define ADC_pin A0  // задаём значение пина АЦП
#define buz_pin D6  // Задаём значения пина для пищалки

CG_RadSens radSens(RS_DEFAULT_I2C_ADDRESS);  // инициализируем RadSens
char ssid[] = "";
char pass[] = "";
char auth[] = BLYNK_AUTH_TOKEN;

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>
#include "FontsRus/FreeSans6.h"
#include "FontsRus/FreeSans18.h"
#include "FontsRus/FreeSans8.h"
//#include "FontsRus/FreeSans7.h"
#include "FontsRus/Bahamas6.h"
#define TFT_CS         15  //GPIO4  Cs  15
#define TFT_RST        1   //           1                                       
#define TFT_DC         02   //GPIO5  AO  0 

uint16_t ADC;         // переменная для значений АЦП
uint32_t timer_cnt;   // таймер для измерений дозиметра
uint32_t timer_cps;   // таймер для измерений импульсов за 1 сек. дозиметра
uint32_t timer_bat;   // таймер для измерения заряда батареи
uint32_t timer_imp;   // таймер опроса импульсов для пьезоизлучателя
uint32_t pulsesPrev;  // число импульсов за предыдущую итерацию

//------------------------------------------------>
float Count = 0;    //счетчик импульсов
float Counts[210];  //массив импульсов
float CountsA[760];  //массив импульсов
uint32_t Time = 0;  //счетчик массива счетчик времени в секундах
uint32_t TimeA = 0;  //счетчик всего времени в секундах 
float SUMofCounts;
float SUMofCountsA;  //усреднение за все время
float MDozi;
float MDoziA;       //усреднение за все время  
int priznC1 = 0;     //признак первого цикла окна
int priznC1000 = 0;  //признак 1000 импульсов
int Cycl = 200;      //начальное значение циклов
float cpulses;
//<------------------------------------------------

int prev_counter_cps = 0;  // предыдущее значение счетчика импульсов
int counter_cps = 0;       // счетчик импульсов
int cps;                   // импульсов в сек
float k_count = 6.15;      // коэффициент счетчика (для СБТ10-а 6,15)
float s_f_c=0.36;
float m_v_c=0.15;
uint32_t ic = 1;
float pP;
float pT;
float din;

int light=1;      //световая индикация вкл по умолчанию
int sound = 0;      //звуковая индикация выкл по умолчанию
int TFTPower = 1;  //экран включен по уолчанию
uint8_t wifiOn = 0;
int Scr = 1;      //вид экрана по умолчанию
int clearSc = 0;  //признак очистки экрана
int puls = 0;
int Sensitivity;
int i = 1;
int i2 =1;
int i24 = 1;
int cps250[8];
int cps24[210];
int md24[210];
int md_col24[210];
int col24[210];
int cps1s;
float cpsMax;
float cpsK;
int warn = 0;
float maswarn[10];
int awarn;
int awarn_prev;

float mi_prev;
uint16_t c_prev;
int o_prev;
String r_prev;
int p_prev;
String m_prev;

int rr;             //предыдущее значение стрелочного индикатора
int okr;            //разрядность округления мощности дозы
int pogresh;        //погрешность % мощности дозы
//float r;
float MD;           //мощность дозы в мкР/ч
float MD_ind;       //мощность дозы масштаб в мкР/ч, мР/ч, Р/ч
float MD_ind_prev;
uint16_t colText;   //цвет текста мощности дозы
String mess;        //текст опасности
String rh="мкР/ч "; //размерность мощности дозы мкР/ч мР/ч
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

const int buttonPin1 = TX;  //1 кнопка контакт TX работает (GPIO1)  1
const int buttonPin2 = RX;  //2 кнопка контакт RX работает (GPIO3)  3
const int buttonPin3 = D3;  //3 кнопка D3 (GPIO0)                   0
const int LedPin = D0;      //подсветка экрана
boolean buttonState1 = false;
boolean buttonState2 = false;
boolean buttonState3 = false;
boolean press_flag = false;       //признак нажатия кнопки
boolean long_press_flag = false;  //признак долгого нажатия кнопки
unsigned long last_press = 0;
boolean press_flag2 = false;       //признак нажатия кнопки
boolean long_press_flag2 = false;  //признак долгого нажатия кнопки
unsigned long last_press2 = 0;
boolean press_flag3 = false;       //признак нажатия кнопки
boolean long_press_flag3 = false;  //признак долгого нажатия кнопки
unsigned long last_press3 = 0;

// управление световой индикацией с приложения
BLYNK_WRITE(V3) {
  int pinValue = param.asInt();
  light = pinValue;
}

// управление звуком с приложения
BLYNK_WRITE(V4) {
  int pinValue = param.asInt();
  sound = pinValue;
}

// управление видом экрана с приложения
BLYNK_WRITE(V7) {
  int pinValue = param.asInt();  
  Scr = pinValue;
}

// вкл/выкл экрана
BLYNK_WRITE(V13) {
  int pinValue = param.asInt();  
  //TFTPower = pinValue;
  //oled.setPower(TFTPower);
}

// чувствительность счетчика для RadSens
BLYNK_WRITE(V14) {
  int pinValue = param.asInt();  
  radSens.setSensitivity(pinValue);
}

//коэффициент счетчика для СБТ10а 6.15 сек (за это время кол.импульсов = мкР/ч)
BLYNK_WRITE(V17) {
  float pinValue = param.asFloat();  
  k_count=pinValue;
  mydata.k_c_e=pinValue;
  //data.update();
  data.updateNow();
}

//собственный фон счетчика
BLYNK_WRITE(V15) {
  float pinValue = param.asFloat();  
  s_f_c=pinValue;
  mydata.s_f_c_e=pinValue;
  //data.update();
  data.updateNow();
}

void hello() {
  if (sound==1) {
  for (int i = 1; i < 5; i++) {
    tone(buz_pin, i * 1000);
    delay(100);
  }
  tone(buz_pin, 0);
  delay(100);
  }
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(0x2F50); //ЗЕЛЕНЫЙ ЦВЕТ
  tft.setTextSize(1);
  tft.setCursor(18, 68);
  tft.setFont(&FreeSans8pt8b); // выбор шрифта
  tft.print("RadSens 7.2");
  tft.setFont(&Bahamas6pt8b); // выбор шрифта
  tft.setCursor(13, 98);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("  Радиометр на");
  tft.setCursor(13, 118);
  tft.print("счетчике СБТ-10а");
  delay(5000);
  tft.setFont();
}

// функция, которая создаёт "трески" пьезоизлучателя при появлении импульсов
void beep() {
  if (sound==1) {
    tone(buz_pin, 3500);
    delay(13);
    tone(buz_pin, 0);
    delay(40);
  }
}

// функция предупреждения при превышении порога излучения
void warning() {
  for (int i = 0; i < 3; i++) {
    tone(buz_pin, 1500);
    delay(250);
    tone(buz_pin, 0);
    delay(250);
  }
}

void shkala(){
  int dX01=2; 
  tft.fillRect(26.033898+1-dX01, 123, 23.235393, 6, 0x2F50);
  tft.fillTriangle(20.338983+1-dX01,123,26.033898+1-dX01,123,26.033898+1-dX01,129,0x2F50);
  //tft.fillTriangle(46.847458-dX01,123,46.847458-dX01,129,49.084746-dX01,129,0x2F50);

  tft.fillRect(51.463226+1-dX01, 123, 25.073547+1-dX01, 6, ST77XX_YELLOW);
  //tft.fillTriangle(46.847458,123,49.084746,123,49.084746,129,ST77XX_CYAN);
  //tft.fillTriangle(71.457627,123,72.576271,123,71.457627,129,ST77XX_CYAN);

  tft.fillRect(78.730709+1-dX01, 123, 23.235393+1, 6, 0xFB6D);
  tft.fillTriangle(76.536774-dX01,129,78.730709-dX01,129,78.730709-dX01,122,0xFB6D);
  tft.fillTriangle(102-dX01,123,107.661017-dX01,123,102-dX01,129,0xFB6D);

  tft.drawLine(13.694915-dX01, 116, 8-dX01, 110, ST77XX_BLACK);
  tft.drawLine(27.158553-dX01, 116, 22.987823-dX01, 110, ST77XX_BLACK);
  tft.drawLine(38.032127-dX01, 116, 35.092368-dX01, 110, ST77XX_BLACK);
  tft.drawLine(47.388775-dX01, 116, 45.508259-dX01, 110, ST77XX_BLACK);
  tft.drawLine(55.888914-dX01, 116, 54.970677-dX01, 110, ST77XX_BLACK);
  tft.drawLine(64-dX01, 116, 64-dX01, 110, ST77XX_BLACK);
  tft.drawLine(72.111086-dX01, 116, 73.029323-dX01, 110, ST77XX_BLACK);
  tft.drawLine(80.611225-dX01, 116, 82.491741-dX01, 110, ST77XX_BLACK);
  tft.drawLine(89.967873-dX01, 116, 92.907632-dX01, 110, ST77XX_BLACK);
  tft.drawLine(100.841447-dX01, 116, 105.012177-dX01, 110, ST77XX_BLACK);
  tft.drawLine(114.305085-dX01, 116, 120-dX01, 110, ST77XX_BLACK);

  tft.drawLine(19.389831-dX01, 122, 30.779661-dX01, 134, ST77XX_BLACK);
  tft.drawLine(108.610169-dX01, 122, 97.220339-dX01, 134, ST77XX_BLACK);
  tft.drawLine(49.269291-dX01, 122, 53.030323-dX01, 134, ST77XX_BLACK);
  tft.drawLine(78.730709-dX01, 122, 74.969677-dX01, 134, ST77XX_BLACK);

  tft.drawLine(14-dX01, 116, 114-dX01, 116, ST77XX_BLACK);
  tft.drawLine(20-dX01, 122, 108-dX01, 122, ST77XX_BLACK);
  tft.drawLine(26-dX01, 129, 102-dX01, 129, ST77XX_BLACK);

  tft.fillRect(0, 88, 128, 3, 0xF7BD); //рамка стрелочного прибора
  tft.fillRect(0, 91, 3, 160, 0xF7BD); //рамка стрелочного прибора
    
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setFont(&Bahamas6pt8b); // выбор шрифта

  int d_Y01=105;
  tft.setCursor(4, d_Y01);
  tft.print(0);
  tft.setCursor(113, d_Y01);
  tft.print(10);

  tft.setCursor(15, d_Y01);
  tft.print(1);
  tft.setCursor(104, d_Y01);
  tft.print(9);

  tft.setCursor(27, d_Y01);
  tft.print(2);
  tft.setCursor(94, d_Y01);
  tft.print(8);

  tft.setCursor(38, d_Y01);
  tft.print(3);
  tft.setCursor(83, d_Y01);
  tft.print(7);

  tft.setCursor(49, d_Y01);
  tft.print(4);
  tft.setCursor(72, d_Y01);
  tft.print(6);

  tft.setCursor(60, d_Y01);
  tft.print(5);
  tft.setFont();
}

void Strel_Indikator(int r) {
  int cX = 62;
  int cY2 = 169;
  int cX0 = 62;
  int rad2 = 81;
  if (rr<1) {rad2=81;}
  if (rr>=1) {rad2=71.854009;}
  if (rr>=2) {rad2=65.701227;}
  if (rr>=3) {rad2=61.829964;}
  if (rr>=4) {rad2=59.686922;}
  if (rr>=5) {rad2=59;}
  if (rr>=6) {rad2=59.686922;}
  if (rr>=7) {rad2=61.829964;}
  if (rr>=8) {rad2=65.701227;}
  if (rr>=9) {rad2=71.854009;}
  if (rr==10) {rad2=81;}
  int angl1=46.49;
  int angl2=133.51;
  float tt=15;
  tft.drawLine(cX, cY2, (cX0+cos(PI-(angl1*PI/180+((rr*angl2*PI/180)/tt)))*rad2), 110, ST77XX_WHITE);
  tft.drawLine(cX-1, cY2, (cX0+cos(PI-(angl1*PI/180+((rr*angl2*PI/180)/tt)))*rad2)-1, 110, ST77XX_WHITE);
  rr=r;
  if (rr<1) {rad2=81;}
  if (rr>=1) {rad2=71.854009;}
  if (rr>=2) {rad2=65.701227;}
  if (rr>=3) {rad2=61.829964;}
  if (rr>=4) {rad2=59.686922;}
  if (rr>=5) {rad2=59;}
  if (rr>=6) {rad2=59.686922;}
  if (rr>=7) {rad2=61.829964;}
  if (rr>=8) {rad2=65.701227;}
  if (rr>=9) {rad2=71.854009;}
  if (rr==10) {rad2=81;}
  shkala();
  tft.drawLine(cX, cY2, (cX0+cos(PI-(angl1*PI/180+((rr*angl2*PI/180)/tt)))*rad2), 110, 0xB000);
  tft.drawLine(cX-1, cY2, (cX0+cos(PI-(angl1*PI/180+((rr*angl2*PI/180)/tt)))*rad2)-1, 110, ST77XX_RED);
  tft.fillCircle(62, 169, 24.5, 0xF7BD); //центр стрелки  0xF7BD
  tft.drawCircle(62, 169, 25, 0xB5B6);
  tft.fillCircle(33, 142, 1.5, 0x7C0F);
  tft.fillCircle(91, 142, 1.5, 0x7C0F);
  //tft.fillCircle(62, 169, 15, 0xFFFF);
}

void bat_level(int b) {
  int dX=4+45;
  int dY=0;
  if (b > 740) {
    tft.drawRect(110-dX, 1+dY, 14, 11, ST77XX_WHITE);
    tft.fillRect(124-dX, 4+dY, 2, 5, ST77XX_WHITE);
    tft.fillRect(112-dX, 3+dY, 2, 7, 0x2F50);  
    tft.fillRect(116-dX, 3+dY, 2, 7, 0x2F50);  
    tft.fillRect(120-dX, 3+dY, 2, 7, 0x2F50);  
  }
  if (b < 740 && b >= 685) {
    tft.drawRect(110-dX, 1+dY, 14, 11, ST77XX_WHITE);
    tft.fillRect(124-dX, 4+dY, 2, 5, ST77XX_WHITE);
    tft.fillRect(112-dX, 3+dY, 2, 6, ST77XX_YELLOW); 
    tft.fillRect(116-dX, 3+dY, 2, 7, ST77XX_YELLOW); 
    tft.fillRect(120-dX, 3+dY, 2, 7, ST77XX_BLACK);  
  }
  if (b < 685 && b >= 642) {
    tft.drawRect(110-dX, 1+dY, 14, 11, ST77XX_WHITE);
    tft.fillRect(124-dX, 4+dY, 2, 5, ST77XX_WHITE);
    tft.fillRect(112-dX, 3+dY, 2, 7, ST77XX_RED); 
    tft.fillRect(116-dX, 3+dY, 2, 7, ST77XX_BLACK);  
    tft.fillRect(120-dX, 3+dY, 2, 7, ST77XX_BLACK); 
  }
  if (b < 642) {
    tft.drawRect(110-dX, 1+dY, 14, 11, ST77XX_RED);
    tft.fillRect(124-dX, 4+dY, 2, 5, ST77XX_RED);
    tft.fillRect(112-dX, 3+dY, 2, 7, ST77XX_BLACK);
    tft.fillRect(116-dX, 3+dY, 2, 7, ST77XX_BLACK); 
    tft.fillRect(120-dX, 3+dY, 2, 7, ST77XX_BLACK); 
  }
}

// рисует значек динамика
void dinamik(uint16_t d) {
  //0x632C-не активен   ST77XX_WHITE-активен
  int dX=72;
  if (d==1) { d=ST77XX_WHITE;} else { d=0x632C;}
  tft.drawRect(93-dX , 4, 4, 6, d);
  tft.drawLine(95-dX , 5, 95-dX , 8, ST77XX_BLACK);
  tft.drawLine(97-dX , 3, 99-dX , 1, d);
  tft.drawLine(97-dX , 10, 99-dX , 12, d);
  tft.drawLine(100-dX , 1, 100-dX , 12, d);
}

// рисует знак световой сигнализации
void drawlight(uint16_t d) {
  //0x632C-не активен   ST77XX_WHITE-активен
  int dX=75;
  if (d==1) { d=ST77XX_WHITE;} else { d=0x632C;}
  tft.fillRect(78-dX , 2, 2, 2, d);
  tft.fillRect(86-dX , 2, 2, 2, d);
  tft.fillRect(78-dX , 10, 2, 2, d);
  tft.fillRect(86-dX , 10, 2, 2, d);
  tft.fillRect(82-dX , 1, 2, 2, d);
  tft.fillRect(82-dX , 11, 2, 2, d);
  tft.fillRect(77-dX , 6, 2, 2, d);
  tft.fillRect(87-dX , 6, 2, 2, d);
  tft.fillRect(82-dX , 5, 2, 4, d);
  tft.drawLine(81-dX , 6, 81-dX , 7, d);
  tft.drawLine(84-dX , 6, 84-dX , 7, d);
}

void sWiFi(uint16_t d) {
  //0x632C-не активен   ST77XX_WHITE-активен
  if (d==1) { d=ST77XX_WHITE;} else { d=ST77XX_BLACK;}
  int s = 30;
  tft.drawLine(72 - s, 1, 73 - s, 1, d);
  tft.drawLine(70 - s, 2, 71 - s, 2, d);
  tft.drawLine(74 - s, 2, 75 - s, 2, d);
  tft.drawLine(68 - s, 3, 69 - s, 3, d);
  tft.drawLine(76 - s, 3, 77 - s, 3, d);
  tft.drawPixel(67 - s, 4, d);
  tft.drawPixel(78 - s, 4, d);
  tft.drawLine(72 - s, 4, 73 - s, 4, d);
  tft.drawLine(70 - s, 5, 71 - s, 5, d);
  tft.drawLine(74 - s, 5, 75 - s, 5, d);
  tft.drawLine(68 - s, 6, 69 - s, 6, d);
  tft.drawLine(76 - s, 6, 77 - s, 6, d);
  tft.drawLine(72 - s, 7, 73 - s, 7, d);
  tft.drawLine(70 - s, 8, 71 - s, 8, d);
  tft.drawLine(74 - s, 8, 75 - s, 8, d);
  tft.drawRect(72 - s, 10, 2, 3, d);

}

void nWiFi(uint16_t d) {
  //0x632C-не активен   ST77XX_WHITE-активен
  if (d==1) { d=ST77XX_WHITE;} else { d=ST77XX_BLACK;}
  int s = 30;
  tft.drawLine(72 - s, 1, 73 - s, 1, d);
  tft.drawLine(70 - s, 2, 71 - s, 2, 0);
  tft.drawLine(74 - s, 2, 75 - s, 2, 0);
  tft.drawLine(68 - s, 3, 69 - s, 3, 0);
  tft.drawLine(76 - s, 3, 77 - s, 3, 0);
  tft.drawPixel(67 - s, 4, 0);
  tft.drawPixel(78 - s, 4, 0);
  tft.drawLine(72 - s, 4, 73 - s, 4, d);
  tft.drawLine(70 - s, 5, 71 - s, 5, 0);
  tft.drawLine(74 - s, 5, 75 - s, 5, 0);
  tft.drawLine(68 - s, 6, 69 - s, 6, 0);
  tft.drawLine(76 - s, 6, 77 - s, 6, 0);
  tft.drawLine(72 - s, 7, 73 - s, 7, d);
  tft.drawLine(70 - s, 8, 71 - s, 8, 0);
  tft.drawLine(74 - s, 8, 75 - s, 8, 0);
  tft.drawRect(72 - s, 10, 2, 3, d);

}

void cps_shkala(int c) {
  tft.fillRect(94,19,93,11,ST77XX_BLACK);
  int dX=2;
  uint16_t col1=0xEF9D; 
  tft.drawLine(94+dX+9, 31, 94+dX+9, 61, col1);
  tft.drawLine(94+dX+9, 31, 97+dX+9, 31, col1);
  tft.drawLine(94+dX+9, 34, 95+dX+9, 34, col1);
  tft.drawLine(94+dX+9, 37, 95+dX+9, 37, col1);
  tft.drawLine(94+dX+9, 40, 95+dX+9, 40, col1);
  tft.drawLine(94+dX+9, 43, 95+dX+9, 43, col1);
  tft.drawLine(94+dX+9, 46, 97+dX+9, 46, col1);
  tft.drawLine(94+dX+9, 49, 95+dX+9, 49, col1);
  tft.drawLine(94+dX+9, 52, 95+dX+9, 52, col1);
  tft.drawLine(94+dX+9, 55, 95+dX+9, 55, col1);
  tft.drawLine(94+dX+9, 58, 95+dX+9, 58, col1);
  tft.drawLine(94+dX+9, 61, 97+dX+9, 61, col1);
  tft.setTextColor(0x2F50, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(97+dX+1, 21);  //95,21
  tft.setFont(&FreeSans6pt8b); // выбор шрифта
  if (c<10) { tft.print("00"); tft.print(c); } 
  if (c>=10 && c<100) { tft.print("0"); tft.print(c); }
  if (c>=100) { tft.print(c); }
  tft.setCursor(96+dX+1, 14);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("CPS");
  tft.setFont();
  if (c > -1) {
    int dec1 = 1+3 * (c % 10) * 1;
    int dec2 = 1+3 * ((c % 100) - (c % 10)) / 10;
    int dec3 = 1+3 * ((c % 1000) - (c % 100)) / 100;
    tft.fillRect(99+dX+4+5, 31, 3, 31-dec1, ST77XX_BLACK);
    tft.fillRect(99+dX+4+5, 31+30-dec1, 3, dec1, 0x6FF0); //зеленый
    tft.fillRect(106+dX+2+5, 31, 3, 31-dec2, ST77XX_BLACK);
    tft.fillRect(106+dX+2+5, 31+30-dec2, 3, dec2, 0xFF70); //желтый
    tft.fillRect(113+dX+5, 31, 3, 31-dec3, ST77XX_BLACK);
    tft.fillRect(113+dX+5, 31+30-dec3, 3, dec3, 0xFC0F); //красный
    tft.drawLine(99+dX+8, 35, 117+dX+8, 35, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 38, 117+dX+8, 38, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 41, 117+dX+8, 41, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 44, 117+dX+8, 44, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 47, 117+dX+8, 47, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 50, 117+dX+8, 50, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 53, 117+dX+8, 53, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 56, 117+dX+8, 56, ST77XX_BLACK);
    tft.drawLine(99+dX+8, 59, 117+dX+8, 59, ST77XX_BLACK);
    tft.drawLine(98-1, 32, 98-1, 36, col1);
    tft.drawPixel(97-1, 33, col1);
    tft.drawRect(100-1 , 32, 3, 5, col1);
    tft.drawRect(100-1 , 56, 3, 5, col1);
    tft.drawLine(100-1, 44, 102-1, 44, col1);
    tft.drawLine(100-1, 48, 102-1, 48, col1);
    tft.drawLine(100-1, 46, 102-1, 46, col1);
    tft.drawPixel(100-1, 45, col1);
    tft.drawPixel(102-1, 47, col1);
  }
}

//логарифмическая шкала мкР/ч
void log_shkala(float r) {
  tft.drawLine(4, 67, 24, 67, 0x2F50);
  tft.drawLine(4, 71, 24, 71, 0x2F50);
  tft.drawLine(4, 67, 4, 71, 0x2F50);
  tft.drawLine(24, 67, 44, 67, ST77XX_YELLOW);
  tft.drawLine(24, 71, 44, 71, ST77XX_YELLOW);
  tft.drawLine(44, 67, 124, 67, ST77XX_RED);
  tft.drawLine(44, 71, 124, 71, ST77XX_RED);
  tft.drawLine(124, 67, 124, 71, ST77XX_RED);
  tft.drawLine(4, 67, 4, 65, ST77XX_WHITE);
  tft.drawLine(24, 67, 24, 65, ST77XX_WHITE);
  tft.drawLine(44, 67, 44, 65, ST77XX_WHITE);
  tft.drawLine(64, 67, 64, 65, ST77XX_WHITE);
  tft.drawLine(84, 67, 84, 65, ST77XX_WHITE);
  tft.drawLine(104, 67, 104, 65, ST77XX_WHITE);
  tft.drawLine(124, 67, 124, 65, ST77XX_WHITE);

  if (r<=10) {
    tft.fillRect(5, 68, r*2, 3, 0x2F50); 
    tft.fillRect(5+r*2, 68, 124-(5+r*2), 3, ST77XX_BLACK);
  }
  if (r>10 && r<=100) {
    tft.fillRect(5, 68, 20, 3, 0x2F50);
    tft.fillRect(25, 68, r*2/10, 3, ST77XX_YELLOW);
    tft.fillRect(25+r*2/10, 68, 124-(25+r*2/10), 3, ST77XX_BLACK);
  }
  if (r>100 && r<=1000) {
    tft.fillRect(5, 68, 20, 3, 0x2F50);
    tft.fillRect(25, 68, 20, 3, ST77XX_YELLOW);
    tft.fillRect(45, 68, r*2/100, 3, ST77XX_RED);
    tft.fillRect(45+r*2/100, 68, 124-(45+r*2/100), 3, ST77XX_BLACK);
  }
  if (r>1000 && r<=10000) {
    tft.fillRect(5, 68, 20, 3, 0x2F50);
    tft.fillRect(25, 68, 20, 3, ST77XX_YELLOW);
    tft.fillRect(45, 68, 20, 3, ST77XX_RED);
    tft.fillRect(65, 68, r*2/1000, 3, ST77XX_RED);
    tft.fillRect(65+r*2/1000, 68, 124-(65+r*2/1000), 3, ST77XX_BLACK);
  }
  if (r>10000 && r<=100000) {
    tft.fillRect(5, 68, 20, 3, 0x2F50);
    tft.fillRect(25, 68, 20, 3, ST77XX_YELLOW);
    tft.fillRect(45, 68, 20, 3, ST77XX_RED);
    tft.fillRect(65, 68, 20, 3, ST77XX_RED);
    tft.fillRect(85, 68, r*2/10000, 3, ST77XX_RED);
    tft.fillRect(85+r*2/10000, 68, 124-(85+r*2/10000), 3, ST77XX_BLACK);
  }
  if (r>10000 && r<=1000000) {
    tft.fillRect(5, 68, 20, 3, 0x2F50);
    tft.fillRect(25, 68, 20, 3, ST77XX_YELLOW);
    tft.fillRect(45, 68, 20, 3, ST77XX_RED);
    tft.fillRect(65, 68, 20, 3, ST77XX_RED);
    tft.fillRect(85, 68, 20, 3, ST77XX_RED);
    tft.fillRect(105, 68, r*2/100000, 3, ST77XX_RED);
    tft.fillRect(105+r*2/100000, 68, 124-(105+r*2/100000), 3, ST77XX_BLACK);
  }
}

void col_Text_MD_Ind_Okrugl(float r){
  if (r<30) {
    colText=0x2F50;
    mess=" норма";
  }
  if (r>=30 && MD<101) {
    colText=ST77XX_YELLOW;
    mess="внимание";
  }
  if (r>=101) {
    colText=ST77XX_RED;
    mess=" опасно";
  }
  if (r<1000)                  {MD_ind=MD/1;       rh="мкР/ч  ";}
  if (r>=1000 && MD<1000000)   {MD_ind=MD/1000;    rh="мР/ч  ";}
  if (r>=1000000)              {MD_ind=MD/1000000; rh="Р/ч   ";}
  if (MD_ind<10) {okr=2;}
  if (MD_ind>=10 && MD<100) {okr=1;}
  if (MD_ind>=100 && MD<1000) {okr=0;}
}

void Top_Screen(float mi, uint16_t c, int o, String r, int p, String m) {
  tft.fillRect(0,20,93,41,ST77XX_BLACK);  //93 0xB147
  tft.fillRect(34,74,60,13,ST77XX_BLACK);
  tft.setTextColor(c, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(10, 38);
  tft.setFont(&FreeSans18pt8b); // выбор шрифта
  if (p>80) {tft.setTextColor(ST77XX_WHITE); tft.print("---");} else {tft.print(mi, o);}
   
  tft.setCursor(10, 60);
  tft.setFont(&FreeSans6pt8b); // выбор шрифта
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print(r);
  tft.print("    ");
  tft.drawLine(60-10,55,65-10,55,ST77XX_WHITE);
  tft.drawLine(62-10,53,62-10,58,ST77XX_WHITE);
  tft.drawLine(60-10,60,65-10,60,ST77XX_WHITE);
  //p=random(150);
  tft.print(p);
  tft.print("%  ");
  tft.setCursor(36, 82);
  tft.setFont(&FreeSans6pt8b); // выбор шрифта
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  if (p>80) { tft.print("  ");} else {tft.print(m);}
  //tft.print(m);
  tft.setFont();
}

void CPS_grph (int c) {
  for (int ic = 1; ic < 32; ic++) {
    cps24[33 - ic] = cps24[33 - ic - 1];
    col24[33 - ic] = col24[33 - ic-1];
    if (ic == 1) {
      cpsMax = cps24[33 - ic];
    } else {
      if (cpsMax < cps24[33 - ic]) { cpsMax = cps24[33 - ic]; }
    }
    if (cpsMax<10) {cpsMax=10;}
    tft.fillRect(127-(4 * ic), 155, -3, -(cps24[ic] * cpsK), col24[ic]);
    tft.fillRect(127-(4 * ic), 105, -3, 50-(cps24[ic] * cpsK), ST77XX_BLACK);
  }
  if (c > 0) {
    cps24[1] = c; //cps1s;
    uint16_t col_Graph;
    if (c<7) {col_Graph=0x2F50;}
    if (c>6) {col_Graph=ST77XX_YELLOW;}
    if (c>16) {col_Graph=ST77XX_RED;}
    col24[1] = col_Graph;
  } else {
    if ((cps24[2] - 1) > 0) {
      cps24[1] = cps24[2] - 1;
    } else {
      cps24[1] = 0;
    }
  }
  if (cpsMax < cps24[1]) { cpsMax = cps24[1]; }
  if (cpsMax == 0) {
    cpsK = 2;
  } else {
    cpsK = 50 / cpsMax;
  }
  tft.fillRect(127, 155, -3, -(cps24[1] * cpsK), col24[1]);
  tft.fillRect(127, 105, -3, 50-(cps24[1] * cpsK), ST77XX_BLACK);
  if (c > 0) {
    tft.setTextSize(1);
    tft.setFont(&FreeSans6pt8b); // выбор шрифта
    tft.setCursor(4, 98);
    tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    tft.fillRect(4, 87, 110, 14, ST77XX_BLACK);
    tft.print("CPS");
    tft.setCursor(37, 98);
    tft.setTextColor(col24[1], ST77XX_BLACK);
    tft.print(c);
    tft.setFont();
    tft.drawLine(0, 103, 128, 103, 0xCE79);
    tft.drawLine(0, 157, 128, 157, 0xCE79);
  }
}

void MD_grph (float m) {
  float k_md_g;
  for (int ic = 1; ic < 127; ic++) {
    md24[128 - ic] = md24[128 - ic - 1];
    md_col24[128 - ic] = md_col24[128 - ic-1];
    if (ic == 1) {
      cpsMax = md24[128 - ic];
    } else {
      if (cpsMax < md24[128 - ic]) { cpsMax = md24[128 - ic]; }
    }
    if (cpsMax<20) {cpsMax=20;}
    tft.fillRect(127-(1 * ic), 155, -1, -(md24[ic] * cpsK), md_col24[ic]);
    tft.fillRect(127-(1 * ic), 105, -1, 50-(md24[ic] * cpsK), ST77XX_BLACK);
    uint16_t col_l=0xB5D6;
    tft.drawPixel(127-(1 * ic), 115, col_l);
    tft.drawPixel(127-(1 * ic), 125, col_l);
    tft.drawPixel(127-(1 * ic), 135, col_l);
    tft.drawPixel(127-(1 * ic), 145, col_l);
  }
  if (m > 0) {
    md24[1] = m; //cps1s;
    uint16_t col_Graph;
    if (m<=30) {col_Graph=0x2F50;}
    if (m>30) {col_Graph=ST77XX_YELLOW;}
    if (m>100) {col_Graph=ST77XX_RED;}
    md_col24[1] = col_Graph;
  } else {
    if ((md24[2] - 1) > 0) {
      md24[1] = md24[2] - 1;
    } else {
      md24[1] = 0;
    }
  }
  if (cpsMax < md24[1]) { cpsMax = md24[1]; }
  if (cpsMax == 0) {
    cpsK = 2;
  } else {
    cpsK = 50 / cpsMax;
  }
  tft.fillRect(127, 155, -1, -(md24[1] * cpsK), md_col24[1]);
  tft.fillRect(127, 105, -1, 50-(md24[1] * cpsK), ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setFont(&FreeSans6pt8b); // выбор шрифта
  tft.setCursor(4, 98);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.print("Мощность дозы");
  tft.setFont();
  tft.drawLine(0, 103, 128, 103, ST77XX_WHITE);
  tft.drawLine(0, 157, 128, 157, ST77XX_WHITE);
}

void ResetDoze() {
  Time = 0;
  memset(Counts, 0, 110);
}

void InitScr1() {
  tft.fillScreen(ST77XX_WHITE);
  tft.fillRect(0,0,128,87,ST77XX_BLACK);
}

void InitScr2() {
  tft.fillScreen(ST77XX_BLACK);
}

void setup(void) {
  Serial.begin(115200);
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab    INITR_GREENTAB   INITR_BLACKTAB
  tft.setSPISpeed(60000000);
  uint16_t time = millis();
  time = millis() - time;
  delay(500);

  //tft.fillScreen(ST77XX_WHITE);
  //tft.fillRect(0,0,128,87,ST77XX_BLACK);
  
  pinMode(ADC_pin, INPUT);                  // Инициализируем АЦП как получатель данных
  hello();                                   // Приветствуем пищанием
  
  
  pinMode(buttonPin1, INPUT);  //Вход кнопки1
  pinMode(buttonPin2, INPUT);  //Вход кнопки2
  pinMode(buttonPin3, INPUT);  //Вход кнопки3
  pinMode(LedPin, OUTPUT);
  digitalWrite(LedPin, HIGH);

  //WiFi.begin(ssid, pass);
  //Blynk.config(auth, "blynk.tk", 8080);
  Wire.begin();

  LittleFS.begin();
  FDstat_t stat = data.read();
  switch (stat) {
    case FD_FS_ERR:
      //Serial.println("FS Error");
      break;
    case FD_FILE_ERR:
      //Serial.println("Error");
      break;
    case FD_WRITE:
      //Serial.println("Data Write");
      break;
    case FD_ADD:
      //Serial.println("Data Add");
      break;
    case FD_READ:
      //Serial.println("Data Read");
      break;
    default:
      break;
  }

  Scr = (mydata.scr_e);

  if (Scr==1) { InitScr1(); }
  if (Scr==2) { InitScr2(); }
  if (Scr==3) { InitScr2(); }

  wifiOn = (mydata.wifi_e);
  sound = (mydata.sound_e);
  Sensitivity = radSens.getSensitivity();
  s_f_c=mydata.s_f_c_e;
  m_v_c=mydata.m_v_c_e;
  k_count=mydata.k_c_e;

  radSens.setSensitivity(615);
  dinamik(sound);
  light = radSens.getLedState();
  drawlight(light);

  if (wifiOn == 0) {WiFi.mode(WIFI_OFF);} 
  else {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    Blynk.config(auth, "blynk.tk", 8080);
    delay(1000);
    Blynk.virtualWrite(V15, s_f_c );
    Blynk.virtualWrite(V16, m_v_c );
    Blynk.virtualWrite(V17, k_count);
  }
  pulsesPrev = radSens.getNumberOfPulses();  // Записываем значение для предотвращения серии тресков на старте
  prev_counter_cps = pulsesPrev;
}

void loop() {

  //if (data.tick() == FD_WRITE) {}

  bool net = WiFi.isConnected();
  if (net) { 
    Blynk.run(); 
    sWiFi(wifiOn);
    Blynk.virtualWrite(V15, s_f_c );
    Blynk.virtualWrite(V16, m_v_c );
    Blynk.virtualWrite(V17, k_count);
  } else { nWiFi(wifiOn);}

  // раз в 250 мс происходит опрос счётчика импульсов для создания тресков, если число импульсов за 250 мс превысит 5, раздастся предупреждение
  if (millis() - timer_imp > 250) {  //250
    timer_imp = millis();
    int pulses = radSens.getNumberOfPulses();
    puls = pulses - pulsesPrev;
    //звук.сигнал. предупреждение
    if (pulses - pulsesPrev > 5) {
      if (sound == 1) { warning(); }
      //rad_sign(0, 1);  //рисует знак радиации
    }
    if (puls < 3) { warn--; if (warn == -1) { warn = 0; } }
    if (puls > 3) { warn = 1; }
    if (puls > 4) { warn = 2; }
    if (puls > 5) { warn = 3; }
    if (puls > 6) { warn = 4; }
    if (puls > 7) { warn = 5; }
    if (puls > 8) { warn = 6; }
    if (puls > 9) { warn = 7; }
    if (puls > 10) { warn = 8; }
    if (puls > 11) { warn = 9; }
    if (puls > 12) { warn = 10; }
    maswarn[i2] = warn;
    cps250[i] = puls;
    i++;
    i2++;
    if (i == 5) { i = 1; }
    if (i2 == 4) { i2 = 1; }
    //awarn = (maswarn[1] + maswarn[2] + maswarn[3] + maswarn[4]) / 4;
    awarn = (maswarn[1] + maswarn[2] + maswarn[3]) / 3;
    cps1s = cps250[1] + cps250[2] + cps250[3] + cps250[4];  //сумма 4х замеров по 250мс
    //звук щелчков
    if (pulses > pulsesPrev) {
      for (int i = 0; i < (puls); i++) {
        if (sound == 1) { beep(); }
      }
    }
    pulsesPrev = pulses;
    // передача данных на blynk.tk
    if (net) {
      Blynk.virtualWrite(V0, cps1s); //Blynk CPS 4значения по 250мс = 1сек
      Blynk.virtualWrite(V6, awarn); //Blynk режим поиска
    }
    if (Scr==1) {
      //значения для стрелочного прибора
      if (awarn!=awarn_prev) { Strel_Indikator(awarn); }
      awarn_prev=awarn;
    }
  }

  //Считаем импульсы за 1 сек
  if (millis() - timer_cps > 1000) {
    timer_cps = millis();
    counter_cps = radSens.getNumberOfPulses();
    Count = (counter_cps - prev_counter_cps);  //получение импульсов за 1сек.
    prev_counter_cps = counter_cps;
    Time++;
    TimeA++;
    if (Time > Cycl) { Time = Cycl; }
    if (TimeA > 750) { TimeA = 750; }
    SUMofCounts = 0;
    SUMofCountsA = 0;
    //наполение массива
    if (priznC1000 == 1) {
      for (int i1 = Cycl; i1 > Time; i1--) { Counts[i1] = 0; }
    }
    for (int i1 = 1; i1 < Time + 1; i1++) {
      if (i1 < Time) {
        Counts[Time + 1 - i1] = Counts[Time + 1 - i1 - 1];
      } else {
        Counts[Time + 1 - i1] = Count;
      }
    }

    for (int i1 = 1; i1 < TimeA + 1; i1++) {
      if (i1 < TimeA) {
        CountsA[TimeA + 1 - i1] = CountsA[TimeA + 1 - i1 - 1];
      } else {
        CountsA[TimeA + 1 - i1] = Count;
      }
    }

    priznC1000 = 0;
    //Сумма импульсов за цикл окна
    for (int i1 = 1; i1 < Time + 1; i1++) {
      SUMofCounts = SUMofCounts + Counts[i1];
      if (SUMofCounts > 1000) {
        priznC1000 = 1;
        Time = i1;
        break;
      }
    }

    for (int i1 = 1; i1 < TimeA + 1; i1++) {
      SUMofCountsA = SUMofCountsA + CountsA[i1];
    }

    SUMofCounts=SUMofCounts-s_f_c*Time;
    SUMofCountsA=SUMofCountsA-s_f_c*TimeA;
    if (SUMofCounts<0) {SUMofCounts=0;}
    if (SUMofCountsA<0) {SUMofCountsA=0;}
    MDozi = (k_count / Time) * SUMofCounts;
    MDoziA = (k_count / TimeA) * SUMofCountsA;
     
    //if ((MDozi*1.2)<MDoziA) {Serial.println("меньше среднего");}
    //if (MDozi>(MDoziA*1.2)) {Serial.println("больше среднего");}
    
    //-----> сумма квадратов
    float sr_a = SUMofCounts / Time;
    float sum_kv = 0;
    for (int i1 = 1; i1 < Time + 1; i1++) {
      sum_kv = sum_kv + sq(Counts[i1] - sr_a);
    }
    //<----- сумма квадратов
    float st_otkl = sqrt(sum_kv / (Time - 1));            //стандартное отклонение
    float delta = ((2 * st_otkl / (sqrt(Time))) / sr_a);  //относительное отклонение
    if (sr_a * 7 < Count) {
      //Serial.println("Скачек вверх");
      ResetDoze();
      if (Scr == 6) {
        //drawDown(0);
        //drawUp(1);
      }

    }
    if ((Count + 1) * 7 < sr_a) {
      //Serial.println("Скачек вниз");
      ResetDoze();
      if (Scr == 6) {
        //drawUp(0);
        //drawDown(1);
      }
    }
    if (net) {
      Blynk.virtualWrite(V8, delta*100); //Blynk отклонение отностит в %
      Blynk.virtualWrite(V9, sr_a); //Blynk среднее арифм CPS за окно
      Blynk.virtualWrite(V10, Count); //Blynk  CPS за 1сек;
      Blynk.virtualWrite(V11, MDozi); //Blynk мощность дозы
      Blynk.virtualWrite(V12, MDoziA); //Blynk мощность дозы усреднение 750сек
    }
    /*
    Serial.print(Time);
    Serial.println(" секундный цикл  ");
    Serial.print(sr_a);
    Serial.println(" сред.арифм.CPS");
    Serial.print(Count);
    Serial.println(" CPS текущий");
    Serial.print("сумма CPS за цикл ");
    Serial.println(SUMofCounts);
    Serial.print("собств. фон за цикл ");
    Serial.println(0.3*Time);
    Serial.print("мощность дозы ");
    Serial.print(MDozi);
    Serial.println("мкР/ч");
    Serial.print("дельта ");
    Serial.print(delta * 100);
    Serial.println("% ");
    Serial.print(buf1);
    Serial.println(" мкР/ч динам. radsens");
    Serial.println("____________________");
    */
    //полученное значение мощности дозы в мкР/ч
    MD=MDozi;
    //лог.шкала мкР/ч
    log_shkala(MD);
    //цвет текст мощности дозы, текст опасности, масштаб мощности дозы, текст размерности мощности дозы
    col_Text_MD_Ind_Okrugl(MD);
    //значения CPS
    int cps=Count;
    cps_shkala(cps);
    //cps_shkala(sr_a);
    //вывод на экран мощности дозы в цвете, погрешностиб текста опасности
    pogresh=delta*100;
    if (pogresh>99) {pogresh=99;}
    if ( MDozi != MD_ind_prev ) {Top_Screen(MD_ind, colText, okr, rh, pogresh, mess); }
    MD_ind_prev = MDozi;

    // экран №2 Obsidin
    if (Scr == 2) {
      CPS_grph (cps); 
      //CPS_grph (sr_a);
    }  

    // экран №3 Граик МЭД
    if (Scr == 3) {
      MD_grph (MDozi);
    }  
  }

  // cчитываем показание с АЦП, рисуем батарею и создаём индикацию заряда, показания АЦП вы можете подстроить под своё удобство
  if (millis() - timer_bat > 5000) {
    timer_bat = millis();
    ADC = analogRead(ADC_pin);
    if (net) { Blynk.virtualWrite(V5, ADC); }  //Blynk заряд аккумулятора
    //уравень батареи
    bat_level(ADC);
  }

  // кнопка 1
  //корткое вкл/выкл свет, длинное вкл/выкл звук
  buttonState1 = !digitalRead(buttonPin1);
  if (buttonState1 == true && press_flag == false && millis() - last_press > 100) {
    press_flag = !press_flag;
    last_press = millis();
  }
  if (buttonState1 == true && press_flag == true && millis() - last_press > 1000) {
    long_press_flag = !long_press_flag;
    last_press = millis();
    //Serial.println("долгое нажатие 1");
    sound = !(sound);
    dinamik(sound);
    mydata.sound_e = sound;
    data.updateNow();
  }
  if (buttonState1 == false && press_flag == true && long_press_flag == true) {
    press_flag = !press_flag;
    long_press_flag = !long_press_flag;
  }
  if (buttonState1 == false && press_flag == true && long_press_flag == false) {
    press_flag = !press_flag;
    //Serial.println("короткое нажатие 1");
    if (Scr == 10) {
      Sensitivity++;
    } else {
      light = !(light);
      drawlight(light);
      radSens.setLedState(light);
    }
  }

  // кнопка 2
  // переключение видов экрана / откл-вкл экрана
  buttonState2 = !digitalRead(buttonPin2);
  if (buttonState2 == true && press_flag2 == false && millis() - last_press2 > 100) {
    press_flag2 = !press_flag2;
    last_press2 = millis();
  }
  if (buttonState2 == true && press_flag2 == true && millis() - last_press2 > 1000) {
    //Serial.println("долгое нажатие 2");
    long_press_flag2 = !long_press_flag2;
    last_press2 = millis();
    TFTPower = !(TFTPower);
    digitalWrite(LedPin, TFTPower);
  }
  if (buttonState2 == false && press_flag2 == true && long_press_flag2 == true) {
    press_flag2 = !press_flag2;
    long_press_flag2 = !long_press_flag2;
  }
  if (buttonState2 == false && press_flag2 == true && long_press_flag2 == false) {
    press_flag2 = !press_flag2;
    //Serial.println("короткое нажатие 2");
    Scr++;
    if (Scr > 3) { Scr = 1; }
    mydata.scr_e = Scr;
    data.updateNow();
    if (Scr==1)  {
      InitScr1();
      Strel_Indikator(awarn);
    }
    if (Scr==2)  {InitScr2();}
    if (Scr==3)  {InitScr2();}
    dinamik(sound);
    drawlight(light);
  }

  // кнопка 3
  // откл-вкл WiFi / откл-вкл экрана
  buttonState3 = !digitalRead(buttonPin3);
  if (buttonState3 == true && press_flag3 == false && millis() - last_press3 > 100) {
    press_flag3 = !press_flag3;
    last_press3 = millis();
  }
  if (buttonState3 == true && press_flag3 == true && millis() - last_press3 > 1000) {
    long_press_flag3 = !long_press_flag3;
    last_press3 = millis();
    //сохранение настроек счетчика
    timer_cnt=0;
    timer_cps=0; 
    timer_bat=0; 
    timer_imp=0; 
    pulsesPrev=0;
    Count = 0;    //счетчик импульсов
    Counts[210];  //массив импульсов
    CountsA[760];  //массив импульсов
    Time = 0;  //счетчик массива счетчик времени в секундах
    TimeA = 0;  //счетчик всего времени в секундах 
    SUMofCounts=0;
    SUMofCountsA=0;  //усреднение за все время
    MDozi=0;
    MDoziA=0;       //усреднение за все время  
    priznC1 = 0;     //признак первого цикла окна
    priznC1000 = 0;  //признак 1000 импульсов
    Cycl = 200;      //начальное значение циклов
    cpulses=0;
    tft.fillRect(10, 20, 83, 30, ST77XX_BLACK);
    tft.setTextColor(0x2F50);
    tft.setFont(&FreeSans6pt8b); // выбор шрифта
    tft.setCursor(17, 30);
    tft.println("   Сброс");
    tft.print("    измерения");
    tft.setFont(); // выбор шрифта
    ResetDoze();
    delay(1500);

    

  }
  if (buttonState3 == false && press_flag3 == true && long_press_flag3 == true) {
    press_flag3 = !press_flag3;
    long_press_flag3 = !long_press_flag3;
  }
  if (buttonState3 == false && press_flag3 == true && long_press_flag3 == false) {
    press_flag3 = !press_flag3;
    //Serial.println("короткое нажатие 3");

    if (Scr != 10) {
      if (wifiOn == 1) {
        wifiOn = 0;
        WiFi.mode(WIFI_OFF);
      } else {
        wifiOn = 1;
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, pass);
        Blynk.config(auth, "blynk.tk", 8080);
      }
      mydata.wifi_e = wifiOn;
      data.updateNow();
    } 
  }

}

