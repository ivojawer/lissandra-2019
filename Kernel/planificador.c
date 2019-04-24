#include "funcionesKernel.h"

extern sem_t* sem_disponibleColaREADY;
extern sem_t* sem_multiprocesamiento;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;

void planificador()
{
	//Pensar mmm
	sem_wait(&sem_disponibleColaREADY);
	sem_wait(&sem_multiprocesamiento);
	//Pensar mmm


	script* scriptAExec = list_get(colaREADY,0); //Ver el tema del list_remove si es que mueve todo y eso

	removerScriptDeLista(scriptAExec->idScript,colaREADY);
	list_add(listaEXEC,scriptAExec); //Pensar sobre semaforos porque un status justo puede hacer que aparezca en ambas listas (muy improbable pero por si acaso)


}
