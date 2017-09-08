/*This file has been prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief Uart routines.
 *
 * - File:               uart.c
 * - Compiler:           IAR EWAAVR 4.11A
 * - Supported devices:  All devices with a 16 bit timer can be used.
 *                       The example is written for ATmega48
 * - AppNote:            AVR446 - Linear speed control of stepper motor
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support email: avr@atmel.com
 *
 * $Name: RELEASE_1_0 $
 * $Revision: 1.2 $
 * $RCSfile: uart.c,v $
 * $Date: 2006/05/08 12:25:58 $
 *****************************************************************************/
#include "global.h"
#include "uart.h"
#include "sm_driver.h"
#include "speed_cntr.h"

//! RX buffer for uart.
unsigned char UART_RxBuffer[UART_RX_BUFFER_SIZE];
//! RX buffer pointer.
unsigned char UART_RxPtr;

// Static Variables.
//! TX buffer for uart.
static unsigned char UART_TxBuffer[UART_TX_BUFFER_SIZE];
//! TX buffer head pointer.
static volatile unsigned char UART_TxHead;
//! TX buffer tail pointer.
static volatile unsigned char UART_TxTail;

/*! \brief Init of uart.
 *
 * Setup uart. The \ref BAUD value must be modified according to clock frqequency.
 * Refer to datasheet for details.
 *
 */
void InitUART(void)
{
  // Flush Buffers
  UART_RxPtr = 0;
  UART_TxTail = 0;
  UART_TxHead = 0;
}


/*! \brief send a byte.
 *
 *  Puts a byte in TX buffer and starts uart TX interrupt.
 *  If TX buffer is full it will hang until space.
 *
 *  \param data  Data to be sent.
 */
void uart_SendByte(unsigned char data)
{
	printf("%c", data);
	fflush(stdout);
}

/*! \brief Sends a string.
 *
 *  Loops thru a string and send each byte with uart_SendByte.
 *  If TX buffer is full it will hang until space.
 *
 *  \param Str  String to be sent.
 */
void uart_SendString(unsigned char Str[])
{
  unsigned char n = 0;
  while(Str[n])
    uart_SendByte(Str[n++]);
}

/*! \brief Sends a integer.
 *
 *  Converts a integer to ASCII and sends it using uart_SendByte.
 *  If TX buffer is full it will hang until space.
 *
 *  \param x  Integer to be sent.
 */
void uart_SendInt(int x)
{
  static const char dec[] = "0123456789";
  unsigned int div_val = 10000;

  if (x < 0){
    x = - x;
    uart_SendByte('-');
  }
  while (div_val > 1 && div_val > x)
    div_val /= 10;
  do{
    uart_SendByte (dec[x / div_val]);
    x %= div_val;
    div_val /= 10;
  }while(div_val);
}

/*! \brief Empties the uart RX buffer.
 *
 *  Empties the uart RX buffer.
 *
 *  \return x  Integer to be sent.
 */
void uart_FlushRxBuffer(void){
  UART_RxPtr = 0;
  UART_RxBuffer[UART_RxPtr] = 0;
}

/*! \brief RX interrupt handler.
 *
 *  RX interrupt handler.
 *  RX interrupt always enabled.
 */
void UART_RX_interrupt( void )
{
  unsigned char data;

  // Read the received data.
  data = UDR0;

  if(status.running == FALSE){
    // If backspace.
    if(data == '\b')
    {
      if(UART_RxPtr)
      // Done if not at beginning of buffer.
      {
        uart_SendByte('\b');
        uart_SendByte(' ');
        uart_SendByte('\b');
        UART_RxPtr--;
        UART_RxBuffer[UART_RxPtr]=0x00;
      }
    }
    // Not backspace.
    else
    {
      // Put the data into RxBuf
      // and place 0x00 after it. If buffer is full,
      // data is written to UART_RX_BUFFER_SIZE - 1.
      if(UART_RxPtr < (UART_RX_BUFFER_SIZE - 1)){
        UART_RxBuffer[UART_RxPtr] = data;
        UART_RxBuffer[UART_RxPtr + 1]=0x00;
        UART_RxPtr++;
      }
      else
      {
        UART_RxBuffer[UART_RxPtr - 1] = data;
        uart_SendByte('\b');
      }
      // If enter.
      if(data == 13){
        status.cmd = TRUE;
      }
      else
        uart_SendByte(data);
    }
  }
}

/*! \brief TX interrupt handler.
 *
 *  TX interrupt handler.
 *  TX interrupt turned on by uart_SendByte,
 *  turned off when TX buffer is empty.
 */
void UART_TX_interrupt( void )
{
  unsigned char UART_TxTail_tmp;
   UART_TxTail_tmp = UART_TxTail;

  // Check if all data is transmitted
  if ( UART_TxHead !=  UART_TxTail_tmp )
  {
    // Calculate buffer index
    UART_TxTail_tmp = ( UART_TxTail + 1 ) & UART_TX_BUFFER_MASK;
    // Store new index
    UART_TxTail =  UART_TxTail_tmp;
    // Start transmition
    UDR0= UART_TxBuffer[ UART_TxTail_tmp];
  }
  else
    // Disable UDRE interrupt
    CLR_UDRIE;
}
