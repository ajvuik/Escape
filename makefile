# Makefile
CC=gcc
LDFLAGS=-lncurses ../wiringX/build/libwiringx.so -lSDL2 -lSDL2_mixer
STD=-std=c11
escape: main.c
	$(CC) -o escape main.c $(LDFLAGS) $(STD)

