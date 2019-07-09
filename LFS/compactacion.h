#ifndef COMPACTACION_H_
#define COMPACTACION_H_

#include "funcionesLFS.h"

void compactar(char* tabla);
int renombrarATmpc(char* tabla);
void string_append_char(char* string, char c);
t_registro* stringRegistroAStruct(char* registro);
char* structRegistroAString(t_registro* registro);
t_list* traerRegistrosBloques(char** bloques);
char** transformarParticionABloques(char* tabla,int nro_particion);
char** transformarTemporalABloques(char* tabla,int nro_temporal);

#endif /* COMPACTACION_H_ */
