#ifndef FUNCIONESKERNEL_H_
#define FUNCIONESKERNEL_H_

#include "funcionesBaseKernel.h"

void consola();
void planificadorREADYAEXEC();
void planificadorEXEC(int IdScript);
void conectarseAUnaMemoria(seed* unaSeed);
void comunicacionConMemoria(memoriaEnLista* memoria);

int ejecutarRequest(request* requestAEjecutar, script* elScript);
void metrics();
int crearScript(request* nuevaRequest);
void status();
int add(char* chocloDeCosas);
void journal();
void refreshMetadatas();
void refreshConfig();


#endif /* FUNCIONESKERNEL_H_ */
