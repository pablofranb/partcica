#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"
#include <stdlib.h>
#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps);
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2);
void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup);
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombre);
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos);
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
              char *nombreantiguo, char *nombrenuevo);
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, 
             EXT_DATOS *memdatos, char *nombre);
int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           char *nombre,  FILE *fich);
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos,
           EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock,
           EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich);
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	 char *comando=(char*)malloc(LONGITUD_COMANDO*sizeof(char));
	 char *orden=(char*)malloc(LONGITUD_COMANDO*sizeof(char));
	 char *argumento1=(char*)malloc(LONGITUD_COMANDO*sizeof(char));
	 char *argumento2=(char*)malloc(LONGITUD_COMANDO*sizeof(char));
	 if(comando==NULL || orden==NULL || argumento1==NULL || argumento2==NULL){
		printf("Error al asignar memoria\n");
		return 1;
	}
	 int i,j;
	 unsigned long int m;
     EXT_SIMPLE_SUPERBLOCK ext_superblock;
     EXT_BYTE_MAPS ext_bytemaps;
     EXT_BLQ_INODOS ext_blq_inodos;
     EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
     EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
     EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
     int entradadir;
     int grabardatos;
     FILE *fent;
     
     // Lectura del fichero completo de una sola vez
     
     
     fent = fopen("particion.bin","r+b");
     fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    
     
     
     memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
     memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
     memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
     memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
     memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);
     
     // Buce de tratamiento de comandos
     for (;;){
		 do {
		 printf (">> ");
		 fflush(stdin);
		 fgets(comando, LONGITUD_COMANDO, stdin);
		 } while (ComprobarComando(comando,orden,argumento1,argumento2) !=0);
	     if (strcmp(orden,"dir")==0) {
     	       Directorio(directorio,&ext_blq_inodos);
		printf("Funcion directorio\n");
        	    continue;
            }
	 if (strcmp(orden, "info") == 0) {
        	LeeSuperBloque(&ext_superblock);
      		  printf("Funcion info\n");
        	continue;
    		}         
         // Escritura de metadatos en comandos rename, remove, copy     
         Grabarinodosydirectorio(directorio,&ext_blq_inodos,fent);
         GrabarByteMaps(&ext_bytemaps,fent);
         GrabarSuperBloque(&ext_superblock,fent);
         if (grabardatos)
           GrabarDatos(memdatos,fent);
         grabardatos = 0;
         //Si el comando es salir se habrán escrito todos los metadatos
         //faltan los datos y cerrar
         if (strcmp(orden,"salir")==0){
            GrabarDatos(memdatos,fent);
            fclose(fent);
            return 0;
         }
     }
}
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){

	sscanf(strcomando,"%s %s %s",orden, argumento1, argumento2); //LEEMOS EL comando del usuario y lo dividimos en tres partes 
	printf("%s %s %s\n",orden,argumento1,argumento2);
	if ((strcmp(orden,"dir")!=0) && (strcmp(orden,"info")!=0) && (strcmp(orden,"rename")!=0) &&
	    (strcmp(orden,"copy")!=0) && (strcmp(orden,"remove")!=0) &&
   		 (strcmp(orden,"imprimir")!=0) && (strcmp(orden,"salir")!=0) &&
   		 (strcmp(orden,"bytemaps")!=0)) {
    		printf("ERROR: Comando ilegal [bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
    return 1; //SI ALGUNO DE  LOS COMANDOS NO ES DE LOS PUESTOS SERA VERDADERO Y XARA ERROR
}


}
void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
		printf("Directorio de archivos:\n");
   		 printf("Nombre de archivo       Tamaño (bytes)\n");
    		printf("----------------------  --------------\n");
		for(int i=0;i<MAX_FICHEROS;i++){
		if(directorio[i].dir_inodo!=NULL_INODO){
		 printf("%-22s %14u\n", directorio[i].dir_nfich,inodos->blq_inodos[i].size_fichero); //obtenemos el nombre del fichero con 22 caracteres a la izquierda y su tamaño

		}
		}
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup) //LeeSuperBloque: Muestra información sobre la partición (tamaño, inodos libres, bloques libres, etc.).
{
        printf("Bloques libres = %d\n", psup -> s_free_blocks_count);
    printf("inodos libres = %d\n", psup -> s_free_inodes_count);
    printf("Bloques partición = %d\n", psup -> s_blocks_count);
	printf("Bloque %d Bytes\n", psup -> s_block_size);
    printf("inodos partición = %d\n", psup -> s_inodes_count);
    printf("Primer bloque de datos = %d\n", psup -> s_first_data_block);
}
// Función para grabar inodos y directorio
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich) {
    // Implementación de la función
}

// Función para grabar byte maps
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich) {
    // Implementación de la función
}

// Función para grabar el superbloque
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich) {
    // Implementación de la función
}

// Función para grabar los datos
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich) {
    // Implementación de la función
}
