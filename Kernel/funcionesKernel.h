#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#include "funcionesBaseKernel.h"

void consola();
void conexiones();
void planificadorREADYAEXEC();
void planificadorEXEC(int IdScript);

int ejecutarRequest(request* requestAEjecutar);
void metrics();
void crearScript(request* nuevaRequest);
void status();
void add (char* consistenciaYMemoriaEnString);

#endif /* FUNCIONESKERNEL_H_ */
