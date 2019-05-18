################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../conexionesMem.c \
../consolaMem.c \
../funcionesMemoria.c \
../memoria.c \
../segmentos.c 

OBJS += \
./conexionesMem.o \
./consolaMem.o \
./funcionesMemoria.o \
./memoria.o \
./segmentos.o 

C_DEPS += \
./conexionesMem.d \
./consolaMem.d \
./funcionesMemoria.d \
./memoria.d \
./segmentos.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -Icommons -Ireadline -I"/home/utnso/workspace/tp-2019-1c-U-TN-Tecno/Librerias/libreriaRequests" -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


