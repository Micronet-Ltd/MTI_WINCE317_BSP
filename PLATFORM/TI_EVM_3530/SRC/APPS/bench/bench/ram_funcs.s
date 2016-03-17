;		INCLUDE armmacros.s
;		INCLUDE platform.s
		INCLUDE kxarm.h

	TEXTAREA
;-----------------------------------------------------------------------
	EXPORT	ramread32
	LEAF_ENTRY		ramread32
	stmfd   sp!, {r0-r11, lr}

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
	EXPORT	ramread4
	LEAF_ENTRY		ramread4
	stmfd   sp!, {r0-r4, lr}

		mov r1, r1, asr #2
ram_read4
			ldr	 r4, [r0], #4
			subs r1, r1,   #1
			bne ram_read4

	ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread4t
	LEAF_ENTRY		ramread4t
	stmfd   sp!, {r0-r4, lr}
		mov r1, r1, asr #2
ram_read4t
			ldr r4, [r0]
			add  r1, r1, #1
			subs r1, r1, #2
			bne ram_read4t

	ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread2
	LEAF_ENTRY		ramread2
	stmfd   sp!, {r0-r4, lr}

		mov r1, r1, asr #1
ram_read2
			ldrh r4, [r0], #2
			subs r1, r1, #1
			bne ram_read2

		ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread2t
	LEAF_ENTRY		ramread2t
	stmfd   sp!, {r0-r4, lr}

		mov r1, r1, asr #1
ram_read2t
			ldrh r4, [r0]
			add  r1, r1, #1
			subs r1, r1, #2
			bne ram_read2t

	ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------
	EXPORT	ramread
	LEAF_ENTRY		ramread
	stmfd   sp!, {r0-r4, lr}

ram_read
		ldrb r4, [r0], #1
		subs r1, r1, #1
		bne ram_read

	ldmfd   sp!, {r0-r4, pc}

;-----------------------------------------------------------------------
	EXPORT	ramreadt
	LEAF_ENTRY		ramreadt
	stmfd   sp!, {r0-r4, lr}

ram_readt
			ldrb r4, [r0]
			add  r1, r1, #1
			subs r1, r1, #2
			bne ram_readt

	ldmfd   sp!, {r0-r4, pc}


	END