#ifndef COMPACTACION_H_
#define COMPACTACION_H_

#include "funcionesLFS.h"

void compactar(char* tabla);


int renombrarATmpc(char* tabla);
t_list* traerRegistrosBloques(char** bloques);
void string_append_char(char* string, char c);


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



void eliminarListaDeStrings(t_list* list);

#endif /* COMPACTACION_H_ */
