#include <rpigpio.h> // pinMode, digitalWrite
#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h> 
#include <unistd.h>  // write, read, lseek, sleep file descriptors
#include <string.h> // strerror(errno)
#include <stdlib.h> // system() 
#include <errno.h>

#define pin1 27

// Modo input/output: input=0, output=1
#define outputMode 1
#define inputMode 0
#define highState 1
#define lowState 0

#define KEYPAD_INPUTS 3  // dos numeros

#define I2C_ADDR 0x08
#define I2C_ADDR_RTC 0x68
#define DATE_BUFF_SZ 44
#define RTC_COMMAND "hwclock -r"


const char * RTC() ;
char date_buffer[DATE_BUFF_SZ] ;

int main(int argc, char ** argv)
{
  char word[KEYPAD_INPUTS] ; 
  char hora[2], min[2] ;

  if(map_peripheral(&gpio) == -1)
  {
    printf("Failed to map the physical GPIO registers into the virtual memory space.\n");
    return -1;
  }

  pinMode(pin1, outputMode) ;

// Obtener la hora 
  const char * rtc = RTC() ; 
  printf("date = %s", rtc);
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
  printf(" hora = %s\n min = %s\n", hora, min) ; 
  
  int minute = (int)strtol(min, (char **)NULL, 10) ; 

  fgets(word, KEYPAD_INPUTS, stdin) ; 
  //char key = (char)word[0] ; 
  int key = (int)strtol(word, (char **)NULL, 10) ; 

  printf("key = %d\n", key) ;

  if(minute > key)
    digitalWrite(pin1, highState) ;
  else
    digitalWrite(pin1, lowState) ;
  
  return 0;
}

const char * RTC(){
  
  FILE * file = popen(RTC_COMMAND, "r");  

  while (fgets(date_buffer, DATE_BUFF_SZ, file) != NULL ) {
    //printf("date = %s", date_buffer);
  }
  pclose(file) ; 
  
  return date_buffer ;
}
