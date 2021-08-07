//*       OLED SSD 1331 Counter            *
// This code was written for Hi-Tech or XC8 compiler with MPLABX.
// PIC16F628A
// This code is free, for more info   http://www.moty22.co.uk       
   
#include <htc.h>
#include "oled_font.c"

#define _XTAL_FREQ 4000000
#define __delay_us(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000000.0)))
#define __delay_ms(x) _delay((unsigned long)((x)*(_XTAL_FREQ/4000.0)))
#define CS   RB4 // pin definition for Arduino UNO
#define DC   RB3
#define SDA  RB1
#define SCK  RB0
#define RES  RB2
// Color definitions
#define  black   0x0000
#define blue    0x001F
#define red     0xF800
#define green   0x07E0
#define cyan    0x07FF
#define magenta 0xF81F
#define yellow  0xFFE0  
#define white   0xFFFF
#define yf 5  
#define yp 35 
#define bgr white

#pragma config LVP=OFF, WDTE=OFF, MCLRE=OFF, FOSC=INTOSCIO, CP=OFF, CPD=OFF

//prototypes
void main(void);
void sspi(unsigned char data);   // send character over spi
void command(unsigned char cmd);
void send_data(unsigned char data);
void send_color(unsigned int color);
void oled_init(void);
void clear_scr(void);
void rectan(unsigned char c0,unsigned char c1, unsigned char r0,unsigned char r1, unsigned int color);
void draw(unsigned char x, unsigned char y, unsigned char c, unsigned int color, unsigned char size);


void main(void){
	unsigned long total;
	unsigned int freq2;
	unsigned char freq1,nz,d[7],i;	
	
	CMCON = 0b111;		//comparator off
	TRISB = 0b11100000;		 //RB 5,6,7 inputs
    TRISA = 0b11110000;
	//TRISA = 0;
    OPTION_REG = 0b10100000;	//TMR1 pre=1:2
    T1CON = 0b1110;        //TMR1 prescale 1:1, osc=on
    SDA=1;
    SCK=1;
    CS=1;
    RES=1;
	oled_init();
    rectan(0,95,0,31,white);  //background
    rectan(0,95,32,63,green);
    draw(80,23,11,blue,1);  //H
    draw(87,23,15,blue,1);  //z
    draw(87,53,12,red,1); //S
	
for(;;){
	freq1 = 0;
	freq2 = 0;
	TMR1L = 0;
	TMR1H = 0;
	TMR1IF = 0;
	nz=0;
	TMR1ON = 1;		// open gate
	TMR0 = 0;		// start count
	T0IF=0;
	
	while (!TMR1IF){		//loop for 2 second
		if (T0IF){++freq2; T0IF=0;}
	}
	
	freq1=TMR0;		//stop count=copy reg
	TMR1ON = 0;		//close gate 

	total=(unsigned long)freq1 + (unsigned long)freq2 * 256; //binary to decimal

    //convert binary to 7 decimal digits
    d[6]=total/1000000;		//1MHz digit
    d[5]=(total/100000) %10;	//100KHz digit
    d[4]=(total/10000) %10;
    d[3]=(total/1000) %10;
    d[2]=(total/100) %10;
    d[1]=(total/10) %10;		//10Hz digit	
    d[0]=total %10;
        //display digits 7 to 2
    for(i=6;i>0;i--){
        if(!d[i] && !nz){draw(80-i*12,yf,10,white,2);}	
        else{draw(80-i*12,yf,10,white,2); draw(80-i*12,yf,d[i],red,2); nz=1;}
     }	
    draw(80,yf,10,white,2) ;draw(80,yf,d[0],red,2);

    nz=0;
    rectan(80,85,53,61,green);
    if(total<1000){
        total=1000000/total; 
        draw(80,53,14,red,1);	//u
    }else{
        total=1000000000/total;
        draw(80,53,13,red,1);	//n
    }
    //convert binary to 7 decimal digits
    d[6]=total/1000000;		//
    d[5]=(total/100000) %10;
    d[4]=(total/10000) %10;
    d[3]=(total/1000) %10;
    d[2]=(total/100) %10;
    d[1]=(total/10) %10;		//	
    d[0]=total %10;
        //display digits 7 to 2			
    for(i=6;i>0;i--){
        if(!d[i] && !nz){draw(80-i*12,yp,10,green,2);}	
        else{draw(80-i*12,yp,10,green,2); draw(80-i*12,yp,d[i],black,2); nz=1;}
    }	
    draw(80,yp,10,green,2) ;draw(80,yp,d[0],black,2);
            
     __delay_ms(2000);  	       

}
		
}

void draw(unsigned char x, unsigned char y, unsigned char c, unsigned int color, unsigned char size) //character
{
	unsigned char i, j, line;
  for (i=0; i<6; i++ ) {
     if (i == 5) 
      line = 0x0;
    else 
      line = font[(c*5)+i];
    for (j = 0; j<8; j++) {
      if (line & 0x1) {
          rectan(x+(i*size), x+(i*size)+size, y+(j*size), y+(j*size)+size, color);
       }
      line >>= 1;
    }
  }
}

void clear_scr(void){
  rectan(0,95,0,63,white);
}

void rectan(unsigned char c0,unsigned char c1, unsigned char r0,unsigned char r1, unsigned int color)
{
  unsigned int i;
  command(0x15); // Column addr set
  command(c0);
  command(c1);
  command(0x75); // row addr set
  command(r0);
  command(r1);
  for(i=(c1 - c0 + 1) * (r1 - r0 + 1); i > 0; i--) {  
    send_color(color);     // color
  }
}

void sspi(unsigned char data){   // send character over software spi
	unsigned char b;
    
    SDA=1; SCK=1;
    for(b=0;b<8;b++){
    SCK=0;
    SDA=(data >> (7-b)) % 2;
    SCK=1;
	}
}	

void command(unsigned char cmd)
{
  DC=0; // Command Mode
  CS=0; // Select the chip (active low)
  sspi(cmd); // set up data on bus
  CS=1;; // Deselect chip (active low)
}

void send_data(unsigned char data)
{
  DC=1;    // data mode
  CS=0;   // chip selected
  sspi(data);  // set up data on bus
  CS=1;       // deselect chip
}

void send_color(unsigned int color)
{
    send_data(color>>8);
    send_data(color);
}

void oled_init(void)
{
  unsigned char i;
  RES=1;   //hardware reset
  __delay_ms(200);
  RES=0;
  __delay_ms(10);
  RES=1;
  __delay_ms(10);
  command(0xAE); //display off
  
  command(0xA0); //remap
  command(0x72); //RGB=0x72, BGR=0x76

  command(0xA1); // CMD STARTLINE
  command(0x0);
  command(0xA2); // CMD DISPLAYOFFSET
  command(0x0);
  command(0xA4); // CMD NORMALDISPLAY
  command(0xA8);  // CMD SETMULTIPLEX
  command(0x3F);                      // 0x3F 1/64 duty
  command(0xAD);     // CMD SETMASTER
  command(0x8E);
  command(0xB0); // CMD POWERMODE
  command(0x0B);
  command(0xB1); // CMD PRECHARGE
  command(0x31);
  command(0xB3); // CMD CLOCKDIV
  command(0xF0); // 7:4 = Oscillator Frequency, 3:0 = CLK Div Ratio
                     // (A[3:0]+1 = 1..16)

  command(0x8A); // CMD PRECHARGEA
  command(0x64);
  command(0x8B); // CMD PRECHARGEB
  command(0x78);
  command(0x8C); // CMD PRECHARGEC
  command(0x64);
  command(0xBB); // CMD PRECHARGELEVEL
  command(0x3A);
  command(0xBE); // CMD VCOMH
  command(0x3E);
  command(0x87); // CMD MASTERCURRENT
  command(0x14);

  command(0x81); // CMD CONTRASTA 91
  command(0xFF);
  command(0x82); // CMD CONTRASTB 50
  command(0xFF);
  command(0x83); // CMD CONTRASTC 7D
  command(0xFF);


//  command(0xA4); //Normal display on
  command(0xAF); //Main screen turn on
}