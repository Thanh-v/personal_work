default rel

section .data
    hello db 'Hello, World!', 13, 10
    hello_len equ $ - hello

section .text
    global main

    extern GetStdHandle
    extern WriteFile
    extern ExitProcess

main:
    ; Windows x64 stack:
    ; 32-byte shadow space + alignment/local space
    sub rsp, 38h

    ; GetStdHandle(STD_OUTPUT_HANDLE)
    mov ecx, -11
    call GetStdHandle

    ; WriteFile(
    ;   hFile,
    ;   lpBuffer,
    ;   nNumberOfBytesToWrite,
    ;   lpNumberOfBytesWritten,
    ;   lpOverlapped
    ; )
    mov rcx, rax
    lea rdx, [hello]
    mov r8d, hello_len
    lea r9, [rsp + 28h]

    mov qword [rsp + 20h], 0
    call WriteFile

    ; ExitProcess(0)
    xor ecx, ecx
    call ExitProcess
