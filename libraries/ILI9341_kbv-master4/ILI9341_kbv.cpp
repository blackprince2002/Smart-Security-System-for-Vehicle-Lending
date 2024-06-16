//to do: check MBED timing
//to do: read ID at faster SPI speeds

#include "ILI9341_kbv.h"

#if defined(__MBED__)
#include "serial_mbed.h"

ILI9341_kbv::ILI9341_kbv(PinName CS, PinName RS, PinName RST)
    : _lcd_pin_rs(RS), _lcd_pin_cs(CS), _lcd_pin_reset(RST), _spi(D11, D12, D13), Adafruit_GFX(240, 320)
{
}

#elif defined(__CC_ARM) || defined(__CROSSWORKS_ARM)
#include "serial_keil.h"

ILI9341_kbv::ILI9341_kbv():Adafruit_GFX(240, 320)
{
}
#else
#include "serial_kbv.h"

ILI9341_kbv::ILI9341_kbv():Adafruit_GFX(240, 320)
{
}
#endif

static uint8_t done_reset;

void ILI9341_kbv::reset(void)
{
    done_reset = 1;
    INIT();
    CS_IDLE;
    RESET_IDLE;
    wait_ms(50);
    RESET_ACTIVE;
    wait_ms(100);
    RESET_IDLE;
    wait_ms(100);
}

void ILI9341_kbv::pushCommand(uint16_t cmd, uint8_t * block, int8_t N)
{
    CS_ACTIVE;
    WriteCmd(cmd);
    write8_block(block, N);
    FLUSH_IDLE;
}

#define ILI9341_CMD_NOP                             0x00
#define ILI9341_CMD_SOFTWARE_RESET                  0x01
#define ILI9341_CMD_READ_DISP_ID                    0x04
#define ILI9341_CMD_READ_DISP_STATUS                0x09
#define ILI9341_CMD_READ_DISP_MADCTRL               0x0B
#define ILI9341_CMD_READ_DISP_PIXEL_FORMAT          0x0C
#define ILI9341_CMD_READ_DISP_IMAGE_FORMAT          0x0D
#define ILI9341_CMD_READ_DISP_SIGNAL_MODE           0x0E
#define ILI9341_CMD_READ_DISP_SELF_DIAGNOSTIC       0x0F
#define ILI9341_CMD_ENTER_SLEEP_MODE                0x10
#define ILI9341_CMD_SLEEP_OUT                       0x11
#define ILI9341_CMD_PARTIAL_MODE_ON                 0x12
#define ILI9341_CMD_NORMAL_DISP_MODE_ON             0x13
#define ILI9341_CMD_DISP_INVERSION_OFF              0x20
#define ILI9341_CMD_DISP_INVERSION_ON               0x21
#define ILI9341_CMD_GAMMA_SET                       0x26
#define ILI9341_CMD_DISPLAY_OFF                     0x28
#define ILI9341_CMD_DISPLAY_ON                      0x29
#define ILI9341_CMD_COLUMN_ADDRESS_SET              0x2A
#define ILI9341_CMD_PAGE_ADDRESS_SET                0x2B
#define ILI9341_CMD_MEMORY_WRITE                    0x2C
#define ILI9341_CMD_COLOR_SET                       0x2D
#define ILI9341_CMD_MEMORY_READ                     0x2E
#define ILI9341_CMD_PARTIAL_AREA                    0x30
#define ILI9341_CMD_VERT_SCROLL_DEFINITION          0x33
#define ILI9341_CMD_TEARING_EFFECT_LINE_OFF         0x34
#define ILI9341_CMD_TEARING_EFFECT_LINE_ON          0x35
#define ILI9341_CMD_MEMORY_ACCESS_CONTROL           0x36
#define ILI9341_CMD_VERT_SCROLL_START_ADDRESS       0x37
#define ILI9341_CMD_IDLE_MODE_OFF                   0x38
#define ILI9341_CMD_IDLE_MODE_ON                    0x39
#define ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET         0x3A
#define ILI9341_CMD_WRITE_MEMORY_CONTINUE           0x3C
#define ILI9341_CMD_READ_MEMORY_CONTINUE            0x3E
#define ILI9341_CMD_SET_TEAR_SCANLINE               0x44
#define ILI9341_CMD_GET_SCANLINE                    0x45
#define ILI9341_CMD_WRITE_DISPLAY_BRIGHTNESS        0x51
#define ILI9341_CMD_READ_DISPLAY_BRIGHTNESS         0x52
#define ILI9341_CMD_WRITE_CTRL_DISPLAY              0x53
#define ILI9341_CMD_READ_CTRL_DISPLAY               0x54
#define ILI9341_CMD_WRITE_CONTENT_ADAPT_BRIGHTNESS  0x55
#define ILI9341_CMD_READ_CONTENT_ADAPT_BRIGHTNESS   0x56
#define ILI9341_CMD_WRITE_MIN_CAB_LEVEL             0x5E
#define ILI9341_CMD_READ_MIN_CAB_LEVEL              0x5F
#define ILI9341_CMD_READ_ID1                        0xDA
#define ILI9341_CMD_READ_ID2                        0xDB
#define ILI9341_CMD_READ_ID3                        0xDC

//static uint8_t readReg8(uint8_t reg, uint8_t dat)
uint8_t ILI9341_kbv::readReg8(uint8_t reg, uint8_t dat)
{
    uint8_t ret;
    CS_ACTIVE;
    WriteCmd(reg);
    ret = xchg8(dat);           //only safe to read @ SCK=16MHz
    FLUSH_IDLE;
    return ret;
}

uint8_t ILI9341_kbv::readcommand8(uint8_t reg, uint8_t idx)         //this is the same as Adafruit_ILI9341
{
    readReg8(0xD9, 0x10 | idx);
    uint16_t ret = readReg8(reg, 0xFF);
    readReg8(0xD9, 0x00);
    return ret;
}

uint16_t ILI9341_kbv::readID(void)                          //{ return 0x9341; }
{
    if (!done_reset) reset();
    return (readcommand8(0xD3, 2) << 8) | readcommand8(0xD3, 3);
}

uint16_t ILI9341_kbv::readReg(uint16_t reg, uint8_t idx)     //note that this reads pairs of data bytes
{
    uint8_t h, l;
    idx <<= 1;
    h = readcommand8(reg, idx);
    l = readcommand8(reg, idx + 1);
    return (h << 8) | l;
}

int16_t ILI9341_kbv::readGRAM(int16_t x, int16_t y, uint16_t * block, int16_t w, int16_t h)
{
    uint8_t r, g, b;
      int16_t n = w * h;    // we are NEVER going to read > 32k pixels at once
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    CS_ACTIVE;
    WriteCmd(ILI9341_CMD_MEMORY_READ);

    // needs 1 dummy read
    r = xchg8(0xFF);
    while (n-- > 0) {
        r = xchg8(0xFF);
        g = xchg8(0xFF);
        b = xchg8(0xFF);
        *block++ = color565(r, g, b);
    }
    FLUSH_IDLE;
    setAddrWindow(0, 0, width() - 1, height() - 1);
    return 0;
}

void ILI9341_kbv::setRotation(uint8_t r)
{
    uint8_t mac = 0x00;
    Adafruit_GFX::setRotation(r & 3);
    switch (rotation) {
    case 0:
        mac = 0x08;
        break;
    case 1:        //LANDSCAPE 90 degrees
        mac = 0x68;
        break;
    case 2:
        mac = 0xD8;
        break;
    case 3:
        mac = 0xB8;
        break;
    }
    pushCommand(ILI9341_CMD_MEMORY_ACCESS_CONTROL, &mac, 1);
}

void ILI9341_kbv::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    // ILI934X just plots at edge if you try to write outside of the box:
    if (x < 0 || y < 0 || x >= width() || y >= height())
        return;
    CS_ACTIVE;
    WriteCmd(ILI9341_CMD_COLUMN_ADDRESS_SET);
    write16(x);
    WriteCmd(ILI9341_CMD_PAGE_ADDRESS_SET);
    write16(y);
    WriteCmd(ILI9341_CMD_MEMORY_WRITE);
    write16(color);
    FLUSH_IDLE;
}

void ILI9341_kbv::setAddrWindow(int16_t x, int16_t y, int16_t x1, int16_t y1)
{
    CS_ACTIVE;
    WriteCmd(ILI9341_CMD_COLUMN_ADDRESS_SET);
    write16(x);
    write16(x1);
    WriteCmd(ILI9341_CMD_PAGE_ADDRESS_SET);
    write16(y);
    write16(y1);
    FLUSH_IDLE;
}

void ILI9341_kbv::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    int16_t end;
    if (w < 0) {
        w = -w;
        x -= w;
    }                           //+ve w
    end = x + w;
    if (x < 0)
        x = 0;
    if (end > width())
        end = width();
    w = end - x;
    if (h < 0) {
        h = -h;
        y -= h;
    }                           //+ve h
    end = y + h;
    if (y < 0)
        y = 0;
    if (end > height())
        end = height();
    h = end - y;
    setAddrWindow(x, y, x + w - 1, y + h - 1);
    CS_ACTIVE;
    WriteCmd(ILI9341_CMD_MEMORY_WRITE);
    if (h > w) { end = h; h = w; w = end; }
    while (h-- > 0) {
        write16_N(color, w);
    }
    FLUSH_IDLE;
    setAddrWindow(0, 0, width() - 1, height() - 1);
}

void ILI9341_kbv::pushColors(uint16_t * block, int16_t n, bool first)
{
    uint16_t color;
    CS_ACTIVE;
    if (first) {
        WriteCmd(ILI9341_CMD_MEMORY_WRITE);
    }
    while (n-- > 0) {
        color = *block++;
        write16(color);
    }
    FLUSH_IDLE;
}

void ILI9341_kbv::pushColors(uint8_t * block, int16_t n, bool first)
{
    uint16_t color;
    uint8_t h, l;
    CS_ACTIVE;
    if (first) {
        WriteCmd(ILI9341_CMD_MEMORY_WRITE);
    }
    while (n-- > 0) {
        h = (*block++);
        l = (*block++);
        color = (h << 8) | l;
        write16(color);
    }
    FLUSH_IDLE;
}

void ILI9341_kbv::pushColors(const uint8_t * block, int16_t n, bool first, bool bigend)
{
    uint16_t color;
    uint8_t h, l;
    CS_ACTIVE;
    if (first) {
        WriteCmd(ILI9341_CMD_MEMORY_WRITE);
    }
    while (n-- > 0) {
        l = pgm_read_byte(block++);
        h = pgm_read_byte(block++);
        color = (bigend) ? (l << 8 ) | h : (h << 8) | l;
        write16(color);
    }
    FLUSH_IDLE;
}

void ILI9341_kbv::invertDisplay(boolean i)
{
    pushCommand(i ? ILI9341_CMD_DISP_INVERSION_ON : ILI9341_CMD_DISP_INVERSION_OFF, NULL, 0);
}

void ILI9341_kbv::vertScroll(int16_t top, int16_t scrollines, int16_t offset)
{
    int16_t bfa = HEIGHT - top - scrollines;  // bottom fixed area
    int16_t vsp;
    vsp = top + offset;  // vertical start position
    if (offset < 0)
        vsp += scrollines;          //keep in unsigned range
    CS_ACTIVE;
    WriteCmd( 0x0033);
    write16(top);        //TOP
    write16(scrollines); //VSA
    write16(bfa);        //BFA

    WriteCmd(0x0037)
    write16(vsp);        //VLSP
    FLUSH_IDLE;
}

#define TFTLCD_DELAY8 0xFF

const uint8_t PROGMEM ILI9341_regValues_kbv[] = {
    //  (COMMAND_BYTE), n, data_bytes....
    (0x01), 0,             //ILI9341_CMD_SOFTWARE_RESET
            TFTLCD_DELAY8, 50,   // .kbv
    (0xCF), 3,                  //ILI9341_CMD_POWER_CONTROL_B
    0x00, 0x8B, 0x30,
    (0xED), 4,                  //ILI9341_CMD_POWER_ON_SEQ_CONTROL
    0x67, 0x03, 0x12, 0x81,
    (0xE8), 3,                  //ILI9341_CMD_DRIVER_TIMING_CONTROL_A
    0x85, 0x10, 0x7A,
    (0xCB), 5,                  //ILI9341_CMD_POWER_CONTROL_A
    0x39, 0x2C, 0x00, 0x34, 0x02,
    (0xF7), 1,                  //ILI9341_CMD_PUMP_RATIO_CONTROL
    0x20,
    (0xEA), 2,                  //ILI9341_CMD_DRIVER_TIMING_CONTROL_B
    0x00, 0x00,
    (0xC0), 1,                  //ILI9341_CMD_POWER_CONTROL_1
    0x1B,                       /* VRH[5:0]                     */
    (0xC1), 1,                  //ILI9341_CMD_POWER_CONTROL_2
    0x10,                       /* SAP[2:0];BT[3:0]             */
    (0xC5), 2,                  //ILI9341_CMD_VCOM_CONTROL_1
    0x3F, 0x3C,
    (0xC7), 1,                  //ILI9341_CMD_VCOM_CONTROL_2
    0xB7,
    (0x36), 1,                  //ILI9341_CMD_MEMORY_ACCESS_CONTROL
    0x08,
    (0x3A), 1,                  //ILI9341_CMD_COLMOD_PIXEL_FORMAT_SET
    0x55,
    (0xB1), 2,                  //ILI9341_CMD_FRAME_RATE_CONTROL_NORMAL
    0x00, 0x1B,
    (0xB4), 1, 0x00,      //Inversion Control [02] .kbv NLA=1, NLB=1, NLC=1  Extended anyway
    (0xB6), 2,                  //ILI9341_CMD_DISPLAY_FUNCTION_CONTROL
    0x0A, 0xA2,
    (0xF2), 1,                  //ILI9341_CMD_ENABLE_3_GAMMA_CONTROL
    0x00,                       /* 3Gamma Function Disable      */
    (0x26), 1,                  //ILI9341_CMD_GAMMA_SET
    0x01,
    (0xE0), 15,                 //ILI9341_CMD_POSITIVE_GAMMA_CORRECTION
    0x0F, 0x2A, 0x28, 0x08, 0x0E, 0x08, 0x54, 0XA9, 0x43, 0x0A, 0x0F, 0x00,
    0x00, 0x00, 0x00,
    (0xE1), 15,                 //ILI9341_CMD_NEGATIVE_GAMMA_CORRECTION
    0x00, 0x15, 0x17, 0x07, 0x11, 0x06, 0x2B, 0x56, 0x3C, 0x05, 0x10, 0x0F,
    0x3F, 0x3F, 0x0F,
    (0x11), 0,             //ILI9341_CMD_SLEEP_OUT
            TFTLCD_DELAY8, 150,   // .kbv
    (0x29), 0,                  //ILI9341_CMD_DISPLAY_ON
};
        static const uint8_t ILI9341_regValues_2_4[] PROGMEM = {   // BOE 2.4"
            0x01, 0,            // software reset
            TFTLCD_DELAY8, 50,   // .kbv
            0xCF, 3, 0x00, 0x81, 0x30,  //Power Control B [00 81 30]
            0xED, 4, 0x64, 0x03, 0x12, 0x81,    //Power On Seq [55 01 23 01]
            0xE8, 3, 0x85, 0x10, 0x78,  //Driver Timing A [04 11 7A]
            0xCB, 5, 0x39, 0x2C, 0x00, 0x34, 0x02,      //Power Control A [39 2C 00 34 02]
            0xF7, 1, 0x20,      //Pump Ratio [10]
            0xEA, 2, 0x00, 0x00,        //Driver Timing B [66 00]
            0xB1, 2, 0x00, 0x1B,        //Frame Control [00 1B]
            0xB6, 2, 0x0A, 0xA2, 0x27, //Display Function [0A 82 27 XX]    .kbv SS=1
            0xB4, 1, 0x00,      //Inversion Control [02] .kbv NLA=1, NLB=1, NLC=1
            0xC0, 1, 0x21,      //Power Control 1 [26]
            0xC1, 1, 0x11,      //Power Control 2 [00]
            0xC5, 2, 0x3F, 0x3C,        //VCOM 1 [31 3C]
            0xC7, 1, 0xB5,      //VCOM 2 [C0]
            0x36, 1, 0x48,      //Memory Access [00]
            0xF2, 1, 0x00,      //Enable 3G [02]
            0x26, 1, 0x01,      //Gamma Set [01]
            0xE0, 15, 0x0f, 0x26, 0x24, 0x0b, 0x0e, 0x09, 0x54, 0xa8, 0x46, 0x0c, 0x17, 0x09, 0x0f, 0x07, 0x00,
            0xE1, 15, 0x00, 0x19, 0x1b, 0x04, 0x10, 0x07, 0x2a, 0x47, 0x39, 0x03, 0x06, 0x06, 0x30, 0x38, 0x0f,
            0x11, 0,            //Sleep Out
            TFTLCD_DELAY8, 150,
            0x29, 0,            //Display On
            0x3A, 1, 0x55,      //Pixel Format [66]
        };
//      init_table(ILI9341_regValues_2_4, sizeof(ILI9341_regValues_2_4));   //

//#define tableNNNN ILI9341_regValues_2_4
#define tableNNNN ILI9341_regValues_kbv

void ILI9341_kbv::begin(uint16_t ID)
{
    _lcd_ID = ID;
    uint8_t *p = (uint8_t *) tableNNNN;
    int16_t size = sizeof(tableNNNN);
    reset();
    while (size > 0) {
        uint8_t cmd = pgm_read_byte(p++);
        uint8_t len = pgm_read_byte(p++);
        if (cmd == TFTLCD_DELAY8) {
            delay(len);
            len = 0;
        } else {
            CS_ACTIVE;
            WriteCmd(cmd);
            for (uint8_t d = 0; d < len; d++) {
                uint8_t x = pgm_read_byte(p++);
                xchg8(x);
            }
            FLUSH_IDLE;
        }
        size -= len + 2;
    }
    setRotation(0);             //PORTRAIT
}
