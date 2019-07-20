#ifndef SELECT_H_
#define SELECT_H_

#include "funcionesLFS.h"

t_bloque *crear_bloque_buscar(char *bloque);
static void agregar_bloque_busqueda(t_list *lista_agregar, t_bloque *bloque_buscado);
t_par_valor_timestamp *crear_valor_timestamp_buscar(unsigned long timestamp, char *valor);
static void agregar_valor_timestamp(t_list *timestamp_valor, t_par_valor_timestamp *par_valor_timestamp);
int buscar_tabla_FS(char *tabla_name);
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
registro* buscar_en_todos_lados(char *tabla, uint16_t key, int particion_buscar);
void comparar_key_y_agregar_valor (uint16_t key_recv, uint16_t key, char *valor, unsigned long timestamp, t_list *timestamp_valor);

#endif /* SELECT_H_ */
