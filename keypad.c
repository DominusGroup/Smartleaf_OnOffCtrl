#include <stdio.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h> 
#include <unistd.h>  // write, read and lseek file descriptors
#include <string.h> // strerror(errno)
#include <stdlib.h> // system() 
#include <errno.h>

#define I2C_ADDR 0x08
#define I2C_ADDR_RTC 0x68

#define DATE_BUFF_SZ 44
#define RTC_COMMAND "hwclock -r"
//int i2c(int i2c_peripheral) ;  
const char * RTC() ;

char date_buffer[DATE_BUFF_SZ] ;

int main (int argc, char ** argv) {

	const char * rtc = RTC() ; 
	printf("date = %s", rtc);

	return 0 ; 
}

const char * RTC(){
	
	FILE * file = popen(RTC_COMMAND, "r");	

	while (fgets(date_buffer, DATE_BUFF_SZ, file) != NULL ) {
		//printf("date = %s", date_buffer);
	}
	pclose(file) ; 
	
	return date_buffer ;
}

/*
int i2c(int i2c_peripheral){
	char buffer[5];
	int fd;

	fd = open("/dev/i2c-1", O_RDWR);

	if (fd < 0) {
		printf("Error opening file: %s\n", strerror(errno));
		return 1;
	}

	if (ioctl(fd, I2C_SLAVE, i2c_peripheral) < 0) {
		printf("ioctl error: %s\n", strerror(errno));
		return 1;
	}

	buffer[1]=0xFF;

	write(fd, buffer, 1);	
	read(fd, buffer, 1);
	
	int buf = buffer[0] ;


	return buf ;
 
}
*/