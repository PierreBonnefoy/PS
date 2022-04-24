CIBLE = initial chef mecano client 
CC = gcc -W -Wall -pedantic -O3
all : $(CIBLE)
	$(CC) $<.c -o $<
clean :
	rm $(CIBLE)