default: all

votante: votante.c
	gcc -o votante votante.c
	@echo Compilación de votante.c terminada con éxito... 

visor:  visor.c
	gcc -o visor visor.c
	@echo Compilación de visor.c terminada con éxito... 

recogeVotos: recogeVotos.c
	gcc -o recogeVotos recogeVotos.c
	@echo Compilación de recogeVotos.c terminada con éxito... 

all: votante visor recogeVotos

clean:
	rm votante
	rm visor
	rm recogeVotos
	@echo Archivos eliminados con éxito...
