#define BMP085_ADDR        	0x77
#define HMC5883L_ADDR		0x1E
#define L3G4200D_ADDR		0x69
#define ADXL345_ADDR		0x53

#define PI		3.14159
#define _2PI		6.28318
#define	_180_PI		57.2958

extern char temp[32];
extern char command[4];
extern char response[10];

void inicia_sensores(void);
int compas(void);
int giroscopo(void);
int acelerometro(void);



