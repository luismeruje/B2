EXEC = cartas


cartas: Etapa2.c
	gcc -o cartas Etapa2.c

instalar : $(EXEC)
	sudo cp $(EXEC) /usr/lib/cgi-bin/
	sudo cp -r images  /var/www/html/cards/

$(EXEC): Etapa2.c
	gcc -o $(EXEC) Etapa2.c 

