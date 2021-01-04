#include <TM1637Display.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <RTClib.h>
#include <Arduino.h>
#include <Wire.h>
#include <RH_NRF24.h>

// Module connection pins (Digital Pins)
#define CLK 3
#define DIO 2

#define SCREEN_WIDTH 128
#define SCREEN_HIGHT 64

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and mayx
// need to be adapted

//#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 4

#define CLK_PIN   13
#define DATA_PIN  11
#define CS_PIN    10


// Hardware SPI connection
MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);
// Arbitrary output pins
// MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HIGHT, &Wire, -1);

RTC_DS1307 rtc;

TM1637Display segmentDisplay(CLK, DIO);


bool am_Pm;
bool set_alarm = true;
bool alarm_AM_PM;
int alarm_Hour = 20;
int alarm_Minute = 48;
int alarm_Hour_Button;
int alarm_Minute_Button;
int current_Minute;
int hour_time;

// Singleton instance of the radio driver
//RH_NRF24 nrf24;
RH_NRF24 nrf24(9, 8); // use this to be electrically compatible with Mirf
// RH_NRF24 nrf24(8, 10);// For Leonardo, need explicit SS pin
// RH_NRF24 nrf24(8, 7); // For RFM73 on Anarduino Mini

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (! rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (! rtc.isrunning())
  {
    Serial.println("RTC is NOT running!");
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  //  while (!Serial); // wait for serial port to connect. Needed for Leonardo only
  if (!nrf24.init())
    Serial.println("init failed");
  // Defaults after init are 2.402 GHz (channel 2), 2Mbps, 0dBm
  if (!nrf24.setChannel(1))
    Serial.println("setChannel failed");
  if (!nrf24.setRF(RH_NRF24::DataRate2Mbps, RH_NRF24::TransmitPower0dBm))
    Serial.println("setRF failed");

  P.begin();

  pinMode(7, OUTPUT);
  pinMode(8, INPUT);
  pinMode(6, INPUT);
  // Pin for Water Pump
  pinMode(11, OUTPUT);
  pinMode(10, INPUT);
  Serial.println("In Setup");
//   Serial.println("In 5 seconds #########################################################################");
//   delay(5000);
    digitalWrite(11, HIGH);
    delay(15000);
    digitalWrite(11, LOW);
}
void loop() {
  //Serial.println("****************************** In Loop ******************************");
  display_Dot_Matrix("Center", PA_CENTER, 7);
  display_Date(getcurrentDate());
  getcurrentTime();
  display_Time(getcurrentTime());
  set_Alarm_Button();
  alarm();
  if_Button_Pressed();
    Serial.println("****************************** In MOTOR ******************************");
  
    Serial.print("set_alarm:");
    Serial.println(set_alarm);
    Serial.print(" alarm_Minute:");
    Serial.println(alarm_Minute);
      Serial.print("current_Minute:");
    Serial.println(current_Minute);
  if (digitalRead(10) == HIGH) {
    set_Alarm_OFF();
  }
  if (set_alarm == true && alarm_Minute <  current_Minute)
  {
    digitalWrite(11, HIGH);
    delay(15000);
    digitalWrite(11, LOW);
    set_alarm = false;
    //      Serial.print("set_alarm reset:");
    //  Serial.println(set_alarm);
  }
      Serial.print("set_alarm:");
    Serial.println(set_alarm);
    Serial.println("****************************** END MOTOR ******************************");
}
String getcurrentDate()
{
  String rtc_Date;
  DateTime now = rtc.now();
  rtc_Date = String(now.month()) + '/' +  String(now.day()) + '/' +  String(now.year());
  return rtc_Date;
}
void set_Alarm_ON()
{
  digitalWrite(7, HIGH);
}
void set_Alarm_OFF()
{
  digitalWrite(7, LOW);
}
int getcurrentTime()
{
  DateTime now = rtc.now();
  int rtc_Time;
  current_Minute = now.minute();
  if (now.hour() > 12)
  {
    hour_time = now.hour() - 12;
    am_Pm = HIGH;
  }
  else
  {
    hour_time = now.hour();
    am_Pm = LOW;
  }
  rtc_Time = (hour_time * 100) + now.minute();
  return rtc_Time;
}
void display_Date(String date_Data)
{
  set_Alarm_Button();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(40, 4);
  display.println("Date:");
  display.setCursor(3, 25);
  display.println(date_Data);
  display.setCursor(3, 45);
  display.setTextSize(2);
  display.println("Alarm:");
  display.setCursor(72 , 45);
  display.println(alarm_Hour);
  display.setCursor(92, 45);
  display.println(":");
  display.setCursor(100, 45);
  display.println(alarm_Minute);
  display.display();
}
void display_Dot_Matrix(String text, int alignment, int intensity)
{
  P.setIntensity(intensity);
  P.displayClear();
  P.setTextAlignment(alignment);
  P.print(text);
}
void display_Time(int current_time)
{
  //  Serial.print("In function display_Time:");
  //  Serial.println(current_time);
  segmentDisplay.setBrightness(0x0f);
  segmentDisplay.showNumberDec (current_time, true);
}
void set_Alarm_Button()
{

  alarm_Hour_Button = digitalRead(4);
  alarm_Minute_Button = digitalRead(6);
  if (alarm_Hour_Button == HIGH)
  {
    alarm_Hour++;
    if (alarm_Hour == 23)
    {
      alarm_Hour = 0;
    }
  }
  if (alarm_Minute_Button == HIGH)
  {
    alarm_Minute++;
    if (alarm_Minute == 60)
    {
      alarm_Minute = 0;
    }
  }
  //  Serial.print("Hour:");
  //  Serial.println(alarm_Hour);
  //  Serial.print("Minute:");
  //  Serial.println(alarm_Minute);
  delay(250);
}
void alarm()
{
  int rtc_Time;
  DateTime now = rtc.now();
  //  Serial.println("###########################################################ALARM#############################");
  //  Serial.print("alarm_Hour:");
  //  Serial.println(alarm_Hour);
  //  Serial.print("alarm_Minute:");
  //  Serial.println(alarm_Minute);
  //  Serial.print(" now.hour():");
  //  Serial.println( now.hour());
  //  Serial.print("now.minute():");
  //  Serial.println(now.minute());
  //  // delay(200);
  //  Serial.println("###########################################################ALARM#############################");
  if (alarm_Hour == now.hour() && alarm_Minute == now.minute())
  {
    set_Alarm_ON();
    set_alarm = true;
    //    Serial.println("###########################################################ALARM#############################");
    //    delay(30000);
  }
  //  else
  //  {
  //    set_Alarm_OFF();
  //  }
  if (now.hour() > 12)
  {
    hour_time = now.hour() - 12;
    am_Pm = HIGH;
  }
  else
  {
    hour_time = now.hour();
    am_Pm = LOW;
  }
}
void if_Button_Pressed()
{
  //  Serial.println("###########################################################Remote Button#############################");
  if (nrf24.available())
  {
    // Should be a message for us now
    uint8_t buf[RH_NRF24_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (nrf24.recv(buf, &len))
    {
      //      NRF24::printBuffer("request: ", buf, len);
      Serial.println("###########################################################Remote Button#############################");
      Serial.print("got request: ");
      Serial.println((char*)buf);
      set_Alarm_OFF();

      //      // Send a reply
      //      uint8_t data[] = "And hello back to you";
      //      nrf24.send(data, sizeof(data));
      //      nrf24.waitPacketSent();
      //      Serial.println("Sent a reply");
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  // Serial.println("###########################################################Remote Button End#############################");
}
