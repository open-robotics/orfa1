; vim: set syntax=avr ai :

#include <avr/io.h>

.global SIG_OUTPUT_COMPARE3A
.global SIG_OUTPUT_COMPARE3B
.global SIG_OUTPUT_COMPARE3C
.global SIG_OUTPUT_COMPARE1C


.macro process_timer OCRX, TCCRX, block, FOC_MASK

	push R31
	in R31, 0x3F
	push R31
	push R30
	push R25
	push R24
	push R23
	push R22

	lds R30, table_ptr + \block * 2
	lds R31, table_ptr + \block * 2 + 1
	ld R24, Z+
	ld R25, Z+
	lds R22, \OCRX
	lds R23, \OCRX+1
	add R22, R24
	adc R23, R25
	sts \OCRX + 1, R23
	sts \OCRX, R22
	lds R25, \TCCRX
	ori R25, \FOC_MASK
	ldi R24, hi8(calc_ocr + \block * 18 + 18)
	cpi R30, lo8(calc_ocr + \block * 18 + 18)
	cpc R31, R24
	brne 1f
	sbiw R30, 18
	sts \TCCRX, R25
	sts \TCCRX, R25
1:	
	sts \TCCRX, R25
	sts table_ptr + \block * 2, R30
	sts table_ptr + \block * 2 + 1, R31
	
	pop R22
	pop R23
	pop R24
	pop R25
	pop R30
	pop R31
	out 0x3F, R31
	pop R31
	reti

;	*OCRX += *table_ptr[block];
;	if (table_ptr[block] == calc_ocr[block] + 8 ) {
;		table_ptr[block] = calc_ocr[block];
;		*TCCRX |= FOC_MASK;
;		*TCCRX |= FOC_MASK;
;	} else {
;		table_ptr[block]++;
;	}
;	*TCCRX |= FOC_MASK;

.endm

SIG_OUTPUT_COMPARE3A:
	process_timer OCR3A, TCCR3C, 2, (1 << FOC3A)

SIG_OUTPUT_COMPARE3B:
	process_timer OCR3B, TCCR3C, 0, (1 << FOC3B)

SIG_OUTPUT_COMPARE3C:
	process_timer OCR3C, TCCR3C, 3, (1 << FOC3C)

SIG_OUTPUT_COMPARE1C:
	process_timer OCR1C, TCCR1C, 1, (1 << FOC1C)


