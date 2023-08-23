add $v0, $at, $zero
addi $a1, $v1, 10
addiu $a3, $a2, 10
addu $t0, $t1, $t2
and $t3, $t4, $t5
andi $t6, $t7, 10
beq $s0, $s1, label_0
label_0:
bne $s2, $s3, label_1
label_1:
j label_2
label_2:
jal label_3
label_3:
jr $s4
lbu $s5, 10($s6)
lhu $s7, 10($t8)
ll $t9, 10($k0)
lui $k1, 10
lw $sp, 10($fp)
nor $ra, $gp, $v1
or $v0, $v1, $v2
ori $v0, $v1, 10
slt $v0, $v1, $v2
slti $v0, $v1, 10
sltiu $v0, $v1, 10
sltu $v0, $v1, 10
sll $v0, $v1, 10
srl $v0, $v1, 10
sb $v0, 10($v1)
sc $v0, 10($v1)
sh $v0, 10($v1)
sw $v0, 10($v1)
sub $v0, $v1, $v2
subu $v0, $v1, $v2
bclt fp_label_0
fp_label_0: 
bclf fp_label_1
fp_label_1: 
div $v0, $v1
divu $v0, $v1
add.s $f0, $f2, $f4
add.d $f6, $f8, $f10
c.eq.s $f12, $f14
c.lt.s $f16, $f18
c.le.s $f20, $f22
c.eq.d $f24, $f26
c.lt.d $f28, $f30
c.le.d $f0, $f2
div.s $f0, $f2, $f4
div.d $f0, $f2, $f4
mul.s $f0, $f2, $f4
mul.d $f0, $f2, $f4
sub.s $f0, $f2, $f4
sub.d $f0, $f2, $f4
lwcl $f0, 10($v1)
ldcl $f0, 10($v1)
mfhi $v1
mflo $v1
mfc0 $v0, $v1
mult $v0, $v1
multu $v0, $v1
sra $v0, $v1, 10
swcl $f0, 10($v1)
sdcl $f0, 10($v1)