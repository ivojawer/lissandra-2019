################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../LFS.c \
../conexionesLFS.c \
../consolaLFS.c \
../funcionesLFS.c 

OBJS += \
./LFS.o \
./conexionesLFS.o \
./consolaLFS.o \
./funcionesLFS.o 

C_DEPS += \
./LFS.d \
./conexionesLFS.d \
./consolaLFS.d \
./funcionesLFS.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


