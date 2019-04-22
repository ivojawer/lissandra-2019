#include "funcionesKernel.h"

extern t_list* colaNEW;
extern t_list* colaREADY;
extern t_list* listaEXEC;
extern t_list* listaEXIT;
extern int idInicial;
extern int quantum;

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

void crearScript(parametros_hiloScript* parametros) {
	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	script* nuevoScript = malloc(sizeof(script));
	nuevoScript->idScript = idInicial;

	list_add(colaNEW, nuevoScript); //Esto es puramente por formalidad del TP

	idInicial++; //TODO: Poner un semaforo por aca para evitar condicion de carrera

	nuevoScript->lineasLeidas = 0;
	nuevoScript->quantumRestante = quantum;

	if (parametros->request == RUN) {

		//TODO: Checkear si existe el archivo

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


}

