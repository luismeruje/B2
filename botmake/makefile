CFLAGS=-Wall -Wextra -pedantic -Wno-long-long -O2
OBJS = $(Bot_1.4.c)
EXEC = bot_inteligente

instalar: $(EXEC)
	touch instalar

$(EXEC): $(OBJS)
	$(CC) Bot_1.4.c $(CFLAGS) -o $(EXEC) $(OBJS)
