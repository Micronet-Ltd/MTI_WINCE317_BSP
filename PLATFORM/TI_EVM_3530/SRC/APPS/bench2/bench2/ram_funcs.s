;		INCLUDE armmacros.s
;		INCLUDE platform.s
		INCLUDE kxarm.h

	TEXTAREA
;-----------------------------------------------------------------------
	EXPORT	ramread32
	LEAF_ENTRY		ramread32
	stmfd   r14!, {r0-r11, r13}

		mov r1, r1, asr #5
ram_read32
			ldmia r0!, {r4-r11}
			subs  r1, r1, #1
			bne ram_read32

	ldmfd   sp!, {r0-r11, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread32t
	LEAF_ENTRY		ramread32t
	stmfd   sp!, {r0-r11, lr}

		mov r1, r1, asr #5
ram_read32t
			ldmia r0!, {r4-r11}
			subs  r0, r0, #32
			subs  r1, r1, #1
			bne ram_read32t

	ldmfd   sp!, {r0-r11, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread8
	LEAF_ENTRY		ramread8
	stmfd   sp!, {r0-r11, lr}

		mov r1, r1, asr #3
ram_read8
			ldmia r0!, {r4-r5}
			subs  r1, r1, #1
			bne ram_read8

	ldmfd   sp!, {r0-r11, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread8t
	LEAF_ENTRY		ramread8t
	stmfd   sp!, {r0-r11, lr}

		mov r1, r1, asr #3
ram_read8t
			ldmia r0!, {r4-r5}
			subs  r0, r0, #8
			subs  r1, r1, #1
			bne ram_read8t

		ldmfd   sp!, {r0-r11, pc}

;-----------------------------------------------------------------------
	EXPORT	mem_cpy_16
	LEAF_ENTRY		mem_cpy_16
	stmfd   sp!, {r0-r6, lr}

;		mov r2, r2, asr #2
memcpy_16
			ldmia	r1!,{r3-r6}
			stmia	r0!,{r3-r6}
			subs	r2,r2,#16
			bne		memcpy_16

	ldmfd   sp!, {r0-r6, pc}


;-----------------------------------------------------------------------
	EXPORT	mem_cpy_2
	LEAF_ENTRY		mem_cpy_2
	stmfd   sp!, {r0-r4, lr}

;		mov r1, r1, asr #1
memcpy_2
			ldrh r4, [r1], #2
			strh r4, [r0], #2
			subs r2, r2, #2
			bne memcpy_2

		ldmfd   sp!, {r0-r4, pc}


;-----------------------------------------------------------------------
	EXPORT	mem_cpy
	LEAF_ENTRY		mem_cpy
	stmfd   sp!, {r0-r4, lr}

memcpy
		ldrb r4, [r1], #1
		strb r4, [r0], #1
		subs r2, r2, #1
		bne memcpy

	ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------

	END