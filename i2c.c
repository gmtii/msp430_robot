#include <msp430g2553.h>
#include "i2c.h"

void iic_tx_init(unsigned char address)
{
    UCB0I2CSA = address;							// Pone la dirección del esclavo en el bus
    IE2  |= UCB0TXIE;                       	// Enable TX interrupt
}

void iic_rx_init(void)
{
    IE2  |= UCB0RXIE;							// enable RX interrupt	
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// send 1 byte, return 2 bytes - needs the return expanded for X bytes
void sendBytes(unsigned char address, int bytes_to_tx)
{

    // transmit slave address and register to read
    iic_tx_init(address);

    __delay_cycles(1000);

    PTxData = (unsigned char *)command;     // TX array start address

    TXByteCtr = bytes_to_tx;             // Load TX byte counter

    start_TX();

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void getBytes(unsigned char address, int bytes_to_rx)
{
  
    iic_tx_init(address);

    // receive requested bytes
    iic_rx_init();								// set RX interrupt
    
    __delay_cycles(1000);
    
    PRxData = (unsigned char *)response;    	// rx buffer
    
    RXByteCtr = bytes_to_rx;     				// number of bytes to receive
    
    start_RX();
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// iic start transmitting
void start_TX(void)
{
    UCB0CTL1 |= UCTR + UCTXSTT;             	// I2C TX, start condition
    
    __bis_SR_register(LPM0_bits + GIE);     	// Enter LPM0, enable interrupts
    __no_operation();                       	// Remain in LPM0 until all data
                                            	// is TX'd
    while (UCB0CTL1 & UCTXSTP);             	// Ensure stop condition got sent
 
}
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// iic restart and receive
void start_RX(void)
{
    while (UCB0CTL1 & UCTXSTP);             	// wait for stop
    
    UCB0CTL1 &= ~UCTR;                      	// restart, set as receiver
    UCB0CTL1 |= UCTXSTT;                    	// start condition
    
    __bis_SR_register(LPM0_bits + GIE);
    __no_operation();                       	// Remain in LPM0 until all data
    
    while (UCB0CTL1 & UCTXSTP);
}
