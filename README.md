Радиометр RadSens 7.2 на базе платы RadSens от ClimateGuard
<br>
https://climateguard.ru/radsens/
<br>
<img src="https://github.com/dimkra/RadSens-Blynk.tk/assets/37121139/c7f5860e-bdcb-4521-91ef-df862f12660b" width="500" >
<br>
Изменяемые параметры платы RadSens:<br>
<br>
Настройка чувствительности счетчика:
radSens.setSensitivity(105); //установить<br> 
radSens.getSensitivity(); //считать состояние<br>
значение 105 - по умолчанию для счетчика СБМ-20.<br>
<br>
Включение-выключение высоковольтного генератора:<br>
radSens.setHVGeneratorState(bool state); //установить <br>
radSens.getHVGeneratorState(); //считать состояние<br>
<br>
Включение-выключение светодиодного индикатора:<br>
radSens.setLedState(bool state); //установить <br>
radSens.getLedState(); //считать состояние<br>
<br>
Получение данных:<br>
radSens.getNumberOfPulses(); //количество зарегистрированных импульсов<br>
radSens.getRadIntensyDynamic(); //динамическое значение мощности дозы в мкР/ч<br>
radSens.getRadIntensyStatic(); //статическое значение мощности дозы в мкР/ч<br>
<br>
В проекте используется:<br>
<br>
EPS8266 D1 mini
<br>
<img src="https://github.com/dimkra/RadSens-Blynk.tk/assets/37121139/f4abab11-0c14-4171-95e0-d45c29d0f995" width="500" >
<br>
LSD дисплей 128x160 ST7735S с шиной SPI
<br>
Модуль заряда аккумуляторов TP4056
<br>
Резисторы: 220кОм, 100кОм
<br>
Литьевый аккумулятор
<br>
Кнопка -3шт
<br>
Выключатель
<br>
Пищалка<br>
<br>
<img src="image/shema7_2.jpg" width="700" >
<br><br>
В проекте используются следующие библиотеки:<br>
<br>
Adafruit-ST7735-Library<br>
https://github.com/adafruit/Adafruit-ST7735-Library<br>
<br>
FileDat от AlexGyver<br>
https://github.com/GyverLibs/FileData<br>
<br>
Official RadSens library by ClimateGuard<br>
https://github.com/climateguard/RadSens<br>
<br>
Blynk от Volodymyr Shymanskyy Внимание!!! есть ограничение на версию - не старше 1.1.0<br>
Blynk сервер - blynk.tk<br>
Руководство по подключению https://wiki.blynk.tk/<br>
<br>
Дополнительные ссылки для менеджера плат: https://arduino.esp8266.com/stable/package_esp8266com_index.json<br>
<br>
Работа кнопок:
<br>
1 кнопка - короткое нажатие - вкл., откл. световых сигналов<br>
долгое нажатие (1сек.) - вкл., откл. звукового сигнала<br>
<br>

2 кнопка - короткое нажатие - переключение экранов (поиск-стрелочный индикатор, поиск по CPS, гафик мощности дозы)<br>
долгое нажатие (1сек.) - вкл., откл. экрана<br>
<br>

3 кнопка - короткое нажатие - вкл., откл. wi-fi<br>
долгое нажатие (1сек.) - сброс измерения<br>
<br>

На экране отображаются следующие данные:
<br>
-Текущая мощность дозы;<br>
-погрешность <br>
Содержит динамическое значение интенсивности ионизирующего гамма-
излучения. При детектировании резкого изменения интенсивности излучения
(как в большую, так и в меньшую сторону) динамически регулирует период счета
скользящего окна, чтобы диапазон охватывал временной промежуток,
содержащий только актуальные данные. Позволяет использовать устройство в
режиме поиска локальных загрязнений. Частота обновления – 1 сек.
<br>
-Статическое значение мощности дозы в мкР или мР
Содержит статистическое значение интенсивности ионизирующего гамма-
излучения. Период счета скользящего окна составляет 500 сек. Позволяет
производить точные измерения постоянного радиационного фона. Частота
обновления – 1 сек.<br>
<br>
пример отображения данных в мобильном приложении Blynk
<br>
<img src="https://github.com/dimkra/RadSens-Blynk.tk/assets/37121139/c710e1c0-8256-446f-a18e-244fac3d791f" height="300" >
<br><br>
Управление через API:
<br>
Пример: https://api.blynk.tk/{TOKEN}/update/V0?value=0
<br>
<br>
Радиометр на счетчике СБТ10-А<br>
<img src="https://github.com/user-attachments/assets/c3d92cc3-b500-42e2-ad2f-a3a8598bb3a0" height="300" >
<img src="https://github.com/user-attachments/assets/8a0f3be5-1232-48b7-af1a-84f5606b523e" height="300" >


![2024-07-31 10-57-26](https://github.com/user-attachments/assets/ad61904d-ca71-4d67-b5c3-2b3166697494)


