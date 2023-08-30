/////////////////////////////
//       fragmenta.c       //
/////////////////////////////

/*
 * Nombre: fragmenta.c
 * Autor: Marcos Burdaspar Celada
 * Sinopsis: #include “fragmenta.h”  
		      char **fragmenta(const char *cadena); 
		      void borrarg(char **arg); 
 * Descripción: fragmenta(): Crea una array de char* con tantos elementos como el número 
							de fragmentos que encuentre en cadena más uno, el último vale siempre NULL
							 y es el único con tal valor, con lo que sirve para determinar el 
							final del array. Cada elemento de este array es un puntero a una zona 
							de memoria donde se encuentra uno de los fragmentos de cadena y en el 
							mismo  orden.  Los  fragmentos  de  cadena  vienen  definidos  por  estar  
							separados por uno o más espacios, pudiendo terminar en un fin de línea. 
							Si la cadena a fragmentar posee múltiples espacios en blanco, ninguno 
							de éstos se almacenará en la estructura de datos resultante.

				borrarg():  libera la memoria asociada con el puntero arg, así como las 
							zonas de memoria apuntadas por cada uno de los char* apuntados por arg
							y colocados uno tras otro hasta uno que valga NULL.

 * Valor devuelto: fragmenta() devuelve el puntero al array creado o NULL si no puede realizar 
					su función. 
 */

//--------------------------------------------------------//
// Definición de librerías a utilizar dentro del programa //
//--------------------------------------------------------//
/*
 * stdlib     -> funcionas básicas estándar
 * string     -> manejo de strings con strcmp, strcpy, strcat...
 *
 */

/* Declaracion de librerias */
#include <stdlib.h>
#include <string.h>

char **fragmenta(const char *cadena){ 						//El const es para que el puntero no cambie su contenido
	/* Declaración de variables */
    char **arg,*token, *copia;
    int numPalabras = 1; 
    int i = 0;
  
    copia = (char*)malloc(strlen(cadena)+1); 				//Reservar la misma cantidad de memoria que cadena
    														//La funcion strlen() no cuenta '\0' por eso se suma 1
    														//No hace falta poner sizeof(char) pues su tamaño es 1 byte
    strcpy(copia, cadena); 									//Se realiza una copia de la cadena original de caracteres
    token = (char *)strtok(copia, " ");  					//Funcion que extrae una cadena que este entre dos espacios de la copia
    while (token!=NULL){									//Calculamos el numero de palabras de la cadena
        if(strlen(token)>0)
            numPalabras++;
        token = strtok(NULL," ");
    }
    arg = (char**)malloc(numPalabras*(sizeof(char*))); 		//Reservamos memoria por el numero de palabras de la cadena
    strcpy(copia, cadena); 									//Volvemos ha realizar una copia exacta a la original
    token = (char *)strtok(copia, " ");
    while (token!=NULL){									//Reservamos memora igual a la palabra a guardar y se guarda dicha palabra
        if(strlen(token)>0){
            arg[i] = (char*)malloc(strlen(token)+1);
            strcpy(arg[i],token);
            i++;
        }
        token = strtok(NULL," ");
    }
    arg[numPalabras] = NULL; 								//Ultima palabra igual a NULL
    
    return arg;												//Devolvemos la estructura creada
}

void borrarg(char **arg){									//Liberamos primero el contenido de cada palabra y despues
    if (arg != NULL)										//el puntero que apunta a cada palabra
    {
        int i = 0;
        while(arg[i] != NULL){    
            free(arg[i]);
            i++;
        }
        free(arg);
    }
}