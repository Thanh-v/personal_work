# How to run hello_world_asm
- Copy this code:

nasm -f win64 hello_world_asm.asm -o hello_asm.obj  
gcc hello_asm.obj -o hello_asm.exe -lkernel32  
./hello_asm.exe  

# How to run hello_world_c
- Copy this code:

gcc hello_world_c.c -o hello_c  
./hello_c  

# How to run hello_world_py
- Copy this code:

python3 hello_world_py.py
