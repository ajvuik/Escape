# Makefile
CC=gcc
LDFLAGS=-lncurses ../wiringX/build/libwiringx.so
STD=-std=c11
escape: main.c
	$(CC) -o escape main.c $(LDFLAGS) $(STD)

