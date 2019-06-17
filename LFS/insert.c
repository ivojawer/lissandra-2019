#include "funcionesLFS.h"

extern t_list* memtable;

extern int tamanioValue;

t_registro *crear_registro(unsigned long timestamp, uint16_t key, char *value)
{
	t_registro *new = malloc(sizeof(t_registro));
	new->key = key;
	new->timestamp = timestamp;
	new->value = strdup(value);

	return new;
}

void registro_destroy(t_registro *self)
{
	free(self->value);
	free(self);
}


t_particion *crear_particion_memtable(int size, int particion_buscar)
{
	t_particion *new = malloc(sizeof(t_particion));
	new->num = particion_buscar;
	new->size = size;
	new->lista_registros = list_create();

	return new;
}

void particion_destroy(t_particion *self)
{
	free(self->lista_registros);
	free(self);
}

t_particion *agregar_registro_en_particion_nueva(t_particion *nueva_particion, t_registro *registro_nuevo)
{
	list_add(nueva_particion->lista_registros, registro_nuevo);
	return nueva_particion;
}

t_tabla *crear_tabla_memtable(char *tabla)
{
	t_tabla *new = malloc(sizeof(t_tabla));
	new->name_tabla = strdup(tabla);
	new->lista_particiones = list_create();
	return new;
}

void tabla_destroy(t_tabla *self)
{
	free(self->name_tabla);
	free(self->lista_particiones);
	free(self);
}


t_tabla *agregar_particion_en_tabla_nueva(t_tabla *nueva_tabla, t_particion *nueva_particion)
{
	list_add(nueva_tabla->lista_particiones, nueva_particion);
	return nueva_tabla;
}

void agregar_tabla_memtable(t_list* memtable, t_tabla *nueva_tabla)
{
	list_add(memtable, nueva_tabla);
}


void agregar_particion_en_tabla_existente(char* tabla, t_particion *nueva_particion)
{
	int i;
	t_tabla *tabla_extraida;
	for(i=0; i<list_size(memtable); i++){
		tabla_extraida = (t_tabla *)list_get(memtable, i);
		if (!strcmp(tabla_extraida->name_tabla, tabla)){
			list_add(tabla_extraida->lista_particiones, nueva_particion);
		}
	}
}


void agregar_registro_en_particion_existente(char *tabla, int particion_buscar, t_registro *registro_nuevo)
{
	int i, j;
	t_tabla *tabla_extraida = malloc(sizeof(t_tabla));
	t_particion *particion_extraida = malloc(sizeof(t_particion));

	for(i=0; i<list_size(memtable); i++){
		tabla_extraida = (t_tabla*)list_get(memtable, i);
		if (!strcmp(tabla_extraida->name_tabla, tabla)){
			for(j=0; j<list_size(tabla_extraida->lista_particiones); j++){
				particion_extraida = (t_particion*)list_get(tabla_extraida->lista_particiones, j);
				if(particion_extraida->num == particion_buscar){
					list_add(particion_extraida->lista_registros, registro_nuevo);
				}
			}
		}
	}
	//free(particion_extraida);
	//free(tabla_extraida);
}




//void liberar_tabla_encontrada(t_list *tabla_encontrada)
//{
//	void liberar_tabla(void *elemento){
//		tabla_destroy((t_tabla *)elemento);
//	}
//
//	list_iterate(tabla_encontrada, liberar_tabla);
//}

void rutina_insert(char* comando)
{
	printf("Operacion: INSERT\n");

	char *tabla=get_tabla(comando);
	printf("Tabla: %s\n", tabla);

	uint16_t key = get_key(comando);
	printf("Key: %d\n", key);

	char* value = get_value(comando);
	if(strlen(value)+1>tamanioValue){
		printf("El value ingresado supera el tamaño maximo permitido.\n");
		return;
	}else{
		printf("Value: %s\n", value);

		unsigned long timestamp = get_timestamp(comando);
		printf("Timestamp: %lu\n", timestamp);

		if(existe_tabla(tabla)){
			int nr_particiones_metadata = obtener_particiones_metadata(tabla);
			int particion_buscar = nr_particion_key(key, nr_particiones_metadata);
			int size = obtener_size_particion(tabla, particion_buscar);
			if(size>0){
				printf("Particion del registro: %d\n", particion_buscar);

				t_list *tabla_encontrada = list_create();
				tabla_encontrada = filtrar_tabla_memtable(tabla);
				t_list *lista_particion_encontrada = filtrar_particion_tabla(tabla_encontrada, particion_buscar);
				t_registro *registro_nuevo = malloc(sizeof(t_registro));
				registro_nuevo = crear_registro(timestamp, key, value);

				if(lista_vacia(tabla_encontrada)){
					t_particion *nueva_particion = malloc(sizeof(t_particion));
					nueva_particion = crear_particion_memtable(size, particion_buscar);
					agregar_registro_en_particion_nueva(nueva_particion, registro_nuevo);
					t_tabla *nueva_tabla = malloc(sizeof(t_tabla));
					nueva_tabla = crear_tabla_memtable(tabla);
					agregar_particion_en_tabla_nueva(nueva_tabla, nueva_particion);
					agregar_tabla_memtable(memtable, nueva_tabla);
				}else if(lista_vacia(lista_particion_encontrada)){ //tabla en memtable pero la particion esta vacia
					t_particion *nueva_particion = crear_particion_memtable(size, particion_buscar);
					agregar_registro_en_particion_nueva(nueva_particion, registro_nuevo);
					agregar_particion_en_tabla_existente(tabla, nueva_particion);
				}else{
					agregar_registro_en_particion_existente(tabla, particion_buscar, registro_nuevo);
				}
				printf("Registro agregado a la particion.\n");
			}

		}else{
			printf("No se pudo encontrar la tabla %s.\n",tabla);
		}
	}
//	liberar_tabla_encontrada(tabla_encontrada);
}

