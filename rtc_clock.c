/*
 * Real-Time Clock Display (AVR + DS1307 + 16x2 LCD)
 * ----------------------------------------------------
 * Reads current time/date from a DS1307 RTC module over I2C (bit-banged
 * TWI) and displays it on a 16x2 character LCD in 4-bit mode, along with
 * the day of the week.
 */

#define F_CPU 1000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

// LCD configuration
#define lcdPort PORTD
#define lcdDir  DDRD
#define RS 0
#define E  1

// DS1307 I2C address
#define DS1307_ADDRESS 0xD0

/****************************************
 * LCD Driver (4-bit mode)
 ****************************************/
void lcdPortInit(void) {
    lcdDir = 0xFF;
    PORTC |= (1 << 0) | (1 << 1);  // Enable pull-ups on SCL and SDA lines
}

void writeCommand(char command) {
    lcdPort = (command & 0xF0) | (1 << E);
    lcdPort = (command & 0xF0);
    _delay_us(50);
    lcdPort = (command << 4) | (1 << E);
    lcdPort = (command << 4);
    _delay_ms(3);
}

void writeCharacter(char character) {
    lcdPort = (character & 0xF0) | (1 << E) | (1 << RS);
    lcdPort = (character & 0xF0) | (1 << RS);
    _delay_us(50);
    lcdPort = (character << 4) | (1 << E) | (1 << RS);
    lcdPort = (character << 4) | (1 << RS);
    _delay_ms(3);
}

void writeString(char *text) {
    while (*text) writeCharacter(*text++);
}

void setXy(int x, int y) {
    char row_offsets[4] = {0x80, 0xC0, 0x94, 0xD4};
    writeCommand(row_offsets[x] + y);
}

void lcdInit(void) {
    lcdPortInit();
    writeCommand(0x33);
    writeCommand(0x32);
    writeCommand(0x28);  // 4-bit mode, 2 lines
    writeCommand(0x0C);  // Display on, cursor off
    writeCommand(0x06);  // Entry mode
}

/****************************************
 * I2C (bit-banged TWI) Driver
 ****************************************/
void i2c_init(void) {
    TWSR = 0x00;
    TWBR = (uint8_t)((F_CPU / 100000UL) - 16) / 2;
    TWCR = (1 << TWEN);
}

void i2c_start(void) {
    TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

void i2c_stop(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}

void i2c_write(uint8_t data) {
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
}

uint8_t i2c_read_ack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

uint8_t i2c_read_nack(void) {
    TWCR = (1 << TWINT) | (1 << TWEN);
    while (!(TWCR & (1 << TWINT)));
    return TWDR;
}

/****************************************
 * DS1307 RTC Driver
 ****************************************/
void ds1307_init(void) {
    i2c_start();
    i2c_write(DS1307_ADDRESS);
    i2c_write(0x07); // Control Register
    i2c_write(0x00); // Disable square wave output
    i2c_stop();
}

void ds1307_set_time(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    i2c_start();
    i2c_write(DS1307_ADDRESS);
    i2c_write(0x00); // Start at seconds register
    i2c_write(seconds);
    i2c_write(minutes);
    i2c_write(hours);
    i2c_stop();
}

void ds1307_set_date(uint8_t day, uint8_t date, uint8_t month, uint8_t year) {
    i2c_start();
    i2c_write(DS1307_ADDRESS);
    i2c_write(0x03); // Start at day register
    i2c_write(day);
    i2c_write(date);
    i2c_write(month);
    i2c_write(year);
    i2c_stop();
}

void ds1307_get_time(uint8_t *hours, uint8_t *minutes, uint8_t *seconds) {
    i2c_start();
    i2c_write(DS1307_ADDRESS);
    i2c_write(0x00); // Start at seconds register
    i2c_start();
    i2c_write(DS1307_ADDRESS | 1);
    *seconds = i2c_read_ack();
    *minutes = i2c_read_ack();
    *hours   = i2c_read_nack();
    i2c_stop();
}

void ds1307_get_date(uint8_t *day, uint8_t *date, uint8_t *month, uint8_t *year) {
    i2c_start();
    i2c_write(DS1307_ADDRESS);
    i2c_write(0x03); // Start at day register
    i2c_start();
    i2c_write(DS1307_ADDRESS | 1);
    *day   = i2c_read_ack();
    *date  = i2c_read_ack();
    *month = i2c_read_ack();
    *year  = i2c_read_nack();
    i2c_stop();
}

/****************************************
 * Main
 ****************************************/
int main(void) {
    uint8_t seconds, minutes, hours, day, date, month, year;
    char timeString[16];
    char dateString[16];
    char *dayOfWeek;

    lcdInit();
    i2c_init();
    ds1307_init();

    while (1) {
        ds1307_get_time(&hours, &minutes, &seconds);
        ds1307_get_date(&day, &date, &month, &year);

        switch (day) {
            case 1: dayOfWeek = "Sun"; break;
            case 2: dayOfWeek = "Mon"; break;
            case 3: dayOfWeek = "Tue"; break;
            case 4: dayOfWeek = "Wed"; break;
            case 5: dayOfWeek = "Thu"; break;
            case 6: dayOfWeek = "Fri"; break;
            case 7: dayOfWeek = "Sat"; break;
            default: dayOfWeek = ""; break;
        }

        snprintf(dateString, sizeof(dateString), "%s %02x/%02x/20%02x",
                 dayOfWeek, date, month, year);
        setXy(0, 0);
        writeString(dateString);

        snprintf(timeString, sizeof(timeString), "Time: %02x:%02x:%02x",
                 hours, minutes, seconds);
        setXy(1, 0);
        writeString(timeString);

        _delay_ms(1000);
    }
}
