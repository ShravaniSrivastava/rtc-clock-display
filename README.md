# RTC Clock Display (AVR + DS1307 + 16x2 LCD)

Bare-metal embedded C firmware for an AVR microcontroller that reads live
time and date from a DS1307 real-time clock module over I2C and displays it
on a 16x2 character LCD, along with the day of the week.

## How it works

- A bit-level I2C (TWI) driver is implemented from scratch to communicate
  with the DS1307 RTC.
- The LCD is driven in 4-bit mode using direct register-level GPIO control
  (no Arduino libraries).
- The main loop continuously polls the RTC and refreshes the display once
  per second.

## Hardware

- AVR microcontroller (ATmega-series)
- DS1307 RTC module
- 16x2 character LCD (HD44780-compatible)

## Key implementation details

- **I2C driver**: `i2c_init`, `i2c_start`, `i2c_write`, `i2c_read_ack/nack`
  — implemented directly against AVR's TWI registers (`TWCR`, `TWDR`,
  `TWSR`, `TWBR`).
- **LCD driver**: 4-bit mode command/character writes with correct enable
  pulse timing (`writeCommand`, `writeCharacter`).
- **RTC read/write**: BCD-encoded time and date registers read from and
  written to the DS1307 over I2C.

## My role

Wrote the full firmware — I2C protocol driver, LCD driver, and RTC
interfacing logic — without using pre-built Arduino abstraction libraries,
working directly against AVR datasheet register definitions.

## Build

Compile with `avr-gcc` for your target MCU and flash with `avrdude`, e.g.:

<img width="975" height="763" alt="image" src="https://github.com/user-attachments/assets/c4954fb1-08b4-462c-b0ff-6cb61f3a9473" />

<img width="975" height="471" alt="image" src="https://github.com/user-attachments/assets/a59d89fa-5462-457e-8b5f-f45513f384b4" />


```
avr-gcc -mmcu=atmega328p -DF_CPU=1000000UL -Os -o rtc_clock.elf rtc_clock.c
avr-objcopy -O ihex rtc_clock.elf rtc_clock.hex
avrdude -c usbasp -p atmega328p -U flash:w:rtc_clock.hex
```

Adjust `-mmcu` and F_CPU to match your actual hardware setup.
