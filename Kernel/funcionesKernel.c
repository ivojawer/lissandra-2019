#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern int idInicial;
extern int quantum;
extern sem_t sem_cambioId;
extern sem_t sem_disponibleColaREADY;

int removerScriptDeLista(int id, t_list* lista) {
	for (int i; i < list_size(lista); i++) {
		script* script = list_get(lista, i);

		if (script->idScript == id) {
			if (script->direccionScript != 0) {
				free(script->direccionScript);
			}

			free(script);

			list_remove(lista, i);

			return 1;
		}
	}
	return 0;
}



int existeArchivo(char* direccion) //TODO
{
	return 0;
}

void crearScript(parametros_hiloScript* parametros) {
	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));

	sem_wait(&sem_cambioId);
	nuevoScript->idScript = idInicial;
	idInicial++;
	sem_post(&sem_cambioId);

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP



	nuevoScript->lineasLeidas = 0;
	nuevoScript->quantumRestante = quantum;

	if (parametros->request == RUN) {

		nuevoScript->direccionScript = malloc(sizeof(parametros->parametros));
		nuevoScript->direccionScript = string_duplicate(parametros->parametros);
	}

	else
	{	//TODO:
		//Crear archivo script de una linea
		//Poner direccion en el script
	}

	removerScriptDeLista(nuevoScript->idScript, colaNEW);
	list_add(colaREADY,nuevoScript);

	sem_post(&sem_disponibleColaREADY);


}

