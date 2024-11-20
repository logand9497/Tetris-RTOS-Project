; G8RTOS_SchedulerASM.s
; Created: 2022-07-26
; Updated: 2022-07-26
; Contains assembly functions for scheduler.

	; Functions Defined
	.def G8RTOS_Start, PendSV_Handler

	; Dependencies
	.ref CurrentlyRunningThread, G8RTOS_Scheduler

	.thumb		; Set to thumb mode
	.align 2	; Align by 2 bytes (thumb mode uses allignment by 2 or 4)
	.text		; Text section

; Need to have the address defined in file
; (label needs to be close enough to asm code to be reached with PC relative addressing)
RunningPtr: .field CurrentlyRunningThread, 32

CPAC:
	.long 0xE000ED88

; G8RTOS_Start
;	Sets the first thread to be the currently running thread
;	Starts the currently running thread by setting Link Register to tcb's Program Counter
G8RTOS_Start:

	.asmfunc

	MOV R0, #0x04
	MSR CONTROL, R0
	ISB

	LDR R4, RunningPtr	;Loads the address of RunningPtr into R4
	LDR R5, [R4]		;Loads the currently running pointer into R5
	LDR R6, [R5]		;Loads the first thread's stack pointer into R6
	ADD R6, R6, #200
	STR R6, [R5]
	MOV SP, R6
	LDR LR, [R6, #-80]	;Loads LR with the first thread's PC
	CPSIE I

	BX LR				;Branches to the first thread

	.endasmfunc

; PendSV_Handler
; - Performs a context switch in G8RTOS
; 	- Saves remaining registers into thread stack
;	- Saves current stack pointer to tcb
;	- Calls G8RTOS_Scheduler to get new tcb
;	- Set stack pointer to new stack pointer from new tcb
;	- Pops registers from thread stack
PendSV_Handler:

	.asmfunc

	CPSID I

	vpush {s16 - s31}
	push {R4 - R11}		;Saving registers

	LDR R4, RunningPtr	;Loading R4 with the address of the currently running thread

	LDR R5, [R4]		;Loading R5 with that TCB of the currently running pointer

	STR SP, [R5]		;Storing the stack pointer in the stack pointer of the currently running thread

	MOV R7, LR		;Protect LR

	BL G8RTOS_Scheduler	;Calling the scheduler

	MOV LR, R7			;Restore LR

	LDR R4, RunningPtr	;Loading R4 with the address of the currenrly running thread

	LDR R5, [R4]		;Loading R5 with the TCB of the currently running thread

	LDR SP, [R5]		;Loading the stack pointer with the stack pointer in the TCB

	pop {R4 - R11}		;Restoring registers
	vpop {s16 - s31}

	CPSIE I

	BX LR				;Branches to new thread

	.endasmfunc

	; end of the asm file
	.align
	.end
