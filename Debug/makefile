################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Example10.axf

# Tool invocations
Example10.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MCU Linker'
	arm-none-eabi-gcc -nostdlib -L"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\CMSISv1p30_LPC17xx\Debug" -L"C:\Users\Tolavio\Documents\Inatel\P9\EC020\Rtos\source-code-for-LPC17xx-examples\FreeRTOS_Source\Debug" -L"C:\Users\Tolavio\Documents\LPCXpresso_6.1.4_194\workspace\Lib_EaBaseBoard\Debug" -L"C:\Users\Tolavio\Documents\LPCXpresso_6.1.4_194\workspace\Lib_MCU\Debug" -Xlinker --gc-sections -Xlinker -Map=Example10.map -mcpu=cortex-m3 -mthumb -T "Example10_Debug.ld" -o "Example10.axf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Example10.axf
	-@echo ' '

post-build:
	-@echo 'Performing post-build steps'
	-arm-none-eabi-size Example10.axf; # arm-none-eabi-objcopy -O ihex Example10.axf Example10.hex ;
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: post-build

-include ../makefile.targets
