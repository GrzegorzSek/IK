#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <softPwm.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

int main (void)
{
	printf("Praca studentow: 241778, 236032 - 08.05.2020\n");

int fd;
int adressMPU6050=0x68;

if(wiringPiSetup() == -1)
exit(1);

if((fd=wiringPiI2CSetup(adressMPU6050)) == -1){
	printf("error initialize I2C");
	exit(1);
}
printf("I2C modul MPU6050\r\n");

//Uruchamia pomiary
int regPWR=0x6B; //<<---- podac adres rejestru PWR_MGMT_1
wiringPiI2CWriteReg8(fd, regPWR, 0);

//------------------------------------------ODCZYT REJESTRU WHO_AM_I-------------------------------
int regWhoValue; //zmienna przechowująca adres rejestru WHO_AM_I
int regWho=0x75; //<<---- podac adres rejestru Who Am I


regWhoValue = wiringPiI2CReadReg8(fd,regWho); //Odczyt danych (8 bitów) z rejestru WHO_AM_I

printf("I am: %d\r\n", regWhoValue); //wyświetlenie wartości odczytanej z rejestru WHO_AM_I
printf("\n");

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
printf("Praca studentow: 241778, 236032 - 08.05.2020\n");
printf("-----TEMPERATURA-----\r\n");
printf("Temperatura: %f\r\n", temperatura);
printf("\n");
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

printf("Praca studentow: 241778, 236032 - 08.05.2020\n");
printf("-----AKCELEROMETR-----\r\n");
printf("dane dla osi OX: %f\r\n", accX); //Wyświetlenie danych uzyskanych z akcelerometru
printf("dane dla osi OY: %f\r\n", accY);
printf("dane dla osi OZ: %f\r\n", accZ);
printf("\n");
 
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

printf("Praca studentow: 241778, 236032 - 08.05.2020\n");
printf("-----ZYROSKOP-----\r\n");
printf("wpolrzedne osi OX: %f\r\n", gyroX); //Wyświetlenie danych uzyskanych z żyroskopu
printf("wpolrzedne osi OY: %f\r\n", gyroY);
printf("wpolrzedne osi OZ: %f\r\n", gyroZ);
printf("\n");

//------------------------------------------POZIOMICA-------------------------------
float poziom = atan2(acc_x, acc_z) * (180 / M_PI); //obliczenie poziomu (wzór z Laboratorium 1.2)

printf("Praca studentow: 241778, 236032 - 08.05.2020\n");
printf("-----POZIOMICA-----\r\n");
printf("POZIOM: %f\r\n", poziom); //Wyświetlenie poziomu

return 0;

}
