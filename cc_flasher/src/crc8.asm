

main:
		MOV DPTR, #0x8000
		MOV 0xC7, #4
		mov		R1,#0

		mov 	R6,#0
		mov		R7,#8
loop:
		clr 	a
		MOVC 	A,@A+DPTR
		inc 	DPTR
		xrl 	a,R1
		rl		A
		mov 	R1,A
		
		DJNZ	R6, loop
		DJNZ	R7, loop
		
		nop;  change to 0xA5 for break

crc8:
		push	acc			; Save Acc
		push	0			; Save R0
		push	acc			; Save Value
		mov	r0,#8			; Number Bits In Byte

lp1:		xrl	a,b			; Calculate CRC
		rrc	a			; Move To Carry
		mov	a,b			; Get The Last CRC Value
		jnc	lp2			; Skip If Data == 0
		xrl	a,#0x18			; Update The CRC Value

lp2:		rrc	a			; Position The New CRC
		mov	b,a			; Store The New CRC
		pop	acc			; Get The Remaining Bits
		rr	a			; Position The Next Bit
		push	acc			; Save The Remaining Bits
		djnz	r0,lp1			; Repeat For 8 Bits
;
		pop	acc			; Clean Up Stack
		pop	0			; Recover R0
		pop	acc			; Recover Acc
		ret				; Return To Caller

		mov a,r1
