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
char** transformarParticionABloques(char* tabla,int nro_particion);
char** transformarTemporalABloques(char* tabla,int nro_temporal);


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


#endif /* COMPACTACION_H_ */
