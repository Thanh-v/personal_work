# How to run hello_world_asm
- Copy this code:

nasm -f elf64 hello_world_asm.asm -o hello_world_asm  
ld hello_world_asm.o -o hello_world_asm  
./hello_world_asm  

# How to run hello_world_c
- Copy this code:

gcc hello_world_c.c -o hello_world_c  
./hello_world_c  

# How to run hello_world_py
- Copy this code:

python3 hello_world_py.py
