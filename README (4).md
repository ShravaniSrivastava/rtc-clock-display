# RTC Clock Display (AVR ATmega32 + DS1307 + 16x2 LCD)

Embedded C firmware for an AVR ATmega32 that reads the date and time from a DS1307 real-time clock over I2C (TWI) and shows them on a 16x2 character LCD, along with the day of the week. Built and validated in Proteus simulation.

## How it works

- The ATmega32's hardware TWI peripheral is used to talk to the DS1307 at 100 kHz (1 MHz system clock).
- The LCD is driven in 4-bit mode over PORTD using direct register-level port writes (no Arduino libraries).
- The main loop reads the time and date from the DS1307, formats them, and refreshes the display once per second.

Display format:

```
Sun 21/10/2026
Time: 14:35:07
```

## Hardware (simulated in Proteus)

- ATmega32 microcontroller (1 MHz)
- DS1307 RTC (I2C address 0x68, written as 0xD0 / 0xD1 in the code)
- 16x2 HD44780-compatible LCD, 4-bit mode (RS on PD0, E on PD1, data on PD4-PD7)
- I2C lines on PC0 (SCL) and PC1 (SDA), with internal pull-ups enabled in firmware

## Firmware overview

- **I2C (TWI) routines:** `i2c_init`, `i2c_start`, `i2c_stop`, `i2c_write`, `i2c_read_ack`, `i2c_read_nack`, using the `TWBR`, `TWCR` and `TWDR` registers.
- **DS1307 access:** `ds1307_get_time`, `ds1307_get_date` (burst read using a repeated start), plus `ds1307_set_time` and `ds1307_set_date`. Values are stored by the chip in BCD and printed in hex format.
- **LCD routines:** `lcdInit`, `writeCommand`, `writeCharacter`, `writeString`, `setXy`, using the high-nibble / low-nibble sequence with an enable pulse.

## How this project was built

The firmware was generated with AI assistance from my written specification of the required behaviour. I did not hand-write the code.

My contribution:

- Defined the requirements and expected output (date, day and time on the LCD from a DS1307).
- Compiled the code in Atmel Studio and iteratively resolved compile errors.
- Designed the circuit and made the connections in Proteus.
- Debugged simulation and wiring issues until the correct date and time were shown on the LCD.
- Afterwards studied the code to understand how the TWI sequence, BCD registers and the 4-bit LCD interface work.

## Known limitations

- The I2C routines do not check the TWI status register, so there is no detection of a missing device or a NACK on a write.
- Delays are blocking (`_delay_ms`), and no interrupts or timeouts are used.
- Time and date are read from the DS1307 only; `main` does not call the set-time or set-date functions, so the displayed value depends on what the RTC already holds.

## Possible improvements

- Check `TWSR` after each TWI operation and add timeouts and error handling.
- Convert BCD to decimal for display and for setting the time.
- Add buttons to set the time and date.
- Replace blocking delays with a timer-based approach.

## Tools

Atmel Studio (AVR GCC), Proteus.
