# Makefile for sched_demo_<student_id>

# Phony targets
.PHONY: all clean

# Variables
CC = g++
CFLAGS = -pthread -Wall -O2
TARGET = sched_demo_313551099
SRC = sched_demo_313551099.cpp

# Targets
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) *.o
