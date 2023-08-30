all: fragmenta msh

fragmenta: fragmenta.c
	gcc -c fragmenta.c
	@echo Compilacion de fragmenta.c completada con éxito...

msh: msh.c fragmenta.o
		gcc -o msh msh.c fragmenta.o
		@echo Compilacion de msh.c completada con éxito...

clean:
		rm msh
		rm fragmenta.o
		@echo Archivos borrados con éxito...
run:
		./msh

minishell: all run clean