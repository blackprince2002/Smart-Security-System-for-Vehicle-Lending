#if defined(__AVR_ATxmega32A4U__) || defined(__AVR_ATxmega128A4U__)    //3.49s @ 32MHz -O2.
 // 100ns/150ns for ILI9341 W/R cycle.   100ns/200ns for ILI920.  20ns/150ns HX8347
 // Xmega @ 60MHz i.e. 30MHz SCK works with 9341.
#warning Using ATxmega32A4U USART_MSPI
#define CD_PORT VPORT2
#define CD_PIN  1
#define CS_PORT VPORT3
#define CS_PIN  0
#define RESET_PORT VPORT2
#define RESET_PIN  0
#define SD_PORT    VPORT2
#define SD_PIN     4
#define SPI_PORT VPORT3 
#define MOSI_PIN 3
#define SCK_PIN  1

#define SPCRVAL (USART_CLK2X_bm | USART_RXEN_bm | USART_TXEN_bm)
#define SETDDR  {PORTCFG.VPCTRLB=PORTCFG_VP13MAP_PORTD_gc | PORTCFG_VP02MAP_PORTC_gc; VPORT3.DIR |= (1<<0)|(1<<1)|(1<<3); VPORT2.DIR |= 0x03; PIN_HIGH(SD_PORT, SD_PIN); SD_PORT.DIR |= (1<<SD_PIN); }
#define INIT()  { CS_IDLE; RESET_IDLE; SETDDR; spi_init(); }

#define PIN_LOW(p, b)        (p).OUT &= ~(1<<(b))
#define PIN_HIGH(p, b)       (p).OUT |= (1<<(b))
#define PIN_OUTPUT(p, b)     (p).DIR |= (1<<(b))
#define PIN_INPUT(p, b)      (p).DIR &= ~(1<<(b))
#define PIN_READ(p, b)       ((p).IN & (1<<(b)))

#define FLUSH_IDLE { CS_IDLE; }

static inline void spi_init(void)
{
    USARTD0.CTRLB = SPCRVAL;
    USARTD0.CTRLC = USART_CMODE_MSPI_gc | 0x00 | 0x00;  //mode #0 
    //   PORTD.PIN1CTRL |= PORT_INVEN_bm;   //CPOL
    USARTD0.BAUDCTRLA = 0x00;   //F_CPU/2
    USARTD0.BAUDCTRLB = ((0x00 << USART_BSCALE_gp) & USART_BSCALE_gm) | 0x00;
    USARTD0.DATA;
}

static inline uint8_t xchg8_1(uint8_t x)
{
//    USARTD0.STATUS = USART_TXCIF_bm;
    USARTD0.DATA = x;
//    while ((USARTD0.STATUS & USART_TXCIF_bm) == 0);
    while ((USARTD0.STATUS & USART_RXCIF_bm) == 0);
    return USARTD0.DATA;
}

#define SDIO_INMODE() uint8_t spcr = USARTD0.CTRLB;USARTD0.CTRLB = 0;MOSI_IN;SCK_OUT    //no braces
#define SDIO_OUTMODE() {MOSI_OUT;SCK_OUT;USARTD0.CTRLB = spcr;}
static uint32_t readbits(uint8_t bits)
{
    uint32_t ret = 0;
    while (bits--) {
        ret <<= 1;
        if (PIN_READ(SPI_PORT, MOSI_PIN))
            ret++;
        SCK_HI;
        SCK_LO;
    }
    return ret;
}

static inline void write16_N(uint16_t color, int16_t n)
{
    uint8_t hi = color >> 8, lo = color;
    USARTD0.DATA = hi;
    while (--n > 0) {
        while ((USARTD0.STATUS & USART_DREIF_bm) == 0);
        USARTD0.DATA = lo;
        while ((USARTD0.STATUS & USART_DREIF_bm) == 0);
        USARTD0.DATA = hi;
    }
    while ((USARTD0.STATUS & USART_DREIF_bm) == 0);
    asm("cli");
    USARTD0.DATA = lo;
    USARTD0.STATUS = USART_TXCIF_bm;
    asm("sei");
    while ((USARTD0.STATUS & USART_TXCIF_bm) == 0);
    while ((USARTD0.STATUS & USART_RXCIF_bm) != 0)
        USARTD0.DATA;
}

static inline void write8_block(uint8_t * block, int16_t n)
{
    USARTD0.STATUS = USART_TXCIF_bm;   //start with a clean slate
    USARTD0.DATA = *block++;
    while (--n > 0) {
        while ((USARTD0.STATUS & USART_DREIF_bm) == 0);
        asm("cli");
        USARTD0.DATA = *block++;
        USARTD0.STATUS = USART_TXCIF_bm;
        asm("sei");
    }
    while ((USARTD0.STATUS & USART_TXCIF_bm) == 0);
    while ((USARTD0.STATUS & USART_RXCIF_bm) != 0)
        USARTD0.DATA;
}
#endif
