// Attiny HT1621 GPS speed
// Davide Barchetta 2024

// ATMEL ATTINY 25/45/85 / ARDUINO
//
//                  +-\/-+
// Ain0 (D 5) PB5  1|    |8  Vcc
// BTN        PB3  2|    |7  PB2 (D 2) CS
// GPS RX     PB4  3|    |6  PB1 (D 1) WR
//            GND  4|    |5  PB0 (D 0) DATA
//                  +----+

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

//#include <avr/sleep.h> //Needed for sleep_mode
//#include <avr/power.h>    // Power management

#define DATA 7	//Pin 7 as Serial data output
#define WR	 6  //Pin 12 as read clock	output
#define CS	 5  //Pin 13 as chip selection output

//#define SWITCH 5 // pin 3 / PCINT4
//#define LED 4 // pin 3 / PCINT4

#define RXPin	 3  //Pin 13 as chip selection output
#define TXPin	 4  //Pin 12 as read clock	output
#define GPSBaud 9600	//Pin 7 as Serial data output

TinyGPSPlus gps;

SoftwareSerial ss(RXPin, TXPin);

#define CS1    digitalWrite(CS, HIGH)
#define CS0    digitalWrite(CS, LOW)
#define WR1    digitalWrite(WR, HIGH)
#define WR0    digitalWrite(WR, LOW)
#define DATA1  digitalWrite(DATA, HIGH)
#define DATA0  digitalWrite(DATA, LOW)

#define sbi(x, y)  (x |= (1 << y))  
#define cbi(x, y)  (x &= ~(1 <<y ))
#define uchar   unsigned char
#define uint   unsigned int

#define  ComMode    0x52  //4COM,1/3bias  1000    010 1001  0
#define  RCosc      0x30  //1000 0011 0000
#define  LCD_on     0x06  //1000 0000 0110
#define  LCD_off    0x04
#define  Sys_en     0x02  //1000 0000 0010
#define  CTRl_cmd   0x80
#define  Data_cmd   0xa0

//                0     1     2     3     4     5     6     7     8     9
const char num[]={0x3f, 0x09, 0x67, 0x4f, 0x59, 0x5e, 0x7e, 0x0b, 0x7f, 0x5f};
//                   0     1     2     3     4     5     6     7     8     9
const char num_r[]={0x3f, 0x30, 0x67, 0x76, 0x78, 0x5e, 0x5f, 0x34, 0x7f, 0x7e};

char wait[] = {0x10,0x02,0x01,0x08,0x04,0x20};

void SendBit_1621(uchar sdata,uchar cnt)
{
	//data cnt HT1621
	uchar i;
	for(i=0;i<cnt;i++)
	{
		WR0;
		if(sdata&0x80) DATA1;
		else DATA0;
		WR1;
		sdata<<=1;
	}
}

void SendCmd_1621(uchar command)
{
	CS0;
	SendBit_1621(0x80,4);
	SendBit_1621(command,8);
	CS1;
}

void Write_1621(uchar addr,uchar sdata)
{
	addr<<=2;
	CS0;
	SendBit_1621(0xa0,3);     
	SendBit_1621(addr,6);    
	SendBit_1621(sdata,8);
	CS1;
}

void HT1621_all_off(uchar num)
{
	uchar i;
	uchar addr=0;
	for(i=0;i<num;i++)
	{
		Write_1621(addr,0x00);
		addr+=2;
	}
}

void Init_1621(void)
{
	SendCmd_1621(Sys_en);
	SendCmd_1621(RCosc);
	SendCmd_1621(ComMode);
	SendCmd_1621(LCD_on);
}

void draw_n(int n){
  if(n >= 0 && n < 10)
    draw(0,0,num[n],0);
  else if(n >= 10 && n < 100)
    draw(0,num[(n /10 % 10)],num[(n% 10)],0);
  else if(n >= 100 && n < 1000)
    draw(num[(n / 100 % 10)],num[(n /10 % 10)],num[(n% 10)],0);
  else
    draw(num[(n / 1000 % 10)],num[(n / 100 % 10)],num[(n /10 % 10)],num[(n% 10)]);
}

void draw_r(int n){
  draw(num_r[(n % 10)],num_r[(n / 10 % 10)],num_r[(n /100 % 10)],num_r[(n /1000 % 10)]);
}

void draw(uint8_t dig1, uint8_t dig2, uint8_t dig3, uint8_t dig4){

  // --2--
  //5     1
  //|--7--|
  //6     4
  // --3--
  // numero 5 = B01011110 = 0x54 = 94

  uint8_t raw6 = ((bitRead(dig1, 6) * 16 *8 ) + (bitRead(dig2, 6) * 16*4 ) +
  (bitRead(dig3, 6) * 16*2) + (bitRead(dig4, 6) * 16) +
  bitRead(dig1, 7)*8 + bitRead(dig2, 7)*4 + bitRead(dig3, 7)*2 + bitRead(dig4, 7));

  uint8_t raw4 = ((bitRead(dig1, 5) * 16 *8) + (bitRead(dig2, 5) * 16*4 ) +
  (bitRead(dig3, 5) * 16*2) + (bitRead(dig4, 5) * 16) +
  bitRead(dig1, 4)*8 + bitRead(dig2, 4)*4 + bitRead(dig3, 4)*2 + bitRead(dig4, 4));

  uint8_t raw2 = ((bitRead(dig1, 3) * 16 *8) + (bitRead(dig2, 3) * 16*4 ) +
  (bitRead(dig3, 3) * 16*2) + (bitRead(dig4, 3) * 16) +
  bitRead(dig1, 2)*8 + bitRead(dig2, 2)*4 + bitRead(dig3, 2)*2 + bitRead(dig4, 2));

  uint8_t raw0 = ((bitRead(dig1, 1) * 16 *8) + (bitRead(dig2, 1) * 16*4 ) +
  (bitRead(dig3, 1) * 16*2) + (bitRead(dig4, 1) * 16) +
  bitRead(dig1, 0) * 8 + bitRead(dig2, 0)* 4 + bitRead(dig3, 0)*2 + bitRead(dig4, 0));

  Write_1621(6,raw6);
  Write_1621(4,raw4);
  Write_1621(2,raw2);
  Write_1621(0,raw0);
}

void setup() {
  ss.begin(GPSBaud);

  pinMode(CS, OUTPUT);
  pinMode(WR, OUTPUT);
  pinMode(DATA, OUTPUT);

  //pinMode(LED, OUTPUT);
  //digitalWrite (LED, LOW);
  //pinMode (SWITCH, INPUT);
  //digitalWrite (SWITCH, HIGH);  // internal pull-up

  CS1;
  DATA1;
  WR1;
  Init_1621();
  //HT1621_all_off(16); //1621

  //draw(0,0x5D,0x53,0);    //ok reverse
  draw(0,0x6C,0x7A,0);    //oK
  //draw(0x53,0xE4,0xCC,0x53);      //^<:>^
  //draw(0,0xC0,0x40,0);      //-.-
  //draw(0,0xD3,0x53,0);      //o.o
  //draw(0,0xEE,0x6E,0);      // /o.o/
}

int i = 0;
//bool switchLed = true;

void loop() {
  while (ss.available() > 0)
    if (gps.encode(ss.read())){
      if(gps.speed.isValid()){
        draw_n(int(gps.speed.kmph()));
      }
      else{
        //draw(wait[i/3],0x5E,0x6D,0x1F);   //reverse
        draw(wait[i/3],0x3E,0x73,0x5E);
        i++;
        if(i == 18)
          i = 0;
      }
      //if(digitalRead(SWITCH) == LOW){
      //    switchLed = !switchLed;
      //    digitalWrite (LED, showCourse);
      //  }
    }
}