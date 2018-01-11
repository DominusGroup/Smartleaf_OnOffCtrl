#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <pthread.h> // threads 
#include <rpigpio.h> // pinMode, digitalWrite
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h> 
#include <unistd.h>  // write, read, lseek, sleep file descriptors
#include <string.h> // strerror(errno)
#include <stdlib.h> // system() 
#include <errno.h>

/* Defines lcd */
#define I2C_ADDR   0x27 // I2C device address
#define LCD_CHR  1 // Mode - Sending data
#define LCD_CMD  0 // Mode - Sending command
#define LINE1  0x80 // 1st line
#define LINE2  0xC0 // 2nd line
#define LCD_BACKLIGHT   0x08  // On  // LCD_BACKLIGHT = 0x00  # Off
#define ENABLE  0b00000100 // Enable bit

/* Defines gpio & rtc & keypad */
#define pin1 27

// Modo input/output: input=0, output=1
#define outputMode 1
#define inputMode 0
#define highState 1
#define lowState 0

#define KEYPAD_INPUTS 3  // dos numeros

//#define I2C_ADDR 0x08
#define I2C_ADDR_RTC 0x68
#define DATE_BUFF_SZ 44
#define RTC_COMMAND "hwclock -r"

const char * RTC() ;
char date_buffer[DATE_BUFF_SZ] ;

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

void typeInt(int i);
void typeFloat(float myFloat);
void lcdLoc(int line); //move cursor
void ClrLcd(void); // clr LCD return home
void typeln(const char *s);
void typeChar(char val);
int fd;

void *f1(void *prt) ;

int main(){
  char word[KEYPAD_INPUTS] ; 
  char hora[2], min[2] ;
  int alarma1 = 1500 ; 
  int tiempo1 = 2 ;
  int RTC2min = 0 ; int x ;

  pthread_t tid ;


  if(map_peripheral(&gpio) == -1)
  {
    printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
    return -1;
  }
  pinMode(pin1, outputMode) ;


  printf("Iniciando LCD\n");
  if (wiringPiSetup () == -1) exit (1);

  fd = wiringPiI2CSetup(I2C_ADDR);
  lcd_init(); // setup LCD



  //printf("date = %s", rtc);  

  //char array1[] = rtc ; 
  //int vuelta=0;


    int USERminute = 60 ; 
    int USERhour = 60 ;   

	printf("Introduzca 7 para configurar hora\n");
    fgets(word, KEYPAD_INPUTS, stdin) ;
    int key = (int)strtol(word, (char **)NULL, 10) ; 

  if(key == 7){
    // Obtiene hora, minuto del teclado, y los convierte a enteros
      printf("Introduzca la hora para encender el Sistema\n") ;
      fgets(word, KEYPAD_INPUTS, stdin) ; 
      USERhour = (int)strtol(word, (char **)NULL, 10) ; 

      printf("Introduzca minuto para encender el Sistema\n") ;
      fgets(word, KEYPAD_INPUTS, stdin) ; 
      USERminute = (int)strtol(word, (char **)NULL, 10) ;       
      
      alarma1 = USERhour*60 + USERminute ; 

    // Imprime la hora establecida 
      if(USERminute <= 9)
        printf("Hora de encendido establecida = %d:0%d\n", USERhour, USERminute) ;
      else  
        printf("Hora de encendido establecida = %d:%d\n", USERhour, USERminute) ;      
    }
  
  if(pthread_create(&tid, NULL, f1, &x)){
	fprintf(stderr, "Error creating thread\n");
	return 1 ; 
  }

  while (1){
  // Obtener la hora 
    const char * rtc = RTC() ;     
    lcdLoc(LINE1);
    typeln(rtc);
    lcdLoc(LINE2);
    typeln("Alarma ") ;
    typeInt(USERhour);
    typeln(":") ;
    typeInt(USERminute) ; 
    
  // parsear la hora
  for(rtc; *rtc; ++rtc){
    if(*rtc == ':'){
      hora[0] = *(rtc-5) ;
      hora[1] = *(rtc-4) ; 
      hora[2] = '\0' ; 
      min[0] = *(rtc-2) ; 
      min[1] = *(rtc-1) ; 
      min[2] = '\0' ;       
    }
  }
  // Imprime hora del sistema 
    //printf(" hora = %s\n min = %s\n", hora, min) ; 
  // Convierte hora y minuto del sistema a enteros
    int RTCminute = (int)strtol(min, (char **)NULL, 10) ;
    int RTChour = (int)strtol(hora, (char **)NULL, 10) ;

    RTC2min = RTChour*60 + RTCminute ; 

  // Condicion de encedido/apagado
    //if(RTCminute >= USERminute && RTChour >= USERhour)
    if(RTC2min >= alarma1 && RTC2min < (alarma1 +tiempo1))
      digitalWrite(pin1, highState) ;
    else
      digitalWrite(pin1, lowState) ;


    delay(5000);
    ClrLcd(); // defaults LINE1

  }

	// Wait until thread is done its work
  if(pthread_join(tid, NULL)){
	fprintf(stderr, "Error joining thread\n") ;
	return 2 ; 
  }   

  return 0;

}

void *f1(void * prt){
	int *x_prt = (int *)prt ; 
    char word1[KEYPAD_INPUTS] ; 

	//while(++(*x_prt) < 25) ; 
	//printf("x incrementado \n");
	
	printf("Menu del sistema: digite 7 para reconfigurar \n") ;
    fgets(word1, KEYPAD_INPUTS, stdin) ; 
	printf("%s\n", word1) ;
	return NULL ; 

	//for(int i=0; i<25; i++)
	//	printf("i = %d\n", i);

	//pthread_exit(0) ;
}


const char * RTC(){
  
  FILE * file = popen(RTC_COMMAND, "r");  

  while (fgets(date_buffer, DATE_BUFF_SZ, file) != NULL ) {
    //printf("date = %s", date_buffer);
  }
  pclose(file) ; 
  
  return date_buffer ;
}


// float to string
void typeFloat(float myFloat)   {
  char buffer[20];
  sprintf(buffer, "%4.2f",  myFloat);
  typeln(buffer);
}

// int to string
void typeInt(int i)   {
  char array1[20];
  sprintf(array1, "%d",  i);
  typeln(array1);
}

// clr lcd go home loc 0x80
void ClrLcd(void)   {
  lcd_byte(0x01, LCD_CMD);
  lcd_byte(0x02, LCD_CMD);
}

// go to location on LCD
void lcdLoc(int line)   {
  lcd_byte(line, LCD_CMD);
}

// out char to LCD at current position
void typeChar(char val)   {
  lcd_byte(val, LCD_CHR);
}

// this allows use of any size string
void typeln(const char *s)   {
  int i = 0 ;
  while ( *s ){
    lcd_byte(*(s++), LCD_CHR);      
    if(i == 16)
      break ;
    i++ ; 
  }
}

void lcd_byte(int bits, int mode)   {

  //Send byte to data pins
  // bits = the data
  // mode = 1 for data, 0 for command
  int bits_high;
  int bits_low;
  // uses the two half byte writes to LCD
  bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT ;
  bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT ;

  // High bits
  wiringPiI2CReadReg8(fd, bits_high);
  lcd_toggle_enable(bits_high);

  // Low bits
  wiringPiI2CReadReg8(fd, bits_low);
  lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits)   {
  // Toggle enable pin on LCD display
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits | ENABLE));
  delayMicroseconds(500);
  wiringPiI2CReadReg8(fd, (bits & ~ENABLE));
  delayMicroseconds(500);
}

void lcd_init()   {
  // Initialise display
  lcd_byte(0x33, LCD_CMD); // Initialise
  lcd_byte(0x32, LCD_CMD); // Initialise
  lcd_byte(0x06, LCD_CMD); // Cursor move direction
  lcd_byte(0x0C, LCD_CMD); // 0x0F On, Blink Off
  lcd_byte(0x28, LCD_CMD); // Data length, number of lines, font size
  lcd_byte(0x01, LCD_CMD); // Clear display
  delayMicroseconds(500);
}
