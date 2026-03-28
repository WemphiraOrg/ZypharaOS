/* irq_stubs.S - Stubs de IRQ en ensamblador puro */
.extern irq_dispatch

.macro IRQ_STUB num
.global irq\num\()_stub
irq\num\()_stub:
    pusha
    push $\num
    call irq_dispatch
    add $4, %esp
    popa
    iret
.endm

IRQ_STUB 0
IRQ_STUB 1
IRQ_STUB 2
IRQ_STUB 3
IRQ_STUB 4
IRQ_STUB 5
IRQ_STUB 6
IRQ_STUB 7
IRQ_STUB 8
IRQ_STUB 9
IRQ_STUB 10
IRQ_STUB 11
IRQ_STUB 12
IRQ_STUB 13
IRQ_STUB 14
IRQ_STUB 15
