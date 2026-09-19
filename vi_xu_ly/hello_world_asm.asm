section .data
    hello db 'Hello, World!', 10
    length equ $ - hello

section .text
    global _start

_start:
    mov rax, 1
    mov rdi, 1
    mov rsi, hello
    mov rdx, length
    syscall

    mov rax, 60
    xor rdi, rdi
    syscall