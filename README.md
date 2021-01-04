### Project init

To init build folder, call `cmake` with the following arguments: 
```commandline
cmake -DCMAKE_C_COMPILER_WORKS=1 -DCMAKE_ASM_COMPILER:PATH=[path to ASM compiler] -DCMAKE_C_COMPILER:PATH=[path to C compiler] -DCMAKE_C_OBJCOPY:PATH=[path to objcopy]
```
