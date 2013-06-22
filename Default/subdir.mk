################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
O_SRCS += \
../conn_client.o \
../conn_io.o \
../conn_server.o \
../getMesh_paket.o \
../llist.o 

C_SRCS += \
../Mesh_Switch.c \
../conn_client.c \
../conn_io.c \
../conn_server.c \
../llist.c 

OBJS += \
./Mesh_Switch.o \
./conn_client.o \
./conn_io.o \
./conn_server.o \
./llist.o 

C_DEPS += \
./Mesh_Switch.d \
./conn_client.d \
./conn_io.d \
./conn_server.d \
./llist.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O2 -g3 -Wall -pthread -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


