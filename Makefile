##########################
# Makefile
# Author: Kevin Littell
# Date: Nov 15, 2018
# COSC 3750, homework 8
# makes a program using the listed dependencies
##########################

CC=gcc
CFLAGS=-ggdb -Wall -std=gnu99
RM=/bin/rm -f

.PHONEY: clean

wyshell:
	${CC} ${CFLAGS} wyshell.c wyscanner.c -o wyshell

clean:
	${RM} wyshell
