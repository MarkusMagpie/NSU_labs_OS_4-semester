write = 4
exit  = 1
.text
_start:
        movl    $write, %eax
        movl    $1, %ebx
        lea     str, %ecx
        movl    $len, %edx
        int     $0x80

        movl    $exit, %eax
        xorl    %ebx, %ebx
        int     $0x80
.data
str:    .ascii "Hello, world!\n"
len = . -str
.globl  _start

// компилируй так: 
// as -o hello.o hello.S
// ld -o hellob hello.o