# Assignment 2

This repository contains the implementation for assignment 2 of the Operating Systems course.

## Overview

In this assignment, we:

1. **Parsed command-line arguments** using `getopt`, including:
   - `-n` : Number of threads to run simultaneously
   - `-t` : Duration of busy period (in seconds)
   - `-s` : Scheduling policy for each thread (`SCHED_FIFO` or `SCHED_NORMAL`)
   - `-p` : Priority for real-time threads

2. **Created multiple worker threads** and stored their information using `Thread_info` structures and C++ vectors.

3. **Set CPU affinity** for all threads to run on CPU 0 using `CPU_ZERO`, `CPU_SET`, and `pthread_attr_setaffinity_np`.

4. **Assigned scheduling policies and priorities** using `pthread_attr_setschedpolicy` and `pthread_attr_setschedparam`.

5. **Synchronized all threads** to start at the same time using `pthread_barrier_t`.

6. **Implemented busy-waiting** by looping and measuring elapsed time with `clock_gettime()`.

7. **Explored real-time scheduling behavior**, including the effect of `kernel.sched_rt_runtime_us`.

## Directory Structure

- `sched_demo_313551099.cpp` – Source code for the scheduling demo
- `Makefile` – Build configuration
- `report.pdf` – Full report with experiment results and analysis

## Build Instructions

To compile the program, simply run:
```
make
```
This will generate an executable named:
```
sched_demo_313551099
```
To clean up the build files:
```
make clean
```

## Usage Example
```
sudo ./sched_demo_313551099 -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30
```
