################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Application/EEPROM.c \
../Application/bbb.c \
../Application/display.c \
../Application/fonts.c \
../Application/measurement.c \
../Application/motor.c \
../Application/ssd1306.c \
../Application/test.c \
../Application/utility.c 

OBJS += \
./Application/EEPROM.o \
./Application/bbb.o \
./Application/display.o \
./Application/fonts.o \
./Application/measurement.o \
./Application/motor.o \
./Application/ssd1306.o \
./Application/test.o \
./Application/utility.o 

C_DEPS += \
./Application/EEPROM.d \
./Application/bbb.d \
./Application/display.d \
./Application/fonts.d \
./Application/measurement.d \
./Application/motor.d \
./Application/ssd1306.d \
./Application/test.d \
./Application/utility.d 


# Each subdirectory must supply rules for building sources it contributes
Application/%.o: ../Application/%.c Application/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F103xB -c -I../Inc -I../Application -I../Drivers/STM32F1xx_HAL_Driver/Inc -I../Drivers/STM32F1xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F1xx/Include -I../Drivers/CMSIS/Include -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

