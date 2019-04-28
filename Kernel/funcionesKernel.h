#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#include "funcionesBaseKernel.h"

void consola();
void conexiones();
void planificadorREADYAEXEC();
void planificadorEXEC(int IdScript);

int ejecutarRequest(char* request);
void metrics();
void crearScript(char* request);
void status();

#endif /* FUNCIONESKERNEL_H_ */
