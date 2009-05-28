; vim: set syntax=avr ai :
#include <avr/io.h>

.global SIG_ADC

SIG_ADC:
	push R24
	in R24, 0x3F
	push R24
	push R25

lds R24, gate_adc_conversion_channel
	cpi R24, 0xFF
	brne 1f
	
	push R30
	push R31

	add R24, R24
	ldi R25, 0
	ldi R30, lo8(gate_adc_result)
	ldi R31, hi8(gate_adc_result)
	add R30, R24
	adc R31, R25

	lds R25, gate_adc_config
	andi R25, 0x04
	breq 2f

	lds R24, ADCL
	std Z+0, R24
	lds R24, ADCH
	std Z+1, R24
	rjmp 3f

2:
	lds R24, ADCH
	std Z+0, R24
	ldi R24, 0
	std Z+1, R24

3:
	lds R24, gate_adc_conversion_channel
	inc R24
	andi R24, 0x07
	lds R25, gate_adc_conversion_mask
	lsl R25
	brne 7f
	ldi R25, 0x01

7:
	pop R31
	pop R30

	rjmp 4f

1: 
	ldi R24, 0x00
	ldi R25, 0x01

4:
	lds R30, gate_adc_mask

6:
	mov R31, R30
	and R31, R25
	brne 5f
	inc R24
	lsl R25
	rjmp 6b

5:
	sts gate_adc_conversion_mask, R25
	sts gate_adc_conversion_channel, R24

	lds R25, ADMUX
	andi R25, ~0x07
	or R25, R24
	sts ADMUX, R25

	pop R25
	pop R24
	out 0x3F, R24
	pop R24
	reti


;	if (conversion_channel != 0xFF) {
;		if (config & 0x04) {
;			result[conversion_channel] = (ADCH << 8) + ADCL;
;		} else {
;			result[conversion_channel] = ADCH;
;		}
;		conversion_channel++;
;		conversion_channel &= 0x07;
;		conversion_mask <<= 1;
;		if (!conversion_mask) {
;			conversion_mask = 0x01;
;		}
;	} else {
;		conversion_channel = 0;
;		conversion_mask = 0x01;
;	}
;	while (!(conversion_mask & mask)) {
;		conversion_mask <<= 1;
;		conversion_channel++;
;	}
;	ADMUX = (ADMUX & ~0x07) | conversion_channel;


