#ifndef COMUN_H_
#define COMUN_H_

#define ENABLE_A	BIT0
#define IN1		BIT1
#define IN2		BIT2

#define ENABLE_B	BIT3
#define IN3		BIT4
#define IN4		BIT5

#define bool unsigned char
#define TRUE 1
#define FALSE 0

#define ASCII_ESC 27



struct
{
  bool avanza :1;
  bool atras :1;
  bool derecha :1;
  bool izquierda :1;
} movimiento;

struct
{
  bool autonomo :1;
  bool debug :1;
} modo;

void delay(unsigned long d);
void delay_timer(int tiempo);
void TXString( char* string );

void LmotorON(void);    
void LmotorFWD(void);    
void LmotorREV(void);    
void LmotorOFF(void);    

void RmotorON(void);    
void RmotorFWD(void);    
void RmotorREV(void);    
void RmotorOFF(void); 

void init( void );
void arranca_motores(void);
void avanza(long tiempo);
void atras(long tiempo);
void derecha(long tiempo);
void izquierda(long tiempo);
void para_motores(void);

void frena(void);
void RmotorBRAKE(void);
void LmotorBRAKE(void);

#endif
