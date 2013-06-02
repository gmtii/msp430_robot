#include <msp430g2553.h>
#include <string.h>
#include "comun.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void init( void ) {

	// Serial

	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2 + UCSWRST;
	UCA0BR0 = 83;
	UCA0BR1 = 6;
	UCA0CTL1 &= ~UCSWRST;


	// I2C

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;
	UCB0CTL1 = UCSSEL_2 + UCSWRST;
	UCB0I2COA = 0x1E;
	UCB0BR0 = 160;
	UCB0BR1 = 0;
	UCB0CTL1 &= ~UCSWRST;

	P1DIR &= ~BIT3;                     // P1.3 is an input pin
	P1REN |= BIT3;
	P1IE |= BIT3;                       // Switch S2 triggers an interrupt
	P1IES |= BIT3;                      // on falling edge
          
	/* Port 1 Port Select Register */
	P1SEL = BIT1 + BIT2 + BIT5 + BIT6 + BIT7;
	/* Port 1 Port Select 2 Register */
	P1SEL2 = BIT1 + BIT2 + BIT6 + BIT7;

	P1DIR |= BIT0 + BIT4 ;                
	P2DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;      

	ADC10CTL1 = INCH_5 + ADC10DIV_3 ;        		// Channel 5, ADC10CLK/4
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;    //Vcc & Vss as reference
	ADC10AE0 |= BIT5;                        		//P1.5 ADC option  

	IE2 |= UCA0RXIE + ADC10IE;







}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void delay(unsigned long d) {
  int i;
  for (i = 0; i<d; i++) {
    nop();
  }
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LmotorON(void) {

	P2OUT |= ENABLE_A;
}

void LmotorFWD(void) {
	P2OUT |= IN1;
	P2OUT &= ~IN2;
}

void LmotorREV(void) {
	P2OUT &= ~IN1;
	P2OUT |= IN2;
}

void LmotorBRAKE(void) {
	P2OUT &= ~IN1;
	P2OUT &= ~IN2;
}

void LmotorOFF(void) {
	P2OUT &= ~ENABLE_A;
}


void RmotorON(void) {
	P2OUT |= ENABLE_B;
}

void RmotorFWD(void) {
	P2OUT |= IN3;
	P2OUT &= ~IN4;
}

void RmotorREV(void) {
	P2OUT &= ~IN3;
	P2OUT |= IN4;
}

void RmotorBRAKE(void) {
	P2OUT &= ~IN3;
	P2OUT &= ~IN4;
}

void RmotorOFF(void) {
	P2OUT &= ~ENABLE_B;
}  

void arranca_motores(void) {
	LmotorON();
	RmotorON();
}

void para_motores(void) {
	LmotorOFF();
	RmotorOFF();
}




void avanza(long tiempo) {
	movimiento.avanza=TRUE;
	movimiento.atras=FALSE;
	arranca_motores();
	LmotorFWD();
	RmotorFWD();
	delay(tiempo);
	para_motores();
}

void atras(long tiempo) {
	movimiento.atras=TRUE;
	movimiento.avanza=FALSE;
	arranca_motores();
	LmotorREV();
	RmotorREV();
	delay(tiempo);
	para_motores();
}

void derecha(long tiempo) {
	movimiento.derecha=TRUE;
	movimiento.izquierda=FALSE;
	arranca_motores();
	LmotorREV();
	RmotorFWD();
	delay(tiempo);
	para_motores();
}

void izquierda(long tiempo) {
	movimiento.izquierda=TRUE;
	movimiento.derecha=FALSE;
	arranca_motores();
	LmotorFWD();
	RmotorREV();
	delay(tiempo);
	para_motores();
}

void frena(void) {

	RmotorBRAKE();
	LmotorBRAKE();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TXString( char* string )
{
  
  while (*string!= '\0')
  {
    UCA0TXBUF = *string;
    string++;
    while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
  }
	UCA0TXBUF = 13;
	while (!(IFG2&UCA0TXIFG));
	UCA0TXBUF = 10;
	while (!(IFG2&UCA0TXIFG));
	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
     int i, sign;
 
     if ((sign = n) < 0)  /* record sign */
         n = -n;          /* make n positive */
     i = 0;
     do {       /* generate digits in reverse order */
         s[i++] = n % 10 + '0';   /* get next digit */
     } while ((n /= 10) > 0);     /* delete it */
     if (sign < 0)
         s[i++] = '-';
     s[i] = '\0';
     reverse(s);
}
 
 /* reverse:  reverse string s in place */

void reverse(char s[])
 {
     int i, j;
     char c;
 
     for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }
 }

