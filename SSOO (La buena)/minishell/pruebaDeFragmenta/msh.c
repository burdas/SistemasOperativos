#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fragmenta.h"

void main()
{
	char cadena[100], **superCadena;
	int palabras;

	printf("Introduce una cadena de caracteres: ");
	fflush(stdout);
	scanf("%[^\n]", cadena);
	printf("Frase leida: %s\n", cadena);

	superCadena = fragmenta(cadena);

	palabras = 3;


	printf("--------------------------------------------\n");
	printf("Numero de palabras: %d\n", palabras);
	for (int i = 0; i < palabras; ++i)
	{
		printf("\tPalabra %d: %s\n", i + 1, superCadena[i]);
	}
	printf("--------------------------------------------\n");

}

