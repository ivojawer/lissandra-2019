#include "funcionesKernel.h"

extern sem_t sem_disponibleColaREADY;
extern sem_t sem_multiprocesamiento;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int quantum;
extern t_log* logger;

void planificadorREADYAEXEC() {

	while (1) {

		//Si el multiprocesamiento no puede variar mientras hay procesos ejecutandose entonces esto esta bien
		sem_wait(&sem_multiprocesamiento);
		sem_wait(&sem_disponibleColaREADY);

		script* scriptAExec = list_get(colaREADY, 0); //El 0 siempre va a ser el mas viejo en la lista

		removerScriptDeLista(scriptAExec->idScript, colaREADY);
		list_add(listaEXEC, scriptAExec); //Pensar sobre semaforos porque un status justo puede hacer que aparezca en ambas listas (muy improbable pero por si acaso)

		pthread_t h_EXEC;
		pthread_create(&h_EXEC, NULL, (void *) planificadorEXEC, &scriptAExec->idScript);
		pthread_detach(h_EXEC);

	}

}

void planificadorEXEC(int IdScript) {

	script* scriptEXEC = list_get(listaEXEC, encontrarScriptEnLista(IdScript,listaEXEC));

	for(int i; i<quantum; i++)
	{
		char* linea = leerLinea(scriptEXEC->direccionScript,scriptEXEC->lineasLeidas);

		scriptEXEC->lineasLeidas++;

		int resultado = ejecutarRequest(linea);

		if (resultado == -1)
		{
			log_error(logger,"%s%i","Hubo un error en la ejecucion del script ",scriptEXEC->idScript);
			break;
		}
		//Semaforo por si cambia el quantum?
	}

	log_info (logger,"%s%i","Termino de ejecutar el script",scriptEXEC->idScript);

}
