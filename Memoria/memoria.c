#include "funcionesMemoria.h"


t_config*config;
t_log* logger;
t_list* tablaSegmentos;


int main() {


	logger = log_create("memoria.log", "memoria", 1, 0); //3er parametro en 1 para mostrarlos en consola. Sino en 0

	config =config_create("memoria.config"); //DUDA: Si es un config por memoria esto va en la carpeta CONFIG tmb  lo hago una por proyecto como aca

	pthread_t h_consola;
	pthread_t h_conexiones;

	pthread_create(&h_consola, NULL, (void *) consola, NULL);
	pthread_create(&h_conexiones, NULL, (void *) conexiones, NULL);

	//reservo toda la memoria
	int tamanioMemoria =config_get_int_value(config,"TAM_MEM");
	log_info(logger,"cree mi memoria tamanio: %d",tamanioMemoria);
	comienzoMemoria = malloc(tamanioMemoria);
	//printf("comienzo memoria:%p\n", comienzoMemoria);
	//divido en marcos
	int caracMaxDeValue = config_get_int_value(config,"CANT_MAX_CARAC");
	tamanioMarco = caracMaxDeValue * sizeof(char) + sizeof(int) + sizeof(int); //value + key + timestamp
	log_info(logger,"el tamanio de mi marco es: %d", tamanioMarco);
	cantMarcos = tamanioMemoria/tamanioMarco; //tamanio marco siempre es mult de 2 asi que da redondo
	log_info(logger,"cantidad de marcos: %d",cantMarcos);
	marcos=malloc(sizeof(disponibilidad)*cantMarcos);
	for(int i = 0; i<cantMarcos;i++){
		marcos[i].vacio=true;
		marcos[i].recentlyUsed=false;//ni idea pero le queria poner un valor para acordarme que se setea aca
	}
//	creo tabla de segmentos
	tablaSegmentos = crearTablaSegmentos();





//	pruebas alto nivel:
	insert("TABLA1 123 hola");
	sleep(1);
	Select("TABLA1 123");
	sleep(1);
	insert("TABLA1 1234 holiiiis");
	sleep(1);
	insert("TABLA2 123 \"buen dia\"");
	sleep(1);
	insert("TABLA1 123 \"chau\"");
	sleep(1);
	insert("TABLA3 99 \"hola soy ivan y trabajo en un ascensor un dia\"");
	sleep(1);
	Select("TABLA1 123");
	sleep(1);
	Select("TABLA1 1234");
	sleep(1);
	Select("TABLA3 123");
	sleep(1);
	Select("TABLA2 123");
	sleep(1);
	Select("TABLA4 123");

//	pruebas bajo nivel:
//	segmento* miSegmento=nuevaTabla(tablaSegmentos,"TABLA1");
//	agregarDato(miSegmento,123,0,"hola",1);

//
//	segmento* miSegmento = tablaSegmentos->head->data;
//	log_info(logger,"Nombre de mi nueva tabla es: %s",miSegmento->nombreDeTabla);
//
//
//	agregarDato(miSegmento,123,0,"hola",1);
//
//	pagina* datoAgregado= miSegmento->tablaDePaginas->head->data;
//	log_info(logger,"dato agregado: key=%d - timestamp=%d - value=%s" , datoAgregado->dato->key,datoAgregado->dato->timestamp,&datoAgregado->dato->value);






	pthread_detach(h_conexiones);
	pthread_join(h_consola, NULL);

	free(comienzoMemoria);
	return 1;
}
