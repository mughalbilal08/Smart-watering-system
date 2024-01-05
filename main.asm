.include "m328pdef.inc"
.include "delay.inc"
.include "UART.inc"
.include "LCD_1602.inc"

.def A = r16
.def AH = r17

.cseg
.org 0x00
	LCD_init
	LCD_backlight_ON
	sbi DDRB,PB3 ; PB3 = 11 Arduino Port
	sbi PORTB,PB3
	; ADC Configuration
	LDI A,0b11000111 ; [ADEN ADSC ADATE ADIF ADIE ADIE ADPS2 ADPS1 ADPS0]
	STS ADCSRA,A
	LDI A,0b01100000 ; [REFS1 REFS0 ADLAR – MUX3 MUX2 MUX1 MUX0]
	STS ADMUX,A ; Select ADC0 (PC0) pin
	SBI PORTC,PC0 ; Enable Pull-up Resistor
	ldi r16, 0x00

	Serial_begin ; start UART

ledSerialOff:
	cbi PORTB, PB3
	delay 2500
	delay 2500
	rjmp loop
ledSerialOn:
	sbi PORTB, PB3
	delay 2500
	delay 2500
	rjmp loop
ledSerialOffJmp:
	rjmp ledSerialOff

loop:
	delay 1000
	; Get Serial Data
	Serial_read
	cpi r20, '1'
	breq ledSerialOn
	cpi r20, '2'
	breq ledSerialOffJmp
	call analogRead ; Read Sensor value in AH
	Serial_writeReg_ASCII AH ; Send Serial Data
	Serial_writeNewLine
	LCD_Clear
	LDI ZL, LOW (2 * reading_string)
	LDI ZH, HIGH (2 * reading_string)
	LCD_send_a_string
	LCD_send_a_register AH
	cpi AH, 180
	brmi ledOff
	rjmp ledOn
loopEnd:
	delay 1000
rjmp loop

ledOn:
	sbi PORTB, PB3
	rjmp loopEnd

ledOff:
	cbi PORTB, PB3
	rjmp loopEnd


analogRead:
	LDS A,ADCSRA ; Start Analog to Digital Conversion
	ORI A,(1<<ADSC)
	STS ADCSRA,A
	wait:
		LDS A,ADCSRA ; wait for conversion to complete
		sbrc A,ADSC
		rjmp wait

	LDS A,ADCL ; Must Read ADCL before ADCH
	LDS AH,ADCH
	delay 100
	ret

reading_string: .db "Reading: ",0