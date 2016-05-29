CFLAGS=-Wall -Wextra -pedantic -Wno-long-long -O2
OBJS = $(main.c, estrutura.c, auxiliares.c, html.c, bots.c)
EXEC = cartas

instalar: $(EXEC)
	sudo cp $(EXEC) /usr/lib/cgi-bin	
	sudo cp -r ./cards/* /var/www/html/cards/
	touch instalar

$(EXEC): $(OBJS)
	$(CC) main.c estrutura.c auxiliares.c html.c bots.c $(CFLAGS) -o $(EXEC) $(OBJS)

limpar:
	rm $(EXEC) *.o