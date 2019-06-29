#include "compactacion.h"

extern char* puntoDeMontaje;

void compactar(char* tabla){





}

void string_add_ending(char* string, char c){
	char str[2];
	str[0] =c;
	str[1]='\0';
	//printf("caracter a agregar:%s\n",str);
	string_append(&string,str);
	//printf("string completo:%s\n", string);
}

t_registro* stringRegistroAStruct(char* registro){//llega sin el \n al final :)
	char** stringRegistroSeparado = string_n_split(registro,3,";");
	t_registro* reg = malloc(sizeof(uint16_t)+sizeof(unsigned long)+sizeof(char)*strlen(stringRegistroSeparado[2]));
	reg->timestamp=atoi(stringRegistroSeparado[0]);
	reg ->key=atoi( stringRegistroSeparado[1]);
	reg->value=string_duplicate(stringRegistroSeparado[2]);
	return reg;
}

t_list* traerRegistrosBloques(char** bloques){
	t_list* registros = list_create();
	char* registroActual = string_new();

	void leerBloque(char* nombreBloque){
		printf("bloque actual:%s\n",nombreBloque);
		FILE* bloqueActual = fopen(nombreBloque,"r");
		char nuevoCaracter = fgetc(bloqueActual);
		while(nuevoCaracter>-1){
			//printf("caracter leido:%c\n",nuevoCaracter);
			if(nuevoCaracter == '\n'){
				printf("termino registro\n");
				list_add(registros, stringRegistroAStruct(registroActual));
				free(registroActual);
				registroActual = string_new();
			}else{
				printf("agrego caracter a reg:%i\n",nuevoCaracter);
				string_add_ending(registroActual,nuevoCaracter);
				printf("registro con caracter agregado:%s\n", registroActual);
			}
			nuevoCaracter = fgetc(bloqueActual);
		}
		fclose(bloqueActual);

	}

	string_iterate_lines(bloques, (void*) leerBloque);

	//free(registroActual);
	printf("hola final\n");
	return registros;
}
