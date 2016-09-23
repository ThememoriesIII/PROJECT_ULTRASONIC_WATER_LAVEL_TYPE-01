#include "SPI.h"
#include "SD.h"
#include "Wire.h"
#include "LiquidCrystal_I2C.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define i2c_address 0x27
#define ds3231address 0x68

#define pingPin 9
#define inPin 8
#define chipSelect 10

#define option_pin 3
#define up 4
#define down 5
#define left 6
#define rigth 7

#define led 2
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LiquidCrystal_I2C lcd(i2c_address, 16, 2);

typedef struct{
    byte sec;
    byte min;
    byte hour;
}time_time;

typedef struct{
    byte date_of_month;
    byte month_of_year;
    byte year_of_all;
}day_day;

typedef struct{
  float cm;
  float dump_cm;
  float in;
  float m;
  float temp;
}ultrasonic_ultrasonic;

typedef struct{
  bool status_option;
  int select;
  int count; 
}swicht_swicht;

day_day day_data;
time_time time_data;
ultrasonic_ultrasonic ultrasonic_data;
swicht_swicht option;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ฟังชั่นแปลงเลขฐาน
byte d2b(byte val);
byte b2d(byte val);
//ฟังชั่นสำหรับร้องขอข้อมูล
void reques_function(byte hex);
//ฟังชั่นสำหรับรับค่าจากmoduleและเซนเซนเซอร์ต่างๆ
void get_time(void);
void get_date(void);
void get_temperature(void);
void get_ultrasonic(void);
//ฟังชั่นสำหรับแสดงผล
void print_time(void);
void print_date(void);
void print_temp(void);
void print_ultrasonic(void);
//หลักหน่วยหลักสิบหลัก100
void digit_rtc_ds3231(byte dec);
void digit_ulrasonic(byte dec);
//ตั้วค่าเวลา
void reset_time(void);
void reset_date(void);
void set_time(void);
void set_date(void);
//
void Initializing(void);
void write_sd(String water_lavel);
void sd_print(void);
//
void setting_time(void);

unsigned int op_sel=1;
boolean sd_state=0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(inPin, INPUT);
  pinMode(option_pin,INPUT);
  pinMode(pingPin, OUTPUT);
  pinMode(chipSelect,INPUT);
  pinMode(up,INPUT);
  pinMode(down,INPUT);
  pinMode(left,INPUT);
  pinMode(rigth,INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(up,LOW);
  digitalWrite(down,LOW);
  digitalWrite(left,LOW);
  digitalWrite(rigth,LOW);
  digitalWrite(led,LOW);
  digitalWrite(chipSelect, HIGH);
  
  Serial.begin(9600);
  Wire.begin();
  lcd.begin();
  //Initializing();
  delay (1000);
  
  lcd.print("WaterLevelMeter.");  
  lcd.setCursor(0, 1);
  lcd.print("Creaet BY.TEC#8.");
  delay (1000);
  
  lcd.clear();
  ultrasonic_data.dump_cm=0.0;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
 if(digitalRead(option_pin)==HIGH)
  setting_time();
 delay(50);
 get_date();
 //print_date();
 delay(50);
 //get_time();
 print_time();
 delay(50);
 print_temp();
 delay(50);
 print_ultrasonic();
 delay(50);
 sd_print();
 delay(50);
 
if(digitalRead(chipSelect)!=LOW)
{
  Initializing();
   if((ultrasonic_data.dump_cm-ultrasonic_data.cm)>=2)
   {
    ultrasonic_data.dump_cm=ultrasonic_data.cm;
    write_sd("DOWN");
   }
   if((ultrasonic_data.dump_cm-ultrasonic_data.cm)<=-1)
   {
    ultrasonic_data.dump_cm=ultrasonic_data.cm;
    write_sd("UP");
   }
}
else
{
    Initializing();
}
 
 delay(300);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ติดต่อกับsd-card
void Initializing(void)
{
  if (!SD.begin(chipSelect))
  {
    sd_state=0;
    return;
  }
  else
  {
    sd_state=1;
  }
}

//แปลงเลขฐาน10เป็นฐาน2
byte d2b(byte val)
{
  return ( (val/10*16) + (val%10) );
}

//แปลงเลขฐาน2เป็นเลขฐาน10
byte b2d(byte val)
{
  return ( (val/16*10) + (val%16) );
}

//ฟังชั่นสำหรับร้องขอการติดต่อกับโมดูล i2c ของ RTC_DS_3231
void reques_function(byte hex)
{
  //ติดต่อกับโมดูล ผ่านการสื่อสารแบบ i2c จาก address 0x68 ของอุปกรณ์
  Wire.beginTransmission(ds3231address);
  //ตั้งค่า register address ของอุปกรณ์เพื่อรอการร้องขอข้อมูลจาก master(บอร์ด arduino uno)
  Wire.write(hex);
  //ยกเลิกการติดต่อเพื่อเปลี่ยน acknowlage
  Wire.endTransmission();
}

//ฟังชั่นรับข้อมูล sec min hour จากโมดูล rtc_ds3231
void get_time(void)
{
  //ติดต่อกับโมดูล rtc_ds3231 ผ่านการสื่อสารแบบ i2c จาก address 0x68 ของอุปกรณ์
  //ตั้งค่า register address ของอุปกรณ์เพื่อรอการร้องขอข้อมูลจาก master(บอร์ด arduino uno)
  //จบการติดต่อกับอุปกรณ์ปัจจุบัน
  //0ฐาน10=0x00ฐาน16
  reques_function(0);
  //ร้องขอข้อมูลจากอุปกรณ์slave(slave1 rtc_modlue_ds3231)ที่มีaddress0x68 โดยร้องขอข้อมูลจากregisterที่ตั้งไว้3byte
  Wire.requestFrom(ds3231address, 3);
  //รับค่าจากการ request data ทีละbyte แล้วแปลงไปเป็น เลขฐาน10แต่เก็บเก็ยไว้ในหน่วยbyte
  time_data.sec=b2d(Wire.read());
  time_data.min=b2d(Wire.read());
  time_data.hour=b2d(Wire.read());
}

//ฟังชั่นรับข้อมูล data month year จากโมดูล rtc_ds3231
void get_date(void)
{
  //ติดต่อกับโมดูล rtc_ds3231 ผ่านการสื่อสารแบบ i2c จาก address 0x68 ของอุปกรณ์
  //ตั้งค่า register address ของอุปกรณ์เพื่อรอการร้องขอข้อมูลจาก master(บอร์ด arduino uno)
  //จบการติดต่อกับอุปกรณ์ปัจจุบัน
  //4ฐาน10=0x04ฐาน16
  reques_function(4);
  //ร้องขอข้อมูลจากอุปกรณ์slave(slave1 rtc_modlue_ds3231)ที่มีaddress0x68 โดยร้องขอข้อมูลจากregisterที่ตั้งไว้3byte
  Wire.requestFrom(ds3231address, 3);
  //รับค่าจากการ request data ทีละbyte แล้วแปลงไปเป็น เลขฐาน10แต่เก็บเก็ยไว้ในหน่วยbyte
  day_data.date_of_month=b2d(Wire.read());
  day_data.month_of_year=b2d(Wire.read());
  day_data.year_of_all=b2d(Wire.read());
}
//รับค่าอุณหภูมิจากโมดูลเวลา
void get_temperature(void)
{
  //ติดต่อกับโมดูล rtc_ds3231 ผ่านการสื่อสารแบบ i2c จาก address 0x68 ของอุปกรณ์
  //ตั้งค่า register address ของอุปกรณ์เพื่อรอการร้องขอข้อมูลจาก master(บอร์ด arduino uno)
  //จบการติดต่อกับอุปกรณ์ปัจจุบัน
  //17ฐาน10=0x11ฐาน16 รีจิสเตอร์อุญหภูมิ
  reques_function(17);
  //ร้องขอข้อมูลจากอุปกรณ์slave(slave1 rtc_modlue_ds3231)ที่มีaddress0x68 โดยร้องขอข้อมูลจากregisterที่ตั้งไว้3byte  
  //ในที่นี้ โมดูล rtc_ds3231 มีเซนเซอร์วัดอุณหภูมิด้วยในตัวของมันเองมีการเปลี่ยนแปลงค่าทุกๆ 64 วินาที 
  //และมีเลขหลักทศนิยมให้ คือ 0 25 50 75 เพราะมีlogicเป็นเลขฐาน2 อยู่ ตั้งแต่ 00 01 10 11
  Wire.requestFrom(ds3231address,2);
  //รับค่าส่วนจำนวนเต็มไว้ก่อน
  ultrasonic_data.temp = Wire.read();
  //รับค่าส่วนทศนิยมมาคำนวณและรวมกัน
  //ที่ต้องการชิปไป6bitเพื่อเลื่อนตำแหน่งของปิด เพราะ ตัวเลขอยู่bitที่ 7 กับ 6 1100000 แต่เราต้องการ 0000011
  ultrasonic_data.temp += (0.25*float(Wire.read()>>6));
}
//รับค่าจากโมดูลultrasonic
void get_ultrasonic(void)
{
  long duration;
  float v;
  //สร้างพัลส่งไปให้ultrasonicเพื่อส่งคลื่นสะท้านไปด้านหน้าของวัตกุ
  digitalWrite(pingPin, LOW);
  delayMicroseconds(3);
  digitalWrite(pingPin, HIGH);
  delayMicroseconds(11);
  digitalWrite(pingPin, LOW);
  //รับค่าจากultrasonicโดยการอ่านค่าpulseที่รับกลับเข้ามา
  duration = pulseIn(inPin, HIGH);
  //รับค่าจาก เรียกใช้ฟังชั่น get_temperature เพื่อเช็คการปรับเปลี่ยนอุณหภูมิของเซนเซอร์
  get_temperature();
  //นำค่าที่ได้มาทำการคำนวณอัตตราความเร็วของเสียงในอากาศเนื่องจากเสียงมีความเร็วมากน้อยไม่เท่ากันตามอุณหภูมิของพื้นที่
  //จึงต้องใช้สูตรเข้ามาช่วย คือ 331.5+(0.61*อุณหภูมิในอากาศ)
  v=1/(((331.5+(0.61*ultrasonic_data.temp))*100)/pow(10,6));
  //เมื่อหาค่ามาได้จะได้เป็นค่าเฟสต่อระยะทางในอากาศ แล้วนำมาหาค่าระยะเวลาที่ได้ แล้วหาร2อีกทีเพื่อหักลบค่าไปกลับของสัญญาณ
  ultrasonic_data.cm=duration/v/2;
  //แปลงเป็นนิ้ว
  ultrasonic_data.in= ultrasonic_data.cm*0.39370;
  //แปลงเป็นเมตรเนื้องจาก V ของเราสูตรเราได้ทำเป็น CM อย่างเดียวจึงต้องนำมา/100ใหม่
  ultrasonic_data.m=ultrasonic_data.cm/100;
}
//ฟังชั่นตรวจสอบตัวเลขหลักหน่วยหลัก10สำหรับแสดงผลของเวลา
void digit_rtc_ds3231(byte dec)
{
  //เช็คค่าตัวเลขว่ามีค่าน้อยกว่า10หรือไม่ถ้าน้อยกว่าให้ปริ้น 0 ออกมา แล้วจึงปริ้นค่าของตัวเลขอื่นๆตาม ในที่นี้ให้แสดงผลเป็นเลขฐาน10 จาก DEC
  if(dec<10)
  lcd.print("0");
  lcd.print(dec,DEC);
}
//ฟังชั่นตรวจสอบตัวเลขระยะทางของโมดูลultrasonicว่าเป็นหลักใดสำหรับแสดงผล
void digit_ultrasonic(byte dec)
{
  //เช็คค่าตัวเลขว่ามีค่าน้อยกว่า10หรือไม่ถ้าน้อยกว่าให้ปริ้น 00 ออกมา แล้วจึงปริ้นค่าของตัวเลขอื่นๆตาม ในที่นี้ให้แสดงผลเป็นเลขฐาน10 จาก DEC
  if(dec<10){
  lcd.print("00");
  }
  else{
    //เช็คค่าตัวเลขว่ามีค่ามากกว่า10และน้อยกว่า100 หรือไม่ถ้าน้อยกว่าให้ปริ้น 0 ออกมา แล้วจึงปริ้นค่าของตัวเลขอื่นๆตาม ในที่นี้ให้แสดงผลเป็นเลขฐาน10 จาก DEC
    if(dec>=10&&dec<100){
    lcd.print("0");
    }
    else{
      //เช็คค่าตัวเลขว่ามีค่ามากกว่า100แบะน้อยกว่า450 หรือไม่ถ้าน้อยกว่าให้ปริ้น ปริ้นตัวเลขออกมาเลย เพราะฉะนั้นใน if นี้จึงไม่มีการแสดงค่าใดๆเพิ่ม ในที่นี้ให้แสดงผลเป็นเลขฐาน10 จาก DEC
      if(dec>=100&&dec<=450){
        //ไมจำเป็น :)
      }
      else{
        lcd.print("error");
      }
    }
  }
   lcd.print(dec,DEC);
}
//แสดงผลเวลา
void print_time(void)
{
  //เรียกใช้functionge get_time เพื่อทำให้เวลาที่รับเข้ามาตรงตาม module rtc_ds_3231 มากที่สุด
  get_time();
  //เซ็ตตำแหน่งบนจอ lcd โดยเซตที่ตำแหน่ง แถวที่ 1(เท่ากับแถวที่2บนหน้าจอเพราะนับจาก 0 1) และเริ่มที่ characterที่0
  lcd.setCursor(0,0);
  //นำค่าเวลาที่ได้ไปเช็คจำนวนตัวเลขเพื่อที่จะปริ้นบนหน้าจอได้อย่างถูกต้องตามที่ออกแบบ
  digit_rtc_ds3231(time_data.hour);
  lcd.print(":");
  digit_rtc_ds3231(time_data.min);
  lcd.print(":");
  digit_rtc_ds3231(time_data.sec);
}
//แสดงผลวัน
void print_date(void)
{
  //เรียกใช้functionge get_date เพื่อทำให้วันที่รับเข้ามาตรงตาม module rtc_ds_3231 มากที่สุด
  get_date();
  //เซ็ตตำแหน่งบนจอ lcd โดยเซตที่ตำแหน่ง แถวที่ 0(เท่ากับแถวที่1บนหน้าจอเพราะนับจาก 0 1) และเริ่มที่ characterที่0
  lcd.setCursor(0,0);
  //นำค่าเวลาที่ได้ไปเช็คจำนวนตัวเลขเพื่อที่จะปริ้นบนหน้าจอได้อย่างถูกต้องตามที่ออกแบบ
  digit_rtc_ds3231(day_data.date_of_month);
  lcd.print(":");
  digit_rtc_ds3231(day_data.month_of_year);
  lcd.print(":");
  digit_rtc_ds3231(day_data.year_of_all);
}
//แสดงผลอุณหภูมิ
void print_temp(void)
{
  //เรียกใช้functionge get_temperature เพื่อทำให้อุณหภูมิที่รับเข้ามาตรงตาม module rtc_ds_3231 มากที่สุด
  get_temperature();
  lcd.setCursor(9,1);
  lcd.print(ultrasonic_data.temp);
  lcd.print(":C");
}
//แสดงผลระยะทางultrasonic
void print_ultrasonic(void)
{
  //เรียกใช้functiongetimeเพื่อทำให้วันที่รับเข้ามาตรงตาม module rtc_ds_3231 มากที่สุด
  get_ultrasonic();
  lcd.setCursor(9,0);
  digit_ultrasonic(ultrasonic_data.cm);
  lcd.print(":CM ");
}
//ฟังชั่นรีเซ็ต เฉพาะเวลา
void reset_time(void)
{
  Wire.beginTransmission(ds3231address);
  Wire.write(0x00);
  Wire.write(d2b(0));
  Wire.write(d2b(0));
  Wire.write(d2b(0));
  Wire.endTransmission();
}
//ฟังชั่นรของโมดูล เฉพาะวัน
void reset_date(void)
{
  Wire.beginTransmission(ds3231address);
  Wire.write(0x04); 
  Wire.write(d2b(0));
  Wire.write(d2b(0));
  Wire.write(d2b(0));
  Wire.endTransmission();
}
//ฟังชั่นตั้งค่าโมดูล เฉพาะวันเวลา
void set_time(void)
{
  unsigned int set=1;
  unsigned int cout=0;
  lcd.clear();
  print_time();
  while(set==1)
  {
    
    if(digitalRead(left)==HIGH)
      cout-=1;
      delay(50);
    if(digitalRead(rigth)==HIGH)
      cout+=1;
      delay(50);
      
    if(cout>5)
      cout=0;
    Serial.println(cout);
    Serial.println(time_data.hour);
    if(cout==0)
    {
      if(digitalRead(up)==HIGH)
        time_data.hour+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.hour-=10;
        delay(50);
      if(time_data.hour>23)
        time_data.hour=0;
      lcd.setCursor(0,0);
      digit_rtc_ds3231(time_data.hour);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(0,0);
      lcd.blink();
    }
    if(cout==1)
    {
      if(digitalRead(up)==HIGH)
        time_data.hour+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.hour-=1;
        delay(50);
      if(time_data.hour>23)
        time_data.hour=0;
        delay(50);
      lcd.setCursor(0,0);
      digit_rtc_ds3231(time_data.hour);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(1,0);
      lcd.blink();
    }
    if(cout==2)
    {
      if(digitalRead(up)==HIGH)
        time_data.min+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.min-=10;
        delay(50);
      if(time_data.min>59)
        time_data.min=0;
        delay(50);
      lcd.setCursor(3,0);
      digit_rtc_ds3231(time_data.min);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(3,0);
      lcd.blink();
    }
    if(cout==3)
    {
      if(digitalRead(up)==HIGH)
        time_data.min+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.min-=1;
        delay(50);
      if(time_data.min>59)
        time_data.min=0;
        delay(50);
      lcd.setCursor(3,0);
      digit_rtc_ds3231(time_data.min);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(4,0);
      lcd.blink();
    }
    if(cout==4)
    {
      if(digitalRead(up)==HIGH)
        time_data.sec+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.sec-=10;
        delay(50);
      if(time_data.sec>59)
        time_data.sec=0;
        delay(50);
      lcd.setCursor(6,0);
      digit_rtc_ds3231(time_data.sec);
      lcd.noBlink();
      lcd.setCursor(6,0);
      lcd.blink();
    }
    if(cout==5)
    {
      if(digitalRead(up)==HIGH)
        time_data.sec+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        time_data.sec-=1;
      if(time_data.sec>59)
        time_data.sec=0;
        delay(50);
      lcd.setCursor(6,0);
      digit_rtc_ds3231(time_data.sec);
      lcd.noBlink();
      lcd.setCursor(7,0);
      lcd.blink();
    }
    if(digitalRead(option_pin)==HIGH)
      set=0;
  }
  Wire.beginTransmission(ds3231address);
  Wire.write(0x00);
  Wire.write(d2b(time_data.sec));
  Wire.write(d2b(time_data.min));
  Wire.write(d2b(time_data.hour));
  Wire.endTransmission();
  lcd.noBlink();
}
//ฟังชั่นรของโมดูล เฉพาวัน
void set_date(void)
{
  unsigned int set=1;
  unsigned int cout=0;
  lcd.clear();
  print_date();
  while(set==1)
  {
    
    if(digitalRead(left)==HIGH)
      cout-=1;
      delay(50);
    if(digitalRead(rigth)==HIGH)
      cout+=1;
      delay(50);
      
    if(cout>5)
      cout=0;
    Serial.println(cout);
    Serial.println(time_data.hour);
    if(cout==0)
    {
      if(digitalRead(up)==HIGH)
        day_data.date_of_month+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.date_of_month-=10;
        delay(50);
      if(day_data.date_of_month>31)
        day_data.date_of_month=0;
      lcd.setCursor(0,0);
      digit_rtc_ds3231(day_data.date_of_month);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(0,0);
      lcd.blink();
    }
    if(cout==1)
    {
      if(digitalRead(up)==HIGH)
        day_data.date_of_month+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.date_of_month-=1;
        delay(50);
      if(day_data.date_of_month>23)
        day_data.date_of_month=0;
        delay(50);
      lcd.setCursor(0,0);
      digit_rtc_ds3231(day_data.date_of_month);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(1,0);
      lcd.blink();
    }
    if(cout==2)
    {
      if(digitalRead(up)==HIGH)
        day_data.month_of_year+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.month_of_year-=10;
        delay(50);
      if(day_data.month_of_year>12)
        day_data.month_of_year=0;
        delay(50);
      lcd.setCursor(3,0);
      digit_rtc_ds3231(day_data.month_of_year);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(3,0);
      lcd.blink();
    }
    if(cout==3)
    {
      if(digitalRead(up)==HIGH)
        day_data.month_of_year+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.month_of_year-=1;
        delay(50);
      if(day_data.month_of_year>12)
        day_data.month_of_year=0;
        delay(50);
      lcd.setCursor(3,0);
      digit_rtc_ds3231(day_data.month_of_year);
      lcd.print(":");
      lcd.noBlink();
      lcd.setCursor(4,0);
      lcd.blink();
    }
    if(cout==4)
    {
      if(digitalRead(up)==HIGH)
        day_data.year_of_all+=10;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.year_of_all-=10;
        delay(50);
      if(day_data.year_of_all>99)
        day_data.year_of_all=0;
        delay(50);
      lcd.setCursor(6,0);
      digit_rtc_ds3231(day_data.year_of_all);
      lcd.noBlink();
      lcd.setCursor(6,0);
      lcd.blink();
    }
    if(cout==5)
    {
      if(digitalRead(up)==HIGH)
        day_data.year_of_all+=1;
        delay(50);
      if(digitalRead(down)==HIGH)
        day_data.year_of_all-=1;
      if(day_data.year_of_all>99)
        day_data.year_of_all=0;
        delay(50);
      lcd.setCursor(6,0);
      digit_rtc_ds3231(day_data.year_of_all);
      lcd.noBlink();
      lcd.setCursor(7,0);
      lcd.blink();
    }
    if(digitalRead(option_pin)==HIGH)
      set=0;
  }
  Wire.beginTransmission(ds3231address);
  Wire.write(0x04); 
  Wire.write(d2b(day_data.date_of_month));
  Wire.write(d2b(day_data.month_of_year));
  Wire.write(d2b(day_data.year_of_all));
  Wire.endTransmission();
  lcd.noBlink();
}
//ฟังชั่นสำหรับสั่งการให้Processorสั่งเขียนข้อมูลลงSD-CardจากLibary SD.h
void write_sd(String water_lavel)
{
   String nameString = String(day_data.date_of_month);
          nameString +="_";
          nameString +=String(day_data.month_of_year);
          nameString +="_";
          nameString +=String(day_data.year_of_all);    
          nameString +=".csv";    
    File dataFile = SD.open(nameString, FILE_WRITE);
    if(dataFile)
    {
      String dataString = "Date:";
      dataString +=String(day_data.month_of_year);
      dataString +="/";
      dataString +=String(day_data.month_of_year);
      dataString +="/";
      dataString +=String(day_data.year_of_all);
      dataString +=",";
      dataString += "Time:";
      dataString +=String(time_data.hour);
      dataString +=":";
      dataString +=String(time_data.min);
      dataString +=":";
      dataString +=String(time_data.sec);
      dataString +=",";
      dataString +="Temparature:";
      dataString +=String(ultrasonic_data.temp);
      dataString +=",";
      dataString +="WaterLevel:";
      dataString +=String(ultrasonic_data.cm);
      dataString +=",";
      dataString +="StatusWater:";
      dataString +=water_lavel;
      dataFile.println(dataString);
      dataFile.close();
      SD.exists(nameString);
      Serial.println(dataString);
    }
    else 
    {
      dataFile.close();
      SD.exists(nameString);
      Initializing();
    }
}
//ฟังชั่นแสดงสถานะการเชื่อมต่อของSD-CARDบนLCD
void sd_print(void)
{
  lcd.setCursor(0,1);
  lcd.print("SD:");
  if(sd_state==0)
  {
    lcd.setCursor(4,1);
    lcd.print("OFF");
  }
  else
  {
    lcd.setCursor(4,1);
    lcd.print("ON ");
  }
}
//ฟังชั่นสำหรับแสดงผลข้อความinter faceในการตั้งค่าโมดูลเวลา
void setting_time(void)
{
  lcd.clear();
  int set=1;
  while(set==1)
  {
    if(digitalRead(up)==HIGH)
    {
      op_sel+=1;
    }
    delay(50);
    if(digitalRead(down)==HIGH)
    {
      op_sel-=1;
    }
    delay(50);
    if(op_sel>6)
      op_sel=1;
    if(op_sel==1)
    {
      lcd.setCursor(0,0);
      lcd.print("*SET DATE ");
      lcd.setCursor(0,1);
      lcd.print(" SET TIME ");
      if(digitalRead(left)==HIGH)
      {
        set_date();
        set=0;
      }
    }
    if(op_sel==2)
    {
      lcd.setCursor(0,0);
      lcd.print(" SET DATE  ");
      lcd.setCursor(0,1);
      lcd.print("*SET TIME  ");
      if(digitalRead(left)==HIGH)
      {
        set_time();
        set=0;
      }
    }
    if(op_sel==3)
    {
      lcd.setCursor(0,0);
      lcd.print("*RESET TIME");
      lcd.setCursor(0,1);
      lcd.print(" RESET DATE");
      if(digitalRead(left)==HIGH)
      {
        reset_time();
        set=0;
      }
    }
    if(op_sel==4)
    {
      lcd.setCursor(0,0);
      lcd.print(" RESET TIME");
      lcd.setCursor(0,1);
      lcd.print("*RESET DATE");
      if(digitalRead(left)==HIGH)
      {
        reset_date();
        set=0;
      }
    }
    if(op_sel==5)
    {
      lcd.setCursor(0,0);
      lcd.print("*RESET ALL ");
      lcd.setCursor(0,1);
      lcd.print(" EXIT      ");
      if(digitalRead(left)==HIGH)
      {
        reset_time();
        reset_date();
        set=0;
      }
    }
    if(op_sel==6)
    {
      lcd.setCursor(0,0);
      lcd.print(" RESET ALL ");
      lcd.setCursor(0,1);
      lcd.print("*EXIT      ");
      if(digitalRead(left)==HIGH)
      {
        set=0;
      }
    } 
    delay(250);
  }
  lcd.clear();
}

