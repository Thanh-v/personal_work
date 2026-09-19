# How to run hello_world_asm
- Copy this code:

nasm -f elf64 hello_world_asm.asm -o hello_world_asm  
ld hello_world_asm.o -o hello_world_asm  
./hello_world_asm  
