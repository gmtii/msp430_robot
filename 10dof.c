#include <msp430g2553.h>
#include <math.h>
#include <stdio.h>
#include "comun.h"
#include "i2c.h"
#include "pid.h"
#include "10dof.h"



//*****************************************************************************
//
//
//*****************************************************************************


void inicia_sensores(void) {

	sprintf ( command , "%c%c", 0x02, 0x00);
	sendBytes(HMC5883L_ADDR,2);

	sprintf ( command , "%c%c", 0x20, 0x0F);
	sendBytes(L3G4200D_ADDR,2);
	sprintf ( command , "%c%c", 0x24, 0x02);
	sendBytes(L3G4200D_ADDR,2);

	sprintf ( command , "%c%c", 0x31, 0x01);
	sendBytes(ADXL345_ADDR,2);
	sprintf ( command , "%c%c", 0x2D, 0x08);
	sendBytes(ADXL345_ADDR,2);

}

int compas(void) {


  int x,y; // ,z; //triple axis data
  float heading;

	sprintf ( command , "%c", 0x03);
	sendBytes(HMC5883L_ADDR,1);
	delay(1000);
	getBytes(HMC5883L_ADDR,6);


	x =  ((int)response[0]<<8) | response[1]; 	
	//z =  ((int)response[2]<<8) | response[3];	
	y =  ((int)response[4]<<8) | response[5]; 	


	//sprintf(temp, "X:%i Y:%i Z:%i", x,y,z);
	//TXString(temp);

	heading= atan2f((float)y,(float)x);
        
        if(heading < 0)
		heading += _2PI;
	else if(heading > _2PI)
    		heading -= _2PI;

	heading *= _180_PI;				// Convierte a grados

	if (modo.debug)
	{
	sprintf(temp,"%c[10;0H", ASCII_ESC);
	TXString(temp);
	sprintf(temp, "compas: %i", (int)heading);
	TXString(temp);
	}
	return (int)heading;

}

//*****************************************************************************
//
//
//*****************************************************************************

int giroscopo(void) {

	int x=0,y=0,z=0;

	sprintf ( command , "%c", 0x29);
	sendBytes(L3G4200D_ADDR,1);
	delay(1000);
	getBytes(L3G4200D_ADDR,6);
        
        x = (int) response[1] << 8 | response[0];
	y = (int) response[3] << 8 | response[2];
 	z = (int) response[5] << 8 | response[4];

	if (modo.debug)
	{
	sprintf(temp,"%c[4;0H", ASCII_ESC);
	TXString(temp);
	sprintf(temp, "Gyro: X: %i      Y: %i      Z: %i      ", x,y,z);
	TXString(temp);
	}
	return x;

}

int acelerometro(void) {

	int x=0,y=0,z=0;

	sprintf ( command , "%c", 0x32);
	sendBytes(ADXL345_ADDR,1);
	delay(1000);
	getBytes(ADXL345_ADDR,6);
        
        x = (int) response[1] << 8 | response[0];
	y = (int) response[3] << 8 | response[2];
 	z = (int) response[5] << 8 | response[4];

	if (modo.debug)
	{
	sprintf(temp,"%c[8;0H", ASCII_ESC);
	TXString(temp);
	sprintf(temp, "Acel: X: %i      Y: %i      Z: %i       ", x,y,z);
	TXString(temp);
	}
	
	if (x<7 && x>-7)
	return 0;
	else
	return 1;

}
