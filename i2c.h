#ifndef I2C_H_
#define I2C_H_

//extern unsigned char *PTxData;                     	// Pointer to TX data
//extern unsigned char *PRxData;                     	// Pointer to RX data

//extern unsigned char TXByteCtr;
//extern unsigned char RXByteCtr;

extern char command[4];
extern char response[10];

unsigned char *PTxData;                     	// Pointer to TX data
unsigned char *PRxData;                     	// Pointer to RX data

unsigned char TXByteCtr;
unsigned char RXByteCtr;


void iic_tx_init(unsigned char address);
void iic_rx_init(void);
void sendBytes(unsigned char address, int bytes_to_tx);
void getBytes(unsigned char address, int bytes_to_rx);
void start_TX(void);
void start_RX(void);

#endif
