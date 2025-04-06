
# Assignment 1

This repository contains the implementation for Assignment 1 of the Operating Systems course.

## Overview

In this assignment, we:

1. **Recompiled the Linux kernel** with a custom release name including the student ID.
2. **Implemented a new system call** `revstr`, which reverses a string in place.
3. **Tested** the system call with a user-space C program.
4. **Packaged configuration and environment** for reproducibility.

## Directory Structure

- `revstr/` – Source code for the custom system call
- `revstr_test.c` – User-space test program for `revstr`
- `config/` – Kernel configuration files
- `packages_list.txt` – List of required packages
- `report.pdf` – Full report with step-by-step documentation

## Usage

Follow the steps in `report.pdf` to recompile the kernel and test the new system call.

## Kernel Version

Customized kernel boot entry:  
`Ubuntu with Linux 6.1.0-os-313551099`

---
