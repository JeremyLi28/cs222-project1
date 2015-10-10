################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../src/rbf/pfm.cc \
../src/rbf/rbfm.cc \
../src/rbf/rbftest4.cc 

OBJS += \
./src/rbf/pfm.o \
./src/rbf/rbfm.o \
./src/rbf/rbftest4.o 

CC_DEPS += \
./src/rbf/pfm.d \
./src/rbf/rbfm.d \
./src/rbf/rbftest4.d 


# Each subdirectory must supply rules for building sources it contributes
src/rbf/%.o: ../src/rbf/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


