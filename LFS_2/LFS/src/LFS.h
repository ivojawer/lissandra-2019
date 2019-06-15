#ifndef LFS_H_
#define LFS_H_

#include "../../Bibliotecas/helper.h"
#include <commons/bitarray.h>

/*** ESTRUCTURAS ***/
typedef struct {
	int puerto_escucha;
	char* punto_montaje;
	int retardo; //milisegundos
	int tamanio_value;
	int tiempo_dump; //milisegundos
} t_LFS_config;

struct FS {
	int block_size;
	int nr_blocks;
	char *magic_number;
};

struct describe {
	char *name;
	char *consistency;
	int partitions;
	int compaction_time;
};

typedef enum {
	SELECT, INSERT, CREATE, DESCRIBE, DROP, OPERACION_INVALIDA
} LFS_OPERACION;


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



/*** GLOBALES ***/
t_log* logger=NULL;
t_LFS_config LFS_config;
struct FS *FS;
int size_particion=0;
t_list *memtable;
t_list *particion_encontrada;
t_list *registros_encontrados;
t_list *tabla_encontrada;
t_list *lista_describe;
int wait_particiones = 1;
t_bitarray *bitarray;
char *consistency;

pthread_t hilo_consola;


/*** PROTOTIPOS ***/
void cargar_configuracion_inicial();
static void cargar_configuracion_FS();
void crear_bitarray(int nr_blocks);
FILE *fp;
void setear_bitarray(t_bitarray *bitarray, int nr_blocks);
int iniciar_servidor();
void atender_cliente(int socket_escucha);

void iniciar_consola();
void procesar_comando(char* comando);
void iniciar_variables();


//funciones para obtener campos individuales de una request
LFS_OPERACION get_operacion(char* comando);
char* get_tabla(char* comando);
char* get_value(char* comando);
double get_timestamp(char* comando);
//char* get_consistencia(comando);
//double get_particiones(comando);
//double get_tiempo_compactacion(comando);

//funciones de rutina para cada tipo de operacion, el parametro comando es el choclo entero leido por consola
void rutina_select(char* comando);
void rutina_insert(char* comando);
void rutina_create(char* comando);
void rutina_drop(char* comando);
void rutina_describe(char* comando);

//unas funciones de string que tuve que incluir para los algoritmos
int str_first_index_of(char c, char* cadena);
int str_last_index_of(char c, char* cadena);

/*
 * Usadas por varias funciones
 */
int obtener_particiones_metadata(char* tabla);
char *obtener_consistencia_metadata(char* tabla);
int obtener_tiempo_compactacion_metadata(char* tabla);

//SELECT
t_bloque *crear_bloque_buscar(char *bloque);
static void agregar_bloque_busqueda(t_list *lista_agregar, t_bloque *bloque_buscado);
t_par_valor_timestamp *crear_valor_timestamp_buscar(unsigned long timestamp, char *valor);
static void agregar_valor_timestamp(t_list *timestamp_valor, t_par_valor_timestamp *par_valor_timestamp);
int buscar_tabla_FS(char *tabla_name);
int nr_particion_key(uint16_t key, int nr_particiones_metadata);
void buscar_key_bloques(char* bloque_nr, uint16_t key, t_list *timestamp_valor, int flag_last_bloque, int size_bloque);
void cargar_timestamp_value(t_list *bloques_buscar, t_list *timestamp_valor, uint16_t key);
int existe_tabla(char *tabla);
int cargar_bloques(char *root, t_list *bloques_buscar);
void buscar_bloques_particion(char *tabla, int particion_buscar, int type_flag, t_list *bloques_buscar);
bool comparar_nombre(char *tabla, void *tabla_mt);
bool comparar_particion(int particion_buscar, void *particion);
bool comparar_registro(uint16_t key, void *registro);
t_list *filtrar_tabla_memtable(char *tabla);
t_list *filtrar_particion(t_list *tabla_encontrada, int particion_buscar);
t_list *filtrar_registros_particion(t_list *particion_encontrada, uint16_t key);
t_par_valor_timestamp *filtrar_timestamp_mayor(t_list *timestamp_valor, int list_size);
void buscar_en_todos_lados(char *tabla, uint16_t key, int particion_buscar);
void comparar_key_y_agregar_valor (uint16_t key_recv, uint16_t key, char *valor, unsigned long timestamp, t_list *timestamp_valor);
/*
 * Variables para control de registros en bloques
 */

int control = 0;
int flag_key_value = 0;
char array_aux[128] = "";


//INSERT
t_registro *crear_registro(unsigned long timestamp, uint16_t key, char *value);
void registro_destroy(t_registro *self);
t_particion *crear_particion_memtable(int size, int particion_buscar);
void particion_destroy(t_particion *self);
t_particion *agregar_registro_en_particion_nueva(t_particion *nueva_particion, t_registro *registro_nuevo);
t_tabla *crear_tabla_memtable(char *tabla);


//CREATE
void agregar_salto_de_linea(char *string);
void guardar_bitarray(t_bitarray *bitarray, long int index);
int elegir_bloque_libre(int nr_bloques);
void crear_particiones(char *dir, int particiones);
int crear_tabla_FS(char *tabla, int particiones, char *consistencia, int compact_time);

//DESCRIBE
int tipo_describe(char *comando);
struct describe *crear_descripcion(char *name, char *consistencia, int particiones, int tiempo_compactacion);
void descripcion_destroy(struct describe *self);
void liberar_descripcion(t_list *lista_describe);
void cargar_datos_tabla(char *tabla);
void mostrar_campos_describe(void *element);
void mostrar_descripciones_metadata(t_list *lista_describe);
void agregar_tablas_a_describir();
void describe_full(char *comando);
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
clock_t t_ini_dump, t_fin_dump;
int bloque_dump;
int espacio_libre;
int size;
int j;
t_list *lista_bloques_tmp;
int space_full;
FILE *fp_dump;
int suma_size;

struct bloques_tmp *crear_bloques_tmp(char *tabla);
void destroy_nr_bloque(void *elemento);
void bloques_tmp_destroy(void *elemento);
struct bloque *crear_nr_bloque(int nr_bloque);
void agregar_bloque_lista_tmp(t_list *bloques, int bloque_dump);
void grabar_registro(char *root, char *registro_completo, int length_registro, int space_full, int index, int table_change,
					 struct bloques_tmp *bloques_tmp, int flag_close_file);
void guardar_registros_en_bloques(t_registro *registro_recv, int table_change, struct bloques_tmp *bloques_tmp_tabla);
int contar_temporales(char *root);
int obtener_size(char *size_array);
int archivo_vacio(FILE *fp);
void agregar_bloque_particion(void *elemento);
void liberar_elementos_particiones(void *elemento);
void liberar_tabla(void *elemento);
void liberar_memtable();
void guardar_bloques_metadata(t_list *lista_bloques_tmp);
void liberar_lista_bloques(t_list *lista_bloques_tmp);
void dump();



#endif /* LFS_H_ */
