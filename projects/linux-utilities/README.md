# Project 1: Unix/Linux Utilities - `tucp`

This project implements a simple version of the Unix `cp` command called `tucp`.

## Objectives

- Catch up with C programming
- Learn basics of Linux shell/command-line
- Understand implementation of Linux utilities
- Introduction to system programming and system calls

## Usage

```bash
cd projects/linux-utilities/src
gcc -o tucp tucp.c

./tucp file dest-file
./tucp file dest-dir
./tucp file1 file2 ... dest-dir
```

## Run Demo

```bash
cd projects/linux-utilities/src
gcc -o demo-tucp demo-tucp.c
./demo-tucp
```
