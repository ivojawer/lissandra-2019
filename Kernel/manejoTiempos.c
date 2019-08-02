#include "funcionesBaseKernel.h"


extern t_list* tiemposInsert;
extern t_list* tiemposSelect;
extern sem_t sem_tiemposInsert;
extern sem_t sem_tiemposSelect;

void insertarTiempo(time_t tiempoInicial, time_t tiempoFinal, int request) {

	tiempoDeOperacion* tiempoOperacion = malloc(sizeof(tiempoOperacion));

	tiempoOperacion->duracion = difftime(tiempoFinal,tiempoInicial);

	tiempoOperacion->tiempoInicio = tiempoInicial;

	if (request == SELECT) {
		sem_wait(&sem_tiemposSelect);
		list_add(tiemposSelect, tiempoOperacion);
		sem_post(&sem_tiemposSelect);
	}

	else if (request == INSERT) {
		sem_wait(&sem_tiemposInsert);
		list_add(tiemposInsert, tiempoOperacion);
		sem_post(&sem_tiemposInsert);
	}
}

int esMasNuevoQue30Segundos(tiempoDeOperacion tiempoOperacion) {

	if (difftime(time(NULL), tiempoOperacion.tiempoInicio) < 30) {
		return 1;
	} else {
		return 0;
	}
}

t_list* filterCasero_esMasNuevoQue30Segundos(t_list* tiempos) {

	t_list* tiemposAux = list_duplicate(tiempos);

	for (int i = 0; i < list_size(tiemposAux); i++) {
		tiempoDeOperacion* elemento = list_get(tiemposAux, i); //Hay que liberar esto?

		if (elemento == NULL) {
			break;
		}

		if (!esMasNuevoQue30Segundos(*elemento)) {
			list_remove(tiemposAux, i);
			i--; //Para que se mantenga en la misma posicion
		}
	}

	return tiemposAux;

}

float promedioDeTiemposDeOperaciones(t_list* tiempos) {

	if (list_size(tiempos) == 0) {
		return -1;
	}

	float sumaTotalTiempos = 0;

	for (int i = 0; i < list_size(tiempos); i++) {
		tiempoDeOperacion* tiempoOperacion = list_get(tiempos, i); //Hay que liberar esto?

		sumaTotalTiempos += tiempoOperacion->duracion;
	}

	float promedio = sumaTotalTiempos / list_size(tiempos);

	return promedio;
}

void limpiarListasTiempos() {
	sem_wait(&sem_tiemposSelect);
	sem_wait(&sem_tiemposInsert);
	t_list* listaInsertAux = tiemposInsert;
	t_list* listaSelectAux = tiemposSelect;

	tiemposSelect = filterCasero_esMasNuevoQue30Segundos(tiemposSelect);

	tiemposInsert = filterCasero_esMasNuevoQue30Segundos(tiemposInsert);

	list_destroy(listaInsertAux);
	list_destroy(listaSelectAux);
	sem_post(&sem_tiemposSelect);
	sem_post(&sem_tiemposInsert);
}






