################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../cr_startup_lpc17.c \
../main.c \
../sensor_luz.c 

OBJS += \
./cr_startup_lpc17.o \
./main.o \
./sensor_luz.o 

C_DEPS += \
./cr_startup_lpc17.d \
./main.d \
./sensor_luz.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU C Compiler'
	arm-none-eabi-gcc -DDEBUG -D__USE_CMSIS=CMSISv1p30_LPC17xx -D__CODE_RED -D__REDLIB__ -I"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\FreeRTOS_Source\demo_code" -I"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\CMSISv1p30_LPC17xx\inc" -I"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\FreeRTOS_Source\include" -I"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\FreeRTOS_Source\portable" -I"C:\Users\Tolavio\Documents\LPCXpresso_6.1.4_194\workspace\Lib_EaBaseBoard\inc" -I"C:\Users\Tolavio\Documents\LPCXpresso_6.1.4_194\workspace\Lib_MCU\inc" -O0 -g3 -fsigned-char -c -fmessage-length=0 -fno-builtin -ffunction-sections -mcpu=cortex-m3 -mthumb -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


