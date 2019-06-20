.text
.globl fun
fun:
# prologo:
pushq %rbp
movq %rsp, %rbp
subq $32, %rsp
# movq %rbx, -28(%rbp) 
# eu posso usar um callee-saved gratis porque sobra espaco
# mas nem vai precisar

return:
movl -4(%rbp), %eax # ret v1
movl -8(%rbp), %eax # ret v2
movl -12(%rbp), %eax # ret v3
movl -16(%rbp), %eax # ret v4
movl -20(%rbp), %eax # ret v5
movl $1, %eax # ret $1
movl $20, %eax # ret $20
movl $-20, %eax # ret $20

params:
movl %edi, -4(%rbp) # v1 < p1
movl %esi, -4(%rbp) # v1 < p2
movl %edx, -4(%rbp) # v1 < p3
movl %edi, -8(%rbp) # v2 < p1

attr:
movl %eax, -4(%rbp) # v1 < %eax

arithmetic:
addl $20, %eax
addl $1000, %eax
addl -4(%rbp), %eax
imull $20, %eax
imull $1000, %eax
imull -4(%rbp), %eax
subl $20, %eax
subl $1000, %eax
subl -4(%rbp), %eax

iflez:
cmpl $0, -4(%rbp)
jle 0

final:
# movq -28(%rbp), %rbx
leave
ret