//*****************************************************************************
// Robot cutre con Launchpad MSP430g2553
// L298 en P2
// Dos diodos IR y un diodo receptor en P1.4 VCC P1.5 VIN
// 10DOF I2c
// Módulo bluetooth HC06 en RX/TX 9600 8N1
// Dos motores DC de una impresora de impacto
//
//*****************************************************************************

#include <msp430g2553.h>
#include "stdio.h"
#include "comun.h"
#include "i2c.h"
#include "pid.h"
#include "10dof.h"

#define s1		32768		// 1 s.
#define ms750		(s1/4)*3	// 750 ms.
#define ms500		s1/2 		// 500 ms.
#define inercia		s1/4		// 250 ms.
#define inercia_giro	s1/8		// 125 ms.
#define inercia_autonomo	s1/16	// 62.5 ms.




//*****************************************************************************
//
// Definición de variables
//*****************************************************************************

unsigned char recibido;


char command[4];
char response[10];

char temp[32];

int lectura=0;
int ambiente=0;
int distancia=0;

int estado=0;

//*****************************************************************************
//
// Definición de funciones
//*****************************************************************************

int compas(void);

//*****************************************************************************
// 
// Devuelve un 1 si encuentra un obstáculo
//*****************************************************************************


int vision(void) {

	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(LPM0_bits + GIE);     // LPM0 with interrupts enabled
	ambiente = ADC10MEM;

	P1OUT |= BIT4;				// Encendemos LEDs IR
	delay_timer(250);			// Estabilizando alimentación.
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(LPM0_bits + GIE);     // LPM0 with interrupts enabled
	P1OUT &= ~BIT4;				// Apagamos LEDs IR
	lectura = ADC10MEM;
	distancia = lectura - ambiente;
	//sprintf(temp, "Distancia: %i, Lectura: %i, Ambiente: %i", distancia,lectura,ambiente);
	//TXString(temp); 

	if ( distancia < 275)			// 275 calibrado a mano en interiores.
	return 1;

	return 0;
}		

//*****************************************************************************
// 
// El robot se mueve sólo comprobando si hay obstáculos mediante IR y 
// emplenado el compás y el acelerómetro para comprobar si se atasca.
//*****************************************************************************

void autonomo(void) {

int cuenta=0;
int bloqueo=0;
int punto1;
int punto2;
int cambio;

	while (modo.autonomo==TRUE) {


		P1OUT &= ~BIT0;
		punto1=compas();				// marca el punto inicial
		avanza(inercia_autonomo);			// avanza un poco

		if ( vision () ) 				// hay obstáculo?
			{
			bloqueo=0;				// reinicia contador
			P1OUT |= BIT0;				// enciende LED
			atras(ms500);				// marcha atras 500ms
				if (cuenta<4) 			// he girado <4 veces a la izquierda?
				{
				izquierda(inercia);		// sigo girando		
				cuenta++; 			
				}
				else 				// ya he rotado y no hay salida
				{
				derecha(inercia);		// giro al lado contrario 4 veces más
				cuenta++;
					if (cuenta>8)
					cuenta=0;		// reiniciamos cuenta
				}
			}
		//else
		delay_timer(1000);				// retraso para permitir desplazamiento

		punto2=compas();
		cambio=punto2-punto1;				// marco punto2
	
			if (cambio==0 && acelerometro()==0  )	// si no ha variado el rumbo y el acel=0 estamos atascados
			bloqueo++;				

		if (bloqueo>5)					// 8 veces bloqueo, tenemos que salir
			{
			P1OUT |= BIT0;
			bloqueo=0;	
			atras(inercia);				// atras fuerte 500ms.

				if (movimiento.izquierda==TRUE) 
				derecha(ms500);			// giro completo si veníamos de la izquierda a la derecha
				else
				izquierda(ms500);		// si veníamos de la derecha, giro a la izquierda
			}
		
			
		}			
	
}



//*****************************************************************************
//
// Orienta a la posición fin mediante PID
//*****************************************************************************

void orienta(void)
{

double K_P = 3; 
double K_I = 0; 
double K_D = 1; 

int fin = 360;
int actual = compas(); 
int valor_pid;
int error=100;

struct PID_DATA pidData1; 


	TXString("Iniciando PID...");
	TXString("valor_fin;valor_pid;valor_compas;error");

   	pid_Init(K_P, K_I, K_D, &pidData1); 

	while ( error > 10 || error < -10)  {

		actual=compas();
		
		valor_pid = pid_Controller(fin, actual, &pidData1); 
	
		if (valor_pid <0) 
		derecha(inercia);
	
		if (valor_pid >0)
		izquierda(inercia);

		para_motores();

		error=compas()-fin;

		sprintf(temp, "%i;%i;%i;%i", fin,actual,valor_pid,error);
		TXString(temp);
		delay_timer(1000);
	}

	TXString("Fin PID...");
	para_motores();
	frena();
	   

}


//*****************************************************************************
//
// Función principal
//*****************************************************************************


int main(void)
{
	WDTCTL =  WDTPW + WDTHOLD;    // Stop watchdog timer

	BCSCTL1 = CALBC1_16MHZ;       /* Set DCO to 16MHz */
	DCOCTL =  CALDCO_16MHZ;

	init();

	sprintf(temp,"%c[2J", ASCII_ESC);
	TXString(temp);
	sprintf(temp,"%c[H", ASCII_ESC);
	TXString(temp);
	TXString("Hola mundo! - ROBOTIJO MSP430G2553 10DOF CUTRE");
	inicia_sensores();
	TXString("Sensores I2C Ok!");

	para_motores();
	frena();
	
	while(1)
	{


		P1OUT |= BIT0;

			if (recibido == 65)					// AVANZA				
				avanza(ms500);
			else if (recibido == 66)				// ATRAS
				atras(ms500);
			else if (recibido == 68)				// IZQUIERDA
				izquierda(inercia_giro);
			else if (recibido == 67)				// DERECHA
				derecha(inercia_giro);				
			else if	(recibido == 32)				// DESACTIVA MOTORES
				frena();
			else if (recibido == 'r') {				// VER RUMBO
				sprintf(temp, "Compas: %i", compas());
				TXString(temp); 
				}

			else if (recibido == 'o')				// Orienta hacia heading_fin mediante PID
				orienta();

			else if (recibido == 's')				// desactiva el modo autónomo
				modo.autonomo==FALSE;
	
			else if (recibido == 'd') {				// modo debug sensores
				modo.debug=TRUE;
				sprintf(temp,"%c[2J", ASCII_ESC);
				TXString(temp);
				sprintf(temp,"%c[H", ASCII_ESC);
				TXString(temp);
				giroscopo();
				acelerometro();
				compas();
				modo.debug=FALSE;
				}

			else if (recibido == 'a') {				// modo autónomo
				modo.autonomo=TRUE;
				autonomo();
				}	
			
				
		P1OUT &= ~BIT0;

	__bis_SR_register(LPM0_bits + GIE);
    	__no_operation(); 
	}


	return 0;


}

//*****************************************************************************
//
// Pulsación S2
//*****************************************************************************

// This handler runs when Switch S2 is pressed.
__attribute__((interrupt(PORT1_VECTOR))) 
void Port_1(void)
{
                            
	TXString("* * * S2 * * * ");
	para_motores();
	delay(500000);					// delay por software
	recibido='a';					// selecciona modo autónomo
	modo.autonomo=TRUE;				// habilita modo
	P1IFG &= ~BIT3; 				// limpia IF

	__bic_SR_register_on_exit(LPM0_bits);		// Activa CPU y vuelve a modo activo
}

//*****************************************************************************
//
// Recepción de caracteres por bluetooth
//*****************************************************************************

__attribute__((interrupt(USCIAB0RX_VECTOR))) 
void USCI0RX_ISR (void)
{
	recibido = UCA0RXBUF;                    

	//while (!(IFG2&UCA0TXIFG));                
	//UCA0TXBUF = recibido;  

	__bic_SR_register_on_exit(LPM0_bits);		// Activa CPU y vuelve a modo activo


}

//*****************************************************************************
//
// I2C RX/TX
//*****************************************************************************

__attribute__((interrupt(USCIAB0TX_VECTOR)))
void USCI0TX_ISR (void)
{
  
   if (IFG2 & UCB0TXIFG) 
    {
		/* USCI_B0 Transmit Interrupt Handler */
              if (TXByteCtr)                          // Check TX byte counter
              {
                UCB0TXBUF = *PTxData++;               // Load TX buffer
                TXByteCtr--;                          // Decrement TX byte counter
              }
              else
              {
                UCB0CTL1 |= UCTXSTP;                  // I2C stop condition
                IFG2 &= ~UCB0TXIFG;                   // Clear USCI_B0 TX int flag
                /* Enter active mode on exit */
                __bic_SR_register_on_exit(LPM0_bits);
              }  


    }
    else 
    {
              /* USCI_B0 Receive Interrupt Handler */
              RXByteCtr--;                            // Decrement RX byte counter
              if (RXByteCtr)
              {
                //P1OUT ^= 0x10;
                *PRxData++ = UCB0RXBUF;               // Move RX data to address PRxData
                if (RXByteCtr == 1)                   // Only one byte left?
                {
                  UCB0CTL1 |= UCTXSTP;                // Generate I2C stop condition
                }
              }
              else
              {
                 *PRxData = UCB0RXBUF;                 // Move final RX data to PRxData
                 /* Enter active mode on exit */
                 
                 __bic_SR_register_on_exit(LPM0_bits);
              }
              



    }
}

//*****************************************************************************
//
// ADC10 
//*****************************************************************************

__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR (void)
{
	__bic_SR_register_on_exit(LPM0_bits);	// Activa CPU y vuelve a modo activo
}


//*****************************************************************************
//
// Timer
//*****************************************************************************

__attribute__((interrupt(TIMER0_A0_VECTOR)))
void TIMER0_A0_ISR(void)
{

	P1OUT ^= BIT0;				// XOR BIT0
	TA0CCTL0 &= ~CCIE;			// Deshabilita interrupcción por CCIE
	__bic_SR_register_on_exit(LPM0_bits);   // Activa CPU y vuelve a modo activo

}
