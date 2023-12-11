	.text
 	.global f
f:
	ret

	.text
 	.global main
main:
	addi sp, sp, -8
	sw s0, 4(sp)
	sw ra, 0(sp)
	call f
	li t0, 1
	mv a0, t0
	lw s0, 4(sp)
	lw ra, 0(sp)
	addi sp, sp, 8
	ret

