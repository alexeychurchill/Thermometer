## Thermometer

A firmware for STM32F103-based device which purpose is to measure temperature, 
log it with given period into the flash memory. Data reading is planned to 
be done through USB. Device also can show immediate temperature, time. 
It has OLED display. 

### Project building

#### Build system
To init build folder, call `cmake` with the following arguments: 
```commandline
cmake -DCMAKE_C_COMPILER_WORKS=1 
    -DCMAKE_ASM_COMPILER:PATH=[path to ASM compiler] 
    -DCMAKE_C_COMPILER:PATH=[path to C compiler] 
    -DCMAKE_C_OBJCOPY:PATH=[path to objcopy]
    -DFONTS_GEN_UTIL:PATH=[path to ssd1306fnt.py]
```

#### IDE
Any IDE which supports cmake projects can be used for work on the project. 
For example, it's possible to use CLion. To use CLion, do the following: 
1. Open CMake project in the CLion
2. Go to `File` -> `Settings` -> `Build, Execution, Deployment` -> `CMake`
3. Set `CMake options`: just take them form previous readme section
4. Set proper `Build directory`. Usually, it's `build`

#### Dependencies
- ```python3``` >= 3.5
- _Python:_ https://github.com/alexeychurchill/SSD1306Fnt
- _Python:_ ```freetype-py```

### TODO
Just some short notes on things I'm going to do:
* [ ] **_Refactoring:_ Stop using `__` for file-privates!** Remove all existing usages
* [x] ~~_GUI:_ Add SETTINGS screen~~
* [ ] _GUI:_ Add LOGGER configuration screen
* [ ] _Logging:_ Implement sleeping mode and wake up by alarm
* [ ] _Logging:_ Implement driver for the W25Q*
* [ ] _1-Wire:_ Implement CRC calculation
* [ ] _1-Wire:_ Add errors "management"
* [ ] _Refactoring:_ Simplify 1-Wire driver: just make some kinda FSM!
* [ ] _Refactoring:_ Get rid of OneWire_t[^1]. Just use .h as an interface
  for other modules. Generally, it would be great to get rid off two 1-Wire headers.
* [ ] _Refactoring:_ Move IRQ handlers outside of 1-Wire drive sources
* [ ] _Refactoring:_ Review DS18B20 driver and clean it up
* [ ] _Refactoring:_ Move all "config" constants to the `config.h` file (e.g., display size)
* [ ] _Refactoring:_ Refactor UART routines (it requires **general heavy refactoring**)
* [ ] _Display:_ Use fast-mode (400kHz) I<sup>2</sup>C
* [ ] _Display:_ Use DMA for frame data transfer
* [ ] _General:_ Implement convenient "virtual timer" using SysTick timer
* [ ] _USB:_ Implement MSC USB device 
* [ ] _USB:_ Implement FAT16 emulation **_Investigate!_**
* [ ] _USB:_ Implement temperature logs file emulation **_Investigate!_**

_To be continued..._

[^1]: Access to several different 1-Wire busses can be organized in easier and more logical way. 
For example, we can pass bus number to the 1-Wire functions. Or, just use another alternative.
