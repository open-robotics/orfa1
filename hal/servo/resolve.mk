# -*- Makefile -*-

HAL_SERVO_CMD = yes

ifeq ($(PLATFORM),OR_AVR_M32_D)
	SLLD = gpio
	HAL_SERVO_NTIM2 = yes
else
	ifeq ($(PLATFORM),OR_AVR_M128_DS)
		SLLD = 4017_half
	else
		SLLD = 4017
	endif
endif

ifeq ($(HAL_SERVO_CMD),yes)
	DEFINES += -DHAL_WITH_SERVO_CMD
	INCLUDE_DIRS += -I${ORFA}/hal/servo
	HAL_SRC += ${ORFA}/hal/servo/servo_cmd_lld.c
endif

ifeq ($(HAL_SERVO_NTIM2),yes)
	DEFINES += -DHAL_SERVO_NTIM2
endif

INCLUDE_DIRS += -I${ORFA}/hal/servo/${SLLD}

HAL_SRC += ${ORFA}/hal/servo/${SLLD}/servo_lld.c

