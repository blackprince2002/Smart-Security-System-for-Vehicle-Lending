#ifndef SERIAL_KEIL_SPI_H_
#define SERIAL_KEIL_SPI_H_

#include "pin_shield_1.h"     //shield pin macros e.g. A2_PORT, PIN_OUTPUT()

// This file could cope with 
// ILI9341  8-bit 5-wire SPI 
// ILI9481  9-bit 3-wire SPI - HWSPI w STM32 else SPI + bit-bang
// ST7735S  8-bit 4-wire SPI - HWSPI w STM32 else SPI + bit-bang
// HX8347D  8-bit 4-wire SPI - no MISO func.  conditional SD,XD pins
// ILI9225  8-bit 4-wire SPI - bit-bang.  conditional all pins

// control pins as used in MCUFRIEND shields 
  #define CD_PORT D9_PORT
  #define CD_PIN  D9_PIN
  #define CS_PORT D10_PORT
  #define CS_PIN  D10_PIN
  #define RESET_PORT D8_PORT
  #define RESET_PIN  D8_PIN
  #define SD_PORT D4_PORT
  #define SD_PIN  D4_PIN
  #define MOSI_PORT D11_PORT
  #define MOSI_PIN  D11_PIN
  #define SCK_PORT D13_PORT
  #define SCK_PIN  D13_PIN

// general purpose pin macros
 #define CD_COMMAND PIN_LOW(CD_PORT, CD_PIN)
 #define CD_DATA    PIN_HIGH(CD_PORT, CD_PIN)
 #define CD_OUTPUT  PIN_OUTPUT(CD_PORT, CD_PIN)
 #define CS_ACTIVE  PIN_LOW(CS_PORT, CS_PIN)
 #define CS_IDLE    PIN_HIGH(CS_PORT, CS_PIN)
 #define CS_OUTPUT  PIN_OUTPUT(CS_PORT, CS_PIN)
 #define RESET_ACTIVE  PIN_LOW(RESET_PORT, RESET_PIN)
 #define RESET_IDLE    PIN_HIGH(RESET_PORT, RESET_PIN)
 #define RESET_OUTPUT  PIN_OUTPUT(RESET_PORT, RESET_PIN)
#define SD_ACTIVE  PIN_LOW(SD_PORT, SD_PIN)
#define SD_IDLE    PIN_HIGH(SD_PORT, SD_PIN)
#define SD_OUTPUT  PIN_OUTPUT(SD_PORT, SD_PIN)
 // bit-bang macros for SDIO
#define SCK_LO     PIN_LOW(SCK_PORT, SCK_PIN)
#define SCK_HI     PIN_HIGH(SCK_PORT, SCK_PIN)
#define SCK_OUT    PIN_OUTPUT(SCK_PORT, SCK_PIN)
#define MOSI_LO    PIN_LOW(MOSI_PORT, MOSI_PIN)
#define MOSI_HI    PIN_HIGH(MOSI_PORT, MOSI_PIN)
#define MOSI_OUT   PIN_OUTPUT(MOSI_PORT, MOSI_PIN)
#define MOSI_IN    PIN_INPUT(MOSI_PORT, MOSI_PIN)
#define LED_LO     PIN_LOW(LED_PORT, LED_PIN)
#define LED_HI     PIN_HIGH(LED_PORT, LED_PIN)
#define LED_OUT    PIN_OUTPUT(LED_PORT, LED_PIN)

extern "C" void spi0_init(uint32_t mode, uint32_t div);
extern "C" void spi0_write(uint8_t c);
extern "C" void spi0_write16(uint16_t c);
extern "C" uint8_t spi0_read(void);
extern "C" uint16_t spi0_read16(void);
extern "C" void spi0_flush(void);
extern "C" void spi1_init(uint32_t mode, uint32_t div);
extern "C" void spi1_write(uint8_t c);
extern "C" void spi1_write16(uint16_t c);
extern "C" uint8_t spi1_read(void);
extern "C" uint16_t spi1_read16(void);
extern "C" void spi1_flush(void);

// General macros.   IOCLR registers are 1 cycle when optimised.
#define wait_ms(ms)  delay(ms)

static uint8_t spibuf[16];


#if defined(ILI9481_KBV_H_)
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; spi1_init(3, 0); }

#define xchg8(x)     readbits(8);
#define write8(x)    ( spi1_write(x), spi1_read())
#define write18(x)   { write18_N(x, 1); }
#define WriteCmd(x)  { SDIO_OUTMODE(); MOSI_LO; SCK_HI; SCK_LO; write8(x); }
#define WriteDat8(x) { MOSI_HI; SCK_HI; SCK_LO; write8(x); }

static inline void write18_N(uint16_t color, int16_t n)
{
	uint8_t r = color >> 8, g = (color >> 5), b = color << 3;
	while (n-- > 0) {
		WriteDat8(r);
		WriteDat8(g);
		WriteDat8(b);
	}
}

static inline void write9_block(uint8_t * block, int16_t n)
{
	while (n-- > 0) {
		WriteDat8(*block++);
	}
}

#elif defined(ILI9341_KBV_H_)
#if defined(NUCLEO) || defined(MKL26Z4)
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; spi1_init(0, 1); }
#define flush()      {int cnt = 3; while (cnt--) { spi1_flush(); } } //spi1_flush()
//#define flush()      spi1_flush()
#define write8(x)    spi1_write(x)
#define read8()      spi1_read()
#else
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; spi0_init(3, 0); }
#define flush()      {int cnt = 3; while (cnt--) { spi0_flush(); } } //spi0_flush()
//#define flush()      spi0_flush()
#define write8(x)    spi0_write(x)
#define read8()      spi0_read()
#endif
#define xchg8(x)     (write8(x), read8())
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { flush(); CD_COMMAND; write8(x); flush(); CD_DATA; }
#define WriteData(x) { write16(x); }
#define FLUSH_IDLE   { flush(); CS_IDLE; }

#elif defined(ST7735X_KBV_H_)
//HW: write @12MHz, readbits @1.5MHz.    128x128 Tests=2.29s
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; SD_OUTPUT; SD_IDLE; spi1_init(0x0700, 1); }
#define SDIO_INMODE()  
#define SDIO_OUTMODE() 

#define xchg8(x)     write8(x)
#define write8(x)    ( spi1_write(x) )
#define flush()      spi1_flush()
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { CD_COMMAND; write8(x); flush(); }
#define WriteData8(x) { CD_DATA; write8(x); flush(); }

// STM32 spouts continuous SCK when BIDIMODE (RX) enabled.
// STM32 let's just blast it.
static inline uint32_t readbits(uint8_t bits)
{
#warning inline readbits
  uint64_t bidibuf = 0;
  uint8_t bidicnt = 0, dummy;
  SD_ACTIVE;
//  flush();
     SPI1->CR1 &= ~(SPI_CR1_SPE);
  uint32_t CR1_save = SPI1->CR1;
  RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;
  RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;
  SPI1->CR1 = CR1_save;
     SPI1->CR1 |= (SPI_CR1_BIDIMODE) | (2<<3);
     SPI1->CR1 &= ~(SPI_CR1_BIDIOE);
     SPI1->CR1 |= (SPI_CR1_SPE);
     while (bidicnt < bits) {
		    bidibuf <<= 8;
        while ((SPI1->SR & ((1<<0))) == 0) ;  //!RXNE
        bidibuf |= *((uint8_t*)&SPI1->DR);
		    bidicnt += 8;
	   }
//     SPI1->CR1 &= ~SPI_CR1_BIDIMODE;          //stop more SCK 
  SPI1->CR1 = CR1_save;
	   for (volatile int cnt = 15; cnt--; ) ;  //let SCK complete
     SPI1->CR1 |= (SPI_CR1_SPE);
  flush();
	SD_IDLE;
	uint8_t shift = bidicnt - bits;
	return (bidibuf >> shift);
}

#elif defined(_ST7735X_KBV_H_)
//HW: write @12MHz, readbits @1.5MHz.    128x128 Tests=2.29s
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; SD_OUTPUT; SD_IDLE; spi1_init(0x0700, 1); }
#define SDIO_INMODE()  
#define SDIO_OUTMODE() 

#define xchg8(x)     write8(x)
#define write8(x)    ( spi1_write(x) )
#define flush()      spi1_flush()
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { CD_COMMAND; write8(x); flush(); }
#define WriteData8(x) { CD_DATA; write8(x); flush(); }

// STM32 spouts continuous SCK when BIDIMODE (RX) enabled.
// STM32 let's just blast it.
static uint64_t bidibuf;
static uint8_t bidicnt;
static inline uint32_t readbits(int8_t bits)
{
#warning inline readbits
  SD_ACTIVE;
	if (bits == 0) {
		flush();
		bidicnt = 0;
	  SD_IDLE;
		return 0;
	}

  while ((SPI1->SR & ((1<<0))) != 0) {  //RXNE
		 bidibuf <<= 8;
     bidibuf |= *((uint8_t*)&SPI1->DR);
		 bidicnt += 8;
	}

	if (bidicnt < bits) {
     SPI1->CR1 |= (SPI_CR1_BIDIMODE);
     SPI1->CR1 &= ~(SPI_CR1_BIDIOE);
     while (bidicnt < bits) {
		    bidibuf <<= 8;
        while ((SPI1->SR & ((1<<0))) == 0) ;  //!RXNE
        bidibuf |= *((uint8_t*)&SPI1->DR);
		    bidicnt += 8;
	   }
     SPI1->CR1 &= ~SPI_CR1_BIDIMODE;          //stop more SCK 
	   for (volatile int cnt = 15; cnt--; ) ;  //let SCK complete
  }
	SD_IDLE;
	uint8_t shift = bidicnt - bits;
	bidicnt -= bits;
	return (bidibuf >> shift);
}

static inline uint32_t read_bidi_block(uint8_t *block, uint8_t n)
{
  uint32_t ret = 0;
	uint8_t dummy;
	SD_ACTIVE;
  for (volatile int cnt = 5; cnt--; ) ;  //let SCK complete

  SPI1->CR1 |= (SPI_CR1_BIDIMODE);
  SPI1->CR1 &= ~(SPI_CR1_BIDIOE);
  while (n--) {
     while ((SPI1->SR & ((1<<0))) == 0) ;  //RXNE
     *block++ = *((uint8_t*)&SPI1->DR);
	}
  SPI1->CR1 &= ~SPI_CR1_BIDIMODE;          //stop more SCK 
  for (volatile int cnt = 5; cnt--; ) ;  //let SCK complete
  while ((SPI1->SR & ((1<<0))) != 0) {  //RXNE
		 ret++;
		 dummy = *((uint8_t*)&SPI1->DR);
	}

	SD_IDLE;
	return ret;
}

#elif defined(_ST7735X_KBV_H_)
//HW: write @12MHz, readbits @1.5MHz.    128x128 Tests=2.29s
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; SD_OUTPUT; SD_IDLE; spi1_init(0x0700, 1); }
#define SDIO_INMODE()  
#define SDIO_OUTMODE() 

#define xchg8(x)     write8(x)
#define write8(x)    ( spi1_write(x), spi1_read())
#define flush()      spi1_flush()
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { CD_COMMAND; write8(x); }
#define WriteData8(x) { CD_DATA; write8(x); }

// STM32 spouts continuous SCK when BIDIMODE (RX) enabled.
// STM32 we have to stop exactly on first bit
static inline uint32_t readbits(uint8_t bits)
{
#warning inline readbits
	uint32_t ret = 0;
  spi1_flush();
  SD_ACTIVE;
  SPI1->CR1 &= ~(SPI_CR1_SPE);
  uint32_t save = SPI1->CR1;
  SPI1->CR1 &= ~(SPI_CR1_BIDIOE | (7<<3));
  SPI1->CR1 |= (SPI_CR1_BIDIMODE) | (4<<3);   //really slow 4,11
  SPI1->CR2 = 0x1700;
  SPI1->CR1 |= (SPI_CR1_SPE);    //start
  while (bits > 8) {
		 ret <<= 8;
     while ((SPI1->SR & ((1<<0))) == 0) ;  //RXNE
     ret |= *((uint8_t*)&SPI1->DR);
		 bits -= 8;
	}
  for (volatile int cnt = 11; cnt--; ) ;  //(4,11-18) (3,3-6) (5,25-38)
//  SPI1->CR1 |= SPI_CR1_BIDIOE;
  SPI1->CR1 &= ~SPI_CR1_BIDIMODE;
//	SPI1->CR1 &= ~(SPI_CR1_SPE);  //turn off SPI
		 ret <<= 8;
     while ((SPI1->SR & ((1<<0))) == 0) ;  //RXNE
     ret |= *((uint8_t*)&SPI1->DR);
	SPI1->CR1 &= ~(SPI_CR1_SPE);  //turn off SPI
  SPI1->CR1 = save;
  SPI1->CR1 |= (SPI_CR1_SPE);    //start
	SD_IDLE;
	return ret;
}

#elif defined(ST7735X_KBV_H_)
//SW: write @1.3MHz, readbits @1.5MHz.    128x128 Tests=8.3s
#define INIT()       { CD_OUTPUT; CS_OUTPUT; RESET_OUTPUT; SD_OUTPUT; SD_IDLE; SDIO_OUTMODE(); }
#define SDIO_INMODE()  MOSI_IN;SCK_OUT    //no braces
#define SDIO_OUTMODE() {MOSI_OUT;SCK_OUT;}

#define flush()
#define xchg8(x)     write8(x)
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { CD_COMMAND; write8(x); }
#define WriteData8(x) { CD_DATA; write8(x); }

static inline void write8(uint8_t val)
{
    for (uint8_t i = 0; i < 8; i++) {   //send command
        if (val & 0x80) MOSI_HI;
	      else MOSI_LO;
		    SCK_HI;
		    SCK_LO;
        val <<= 1;
    }
}

static inline uint32_t readbits(uint8_t bits)
{
	uint32_t ret = 0;
  SD_ACTIVE;
    SDIO_INMODE();
	while (bits--) {
		ret <<= 1;
		if (PIN_READ(MOSI_PORT, MOSI_PIN))
		    ret++;
		SCK_HI;
		SCK_LO;
	}
    SDIO_OUTMODE();
  SD_IDLE;
	return ret;
}

#else
#error unsupported
#endif


static inline void write16_N(uint16_t color, int16_t n)
{
	uint8_t hi = color >> 8, lo = color;
	while (n-- > 0) {
		write8(hi);
		write8(lo);
	}
	flush();
}

static inline void write8_block(uint8_t * block, int16_t n)
{
	while (n-- > 0) {
		write8(*block++);
	}
	flush();
}

#endif
