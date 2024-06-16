#define CS_IDLE       { _lcd_pin_cs = 1; }
#define CS_ACTIVE     _lcd_pin_cs = 0
#define CD_DATA       { _lcd_pin_rs = 1; }
#define CD_COMMAND    _lcd_pin_rs = 0
#define RESET_IDLE    _lcd_pin_reset = 1
#define RESET_ACTIVE  _lcd_pin_reset = 0
#define xchg8(x)     _spi.write((uint8_t)x)
#define write8(x)    { _spi.write((uint8_t)x); }
#define write16(x)   { uint8_t h = (x)>>8, l = x; write8(h); write8(l); }
#define WriteCmd(x)  { CD_COMMAND; xchg8(x); CD_DATA; }
#define WriteData(x) { write16(x); }
#define write8_block(block, N) _spi.write((const char*)block, N, NULL, 0)
#define INIT()       { _spi.format(8, 0); _spi.frequency(18000000); }
#define spi_flush()  { }
#define FLUSH_IDLE   { spi_flush(); CS_IDLE; }

#if 0
#elif defined(MKL25Z4) || defined(MKL05Z4_H_)
#define TXEMPTY()   (SPI0->S & SPI_S_SPTEF_MASK)
#define RX_FULL()   (SPI0->S & SPI_S_SPRF_MASK)
#define SPI_DATA    SPI0->D
#define SPI_RXDATA  SPI0->D
#elif defined(MK20D5_H_)
#define TXEMPTY()   (SPI0->SR & SPI_SR_TFFF_MASK)
#define RX_FULL()   (SPI0->SR & SPI_SR_RFDF_MASK)
#define SPI_DATA    SPI0->PUSHR
#define SPI_RXDATA    SPI0->POPR
#elif defined(STM32F072xB) || defined(STM32F103xB) 
#define TXEMPTY()   (SPI1->SR & (1<<1))
#define RX_FULL()   (SPI1->SR & (1<<0))
#define SPI_DATA    *((uint8_t*)&(SPI1->DR))
#define SPI_RXDATA  *((uint16_t*)&(SPI1->DR))
#endif

#define SPI_SEND(x) {while (!TXEMPTY()); (SPI_DATA) = (x); }
#define SPI_RECV(x) {while (!RX_FULL()); x = SPI_RXDATA; }
#define SPI_XFER(snd, rcv) { SPI__SEND(snd); SPI_RECV(rcv); }

static inline uint8_t spi_xfer(uint8_t d)
{
    uint8_t ret;
    SPI_SEND(d);
    SPI_RECV(ret);
    return ret;
}

void write_pattern_N(uint8_t *pattern, int len, int rpt = 1)
{
    uint8_t *txptr;
    uint8_t c, d;
    int n = len;
    if (n == 0) return;
    if (len == 2) {
        uint8_t h = pattern[0], l = pattern[1];
        SPI_SEND(h);       //straight to shifter
        c = spi_xfer(l);   //reads H
        while (--rpt) {
            c = spi_xfer(h);   //reads L
            c = spi_xfer(l);   //reads H
        }
        SPI_RECV(c);   //reads L
    } else if (len == 3) {
        uint8_t r = pattern[0], g = pattern[1], b = pattern[2];
        SPI_SEND(r);
        c = spi_xfer(g);   //reads R
        c = spi_xfer(b);   //reads G
        while (--rpt) {
            c = spi_xfer(r);   //reads B
            c = spi_xfer(g);   //reads R
            c = spi_xfer(b);   //reads G
        }
        SPI_RECV(c);   //reads B
    } else {
        while (rpt--) {
            n = len;          //ready for next time
            txptr = pattern;
            d = *txptr++;
            SPI_SEND(d);      //first goes to shifter
            while (--n) {
                c = spi_xfer(*txptr++);
            }
            SPI_RECV(c);   //reads last
        }
    }
}

void ILI9341_kbv::write16_N(uint16_t color, int16_t n)
{
    uint16_t buf[16];
    color = (color << 8)|(color >> 8);
#if defined(MKL05Z4)
    write_pattern_N((uint8_t*)&color, 2, n);
#else
    for (int i = 0; i < 16 && i < n; i++) buf[i] = color;
    while (n > 0) {
        int cnt = (n > 16) ? 16 : n;
        _spi.write((const char*)buf, cnt * 2, NULL, 0);
        n -= cnt;
    }
#endif
}
