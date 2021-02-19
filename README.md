### Project init

I'm sorry, but now project can be build only using Linux-based systems. 
I work on the issue, but this isn't a first priority now, so it will take 
some time to resolve this.

#### Build system
To init build folder, call `cmake` with the following arguments: 
```commandline
cmake -DCMAKE_C_COMPILER_WORKS=1 
    -DCMAKE_ASM_COMPILER:PATH=[path to ASM compiler] 
    -DCMAKE_C_COMPILER:PATH=[path to C compiler] 
    -DCMAKE_C_OBJCOPY:PATH=[path to objcopy]
    -DFONTS_GEN_UTIL:PATH=[path to ssd1306fnt.py]
```

#### Dependencies
- ```python3``` >= 3.5
- _Python:_ https://github.com/alexeychurchill/SSD1306Fnt
- _Python:_ ```freetype-py```
