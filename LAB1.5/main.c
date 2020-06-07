#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>
#include<wiringPiSPI.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <unistd.h>

#define LAST(k,n) ((k) & ((1<<(n))-1))
#define MID(k,m,n) LAST((k)>>(m),((n)-(m)))

int chan = 0;
int t_fine;
int adresEEPROM;

enum {
  BMP280_REGISTER_DIG_T1 = 0x88,
  BMP280_REGISTER_DIG_T2 = 0x8A,
  BMP280_REGISTER_DIG_T3 = 0x8C,
  BMP280_REGISTER_DIG_P1 = 0x8E,
  BMP280_REGISTER_DIG_P2 = 0x90,
  BMP280_REGISTER_DIG_P3 = 0x92,
  BMP280_REGISTER_DIG_P4 = 0x94,
  BMP280_REGISTER_DIG_P5 = 0x96,
  BMP280_REGISTER_DIG_P6 = 0x98,
  BMP280_REGISTER_DIG_P7 = 0x9A,
  BMP280_REGISTER_DIG_P8 = 0x9C,
  BMP280_REGISTER_DIG_P9 = 0x9E,
  BMP280_REGISTER_CHIPID = 0xD0,
  BMP280_REGISTER_VERSION = 0xD1,
  BMP280_REGISTER_SOFTRESET = 0xE0,
  BMP280_REGISTER_CAL26 = 0xE1, /**< R calibration = 0xE1-0xF0 */
  BMP280_REGISTER_STATUS = 0xF3,
  BMP280_REGISTER_CONTROL = 0xF4,
  BMP280_REGISTER_CONFIG = 0xF5,
  BMP280_REGISTER_PRESSUREDATA = 0xF7,
  BMP280_REGISTER_TEMPDATA = 0xFA,
};

typedef struct {
  uint16_t dig_T1; /**< dig_T1 cal register. */
  int16_t dig_T2;  /**<  dig_T2 cal register. */
  int16_t dig_T3;  /**< dig_T3 cal register. */

  uint16_t dig_P1; /**< dig_P1 cal register. */
  int16_t dig_P2;  /**< dig_P2 cal register. */
  int16_t dig_P3;  /**< dig_P3 cal register. */
  int16_t dig_P4;  /**< dig_P4 cal register. */
  int16_t dig_P5;  /**< dig_P5 cal register. */
  int16_t dig_P6;  /**< dig_P6 cal register. */
  int16_t dig_P7;  /**< dig_P7 cal register. */
  int16_t dig_P8;  /**< dig_P8 cal register. */
  int16_t dig_P9;  /**< dig_P9 cal register. */
} bmp280_calib_data;

bmp280_calib_data calib_data;

void menu(void);
void temperatura(int fd);
void WhoAmI(int fd);
void ackelerometr(int fd);
void zyroskop(int fd);
void poziomica(int fd);
void data(int fdRTC);
void WpiszDate(int fdRTC);
void eepromOdczyt(int fdAT);
void eepromZapis(int fdAT);
int BMP280(int chan, int speed);
float cisnienie(int32_t press);
float tempBMP280(int32_t temp);
int init();
int writeRegister(int adress,int data);
int readRegister(int adress);
uint16_t readRegister_uint16(int adress);
int16_t readRegister_int16(int adress);
uint32_t  readRegister_uint32(int adress);

int main (void)
{

menu();
return 0;
}
void menu()
{
	int chan=0;
	int speed=1000000;
	int choice;
	int fdAT;
	int adressAT24C32=0x50;
	int fd;
	int adressMPU6050=0x69;
	if(wiringPiSetup() == -1)
	exit(1);

	if((fd=wiringPiI2CSetup(adressMPU6050)) == -1){
	printf("error initialize I2C");
	exit(1);
	}
	//Uruchamia pomiary
	int regPWR=0x6B; //<<---- podac adres rejestru PWR_MGMT_1
	wiringPiI2CWriteReg8(fd, regPWR, 0);

	int fdRTC;
	int adressRTCDS1307=0x68;

	if(wiringPiSetup() == -1)
	exit(1);
	//-------Podaczenie RTCDS1307----------------
	if((fdRTC=wiringPiI2CSetup(adressRTCDS1307)) == -1){
		printf("error initialize I2C");
		exit(1);
	}
		//-------Podaczenie RTCDS1307----------------
		//-------Podaczenie AT24C32----------------
	if((fdAT=wiringPiI2CSetup(adressAT24C32)) == -1){
		printf("error initialize I2C");
		exit(1);
	}
	
	//-------Podaczenie AT24C32----------------
	do
	{
		printf("Menu\n\n");
		printf("1. Temperatura\n");
		printf("2. Akcelerometr i zyroskop\n");
		printf("3. Odczytanie daty RTC\n");
		printf("4. Ustawienie nowej daty RTC\n");
		printf("5. Odczytanie 16 bitowej wartosci EEPROM\n");
		printf("6. Zapisanie 16 bitowej wartosci EEPROM\n");
		printf("7. BMP280 – odczytanie ID modułu, temperatury i ciśnienia atmosferycznego\n");
		printf("8. Informacje o dostepnych opcjach\n");
		printf("9. Autorzy \n");
		printf("10. Exit\n");
		scanf("%d", &choice);

		switch (choice)
		{
		case 1:
			system("clear");
			printf("I2C modul MPU6050\n\n");
			WhoAmI(fd);
			temperatura(fd);
			sleep(5);
			break;
		case 2: 
			system("clear");
			printf("I2C modul MPU6050\n\n");
			ackelerometr(fd);
			zyroskop(fd);
			poziomica(fd);
			sleep(5);
			break;
		case 3:
			system("clear");
			printf("I2C modul RTCDS1307\n\n");
			data(fdRTC);
			sleep(5);
			break;
		case 4:
			system("clear");
			printf("I2C modul RTCDS1307\n\n");
			WpiszDate(fdRTC);
			//sleep(5);
			break;
		case 5:
			system("clear");
			printf("I2C modul AT24C32\n\n");
			eepromOdczyt(fdAT);
			sleep(2);
			break;
		case 6:
			system("clear");
			printf("I2C modul AT24C32\n\n");
			eepromZapis(fdAT);
			//coś tu będzie
			sleep(1);
			break;
		case 7:
			system("clear");
			BMP280(chan, speed);
			sleep(5);
			break;
		case 8:

			break;
		case 9:
			system("clear");
			printf("Praca studentow:\n");
			printf("Grzegorz Sęk, 241778\n");
			printf("Adam Talarczyk, 236032\n");
			sleep(5);
			break;
		case 10: 
			printf("Quitting program!\n");
			sleep(2);
			exit(0);
			break;
		default: 
			system("clear");
			printf("Invalid choice!\n");
			sleep(3);
			break;
		}
		system("clear");
		
	} while (choice != 10);

}

void temperatura(int fd){
//------------------------------------------POMIARY - TEMPERATURA-------------------------------
int regTempOutH = 0x41; //zmienna przechowująca adres rejestru TempOutH
int regTempOutL = 0x42; //zmienna przechowująca adres rejestru TempOutL


int wartosc[2]={0};
wartosc[0] = wiringPiI2CReadReg8(fd, regTempOutH); //Odczyt danych (8 bitów) z rejestru TempOutH
wartosc[1] = wiringPiI2CReadReg8(fd, regTempOutL); //Odczyt danych (8 bitów) z rejestru TempOutL

int16_t tempOut= wartosc[0]<<8|wartosc[1];
float temperatura = tempOut/ 340 + 36.53; //Zmiana int na float w celu uzyskania większej dokładności oraz 
										  //wykorzystanie wzoru z dokmunetacji w celu zamiany "surowych danych"
										  //na dane odpowiadające rzeczywistej temperaturze
printf("\nTEMPERATURA: %f\r\n", temperatura);
printf("\n");
} 

void WhoAmI(int fd){
		//------------------------------------------ODCZYT REJESTRU WHO_AM_I-------------------------------
int regWhoValue; //zmienna przechowująca adres rejestru WHO_AM_I
int regWho=0x75; //<<---- podac adres rejestru Who Am I


regWhoValue = wiringPiI2CReadReg8(fd, regWho); //Odczyt danych (8 bitów) z rejestru WHO_AM_I

printf("Rejestr WHO_AM_I:\n");
printf("I am: %d\r\n", regWhoValue); //wyświetlenie wartości odczytanej z rejestru WHO_AM_I
printf("\n");
}

void ackelerometr(int fd){
//------------------------------------------POMIARY - AKCELEROMETR-------------------------------
int akcelerometr[6]={0};  //Utworzenie tablicy przechowującej dane z rejestrów akcelerometru

int regAccel_XOut_H=0x3B; //zmienna przechowująca adres rejestru Accel_XOut_H
int regAccel_XOut_L=0x3C; //zmienna przechowująca adres rejestru Accel_XOut_L
int regAccel_YOut_H=0x3D; //zmienna przechowująca adres rejestru Accel_YOut_H
int regAccel_YOut_L=0x3E; //zmienna przechowująca adres rejestru Accel_YOut_L
int regAccel_ZOut_H=0x3F; //zmienna przechowująca adres rejestru Accel_ZOut_H
int regAccel_ZOut_L=0x40; //zmienna przechowująca adres rejestru Accel_ZOut_L


akcelerometr[0]=wiringPiI2CReadReg8(fd, regAccel_XOut_H); //Odczyt danych (8 bitów) z rejestru Accel_XOut_H
akcelerometr[1]=wiringPiI2CReadReg8(fd, regAccel_XOut_L); //Odczyt danych (8 bitów) z rejestru Accel_XOut_L
akcelerometr[2]=wiringPiI2CReadReg8(fd, regAccel_YOut_H); //Odczyt danych (8 bitów) z rejestru Accel_YOut_H
akcelerometr[3]=wiringPiI2CReadReg8(fd, regAccel_YOut_L); //Odczyt danych (8 bitów) z rejestru Accel_YOut_L
akcelerometr[4]=wiringPiI2CReadReg8(fd, regAccel_ZOut_H); //Odczyt danych (8 bitów) z rejestru Accel_ZOut_H 
akcelerometr[5]=wiringPiI2CReadReg8(fd, regAccel_ZOut_L); //Odczyt danych (8 bitów) z rejestru Accel_ZOut_L

int16_t acc_x = akcelerometr[0]<<8|akcelerometr[1]; //złożenie jednej liczby z dwóch rejestrów (Accel_XOut_H oraz Accel_XOut_L)
int16_t acc_y = akcelerometr[2]<<8|akcelerometr[3]; //złożenie jednej liczby z dwóch rejestrów (Accel_YOut_H oraz Accel_YOut_L)
int16_t acc_z = akcelerometr[4]<<8|akcelerometr[5]; //złożenie jednej liczby z dwóch rejestrów (Accel_ZOut_H oraz Accel_ZOut_L)

float accX = acc_x/16384.0; //konwersja "surowych danych" do float(aby uzyskać większą dokładność)
float accY = acc_y/16384.0; //oraz do odpowiedniej wartości poprzez podzielenie przez dokładność sensora (z dokumentacji)
float accZ = acc_z/16384.0;

printf("AKCELEROMETR:\n");
printf("przyspieszenie dla osi OX: %f g\n", accX); //Wyświetlenie danych uzyskanych z akcelerometru
printf("przyspieszenie dla osi OY: %f g\n", accY);
printf("przyspieszenie dla osi OZ: %f g\n", accZ);
printf("\n");
 
}

void zyroskop(int fd){
//------------------------------------------POMIARY - ZYROSKOP-------------------------------
int zyroskop[6]={0}; //Utworzenie tablicy przechowującej dane z rejestrów żyroskopu

int regGyro_XOut_H=0x43; //zmienna przechowująca adres rejestru Gyro_XOut_H
int regGyro_XOut_L=0x44; //zmienna przechowująca adres rejestru Gyro_XOut_L
int regGyro_YOut_H=0x45; //zmienna przechowująca adres rejestru Gyro_YOut_H
int regGyro_YOut_L=0x46; //zmienna przechowująca adres rejestru Gyro_YOut_L
int regGyro_ZOut_H=0x47; //zmienna przechowująca adres rejestru Gyro_ZOut_H
int regGyro_ZOut_L=0x48; //zmienna przechowująca adres rejestru Gyro_ZOut_L


zyroskop[0]=wiringPiI2CReadReg8(fd, regGyro_XOut_H); //Odczyt danych (8 bitów) z rejestru Gyro_XOut_H
zyroskop[1]=wiringPiI2CReadReg8(fd, regGyro_XOut_L); //Odczyt danych (8 bitów) z rejestru Gyro_XOut_L
zyroskop[2]=wiringPiI2CReadReg8(fd, regGyro_YOut_H); //Odczyt danych (8 bitów) z rejestru Gyro_YOut_H
zyroskop[3]=wiringPiI2CReadReg8(fd, regGyro_YOut_L); //Odczyt danych (8 bitów) z rejestru Gyro_YOut_L
zyroskop[4]=wiringPiI2CReadReg8(fd, regGyro_ZOut_H); //Odczyt danych (8 bitów) z rejestru Gyro_ZOut_H
zyroskop[5]=wiringPiI2CReadReg8(fd, regGyro_ZOut_L); //Odczyt danych (8 bitów) z rejestru Gyro_ZOut_L

int16_t gyro_x = zyroskop[0]<<8|zyroskop[1]; //złożenie jednej liczby z dwóch rejestrów (Gyro_XOut_H oraz Gyro_XOut_L)
int16_t gyro_y = zyroskop[2]<<8|zyroskop[3]; //złożenie jednej liczby z dwóch rejestrów (Gyro_YOut_H oraz Gyro_YOut_L)
int16_t gyro_z = zyroskop[4]<<8|zyroskop[5]; //złożenie jednej liczby z dwóch rejestrów (Gyro_ZOut_H oraz Gyro_ZOut_L)

float gyroX = gyro_x/131.0; //konwersja "surowych danych" do float(aby uzyskać większą dokładność)
float gyroY = gyro_y/131.0; //oraz do odpowiedniej wartości poprzez podzielenie przez dokładność sensora (z dokumentacji)
float gyroZ = gyro_z/131.0;

printf("ZYROSKOP:\n");
printf("przyspieszenie - oś OX: %f stopni/s\n", gyroX); //Wyświetlenie danych uzyskanych z żyroskopu
printf("przyspieszenie - oś OY: %f stopni/s\n", gyroY);
printf("przyspieszenie - oś OZ: %f stopni/s\n", gyroZ);
printf("\n");
}


void poziomica(int fd){
	//------------------------------------------POZIOMICA-------------------------------
	int akcelerometr[4]={0};  //Utworzenie tablicy przechowującej dane z rejestrów akcelerometru

	int regAccel_XOut_H=0x3B; //zmienna przechowująca adres rejestru Accel_XOut_H
	int regAccel_XOut_L=0x3C; //zmienna przechowująca adres rejestru Accel_XOut_L
	int regAccel_ZOut_H=0x3F; //zmienna przechowująca adres rejestru Accel_ZOut_H
	int regAccel_ZOut_L=0x40; //zmienna przechowująca adres rejestru Accel_ZOut_L


	akcelerometr[0]=wiringPiI2CReadReg8(fd, regAccel_XOut_H); //Odczyt danych (8 bitów) z rejestru Accel_XOut_H
	akcelerometr[1]=wiringPiI2CReadReg8(fd, regAccel_XOut_L); //Odczyt danych (8 bitów) z rejestru Accel_XOut_L
	akcelerometr[4]=wiringPiI2CReadReg8(fd, regAccel_ZOut_H); //Odczyt danych (8 bitów) z rejestru Accel_ZOut_H 
	akcelerometr[5]=wiringPiI2CReadReg8(fd, regAccel_ZOut_L); //Odczyt danych (8 bitów) z rejestru Accel_ZOut_L

	int16_t acc_x = akcelerometr[0]<<8|akcelerometr[1]; //złożenie jednej liczby z dwóch rejestrów (Accel_XOut_H oraz Accel_XOut_L)
	int16_t acc_z = akcelerometr[4]<<8|akcelerometr[5]; //złożenie jednej liczby z dwóch rejestrów (Accel_ZOut_H oraz Accel_ZOut_L)

	//float accX = acc_x/16384.0; //konwersja "surowych danych" do float(aby uzyskać większą dokładność)
	//float accZ = acc_z/16384.0;	//oraz do odpowiedniej wartości poprzez podzielenie przez dokładność sensora (z dokumentacji)


	float poziom = atan2(acc_x, acc_z) * (180 / M_PI); //obliczenie poziomu (wzór z Laboratorium 1.2)

	printf("POZIOMICA:\r\n");
	printf("POZIOM: %f stopni\n", poziom); //Wyświetlenie poziomu
}

void data(int fdRTC){
	//------------------------------LAB1.3-----------------------------*/

	int regSeconds=0x00; //zmienna przechowująca adres rejestru  seconds
	int regMinutes=0x01;
	int regHours=0x02;
	int regDate=0x04;
	int regMonth=0x05;
	int regYear=0x06;

	int DATA[6]={0}; //stworzenie 6 elementowej tablicy
	DATA[0]=wiringPiI2CReadReg8(fdRTC, regSeconds); //odczytanie wartosci z rejestru
	DATA[1]=wiringPiI2CReadReg8(fdRTC, regMinutes);
	DATA[2]=wiringPiI2CReadReg8(fdRTC, regHours);
	DATA[3]=wiringPiI2CReadReg8(fdRTC, regDate);
	DATA[4]=wiringPiI2CReadReg8(fdRTC, regMonth);
	DATA[5]=wiringPiI2CReadReg8(fdRTC, regYear);

	int seconds=(DATA[0]&0b00001111)+(((DATA[0]&0b01110000)>>4)*10); //składanie wartosci z rejestru w odpowiednia liczbe
	int minutes=(DATA[1]&0b00001111)+(((DATA[1]&0b01110000)>>4)*10);
	int hours=(DATA[2]&0b00001111)+(((DATA[2]&0b00110000)>>4)*10);
	int date=(DATA[3]&0b00001111)+(((DATA[3]&0b00110000)>>4)*10);
	int month=(DATA[4]&0b00001111)+(((DATA[4]&0b00010000)>>4)*10);
	int year=(DATA[5]&0b00001111)+(((DATA[5]&0b11110000)>>4)*10);

	printf("%d-%d-%d %d:%d:%d\n", year+2000, month, date, hours, minutes, seconds); //wyswietlenie daty wraz z godzina

}

void WpiszDate(int fdRTC){
	//------------------------------LAB1.3-----------------------------*/
	int regSeconds=0x00; //zmienna przechowująca adres rejestru  seconds
	int regMinutes=0x01;
	int regHours=0x02;
	int regDate=0x04;
	int regMonth=0x05;
	int regYear=0x06;

	int year;
	int month;
	int date;
	int hours;
	int minutes;
	int seconds;

	printf("rok: ");
	scanf("%d", &year); //pobranie liczby z klawiatury
	printf("miesiac: ");
	scanf("%d", &month);
	printf("dzien: ");
	scanf("%d", &date);
	printf("godzina: ");
	scanf("%d", &hours);
	printf("minuty: ");
	scanf("%d", &minutes);
	printf("sekundy: ");
	scanf("%d", &seconds);

	year=year-2000;
	if(year > 10){ //podział na dziesiatki i jednosci
		int dziesiatki=year/10; 
		int jednosci = year-dziesiatki*10;

		year=jednosci+(dziesiatki<<4); // złozenie w całosc  liczby jednosci i dziesiatek
		wiringPiI2CWriteReg8(fdRTC, regYear, year);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regYear, year);
	}
	if(month > 10){
		int dziesiatki=month/10;
		int jednosci = month-dziesiatki*10;

		month=jednosci+(dziesiatki<<4); 
		wiringPiI2CWriteReg8(fdRTC, regMonth, month);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regMonth, month);
	}
	if(date > 10){
		int dziesiatki=date/10;
		int jednosci = date-dziesiatki*10;

		date=jednosci+(dziesiatki<<4);
		wiringPiI2CWriteReg8(fdRTC, regDate, date);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regDate, date);
	}
	if(hours > 10){
		int dziesiatki=hours/10;
		int jednosci = hours-dziesiatki*10;

		hours=jednosci+((dziesiatki<<4)&0b00111111);
		wiringPiI2CWriteReg8(fdRTC, regHours, hours);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regHours, hours);
	}
	if(minutes > 10){
		int dziesiatki=minutes/10;
		int jednosci = minutes-dziesiatki*10;

		minutes=jednosci+(dziesiatki<<4);
		wiringPiI2CWriteReg8(fdRTC, regMinutes, minutes);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regMinutes, minutes);
	}
	if(seconds > 10){
		int dziesiatki=seconds/10;
		int jednosci = seconds-dziesiatki*10;

		seconds=jednosci+(dziesiatki<<4);
		wiringPiI2CWriteReg8(fdRTC, regSeconds, seconds);
	}else{
		wiringPiI2CWriteReg8(fdRTC, regSeconds, seconds);
	}
}
//--------------------------------------------------EEPROM
int i2cWriteEEPROM(int fdAT, uint16_t adress, uint8_t value) {
	wiringPiI2CWriteReg16(fdAT, (adress >> 8), (value << 8) | (adress & 0xff));
	delay(20);
}

/*Funkcja odczytuje jeden bajt z podanego adresu komorki pamieci
 */
uint8_t i2cReadEeeprom(int fdAT, uint16_t adress) {
	wiringPiI2CWriteReg8(fdAT, (adress > 8), (adress & 0xff));
	delay(20);
	return wiringPiI2CRead(fdAT);
}
void eepromZapis(int fdAT){
	int value;
	printf("podaj adres rejestru do zapisu liczby: ");
	scanf("%d", &adresEEPROM); //pobranie adresu z klawiatury
	printf("podaj liczbe: ");
	scanf("%d", &value); //pobranie liczby z klawiatury

    uint8_t valueH = (value & 0xff00) >> 8; //zamiana bitów 0-7 na "0" oraz przesunięcie bitów od 8 do 15 na miejsca 0-7
    uint8_t valueL = value & 0x00ff; //zamiana bitów 8-15 na "0"
	i2cWriteEEPROM(fdAT, adresEEPROM, valueH); //zapis bitów 8-15 z liczby do EEPROM
    i2cWriteEEPROM(fdAT, adresEEPROM + 1, valueL); //zapis bitów 0-7 z liczby do EEPROM
}
void eepromOdczyt(int fdAT){
	printf("podaj adres rejestru do odczytu liczby: ");
	scanf("%d", &adresEEPROM); //pobranie adresu z klawiatury
	int valueH = (i2cReadEeeprom(fdAT, adresEEPROM) << 8); //odczyt 8 bitów i przesunięcie ich na miejsca 8-15
    int valueL = i2cReadEeeprom(fdAT, adresEEPROM + 1); //odczyt 8 bitów
	int value = valueH + valueL; //złączenie bitów 0-7 oraz 8-15 w jedną 16-bitową liczbę
	printf("Liczba: %d\n", value); //wyswietlenie liczby
}
//-------------------------------------EEPROM---------------------------
//-------------------------------------BMP280---------------------------
int init() {
short unsigned wartosc=0;
	//reset
	wartosc=0xb6;
writeRegister(BMP280_REGISTER_SOFTRESET, wartosc);
delay(1000);						   
}
float cisnienie(int32_t press){
  int64_t var1, var2, p;
	var1 = ((int64_t)t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)calib_data.dig_P6;
	var2 = var2 + ((var1 * (int64_t)calib_data.dig_P5) << 17);
	var2 = var2 + (((int64_t)calib_data.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)calib_data.dig_P3) >> 8) + ((var1 * (int64_t)calib_data.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib_data.dig_P1) >> 33;
	if(var1 == 0){
		return 0;
	}
	p = 1048576 - press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)calib_data.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)calib_data.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)calib_data.dig_P7) << 4);
  
  float cisnienie = p/25600.0;
  return cisnienie;
}
float tempBMP280(int32_t temp){
  int32_t var1, var2, T;
	var1 = ((((temp >> 3) - ((int32_t)calib_data.dig_T1 << 1))) * ((int32_t)calib_data.dig_T2)) >> 11;
	var2 = (((((temp >> 4) - ((int32_t)calib_data.dig_T1)) * ((temp >> 4) - ((int32_t)calib_data.dig_T1))) >> 12) * ((int32_t)calib_data.dig_T3)) >> 14;
	t_fine = var1 + var2;
	T = (t_fine * 5 + 128) >> 8;

  float temperatura = T/100.0;
  return temperatura;
}
int writeRegister(int adress,int data) {
	 unsigned char buff[2];
	buff[0]=(adress& ~0x80);
	buff[1]=data;
	wiringPiSPIDataRW(chan, buff, 2);
	return 1;
}
int readRegister(int adress) {
	 unsigned char buff[2];
	buff[0]=(adress | 0x80);
	wiringPiSPIDataRW(chan, buff, 2);
	return buff[1];
}
uint16_t readRegister_uint16(int adress) {
         uint8_t buff[3];
        buff[0]=(adress | 0x80);
        wiringPiSPIDataRW(chan, buff, 3);
        return (uint16_t )(buff[1]+(buff[2]<<8));
}
int16_t readRegister_int16(int adress) {
         uint8_t buff[3];
        buff[0]=(adress | 0x80);
        wiringPiSPIDataRW(chan, buff, 3);
        return (int16_t )(buff[1]+(buff[2]<<8));
}
uint32_t  readRegister_uint32(int adress) {
	
         uint8_t buff[4];
        buff[0]=(adress | 0x80);
        wiringPiSPIDataRW(chan, buff, 4);
        return (uint32_t )((buff[1]<<12)+(buff[2]<<4)+buff[3]);
}
int BMP280(int chan, int speed){
	 
    if (wiringPiSPISetup(chan, speed) == -1)
    {
        printf("Could not initialise SPI\n");
        return 0;
    }
    
	init();
	delay(600);

	writeRegister(BMP280_REGISTER_CONTROL, 0b00100111); //001 001 11 => 1x temperature oversampling, 1x preassure oversampling, mode normal
	int ctrl_meas = readRegister(BMP280_REGISTER_CONTROL);
	//printf("ctrl_meas: %x\n", ctrl_meas);

	writeRegister(BMP280_REGISTER_CONFIG, 0b00000000); //000 000 00 => t_sb = 0.5ms, filter off, 4-wire model (dla 1 -> 3-wire model)
	int config = readRegister(BMP280_REGISTER_CONFIG);
	//printf("config: %x\n", config);

		
		
			
	int id=readRegister(BMP280_REGISTER_CHIPID);
	printf("Czujnik BMP280 - ChipID: %x\r\n",id);

	calib_data.dig_T1 = readRegister_uint16(BMP280_REGISTER_DIG_T1);
	calib_data.dig_T2 = readRegister_int16(BMP280_REGISTER_DIG_T2);
	calib_data.dig_T3 = readRegister_int16(BMP280_REGISTER_DIG_T3);
	// printf("dig_T1: %d\n", calib_data.dig_T1);
	// printf("dig_T2: %d\n", calib_data.dig_T2);
	// printf("dig_T3: %d\n", calib_data.dig_T3);
	calib_data.dig_P1 = readRegister_uint16(BMP280_REGISTER_DIG_P1);
	calib_data.dig_P2 = readRegister_int16(BMP280_REGISTER_DIG_P2);
	calib_data.dig_P3 = readRegister_int16(BMP280_REGISTER_DIG_P3);
	calib_data.dig_P4 = readRegister_int16(BMP280_REGISTER_DIG_P4);
	calib_data.dig_P5 = readRegister_int16(BMP280_REGISTER_DIG_P5);
	calib_data.dig_P6 = readRegister_int16(BMP280_REGISTER_DIG_P6);
	calib_data.dig_P7 = readRegister_int16(BMP280_REGISTER_DIG_P7);
	calib_data.dig_P8 = readRegister_int16(BMP280_REGISTER_DIG_P8);
	calib_data.dig_P9 = readRegister_int16(BMP280_REGISTER_DIG_P9);
	// printf("dig_P1: %d\n", calib_data.dig_P1);
	// printf("dig_P2: %d\n", calib_data.dig_P2);
	// printf("dig_P3: %d\n", calib_data.dig_P3);
	// printf("dig_P4: %d\n", calib_data.dig_P4);
	// printf("dig_P5: %d\n", calib_data.dig_P5);
	// printf("dig_P6: %d\n", calib_data.dig_P6);
	// printf("dig_P7: %d\n", calib_data.dig_P7);
	// printf("dig_P8: %d\n", calib_data.dig_P8);
	// printf("dig_P9: %d\n", calib_data.dig_P9);

	int32_t temp = readRegister_uint32(BMP280_REGISTER_TEMPDATA);
	int32_t press = readRegister_uint32(BMP280_REGISTER_PRESSUREDATA);
	// printf("temperatura_RAW: %d\n", temp);
	// printf("cisnienie_RAW: %d\n", press);
	float tempe;
	float cis;
	
	tempe = tempBMP280(temp);
	cis = cisnienie(press);
	
	delay(3000);

	temp = readRegister_uint32(BMP280_REGISTER_TEMPDATA);
	press =readRegister_uint32(BMP280_REGISTER_PRESSUREDATA);

	tempe = tempBMP280(temp);
	printf("\ntemperatura: %f\n", tempe);

	cis = cisnienie(press);
	printf("cisnienie: %f\n", cis);

	return 0;
}
