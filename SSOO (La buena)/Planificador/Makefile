all: fragmenta procshed

fragmenta: fragmenta.c
	gcc -c fragmenta.c
	@echo Compilacion de fragmenta.c completada con éxito...

procshed: procshed.c fragmenta.o
		gcc -o procshed procshed.c fragmenta.o
		@echo Compilacion de procshed.c completada con éxito...

clean:
		rm procshed
		rm fragmenta.o
		@echo Archivos borrados con éxito...
run:
		./procshed