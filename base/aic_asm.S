/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

.text
.code 32
.align 	0

#include "asm_decls.h"

/*
 * Definition of a few constants, for readability.
 */
#define AIC_IVR 0xFFFFF100   /* AIC Interrupt Vector Register */
#define AIC_EOICR 0xFFFFF130 /* End Of Interrupt Control Register */

/**********************************************************
 * Default handlers for unknown IRQ and FIQ interrupts, and
 * the spurious IRQ handler and unknown exception handler.
 */
	.global nxos__unhandled_exception
nxos__unhandled_exception:
	b nxos__unhandled_exception


/**********************************************************
 * IRQ entry/exit routine. This gets called for all
 * interrupts, saves state, switches to Supervisor mode,
 * enables nested IRQ handling, and calls the service
 * routing that was registered with the AIC for the given
 * IRQ.
 */
	.global nxos__irq_handler
nxos__irq_handler:
	/* Save the return address and return processor mode registers
	 * on the IRQ stack. This is needed to allow nested interrupts,
	 * where a higher-priority interrupt halts execution of a lower
	 * priority interrupt.
	 */
	sub lr, lr, #4
	stmfd sp!, {lr}
	mrs lr, spsr
	stmfd sp!, {lr}

	/* If we were interrupted out of User or System mode, there is a
	 * scheduler running, and we need to save the task state nicely.
	 *
	 * This code is beautiful. An ode of joy to everything ARM. Conditional
	 * execution, status bit updates, the next few instructions
	 * have it all.
	 *
	 * Note that each code path pushes one final value on the
	 * stack. This value is 0 if this interrupt is nested, and !0 if
	 * this interrupt is interrupting a user task. This value will be
	 * used to figure out how to return from the interrupt later.
	 */

	/* Compare the SPSR mode bits to both 0000 and 1111 (usr/sys mode). */
	ands lr, lr, #7
	eornes lr, lr, #7

	/* Switch to either sys or svc mode to save the state. */
	msreq cpsr_c, #(MODE_SYS | IRQ_FIQ_MASK)
	msrne cpsr_c, #(MODE_SVC | IRQ_FIQ_MASK)

	/* Save the state. If we're saving a user task, also grab the task
	 * CPSR and PC from the IRQ stack.
	 */
	stmfd sp!, {r0-r12,lr}
	ldreq r0, =__irq_stack__
	ldmeqdb r0, {r1,r2}
	stmeqfd sp!, {r1,r2}
	movne r0, #0

	/* Get the IVR value. */
	ldr r1, =AIC_IVR
	ldr r2, [r1]

	/* If we're in Protected mode (usually for JTAG debugging), we
	 * need to write back to the IVR register to tell the AIC it
	 * can dispatch other higher priority interrupts again.
	 * In normal mode, this has no effect, so we can safely do it.
	 */
	str r1, [r1]

	/* Switch to Supervisor mode if necessary, reenable IRQ and FIQ handling,
	 * and stack either 0 (nested IRQ) or the address of the IRQ stack base (for
	 * a task IRQ).
	 */
	msr cpsr_c, #MODE_SVC
	stmfd sp!, {r0}

	/* Dispatch the IRQ to the registered handler. */
	mov lr, pc
	bx r2

	/* Restore the interrupted state. How this is done depends on the value at
	 * the top of the stack, as explained above.
	 *
	 * Note that we inhibit IRQ and FIQ handling during task restoration only if
	 * we're restoring a user task. Getting interrupted while we're tweaking around
	 * in system mode would confuse the IRQ handler into breaking things.
	 */
	ldmfd sp!, {r0}
	cmp r0, #0
	msrne cpsr_c, #(MODE_SYS | IRQ_FIQ_MASK)
	ldrne r0, =__irq_stack__
	ldmnefd sp!, {r1,r2}
	stmnedb r0, {r1,r2}
	ldmfd sp!, {r0-r12,lr}

	/* Switch back to IRQ mode and tell the AIC that the interrupt has been
	 * handled.
	 */
	msr cpsr_c, #(MODE_IRQ | IRQ_FIQ_MASK)
	ldr lr, =AIC_EOICR
	str lr, [lr]

	/* Restore the SPSR */
	ldmfd sp!, {lr}
	msr spsr_all, lr

	/* Restore execution to the main code. */
	ldmfd sp!, {pc}^

// TODO(dave): Reimplement aborts.

/**********************************************************
 * Nested interrupt disable/enable routines. The
 * interrupts_disable routine disables IRQ/FIQ handling,
 * and increments a nesting counter. The interrupts_enable
 * routine decrements the counter, and reenables interrupts
 * when the counter reaches zero.
 *
 * The counter is managed by the AIC driver. Its address
 * is passed to these functions in r0.
 */
	.global nxos__interrupts_mask
	.global nxos__interrupts_unmask
nxos__interrupts_mask:
	mrs r1, cpsr
	orr r1, r1, #IRQ_FIQ_MASK
	msr cpsr_c, r1

	ldrb r1, [r0]
	add r1, r1, #1
	strb r1, [r0]

	bx lr

nxos__interrupts_unmask:
	ldrb r1, [r0]
	subs r1, r1, #1
	strb r1, [r0]

	bxne lr

	mrs r0, cpsr
	bic r0, r0, #IRQ_FIQ_MASK
	msr cpsr_c, r0
	bx lr
