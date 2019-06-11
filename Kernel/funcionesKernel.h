#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#include "funcionesBaseKernel.h"

void consola();
void conexiones();
void planificadorREADYAEXEC();
void planificadorEXEC(int IdScript);

int ejecutarRequest(request* requestAEjecutar, script* elScript);
void metrics();
int crearScript(request* nuevaRequest);
void status();
int add(char* chocloDeCosas);
void journal();
#endif /* FUNCIONESKERNEL_H_ */
