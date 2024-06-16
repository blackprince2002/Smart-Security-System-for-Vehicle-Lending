// see if 4-line mode can read anything

#define TFT_MOSI 11
#define TFT_SCK  13
#define TFT_SS   10
#define TFT_DC  (9)     //DC=7 for HX8347
#define TFT_RESET (8)   //Backlight on HX8347
char *chip = "controller";

#if defined(SPDR)
// use SPI mode #0
uint8_t spi(uint8_t c)
{
    SPDR = c;
    while ((SPSR & 0x80) == 0) ;
    return SPDR;
}
#endif

uint32_t readwrite8(uint8_t cmd, uint8_t bits, uint8_t dummy)
{
    uint32_t ret = 0;
    uint8_t val = cmd;
    int cnt = 8;
    digitalWrite(TFT_SS, LOW);
#if 0
    SPSR = (0 << SPI2X);
    SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); //1MHz
    digitalWrite(TFT_DC, LOW);
    pinMode(TFT_MOSI, OUTPUT);
    spi(cmd);
    if (bits) {
        digitalWrite(TFT_DC, HIGH);
        pinMode(TFT_MOSI, INPUT);
        while (bits) {
            ret <<= 8;
            ret |= spi(0x00);
            bits -= 8;
        }
        ret >>= dummy;
    }
#elif 0
    digitalWrite(TFT_DC, LOW);
    pinMode(TFT_MOSI, OUTPUT);
    for (uint8_t i = 0; i < 8; i++) {   //send command
        if (val & 0x80) PORTB |= (1 << 3);
        else PORTB &= ~(1 << 3);
        PORTB |= (1 << 5);
        PORTB &= ~(1 << 5);
        val <<= 1;
    }
    if (bits == 0) {
        digitalWrite(TFT_SS, HIGH);
        return 0;
    }
    pinMode(TFT_MOSI, INPUT_PULLUP);
    digitalWrite(TFT_DC, HIGH);
    for (uint8_t i = 0; i < dummy; i++) {  //any dummy clocks
        PORTB |= (1 << 5);
        PORTB &= ~(1 << 5);
    }
    while (bits) {
        for (uint8_t i = 8; i-- > 0; ) {  // read results
            val <<= 1;
            if (PINB & (1 << 3)) val |= 1;;
            PORTB |= (1 << 5);
            PORTB &= ~(1 << 5);
        }
        bits -= 8;
        ret <<= 8;
        ret |= val;
    }
#else
    digitalWrite(TFT_DC, LOW);
    pinMode(TFT_MOSI, OUTPUT);
    for (int i = 0; i < 8; i++) {   //send command
        digitalWrite(TFT_MOSI, (val & 0x80) != 0);
        digitalWrite(TFT_SCK, HIGH);
        digitalWrite(TFT_SCK, LOW);
        val <<= 1;
    }
    if (bits == 0) {
        digitalWrite(TFT_SS, HIGH);
        return 0;
    }
    pinMode(TFT_MOSI, INPUT_PULLUP);
    digitalWrite(TFT_DC, HIGH);
    for (int i = 0; i < dummy; i++) {  //any dummy clocks
        digitalWrite(TFT_SCK, HIGH);
        digitalWrite(TFT_SCK, LOW);
    }
    for (int i = 0; i < bits; i++) {  // read results
        ret <<= 1;
        if (digitalRead(TFT_MOSI)) ret |= 1;;
        digitalWrite(TFT_SCK, HIGH);
        digitalWrite(TFT_SCK, LOW);
    }
#endif
    digitalWrite(TFT_SS, HIGH);
    return ret;
}

void showreg(uint8_t reg, uint8_t bits, uint8_t dummy)
{
    uint32_t val;
    val = readwrite8(reg, bits, dummy);

    Serial.print(chip);
    Serial.print(" reg(0x");
    if (reg < 0x10) Serial.print("0");
    Serial.print(reg , HEX);
    Serial.print(") = 0x");
    if (val < 0x10) Serial.print("0");
    Serial.println(val, HEX);

}

void setup() {
    // put your setup code here, to run once:
    uint32_t ID = 0;
    Serial.begin(9600);
    Serial.println("Bi-directional Read registers");
    digitalWrite(TFT_SS, HIGH);
    //    digitalWrite(TFT_SCK, HIGH);
    pinMode(TFT_SS, OUTPUT);
    pinMode(TFT_SCK, OUTPUT);
    pinMode(TFT_MOSI, OUTPUT);
    pinMode(MISO, INPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RESET, OUTPUT);
    digitalWrite(TFT_RESET, HIGH);
    digitalWrite(TFT_RESET, LOW);   //Hardware Reset
    delay(50);
    digitalWrite(TFT_RESET, HIGH);
    showreg(0x01, 0, 0);            //Software Reset
    delay(100);
    ID = readwrite8(0x04, 24, 1);
    if (ID == 0x7C89F0uL) chip = "ST7735S";
    if (ID == 0x548066uL) chip = "ILI9163C";
    ID &= 0xFF0000;
    if (ID == 0x5C0000) chip = "ST7735";
    showreg(0x04, 24, 1);  //RDDID
    showreg(0x09, 32, 1);  //RDDSTATUS
    showreg(0x0A, 8, 0);
    showreg(0x0B, 8, 0);   //RDDMADCTL
    showreg(0x0C, 8, 0);   //RDDCOLMOD
    showreg(0x0D, 8, 0);
    showreg(0x0E, 8, 0);
    showreg(0x0F, 8, 0);
    showreg(0x2E, 24, 8);  //readGRAM
    showreg(0xDA, 8, 0);   //RDID1
    showreg(0xDB, 8, 0);   //RDID2
    showreg(0xDC, 8, 0);   //RDID3
}

void loop() {
    // put your main code here, to run repeatedly:

}
