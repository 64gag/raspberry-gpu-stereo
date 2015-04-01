#ifndef __SCCBC_H
#define __SCCBC_H

void SCCB_Init(void);
void SCCB_sendStart(void);
void SCCB_sendStop(void);
void SCCB_sendNACK(void);
uint8_t SCCB_sendByte(uint8_t data);
uint8_t SCCB_getByte(void);
uint8_t SCCB_writeSlaveRegister( uint8_t const slaveAddress, uint8_t const registerAddress, uint8_t const value);
uint8_t SCCB_readSlaveRegister(uint8_t const slaveAddress, uint8_t const registerAddress, volatile uint8_t *value);

#endif
