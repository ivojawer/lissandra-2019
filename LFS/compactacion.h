#ifndef COMPACTACION_H_
#define COMPACTACION_H_

#include "funcionesLFS.h"

void compactar(char* tabla);
void string_add_ending(char* string, char c);
t_registro* stringRegistroAStruct(char* registro);
t_list* traerRegistrosBloques(char** bloques);


#endif /* COMPACTACION_H_ */
