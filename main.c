//*****************************************************************************
//
//
//*****************************************************************************

#include <msp430g2553.h>
#include "stdio.h"
#include "comun.h"
#include "i2c.h"
#include "pid.h"
#include "10dof.h"
 
#define inercia		500000
#define inercia_giro	250000


//*****************************************************************************
//
//
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
//
//*****************************************************************************

int compas(void);

//*****************************************************************************
// 1 si hay obstáculo
//
//*****************************************************************************


int vision(void) {

	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(LPM0_bits + GIE);        // LPM0 with interrupts enabled
	ambiente = ADC10MEM;

	P1OUT |= BIT4;				// Encendemos LEDs IR
	delay(1000);				// Estabilizando...
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
	__bis_SR_register(LPM0_bits + GIE);        // LPM0 with interrupts enabled
	P1OUT &= ~BIT4;				// Apagamos LEDs IR
	lectura = ADC10MEM;
	distancia = lectura - ambiente;
	//sprintf(temp, "Distancia: %i, Lectura: %i, Ambiente: %i", distancia,lectura,ambiente);
	//TXString(temp); 

	if ( distancia < 275)
	return 1;

	return 0;
}		

//*****************************************************************************
// 1 si hay obstáculo
//
//*****************************************************************************

void autonomo(void) {

int cuenta=0;
int bloqueo=0;
int punto1;
int punto2;
int cambio;



	while (modo.autonomo==TRUE) {


		P1OUT &= ~BIT0;
		punto1=compas();
		avanza(75000);	

		if ( vision () ) 
			{
			bloqueo=0;
			P1OUT |= BIT0;
			atras(999999);
				if (cuenta<4) 
				{
				izquierda(500000);
				cuenta++; 
				}
				else 
				{
				derecha(500000);
				cuenta++;
					if (cuenta>6)
					cuenta=0;
				}
			}


		delay(25000);

		punto2=compas();
		cambio=punto2-punto1;

			if (cambio==0 && acelerometro()==0  )
			bloqueo++;

		if (bloqueo>15)
			{
			P1OUT |= BIT0;
			bloqueo=0;
			atras(999999);

				if (movimiento.izquierda==TRUE) 
				derecha(999999);
				else
				izquierda(999999);
			}
		
			
		}			
	
}



//*****************************************************************************
//
//
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
		derecha(100000);
	
		if (valor_pid >0)
		izquierda(100000);

		para_motores();

		error=compas()-fin;

		sprintf(temp, "%i;%i;%i;%i", fin,actual,valor_pid,error);
		TXString(temp);
		delay(50000);
	}

	TXString("Fin PID...");
	para_motores();
	frena();
	   

}


//*****************************************************************************
//
//
//*****************************************************************************


int main(void)
{
	WDTCTL =  WDTPW + WDTHOLD;    // Stop watchdog timer

	BCSCTL1 = CALBC1_16MHZ;       /* Set DCO to 16MHz */
	DCOCTL =  CALDCO_16MHZ;

	sprintf(temp,"%c[2J", ASCII_ESC);
	TXString(temp);
	sprintf(temp,"%c[H", ASCII_ESC);
	TXString(temp);

	init();
	TXString("Hola mundo! - ROBOTIJO MSP430G2553 10DOF CUTRE");
	inicia_sensores();
	TXString("Sensores I2C Ok!");

	para_motores();
	frena();
		
	while(1)
	{
	__bis_SR_register(LPM0_bits + GIE);
    	__no_operation(); 

		P1OUT |= BIT0;

			if (recibido == 65)					
				avanza(inercia);
			else if (recibido == 66)
				atras(inercia);
			else if (recibido == 68)
				izquierda(inercia_giro);
			else if (recibido == 67)
				derecha(inercia_giro);
			else if	(recibido == 32)
				frena();
			else if (recibido == 'r') {
				sprintf(temp, "Compas: %i", compas());
				TXString(temp); 
				}

			else if (recibido == 'o')
				orienta();

			else if (recibido == 's')
				modo.autonomo==FALSE;
	
			else if (recibido == 'd') {
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

			else if (recibido == 'a') {
				modo.autonomo=TRUE;
				autonomo();
				}	
			
				
		P1OUT &= ~BIT0;
	}

	return 0;


}

//*****************************************************************************
//
//
//*****************************************************************************

// This handler runs when Switch S2 is pressed.
__attribute__((interrupt(PORT1_VECTOR))) 
void Port_1(void)
{
                            
	TXString("* * * S2 * * * ");
	//para_motores();
	delay(500000);
	recibido='a';
	modo.autonomo=TRUE;
	P1IFG &= ~BIT3; 

		__bic_SR_register_on_exit(LPM0_bits);
}

//*****************************************************************************
//
//
//*****************************************************************************

__attribute__((interrupt(USCIAB0RX_VECTOR))) 
void USCI0RX_ISR (void)
{
	recibido = UCA0RXBUF;                    

	//while (!(IFG2&UCA0TXIFG));                
	//UCA0TXBUF = recibido;  

	__bic_SR_register_on_exit(LPM0_bits);


}

//*****************************************************************************
//
//
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
//
//*****************************************************************************

__attribute__((interrupt(ADC10_VECTOR)))
void ADC10_ISR (void)
{
		__bic_SR_register_on_exit(LPM0_bits);        // Return to active mode }
}
