section .data
    hello db 'Hello, World!', 10
    length equ $ - hello

section .text
    global _start

_start:
    mov rax, 1          ; sys_write
    mov rdi, 1          ; stdout
    mov rsi, hello
    mov rdx, length
    syscall

    mov rax, 60         ; sys_exit
    xor rdi, rdi
    syscall