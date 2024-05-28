    addi x1, x0, 0
    addi x2, x0, 0
    addi x3, x0, 18

START_LOOP:
    beq x2, x3, END_LOOP
    sw x2, 16(x0)
    add x1, x1, x2
    lw x2, 16(x0)
    addi x2, x2, 1
    beq x0, x0, START_LOOP

END_LOOP:
    addi x2, x2, -1
    sub x1, x1, x2

    addi x1, x0, 11
    addi x2, x0, 37
    and x3, x1, x2
    or x4, x1, x2

    andi x3, x1, 45
    ori x4, x2, 23

    nop
