CC 	   = gcc
CFLAGS = -g -Wall
SRC    = main.c utils.c processes.c
OBJS   = $(SRC:.c=.o)

fshell: $(OBJS)
	$(CC) $(OBJS) -g -o fshell
	rm -f *.o