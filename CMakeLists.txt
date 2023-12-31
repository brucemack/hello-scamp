cmake_minimum_required(VERSION 3.13)
# OPTIONAL!
#include(pico_sdk_import.cmake)
project(hello-scamp C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-fstack-protector-all -Wall -Wpedantic -g)

# Desktop build
add_executable(unit-test-7a
  unit-test-7a.cpp 
  Symbol6.cpp CodeWord12.cpp CodeWord24.cpp 
  Frame30.cpp FileModulator.cpp Util.cpp TestModem.cpp 
  TestModem2.cpp 
  ClockRecoveryPLL.cpp
  ClockRecoveryDLL.cpp
  fixed_math.cpp fixed_fft.cpp 
  Demodulator.cpp TestDemodulatorListener.cpp
)

add_executable(unit-test-3
  unit-test-3.cpp 
  Util.cpp
  fixed_math.cpp fixed_fft.cpp 
  Symbol6.cpp CodeWord12.cpp CodeWord24.cpp 
  Frame30.cpp FileModulator.cpp TestModem.cpp 
  TestModem2.cpp ClockRecoveryPLL.cpp
  Demodulator.cpp TestDemodulatorListener.cpp
)

add_executable(unit-test-2
  unit-test-2.cpp 
  fixed_math.cpp 
  fixed_fft.cpp 
  Util.cpp
  ClockRecoveryPLL.cpp
  ClockRecoveryDLL.cpp
  Symbol6.cpp 
  CodeWord12.cpp 
  CodeWord24.cpp 
  Frame30.cpp   
)

add_executable(unit-test-4
  unit-test-4.cpp 
  Util.cpp
  fixed_math.cpp fixed_fft.cpp 
  Symbol6.cpp CodeWord12.cpp CodeWord24.cpp 
  Frame30.cpp FileModulator.cpp TestModem.cpp 
  TestModem2.cpp ClockRecoveryPLL.cpp
  Demodulator.cpp TestDemodulatorListener.cpp
)

# PICO build
#pico_sdk_init()

#add_executable(main
#  main.cpp HD44780.cpp HD44780_PCF8574.cpp 
#  tests/TestI2CInterface.cpp
#  pico/PICOI2CInterface.cpp pico/PICOClockInterface.cpp
#)
#pico_enable_stdio_usb(main 1)
#pico_enable_stdio_uart(main 1)

#target_link_libraries(main pico_stdlib hardware_i2c)
