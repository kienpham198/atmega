/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
© Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : 
Version : 
Date    : 31/10/2021
Author  : 
Company : 
Comments: 


Chip type               : ATmega328P
Program type            : Application
AVR Core Clock frequency: 16,000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*******************************************************/

#include <mega328p.h>
#include "Link.h"     
#include <delay.h>

// I2C Bus functions
#include <i2c.h>

// DS1307 Real Time Clock functions
#include <ds1307.h>

// Alphanumeric LCD functions
#include <alcd.h>

// Declare your define here
#define UP 1
#define DOWN 2
#define RIGHT 3
#define LEFT 4
#define ENTER 5

#define NONGLANH PORTD.2
#define DEN PORTD.3
#define SUOI PORTD.4
#define CB_ChuyenDong PINB.0
#define analog 2    // chan do nhiet do
#define tempMAX 60

// =================global variable =========== Luu bien toan cuc
float NhietDo = 0;                                //luu nhiet do hien tai
float Set_temp = 25.00;                           //luu nhiet do cai dat
char NongLanh_Time_On2_gio =0;                    // gio bat nong lanh so 2
char NongLanh_Time_On2_phut = 0;                  // phut  bat nong lanh so 2
char NongLanh_Time_On1_gio =0;                    //  gio bat nong lanh so 1
char NongLanh_Time_On1_phut = 0;                  // phut  bat nong lanh so 1
char NongLanh_Off_gio =0;                         // gio tat nong lanh
char NongLanh_Off_phut = 0;                       // phut tat nong lanh
char star_gio=0,star_phut=0,star_giay=0;          //cai dat thoi gian bat dau chay cho DS1307
unsigned char Hour=0,Min=0,Sec=0;
char NongLanh_Auto_On = 1;                        // trang thai on off che do tu dong cua nong lanh
/*
     Off | On1 | On2 | On12
      1     0      0     0    =1                  // = 1 thi tat toan bo cac che do tu dong 
      0     1      0     0    =2                  // = 2 thoi gian ON1 hoat dong
      0     0      1     0    =4                  // = 4 thoi gian ON2 hoat dong
      0     0      0     1    =8                  // = 8 bat ca 2 thoi gian
*/


char NongLanh_Auto_Off =1;                        // kich hoat che do tu dong tat cua Nong Lanh
char NongLanh_trangthai=0;                        // trang thai hien tai cua Nong Lanh
char Suoi_trangthai    =0;                        // trang thai hien tai cua Den Suoi
char Den_trangthai     =0;                        // trang thai hien tai cua Den Dien
char Den_auto = 1;                                // = 1 => cho phep che do Auto cua Den
char Suoi_auto = 1;                               // = 1 => cho phep che do Auto cua Suoi
char Time_3s=0;                                   // bien chay timer 6s tang 1 don vi
char out = 0;                                     // chi co tac dung de thoat ra man hinh chinh
float voltage = 0;   // Bien luu dien ap chan analog
float old_vot = 0;
// ================= KHU VUC LUU TEXT ==========
flash unsigned char Text_Time1[]={" SETTING TIMER"};    // luu chuoi vao bo nho flash
flash unsigned char Text2[] = {"KICH HOAT AUTO"};
//unsigned char *p_Set_T = Text_Time1;
// ============== THE END =====================
// Timer1 overflow interrupt service routine


// Voltage Reference: AREF pin
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))   // khong quan tam

// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)           // doc du lieu cong ADC (analog)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}
interrupt [TIM1_OVF] void timer1_ovf_isr(void)          // ham timer 6s chay 1 lan
{
// Reinitialize Timer1 value
TCNT1H=0x48E5 >> 8;
TCNT1L=0x48E5 & 0xff;
// Place your code here
Time_3s++;
//voltage = read_adc(analog)* 5.0 / 1024.0;
}
//========================= ham chuc nang  =====
void Print(char *text,char x,char y)                     // In chuoi text vao toa do x y
{
     lcd_gotoxy(x,y);
     lcd_puts(text);
}
void Print_Int(int number,char x,char y)                  // in so nguyen dang INT vao toa do x y
{

    lcd_gotoxy(x,y);
    lcd_putchar((number/10)+48);
    lcd_putchar((number%10)+48);
}
void Print_float(float a)                          // in gia tri thap phan ra man hinh xy,ab
{
    int b = (a*100);                               // loai bo so dang sau dau ,
    int chuc,dv,sau1,sau2;
    chuc = b/1000;
    dv = (b-(chuc*1000))/100;
    sau1 = (b%100)/10;
    sau2 = b%10;
    lcd_putchar(chuc+48);
    lcd_putchar(dv+48);
    lcd_putchar(',');
    lcd_putchar(sau1+48);
    lcd_putchar(sau2+48);
}
void Read_NhietDo(int x,int y)         // Doc nhiet do chan Analog va hien thi ra vi tri x y
{
     float temp=0; 
     voltage = read_adc(analog)* 5.0 / 1024.0;
          temp = voltage * 100.0;
          NhietDo = temp;
          lcd_gotoxy(x,y);
          Print_float(temp);
          lcd_gotoxy(x+5,y);
          lcd_putchar(223);   // ki tu oC
          lcd_putchar('C');
          delay_ms(10);       // can thay bang timer 
    
}
//============== hien thi thoi gian tai toa do x y =============
void TimeDisplay(unsigned char hour,unsigned char min,unsigned char sec,int x,int y)
{
     Print_Int(hour,x,y);
     lcd_putchar(':');
     Print_Int(min,x+3,y);
     lcd_putchar(':');
     Print_Int(sec,x+6,y);
     
}
char Read_Key(char Button)                       // doc nut bam co tich hop ham while
{                                            // ham tra ve 1 neu Button bang x
    switch (Button){
    case UP:
         if(PINC.1==0)  {  while(PINC.1==0);  return 1; }   return 0;  break;
    case DOWN:
         if(PINC.0==0)  {  while(PINC.0==0);  return 1; }   return 0;  break;
    case RIGHT:
         if(PINB.5==0)  {  while(PINB.5==0);  return 1; }   return 0;  break;
    case LEFT:
         if(PIND.1==0)  {  while(PIND.1==0);  return 1; }   return 0;  break;
    case ENTER:
         if(PIND.0==0)  {  while(PIND.0==0);  return 1; }   return 0;  break;
    }
}
void Gio24_phut60(char *gio,char *phut)           // gioi han khoang dieu chinh thoi gian
{
    if(*gio>23)         *gio=23;  
    else if(*gio<0)     *gio=0;
    
    if(*phut>59)        *phut=59;
    else if(*phut<0)    *phut=0;
}
void MenuDisplay(Menu *menu,unsigned char Selection)       // XUAT RA MENU 3 DONG CO SELEC
{

      Print(menu->TieuDe,0,0);
      Print(menu->NoiDung1,0,1);
      Print(menu->NoiDung2,0,2);
      Print(menu->NoiDung3,0,3);
      Print("->",0,Selection);
}
void Print_Flash(unsigned char flash *text,char x,char y)       // in gia tri tu bo nho flash
{
     lcd_gotoxy(x,y);
     lcd_putsf(text);
}
//==============================================
//=============== CAC HAM HO TRO CAC TINH NANG ========
void Time_Up_Down(char *gio,char *phut)            // chinh thoi gian len xuong dung con tro
{
char luachon = 0;
    //if(ENTER==0){
    
    Print("GIO:",2,1);      Print("PHUT:",10,1);
    while(1)
    { 
    if(Read_Key(LEFT))
        {   luachon = luachon - 1;    }
    else if (Read_Key(RIGHT))
        {   luachon = luachon + 1;  }
    if(luachon%2 == 0)
    {
        if(Read_Key(UP))
        {     (*gio) = (*gio)+1;    }
        else if (Read_Key(DOWN))
        {     (*gio) = (*gio)-1;    }
    }
    else if(luachon%2 != 0)
    {
        if(Read_Key(UP))
        {     (*phut) = (*phut)+1;    }
        else if (Read_Key(DOWN))
        {     (*phut) = (*phut)-1;    }
    } 
       Gio24_phut60(gio,phut);
       Print_Int((*gio),6,1);
       Print_Int((*phut),15,1);
       if(Read_Key(ENTER))  break; 
    };
    lcd_clear();
    Print("NHAN <- DE THOAT",0,0);
}
//                        case 0:       Print("  OFF >",13,1);   break;
//                        case 1:       Print("< ON1 >",13,1);   break;
//                        case 2:       Print("< ON2 >",13,1);   break;
//                        case 3:       Print("< 1&2  ",13,1);   break; NongLanh_Auto_On
void HenGio_ON(unsigned char Hour,unsigned char Min,unsigned char Sec)
{
     // Hen gio thu 1
     /*
     char NongLanh_Time_On2_gio =0;
     char NongLanh_Time_On2_phut = 0;
     char NongLanh_Time_On1_gio =0;
     char NongLanh_Time_On1_phut = 0;
     */ 
     if(NongLanh_Auto_On==1||NongLanh_Auto_On==3) // kiem tra xem hen gio 1 co duoc phep hoat dong khong
     { 
          // // neu gio phut hien tai bang thoi gian cai dat thi kich hoat ( chay trong 5s )
          if(NongLanh_Time_On1_gio==Hour && NongLanh_Time_On1_phut==Min && Sec<5)     
          {
                    NONGLANH=1;
                    lcd_clear();
                    Print("HEN GIO ON1 ",6,2); 
                    
          }
     }else if(NongLanh_Auto_On==2||NongLanh_Auto_On==3 && Sec<5) // kiem tra xem hen gio 2 co duoc phep hoat dong khong
     {
          if(NongLanh_Time_On2_gio==Hour && NongLanh_Time_On2_phut==Min)
          {
                    NONGLANH=1;
                    lcd_clear();
                    Print("HEN GIO ON2 ",6,2); 
                    delay_ms(5000);
          }
     }
}
void HenGio_Off(unsigned char Hour,unsigned char Min,unsigned char Sec)
{
     if(NongLanh_Auto_Off==1) // kiem tra xem hen gio tat co duoc phep hoat dong khong 
     // KIEM TRA DA DEN GIO TAT CHUA
     // GIO TAT = TIME OFF + TIME ON 1 HOAC 2
     {
          if((NongLanh_Off_gio+NongLanh_Time_On1_gio==Hour && NongLanh_Off_phut+NongLanh_Time_On1_phut==Min && Sec<5)||
             (NongLanh_Off_gio+NongLanh_Time_On2_gio==Hour && NongLanh_Off_phut+NongLanh_Time_On2_phut==Min && Sec<5))
          {
                    NONGLANH=0;
                    lcd_clear();
                    Print("NONG LANH DA TAT ",0,2); 
                    
          }
     }
}
void Setting_temp()            // cai dat nhiet do toi da
{              //Set_temp
               lcd_clear();
               Print("CAT DAT NHIET DO",0,0); 
               Print("Tat Khi t>",0,2);
               Print("Nhan ENTER de thoat",0,3);
     while(1)
               {
               // cho phep giu nut bam de tang lien tuc ( moi lan tang 0.1 do )
                    if(PINC.1==0){  delay_ms(100);  Set_temp=Set_temp+0.1; }    
                    if(PINC.0==0){  delay_ms(100);  Set_temp=Set_temp-0.1; }
                    lcd_gotoxy(12,2);   
                    Print_float(Set_temp);
                    //SUOI=Suoi_trangthai=1;
                    if(Read_Key(ENTER)) break;
                    
               }
                    lcd_clear();     
                    Print("Nhan LEFT",3,0);
}
//====================================================

// cac ham tinh nang cho Menu
void Time_ON_1(char ID)          // chay vao cai dat time 1
{
     while(ENTER==0);
     lcd_clear();
     Print_Flash(Text_Time1,0,0); lcd_putchar('1');   // in ra chuoi SETTING TIMER 1 
          // truyen dia chi cua 2 bien vao de ham Time_Up_Down co the sua gia tri cua no
     Time_Up_Down(&NongLanh_Time_On1_gio,&NongLanh_Time_On1_phut);
}
void Time_ON_2(char ID)            // cai dat thoi gian On 2
{
     while(ENTER==0);
     lcd_clear();
     Print_Flash(Text_Time1,0,0); lcd_putchar('2');   // in ra chuoi SETTING TIMER 2
     Time_Up_Down(&NongLanh_Time_On2_gio,&NongLanh_Time_On2_phut);
}
void Auto_ON_OFF(char ID)             // cai dat Âuto cho Den Và suoi
{
     switch (ID){
        case ID_NongLanh :
        

        break;
        case ID_MaySuoi :             // ID o Link.h
        Suoi_auto = (Suoi_auto)?0:1;      // doi trang thai cua bien Suoi_auto 
        if(Suoi_auto==1)              // neu suoi auto tu off len on thi vao setting
        Setting_temp();
        break;
        case ID_BongDien :
        Den_auto = (Den_auto)?0:1;
        break;
    }
}
void ThuCong(char ID)           // Bat tat thu cong cac thiet bi
{
switch (ID){
        case ID_NongLanh :
        NONGLANH=NongLanh_trangthai=(NongLanh_trangthai)?0:1;
        
        break;
        case ID_MaySuoi : 
        
        SUOI=Suoi_trangthai = (Suoi_trangthai)?0:1;

        break;
        case ID_BongDien :
        DEN=Den_trangthai = (Den_trangthai)?0:1;

        break;
    }
}
void NongLanh_Auto(char ID)            // CHUYEN DOI CHE DO AUTO ON VA OFF cua nong lanh
{
    
    
    switch (ID)
    {
          case ID_Auto_Off: 
               lcd_clear();
               if(NongLanh_Auto_Off==1)
                    {
                    Print("----HUY AUTO TAT----",0,0);
                    NongLanh_Auto_Off=(NongLanh_Auto_Off)?0:1;
                    }
               else  
                    {
                    Print_Flash(Text2,0,0);   lcd_puts("TAT");    //print    :KICH HOAT AUTO TAT
                    NongLanh_Auto_Off=(NongLanh_Auto_Off)?0:1;
                    }
          break;
          case ID_Auto_On: 
               while(1)
               {
                    if(Read_Key(RIGHT))  NongLanh_Auto_On=NongLanh_Auto_On+1;     // dich phai 1 bit
                    if(Read_Key(LEFT))   NongLanh_Auto_On=NongLanh_Auto_On-1; 
//                    Print_Int(NongLanh_Auto_On,10,2);
                    if(NongLanh_Auto_On!=0&&NongLanh_Auto_On>3)
                    NongLanh_Auto_On=(NongLanh_Auto_On>3)?0:3;
                    
                    switch(NongLanh_Auto_On)
                    {
                        case 0:       Print("  OFF >",13,1);   break;
                        case 1:       Print("< ON1 >",13,1);   break;
                        case 2:       Print("< ON2 >",13,1);   break;
                        case 3:       Print("< 1&2  ",13,1);   break;
                    }
                    if(Read_Key(ENTER)) break;
               }
          
          /* 
               if(NongLanh_Auto_On==1||NongLanh_Auto_On==3)
                    {
                    Print("----HUY AUTO BAT----",0,0);
                    NongLanh_Auto_On=NongLanh_Auto_On-1;    
                    }
               else{
                    Print_Flash(Text2,0,0);   lcd_puts("BAT");
                    NongLanh_Auto_On=NongLanh_Auto_On+1;
                   }
          */
          break;
    }
        Print("ENTER DE XAC NHAN   ",0,1); 
        while(1)
        {
            if(Read_Key(ENTER))  break;
            
        }              
        lcd_clear();                               
        Print("THANH CONG",0,0);          
        delay_ms(500);
}

void TimeOFF(char ID)            // set thoi gian tat   17 bytes
{
     while(ENTER==0);
          lcd_clear();
          Print_Flash(Text_Time1,0,0); lcd_puts("OFF");
          Time_Up_Down(&NongLanh_Off_gio,&NongLanh_Off_phut);
}
void Out_Home(char ID) 
{
     lcd_clear();
     out=0;
}
void Set_Time(char ID) 
{
     char luachon = 0;
    while(ENTER==0);
    lcd_clear();
    Print("    TIMER START    ",0,0);
    Print("GIO:",2,1);      Print("PHUT:",10,1);
    Time_Up_Down(&star_gio,&star_phut); 
    rtc_set_time(star_gio,star_phut,0);
}
void Auto_Dem_Suoi()                // che do tu dong cua den va may suoi
{
   if(Den_auto==1)                  //CHO PHEP CHE DO AUTO
   {
     if(CB_ChuyenDong==0)           //  cam bien kich hoat
     {
        DEN=1;                      // bat den khi cam bien hoat dong
        Time_3s = 0;
     }else{
        if(Time_3s >5)             // thoi gian lon hon time = 6*5  thi tat den  
        {
        DEN=0;
        }
     }
   }
   if(Suoi_auto==1)                 //CHO PHEP CHE DO AUTO
   {
      if(NhietDo>Set_temp || (CB_ChuyenDong!=0&&Time_3s >5))           //  cam bien kich hoat
     // may suoi se tat khi khong co nguoi trong 30s hoac nhiet do lon hon temp
     {
        SUOI=0;                      // bat den khi cam bien hoat dong
        Time_3s = 0;
     }else{
        if(CB_ChuyenDong==0)               // co nguoi thi bat suoi neu che do auto bat
        {
        SUOI=1;
        }
     }
   }
}
void ButtonON_OFF()
{
          //#define NONGLANH PORTD.2
          //#define DEN PORTD.3
          //#define SUOI PORTD.4
     if(Read_Key(LEFT))
     SUOI=Suoi_trangthai = (Suoi_trangthai)?0:1;  
     if(Read_Key(UP))
     DEN=Den_trangthai = (Den_trangthai)?0:1;
     if(Read_Key(RIGHT))
     NONGLANH=NongLanh_trangthai=(NongLanh_trangthai)?0:1;
}
//=============== END ============
void main(void)
{
// Declare your local variables here


Menu *menu;
char Selection;
//======================== DOAN NAY KHONG CAN QUAN TAM NHIEU ===============
// Crystal Oscillator division factor: 1
#pragma optsize-
CLKPR=(1<<CLKPCE);
CLKPR=(0<<CLKPCE) | (0<<CLKPS3) | (0<<CLKPS2) | (0<<CLKPS1) | (0<<CLKPS0);
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif

// Input/Output Ports initialization
// Port B initialization
// Function: Bit7=In Bit6=In Bit5=In Bit4=Out Bit3=Out Bit2=Out Bit1=Out Bit0=In 
DDRB=(0<<DDB7) | (0<<DDB6) | (0<<DDB5) | (1<<DDB4) | (1<<DDB3) | (1<<DDB2) | (1<<DDB1) | (0<<DDB0);
// State: Bit7=T Bit6=T Bit5=P Bit4=0 Bit3=0 Bit2=0 Bit1=0 Bit0=T 
PORTB=(0<<PORTB7) | (0<<PORTB6) | (1<<PORTB5) | (0<<PORTB4) | (0<<PORTB3) | (0<<PORTB2) | (0<<PORTB1) | (0<<PORTB0);

// Port C initialization
// Function: Bit6=In Bit5=Out Bit4=Out Bit3=In Bit2=In Bit1=In Bit0=In 
DDRC=(0<<DDC6) | (1<<DDC5) | (1<<DDC4) | (0<<DDC3) | (0<<DDC2) | (0<<DDC1) | (0<<DDC0);
// State: Bit6=T Bit5=0 Bit4=0 Bit3=T Bit2=T Bit1=P Bit0=P 
PORTC=(0<<PORTC6) | (0<<PORTC5) | (0<<PORTC4) | (0<<PORTC3) | (1<<PORTC2) | (1<<PORTC1) | (1<<PORTC0);

// Port D initialization
// Function: Bit7=Out Bit6=Out Bit5=Out Bit4=Out Bit3=Out Bit2=Out Bit1=In Bit0=In 
DDRD=(1<<DDD7) | (1<<DDD6) | (1<<DDD5) | (1<<DDD4) | (1<<DDD3) | (1<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=0 Bit6=0 Bit5=0 Bit4=0 Bit3=0 Bit2=0 Bit1=P Bit0=P 
PORTD=(0<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (1<<PORTD1) | (1<<PORTD0);

// Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: Timer 0 Stopped
// Mode: Normal top=0xFF
// OC0A output: Disconnected
// OC0B output: Disconnected
TCCR0A=(0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00);
TCCR0B=(0<<WGM02) | (0<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x00;
OCR0A=0x00;
OCR0B=0x00;

// Timer/Counter 1 initialization
// Clock source: System Clock
// Clock value: 15,625 kHz
// Mode: Normal top=0xFFFF
// OC1A output: Disconnected
// OC1B output: Disconnected
// Noise Canceler: Off
// Input Capture on Falling Edge
// Timer Period: 3 s
// Timer1 Overflow Interrupt: On
// Input Capture Interrupt: Off
// Compare A Match Interrupt: Off
// Compare B Match Interrupt: Off
TCCR1A=(0<<COM1A1) | (0<<COM1A0) | (0<<COM1B1) | (0<<COM1B0) | (0<<WGM11) | (0<<WGM10);
TCCR1B=(0<<ICNC1) | (0<<ICES1) | (0<<WGM13) | (0<<WGM12) | (1<<CS12) | (0<<CS11) | (1<<CS10);
TCNT1H=0x48;
TCNT1L=0xE5;
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
// OC2A output: Disconnected
// OC2B output: Disconnected
ASSR=(0<<EXCLK) | (0<<AS2);
TCCR2A=(0<<COM2A1) | (0<<COM2A0) | (0<<COM2B1) | (0<<COM2B0) | (0<<WGM21) | (0<<WGM20);
TCCR2B=(0<<WGM22) | (0<<CS22) | (0<<CS21) | (0<<CS20);
TCNT2=0x00;
OCR2A=0x00;
OCR2B=0x00;

// Timer/Counter 0 Interrupt(s) initialization
TIMSK0=(0<<OCIE0B) | (0<<OCIE0A) | (0<<TOIE0);

// Timer/Counter 1 Interrupt(s) initialization
TIMSK1=(0<<ICIE1) | (0<<OCIE1B) | (0<<OCIE1A) | (1<<TOIE1);

// Timer/Counter 2 Interrupt(s) initialization
TIMSK2=(0<<OCIE2B) | (0<<OCIE2A) | (0<<TOIE2);


// Analog Comparator initialization
// Analog Comparator: Off
// The Analog Comparator's positive input is
// connected to the AIN0 pin
// The Analog Comparator's negative input is
// connected to the AIN1 pin
ACSR=(1<<ACD) | (0<<ACBG) | (0<<ACO) | (0<<ACI) | (0<<ACIE) | (0<<ACIC) | (0<<ACIS1) | (0<<ACIS0);
// Digital input buffer on AIN0: On
// Digital input buffer on AIN1: On
DIDR1=(0<<AIN0D) | (0<<AIN1D);

// ADC initialization
// ADC Clock frequency: 1000,000 kHz
// ADC Voltage Reference: AREF pin
// ADC Auto Trigger Source: ADC Stopped
// Digital input buffers on ADC0: On, ADC1: On, ADC2: On, ADC3: On
// ADC4: On, ADC5: On
DIDR0=(0<<ADC5D) | (0<<ADC4D) | (0<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D);
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (0<<ADPS1) | (0<<ADPS0);
ADCSRB=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);

// SPI initialization
// SPI disabled
SPCR=(0<<SPIE) | (0<<SPE) | (0<<DORD) | (0<<MSTR) | (0<<CPOL) | (0<<CPHA) | (0<<SPR1) | (0<<SPR0);

// TWI initialization
// TWI disabled
TWCR=(0<<TWEA) | (0<<TWSTA) | (0<<TWSTO) | (0<<TWEN) | (0<<TWIE);

// Bit-Banged I2C Bus initialization
// I2C Port: PORTC
// I2C SDA bit: 4
// I2C SCL bit: 5
// Bit Rate: 100 kHz
// Note: I2C settings are specified in the
// Project|Configure|C Compiler|Libraries|I2C menu.
i2c_init();

// DS1307 Real Time Clock initialization
// Square wave output on pin SQW/OUT: Off
// SQW/OUT pin state: 0
rtc_init(0,0,0);

// Alphanumeric LCD initialization
// Connections are specified in the
// Project|Configure|C Compiler|Libraries|Alphanumeric LCD menu:
// RS - PORTD Bit 5
// RD - PORTD Bit 6
// EN - PORTD Bit 7
// D4 - PORTB Bit 1
// D5 - PORTB Bit 2
// D6 - PORTB Bit 3
// D7 - PORTB Bit 4
// Characters/line: 20
lcd_init(20);
//======================== DOAN NAY KHONG CAN QUAN TAM NHIEU ===============
//rtc_set_time(star_gio,star_phut,0);
menu = &MainMenu;

Selection=1;
// Global enable interrupts
#asm("sei")

while (1)          // while chinh cua chuong trinh Khong thoat ra
      {  

      // Place your code here       
      // ================= DOAN CODE CUA CAI MAN HINH CHINH HIEN THI THONG TIN
         Print("------NHA TAM------",0,0);
         Print("DATE : ",0,1);
         Print("NHIET DO : ",0,2);
         while(out==0)
             {     
                 rtc_get_time(&Hour,&Min,&Sec);
                 TimeDisplay(Hour,Min,Sec,8,1);      
                 Read_NhietDo(12,2);
                 ButtonON_OFF();                  // dieu khien thiet bi bang nut nhan
                 if(Read_Key(ENTER))              // KHI CO PHIME ENTER THI VAO MAN HINH MENU
                 { 
                    out=1;                   
                    lcd_clear();
                    break;
                 }
                 Auto_Dem_Suoi();                  // CAC HAM HEN GIO VA AUTO LUON CHAY
                 HenGio_ON(Hour,Min,Sec); 
                 HenGio_Off(Hour,Min,Sec);   
             } 
         //=========== KET THUC MENU CHINH SAU KHI NHAANS ENTER
         MenuDisplay(menu,1); 
         // BAT DAU VAO MENU CHINH
         while (out==1)
          {
            if(Read_Key(DOWN))   // down      LEN XUONG CON TRO VA SELECTION
              {
                   lcd_clear();
                   
                   Selection = (Selection==3)?1:Selection+1;   // SELECTION KHONG THE DI QUA 3
                   MenuDisplay(menu,Selection);
              }
              if(Read_Key(UP)) //  up
              {
                   lcd_clear();
                   
                   Selection = (Selection==1)?3:Selection-1;   // SELECTION KHONG THE DI QUA 1
                   MenuDisplay(menu,Selection);
              }
              if(Read_Key(RIGHT)) //  up
              {
                   lcd_clear();
                   // TUY VAO SELECTION CHAY VAO CAC MENU KHA NHAU DUA VAO CAC CON TRO
                   switch (Selection)                       
                   {
                   case 1:
                         menu=(menu->Menu1==NULL)?menu:menu->Menu1;

                         break;
                   case 2:
                         menu=(menu->Menu2==NULL)?menu:menu->Menu2;

                         break;
                   case 3:
                         menu=(menu->Menu3==NULL)?menu:menu->Menu3;

                         break;
                   }
                   MenuDisplay(menu,Selection);       // HIEN THI MENU MA CON TRO MENU DANG TRO VAO
              }
              if(Read_Key(LEFT)) //  up
              {
                   lcd_clear();
                   // NEU CO MENU CON THI CHAY VAO MENU CON
                   menu=(menu->Previous==NULL)?menu:menu->Previous;
                   MenuDisplay(menu,Selection);
              } 
              // NEU MENU DO CO HAM CHUC NANG THI CHAY VAO DO VA TRUYEN VAO HAM SO ID
              if(Read_Key(ENTER))
              {
                    switch (Selection)
                       {
                 case 1:
                       //menu=(menu->Menu1==NULL)?menu:menu->Menu1;
                       if(menu->Ham_LienKet1!=NULL)
                         menu->Ham_LienKet1(menu->ID_device);
                       //Doi_CheDo(menu->ID_device);   }
                       break;
                 case 2:
                       //menu=(menu->Menu2==NULL)?menu:menu->Menu2;
                       if(menu->Ham_LienKet2!=NULL)
                         menu->Ham_LienKet2(menu->ID_device);
                        //Doi_CheDo(menu->ID_device);   }
                       break;
                 case 3:
                       //menu=(menu->Menu3==NULL)?menu:menu->Menu3;
                       if(menu->Ham_LienKet3!=NULL)
                         menu->Ham_LienKet3(menu->ID_device);
                        //Doi_CheDo(menu->ID_device);   }
                       break;
                 }
              }
          Auto_Dem_Suoi(); 
          HenGio_ON(Hour,Min,Sec); 
          HenGio_Off(Hour,Min,Sec);
          }
          
      }
}
