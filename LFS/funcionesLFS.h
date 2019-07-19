#ifndef FUNCIONESLFS_H_
#define FUNCIONESLFS_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdbool.h>
#include <sys/time.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/stat.h>
#include "requests.h"
#include "conexionesLFS.h"
#include <signal.h>
#include <semaphore.h>

#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <commons/bitarray.h>
#include <commons/config.h>

#include <arpa/inet.h>
#include <sys/socket.h>

/*** ESTRUCTURAS ***/

struct op_control{
	pthread_mutex_t tabla_sem;
	pthread_mutex_t mutex;
	sem_t drop_sem;
	int drop_flag;
	int otros_flag;
	char *tabla;
};

struct describe {
	char *name;
	char *consistency;
	int partitions;
	int compaction_time;
};

struct param_compactacion {
	char *tabla;
	int tiempo_compact;
};

typedef struct {
	unsigned long timestamp;
	uint16_t key;
	char* value;
}t_registro;

typedef struct {
	int size;
	int num;
	t_list *lista_registros;
}t_particion;

typedef struct {
	char *name_tabla;
	t_list *lista_particiones;
}t_tabla;

struct bloques_tmp{
	char *tabla;
	t_list *bloques;
	int size_total;
};

struct bloque{
	int nr_block;
};

//↓↓↓↓↓↓↓↓BUSQUEDA DE TABLAS Y BLOQUES↓↓↓↓↓↓↓↓//
typedef struct{
	char *name;
}t_bloque;

typedef struct{
	unsigned long timestamp;
	char *valor;
}t_par_valor_timestamp;

//↑↑↑↑↑↑↑BUSQUEDA DE TABLAS Y BLOQUES↑↑↑↑↑↑↑//

//Generales
void consola();
void mandarAEjecutarRequest(request* requestAEjecutar);
void iniciar_variables();
void crear_control_op(char *tabla);
t_list* op_control_list;
sem_t requests_disponibles;
sem_t bloques_bitmap;

//Compactacion
t_list *lista_tabla_compact;
pthread_t h_compactacion;

struct flag_y_tabla{
	char *tabla;
	int exit_flag;
};

//Usadas por varias funciones
t_bitarray *bitarray;
int obtener_particiones_metadata(char* tabla);
char *obtener_consistencia_metadata(char* tabla);
int obtener_tiempo_compactacion_metadata(char* tabla);
void buscar_bloques_particion(char *tabla, int particion_buscar, int type_flag, t_list *bloques_buscar);
int existe_tabla(char *tabla);
int nr_particion_key(uint16_t key, int nr_particiones_metadata);
int obtener_size_particion(char *tabla, int particion_buscar);
t_list *filtrar_tabla_memtable(char *tabla);
t_list *filtrar_particion_tabla(t_list *tabla_encontrada, int particion_buscar);
void crear_bitarray(int nr_blocks);
void crear_bloques(int nr_blocks);

void tabla_destroy(t_tabla *self);
t_list* cola_requests;
void cargar_op_control_tablas();
void modificar_op_control(char *tabla, int mod_flag);
int contar_archivos_con_extension(char *root,char* extension);
int nr_particion_key(uint16_t key, int nr_particiones_metadata);
int controlar_bloques_disponibles(int cantArchivos);
void desmarcar_bloque_bitmap(t_bloque *elemento);
t_bloque *crear_bloque_buscar(char *bloque);
void bloque_destroy(t_bloque *self);
void compactacion_tablas_existentes();


//para ejecutar requests
void ejecutar_peticion();

//funciones para obtener campos individuales de una request
char* get_tabla(char* comando);
char* get_value(char* comando);
double get_timestamp(char* comando);
int get_key(char* comando);
//double get_particiones(comando);
//double get_tiempo_compactacion(comando);
char* get_consistencia(char* comando);
int get_particiones(char *comando);
int get_tiempo_compactacion(char *comando);

//funciones de rutina para cada tipo de operacion, el parametro comando es el choclo entero leido por consola
void rutina_select(char* comando);
void rutina_insert(char* comando);
void rutina_create(char* comando);
void rutina_drop(char* comando);
void rutina_describe(char* comando);

//INSERT
t_registro *crear_registro(unsigned long timestamp, uint16_t key, char *value);
void registro_destroy(t_registro *self);
t_particion *crear_particion_memtable(int size, int particion_buscar);
void particion_destroy(t_particion *self);
t_particion *agregar_registro_en_particion_nueva(t_particion *nueva_particion, t_registro *registro_nuevo);
t_tabla *crear_tabla_memtable(char *tabla);


//CREATE
void agregar_salto_de_linea(char *string);
void guardar_bitarray(int index);
int elegir_bloque_libre(int nr_bloques);
void crear_particiones(char *dir, int particiones);
int crear_tabla_FS(char *tabla, int particiones, char *consistencia, int compact_time);
void crear_hilo_compactacion(char *tabla, int tiempo_compactacion);

//DESCRIBE
int tipo_describe(char *comando);
struct describe *crear_descripcion(char *name, char *consistencia, int particiones, int tiempo_compactacion);
void descripcion_destroy(struct describe *self);
void liberar_descripcion(t_list *lista_describe);
void cargar_datos_tabla(char *tabla);
void mostrar_campos_describe(void *element);
void mostrar_descripciones_metadata(t_list *lista_describe);
void agregar_tablas_a_describir();
void describe_full();
void describe_particular(char *comando);

//DROP
void iterar_busqueda_de_bloques(void (foo)(char *, int, int, t_list *), char *name, int part, int flag, t_list *lista, int cant);
void eliminar_contenido_de_tabla(char *tabla);
void eliminar_tabla_fisicamente(char *tabla);
void desmarcar_bloque_bitmap(t_bloque *elemento);
void liberar_bloques(t_list *bloques_buscar);
void eliminar_tabla(char *tabla);

//DUMP
void ejecutar_dump();
pthread_mutex_t dump_semaphore;



#endif /* FUNCIONESLFS_H_ */
