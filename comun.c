#include <msp430g2553.h>
#include <string.h>
#include "comun.h"


//*****************************************************************************
//
// Inicialización del sistema
//*****************************************************************************

void init( void ) {

	// Puerto serie : 16mhz/9600=683

	UCA0CTL1 |= UCSWRST;
	UCA0CTL1 = UCSSEL_2 + UCSWRST;
	UCA0BR0 = 83;
	UCA0BR1 = 6;
	UCA0CTL1 &= ~UCSWRST;


	// I2C 100 kHz Master

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
          
	/* Habilita funciones puerto 1: RX,TX,ADC CH5,SDA,SCL */
	P1SEL = BIT1 + BIT2 + BIT5 + BIT6 + BIT7;
	/* Habilita funciones puerto 1: RX,TX,ADC CH5,SDA,SCL */
	P1SEL2 = BIT1 + BIT2 + BIT6 + BIT7;

	/* Puertos P1.0 y 4 de salida */
	
	P1DIR |= BIT0 + BIT4 ;
	/* Todos los puertos P2 de salida*/
		
	P2DIR |= BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5;      
	
	/* Configura módulo ADC10 para puerto P1.5 */

	ADC10CTL1 = INCH_5 + ADC10DIV_3 ;        				// Channel 5, ADC10CLK/4
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;    //Vcc & Vss as reference
	ADC10AE0 |= BIT5;                        				//P1.5 ADC option  

	IE2 |= UCA0RXIE + ADC10IE;

}

//*****************************************************************************
//
// Funciones de delay por software
//*****************************************************************************


void delay(unsigned long d) {
  int i;
  for (i = 0; i<d; i++) {
    nop();
  }
}

//*****************************************************************************
//
// Funciones de delay por interrupción
//*****************************************************************************

void delay_timer(int tiempo)
{

	/* Configura el timer0 */
	
	 /* 
     * TA0CCTL0, Capture/Compare Control Register 0
     * 
     * CM_0 -- No Capture
     * CCIS_0 -- CCIxA
     * ~SCS -- Asynchronous Capture
     * ~SCCI -- Latched capture signal (read)
     * ~CAP -- Compare mode
     * OUTMOD_0 -- PWM output mode: 0 - OUT bit value
     * 
     * Note: ~<BIT> indicates that <BIT> has value zero
     */
    TA0CCTL0 = CM_0 + CCIS_0 + OUTMOD_0 + CCIE;

    /* TA0CCR0, Timer_A Capture/Compare Register 0 */
    TA0CCR0 = tiempo;

    /* 
     * TA0CTL, Timer_A3 Control Register
     * 
     * TASSEL_1 -- ACLK
     * ID_0 -- Divider - /1
     * MC_1 -- Up Mode
     */
    TA0CTL = TASSEL_1 + ID_0 + MC_1;

	__bis_SR_register(LPM0_bits + GIE);
	__no_operation(); 

	
}


//*****************************************************************************
//
// Funciones básicas de motores L298
//*****************************************************************************

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

//*****************************************************************************
//
// Movimientos elementales
//*****************************************************************************


void avanza(long tiempo) {
	movimiento.avanza=TRUE;
	movimiento.atras=FALSE;
	arranca_motores();
	LmotorFWD();
	RmotorFWD();
	delay_timer(tiempo);			
	para_motores();
}

void atras(long tiempo) {
	movimiento.atras=TRUE;
	movimiento.avanza=FALSE;
	arranca_motores();
	LmotorREV();
	RmotorREV();
	delay_timer(tiempo);			
	para_motores();
}

void derecha(long tiempo) {
	movimiento.derecha=TRUE;
	movimiento.izquierda=FALSE;
	arranca_motores();
	LmotorREV();
	RmotorFWD();
	delay_timer(tiempo);		
	para_motores();
}

void izquierda(long tiempo) {
	movimiento.izquierda=TRUE;
	movimiento.derecha=FALSE;
	arranca_motores();
	LmotorFWD();
	RmotorREV();
	delay_timer(tiempo);		
	para_motores();
}

void frena(void) {

	RmotorBRAKE();
	LmotorBRAKE();

}
//*****************************************************************************
//
// Envía char string por puerto serie
//*****************************************************************************

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



