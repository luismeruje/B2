EXEC = cartas


cartas: Etapa2.c
	gcc -o cartas Etapa2.c
	sudo cp cartas /usr/lib/cgi-bin
	sudo cp -r images /var/www/html/cards


