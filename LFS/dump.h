#ifndef DUMP_H_
#define DUMP_H_

#include "funcionesLFS.h"

int err_flag;
int full_space;
int bloque_dump;
int espacio_libre;
int size;
int j;
t_list *lista_bloques_tmp;
t_list *memtable_aux;
int space_full;
extern FILE *fp_dump;
int suma_size;
char root[128] = "";


void ejecutar_dump();

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
void liberar_memtable_aux();
void guardar_bloques_metadata(t_list *lista_bloques_tmp);
void liberar_lista_bloques(t_list *lista_bloques_tmp);
void dump();

#endif /* DUMP_H_ */
