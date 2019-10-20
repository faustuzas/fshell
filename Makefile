CC 	   = gcc
CFLAGS = -g -Wall
SRC    = main.c utils.c
# HDR    = $(SRC:.c=.h)
OBJS   = $(SRC:.c=.o)

fshell: $(OBJS)
	$(CC) $(OBJS) -g -o fshell
	rm -f *.o

# .c.o: $*.c $(HDR)

# $(SRC): $(HDR)
