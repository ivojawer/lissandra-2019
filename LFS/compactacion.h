#ifndef COMPACTACION_H_
#define COMPACTACION_H_

#include "funcionesLFS.h"

void compactar(char* tabla);
bool coincide_tabla(void *elemento, char *tabla);

void escribirEnBloquesTabla(t_list* tablaParticiones,char* nombreTabla);
void escribirEnBloquesParticion(t_list* registrosDeParticion, int numeroParticion, char* tabla);
int renombrarATmpc(char* tabla);
void destruirTmpc(char* tabla, int cantidadTmpc);
t_list* traerRegistrosBloques(char** bloques);
void string_append_char(char** string, char c);

//-----------------------------------------------------
// ------------- getters paths completos --------------
//-----------------------------------------------------
char* generarNombreCompletoBloque(int nro_bloque);
char* generarNombreCompletoParticion(int numeroDeParticion,char* nombreTabla);

//-----------------------------------------------------
//--conversores array de numeroBloque --> path bloque--
//-----------------------------------------------------
char** transformarBloquesAPathBloques(char** nro_bloques);
char** transformarParticionABloques(char* tabla,int nro_particion);
char** transformarTemporalABloques(char* tabla,int nro_temporal);

//-----------------------------------------------------
// ---------- getters de nro Bloque en char* ----------
//-----------------------------------------------------
char** getBloquesParticion(char* tabla,int nro_particion);
char** getBloquesTemporal(char* tabla,int nro_temporal);


//-----------------------------------------------------
// --------- conversores registro <--> char* ----------
//-----------------------------------------------------
t_registro* stringRegistroAStruct(char* registro);
char* structRegistroAString(t_registro* registro);

//-----------------------------------------------------
// ------------- funciones para probar ----------------
//-----------------------------------------------------
void mostrarRegistro(t_registro* reg);
void mostrarListaDeRegistros(t_list* listReg);
void mostrarListaDeListasDeRegistros(t_list* lista);

void mostrarString(char* str);
void mostrarListaDeStrings(t_list* listStr);
void mostrarListaDeListasDeStrings(t_list* lista);

void mostrarBytes(int* bytes);

//-----------------------------------------------------
// ---------- funciones para hacer frees --------------
//-----------------------------------------------------
void funcionDestroyerChars (char* elemento);
void funcionDestroyerInts(int *elemento);
void funcionDestroyerLista(t_list *lista);
void destroyListaDeChars(t_list* lista);

#endif /* COMPACTACION_H_ */
