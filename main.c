/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.3 Standard
Automatic Program Generator
© Copyright 1998-2011 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 6/30/2013
Author  : Ardika
Company : CrowjaEmbedder
Comments: 


Chip type               : ATmega32
Program type            : Application
AVR Core Clock frequency: 16.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*****************************************************/

#include <mega32.h>
#include <alcd.h>
#include <delay.h>

#define ADC_VREF_TYPE 0x60

// definisi tombol-tombol
#define CMD_UP      PINC.4
#define CMD_DOWN    PINC.5
#define CMD_OK      PINC.6
#define CMD_CANCEL  PINC.7

// Detektor persimpangan jalan
#define RIGHT_WING  PIND.0
#define LEFT_WING   PIND.1

// definisi kendali motor
#define RIGHT_PWM   OCR1AL
#define LEFT_PWM    OCR1BL
#define RIGHT_DR1   PORTD.6
#define RIGHT_DR2   PORTD.7
#define LEFT_DR1    PORTD.2
#define LEFT_DR2    PORTD.3 

// definisi custom character LCD
#define FULL_BLOCK  0
#define EMPTY_BLOCK 1


flash unsigned char fullBlock[8] = {0b11111,
                                    0b11111,
                                    0b11111,
                                    0b11111,
                                    0b11111,
                                    0b11111,
                                    0b11111,
                                    0b11111};   
                                    
flash unsigned char emptyBlock[8] = {0b11111,
                                     0b10001,
                                     0b10001,
                                     0b10001,
                                     0b10001,
                                     0b10001,
                                     0b10001,
                                     0b11111};

// Variabel-variabel kontrol yang tersimpan di memory non-volatile
eeprom unsigned char eeSpeed = 255;
eeprom unsigned char eeKp = 0;
eeprom unsigned char eeKd = 0;
eeprom unsigned char eeKi = 0;

// Varibel kepekaan sensor dalam memory non-volaitile
eeprom unsigned char eeWhite[8] = {0}, eeBlack[8] = {0};

// Varibael-varibel kontrol yang disimpan di memory volatile untuk perhitungan kontrol
unsigned char speed, kp, kd, ki, error, sp;

// Variabel kepekaan sensor dalam memory volatile untuk perhitungan
unsigned char white[8] = {0}, black[8] = {0};

// Varibel penyimpan nilai sensor biner, dimana tiap satu sensor nilainya diwakili oleh 1-bit
// yang merupakan hasil perbandingan pembacaan nilai analog sensor dengan nilai kepekaan sensor
unsigned char sensor = 0;

//prototype fungsi
void define_char(unsigned char flash *pc,unsigned char char_code);
unsigned char read_adc(unsigned char adc_input);
void scanLine();
void loadVariables();
void saveVariables();
void lcdOn(unsigned char on);
void lcdOnWing();
void go();
void back();
void left();
void right();
void stop(unsigned char usingPowerBrake);
void lcdPutsByte(unsigned char value);
void printADCSensor();

void main(void)
{

PORTA=0x00;
DDRA=0x00;

PORTB=0xFF;
DDRB=0xFF;
 
PORTC=0x00;
DDRC=0x00;

PORTD=0x00;
DDRD=0xFC;

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 2000.000 kHz
// Mode: Fast PWM top=0xFF
// OC0 output: Disconnected
TCCR0=0x4A;
TCNT0=0x00;
OCR0=0x0F;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 250.000 kHz
// Mode: Fast PWM top=0x00FF
// OC1A output: Non-Inv.
// OC1B output: Non-Inv.
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer1 Overflow Interrupt: Off
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=0xA1;
TCCR1B=0x0B;
TCNT1H=0x00;
TCNT1L=0x00;
ICR1H=0x00;
ICR1L=0x00;
OCR1AH=0x00;
OCR1AL=0x00;
OCR1BH=0x00;
OCR1BL=0x00;

// Timer/Counter 2 initialization
// Clock source: System Clock
// Clock value: Timer2 Stopped
// Mode: Normal top=0xFF
// OC2 output: Disconnected
ASSR=0x00;
TCCR2=0x00;
TCNT2=0x00;
OCR2=0x00;

// External Interrupt(s) initialization
// INT0: Off
// INT1: Off
// INT2: Off
MCUCR=0x00;
MCUCSR=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=0x00;

// USART initialization
// USART disabled
UCSRB=0x00;

// Analog Comparator initialization
// Analog Comparator: Off
// Analog Comparator Input Capture by Timer/Counter 1: Off
ACSR=0x80;
SFIOR=0x00;

// ADC initialization
// ADC Clock frequency: 125.000 kHz
// ADC Voltage Reference: AVCC pin
// Only the 8 most significant bits of
// the AD conversion result are used
ADMUX=ADC_VREF_TYPE & 0xff;
ADCSRA=0x87;

// SPI initialization
// SPI disabled
SPCR=0x00;

// TWI initialization
// TWI disabled
TWCR=0x00;


// Alphanumeric LCD initialization
// Connections are specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTB Bit 0
// RD - PORTB Bit 1
// EN - PORTB Bit 2
// D4 - PORTB Bit 4
// D5 - PORTB Bit 5
// D6 - PORTB Bit 6
// D7 - PORTB Bit 7
// Characters/line: 16
lcd_init(16);   
lcd_clear();
define_char(fullBlock,FULL_BLOCK);
define_char(emptyBlock,EMPTY_BLOCK);
lcdOn(1);

    while (1) {  
          
    }
}


/* function used to define user characters */
void define_char(unsigned char flash *pc,unsigned char char_code)
{
    unsigned char i,a;
    a=(char_code<<3) | 0x40;
    for (i=0; i<8; i++) lcd_write_byte(a++,*pc++);
}


// Read the 8 most significant bits
// of the AD conversion result
unsigned char read_adc(unsigned char adc_input)
{
    ADMUX=adc_input | (ADC_VREF_TYPE & 0xff);
    // Delay needed for the stabilization of the ADC input voltage
    //delay_us(10);
    // Start the AD conversion
    ADCSRA|=0x40;
    // Wait for the AD conversion to complete
    while (!(ADCSRA & 0x10));
        ADCSRA |= 0x10;
    return ADCH;
}


void scanLine()
{
    unsigned char i = 0;      
    unsigned char adcRead[i];  // Variabel pembacaan nilai ADC          
    // JUmlah warna putih dan hitam yang terdeteksi oleh sensor
    unsigned char blackCount = 0, whiteCount = 0;   
    
    sensor = 0x00;   // Hapus nilai sensor sebelumnya
    
    for (; i<8; i++) {     
        adcRead[i] = read_adc(i);  // Baca nilai ADC ada bit ke-i
        if (adcRead[i] > white)  // Jika hasil pembacaan > nilai putih
            blackCount++;       // Increment jumlah blok hitam yang terbaca
        else 
            whiteCount++;      // Increment jumlah blok putih yang terbaca
    }                   
    if (whiteCount > blackCount) {  // Banyaknya blok warna putih yang terdeteksi > dari blok warna hitam, maka garis nya adalah hitam
        for (i=0; i<8; i++) {
            if (adcRead[i] > white)
                sensor |= (1<<i);
        }                                  
    }
    else { // Banyaknya blok warna putih yang terdeteksi < dari blok warna hitam, maka garis nya adalah putih
        for (i=0; i<8; i++) {
            if (adcRead[i] < black)
                sensor |= (1<<i);
        }
    } 
}

void loadVariables()
{
    unsigned char i = 0;
    
    speed = eeSpeed;
    kp = eeKp;
    kd = eeKd;
    ki = eeKi;
    
    for (; i<8; i++) {
        white[i] = eeWhite[i];
        black[i] = eeBlack[i];    
    }    
}

void saveVariables()
{
    unsigned char i = 0;
    
    eeSpeed = speed;
    eeKp = kp;
    eeKd = kd;
    eeKi = ki;
    
    for (; i<8; i++) {
        eeWhite[i] = white[i];
        eeBlack[i] = black[i];
    }
}


void lcdOn(unsigned char on)
{
    PORTB.3 = on;
}

void lcdOnWing()
{
    PORTB.3 = !((LEFT_WING) | (RIGHT_WING));
}

void go()
{
    RIGHT_DR1 = 0; RIGHT_DR2 = 1;
    LEFT_DR1 = 0; LEFT_DR2 = 1; 
}

void back()
{
    RIGHT_DR1 = 1; RIGHT_DR2 = 0;
    LEFT_DR1 = 1; LEFT_DR2 = 0;        
}

void left()
{
    RIGHT_DR1 = 0; RIGHT_DR2 = 1;
    LEFT_DR1 = 0; LEFT_DR2 = 0;
}

void right()
{
    RIGHT_DR1 = 0; RIGHT_DR2 = 0;
    LEFT_DR1 = 0; LEFT_DR2 = 1;
}

void stop(unsigned char usingPowerBrake)
{
    RIGHT_DR1 = RIGHT_DR2 = LEFT_DR1 = LEFT_DR2 = 0;
    if (usingPowerBrake) {
        back();
        LEFT_PWM = RIGHT_PWM = 255;
        delay_ms(100);
        LEFT_PWM = RIGHT_PWM = 0;
    }     
    
}



void lcdPutsByte(unsigned char value)
{
    unsigned char ten = (value % 100) / 10;
    lcd_putchar('0' + (value / 100));  
    lcd_putchar('0' + ten);
    lcd_putchar('0' + (value % 10));
}

void printADCSensor()
{
    lcd_gotoxy(0,0); lcdPutsByte(read_adc(0));
    lcd_gotoxy(4,0); lcdPutsByte(read_adc(1));
    lcd_gotoxy(8,0); lcdPutsByte(read_adc(2));
    lcd_gotoxy(12,0); lcdPutsByte(read_adc(3));
    lcd_gotoxy(0,1); lcdPutsByte(read_adc(4));
    lcd_gotoxy(4,1); lcdPutsByte(read_adc(5));
    lcd_gotoxy(8,1); lcdPutsByte(read_adc(6));
    lcd_gotoxy(12,1); lcdPutsByte(read_adc(7)); 
}
