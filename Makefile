CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -O2 -Isensor -Iactuators
LDFLAGS =

SRC = \
    sensor/sensor.c \
    actuators/led_actuator.c \
    actuators/buzzer_actuator.c \
    controller/ctl.c

.PHONY: all ctl64 ctl32 clean

all: ctl64

ctl64:
	$(CC) $(CFLAGS) -m64 -o ctl64 $(SRC) $(LDFLAGS)

ctl32:
	$(CC) $(CFLAGS) -m32 -o ctl32 $(SRC) $(LDFLAGS)

clean:
	rm -f ctl64 ctl32
	rm -f sensor/*.o actuators/*.o controller/*.o
