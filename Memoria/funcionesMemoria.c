#include "funcionesMemoria.h"


extern t_log* logger;
extern t_list* tablaSegmentos;


//TABLA DE SEGMENTOS

t_list* crearTablaSegmentos(){
	return list_create();
}



segmento* ultimoSegmento(t_list* tablaSegmentos){
	return list_get(tablaSegmentos,tablaSegmentos->elements_count - 1);
}

segmento* nuevaTabla(t_list* tablaSegmentos,char* nombreTabla){ //todo: destroy de esto
	segmento* nuevoSegmento = malloc(sizeof(segmento));

	nuevoSegmento->nombreDeTabla = string_duplicate(nombreTabla);

	nuevoSegmento->tablaDePaginas = crearTablaPaginas();

//	log_info(logger,"tabla por agregar: %s",nuevoSegmento->nombreDeTabla);

	list_add(tablaSegmentos,nuevoSegmento);

	log_info(logger,"tabla agregada: %s",ultimoSegmento(tablaSegmentos)->nombreDeTabla);
	log_info(logger,"tabla agregada: %p",ultimoSegmento(tablaSegmentos));
	return nuevoSegmento;
}





//TABLA DE PAGINAS

t_list* crearTablaPaginas(){
	return list_create();
}

pagina* ultimaPagina(t_list* tablaPaginas){
	pagina* lastPagina = list_get(tablaPaginas,tablaPaginas->elements_count - 1);
	return lastPagina ;
}


void asignoPaginaEnMarco(int key, int timestamp,char* value, void* comienzoMarco){
	//timestamp->key->value el orden importa

//	printf("value:%s\n",value);
//	printf("key:%d\n",key);
//	printf("timestamp:%d\n",timestamp);

	void* marcoParaKey=mempcpy(comienzoMarco,&timestamp,sizeof(int));
	void* marcoParaValue = mempcpy(marcoParaKey,&key,sizeof(int));
	memcpy(marcoParaValue,value,20);//maximo carac string
//	log_info(logger,"Marco donde asigne: %p",comienzoMarco);
}


int numeroMarcoDondeAlocar(){
//	printf("cant marcos:%d\n",cantMarcos);
	for(int i = 0; i<cantMarcos; i++){
//		printf("diponibilidad marco nro:%d=%d\n",i,marcos[i].vacio);
		if(marcos[i].vacio){
//			printf("numero marco libre encontrado:%d\n",i);
			marcos[i].vacio=false;
			return i;
		}
	}
	return -1;//falta aplicar algoritmo LRU si no encontro ninguna libre
}

void* marcoDondeAlocar(){
	printf("busco marco donde alocar\n");
	return comienzoMemoria + numeroMarcoDondeAlocar()*tamanioMarco;
}

pagina* nuevoDato(t_list* tablaPaginas,int flagModificado,int key, int timestamp, char* value){

	pagina* nuevaPagina = malloc(sizeof(pagina));

	nuevaPagina->flagModificado= flagModificado;

	nuevaPagina->dato =marcoDondeAlocar();
//	printf("marco pagina a asigar en marco:%p\n", nuevaPagina->dato);
//	printf("marco proxima pagina a asigar en marco:%p\n", nuevaPagina->dato+28);

	asignoPaginaEnMarco(key,timestamp,value,nuevaPagina->dato);
//	nuevaPagina->dato =comienzoMemoria;
//	asignoPaginaEnMarco(key,timestamp,value,comienzoMemoria);


	log_info(logger,"value pagina a agregar: %s", &nuevaPagina->dato->value);

	list_add(tablaPaginas,nuevaPagina);

	pagina* paginaAgregada = ultimaPagina(tablaPaginas);//para testear
	log_info(logger,"value de mi pagina agregada: %s",&paginaAgregada->dato->value);




	return nuevaPagina;
}






void mandarAEjecutarRequest(request* requestAEjecutar) {

	//Importante: Si el parametro es enteramente vacio, aca tiene que entrar aca como " ".

	char* parametros = string_duplicate(requestAEjecutar->parametros);

	switch (requestAEjecutar->requestEnInt) {
	case SELECT:
		;
		{
			pthread_t h_select;

			pthread_create(&h_select, NULL, (void *) Select, parametros);

			pthread_detach(h_select);

			break;
		}

	case INSERT:
		;
		{
			pthread_t h_insert;

			pthread_create(&h_insert, NULL, (void *) insert, parametros);

			pthread_detach(h_insert);

			break;
		}

	case CREATE:
		;

		{
			pthread_t h_create;

			pthread_create(&h_create, NULL, (void *) create, parametros);

			pthread_detach(h_create);

			break;
		}

	case DESCRIBE:
		;
		{

			pthread_t h_describe;

			pthread_create(&h_describe, NULL, (void *) describe, parametros);

			pthread_detach(h_describe);

			break;
		}

	case DROP:
		;
		{

			pthread_t h_drop;

			pthread_create(&h_drop, NULL, (void *) drop, parametros);

			pthread_detach(h_drop);

			break;

		}

	}

	liberarRequest(requestAEjecutar);
}



segmento* encuentroTablaPorNombre(char* nombreTabla, t_list* tablaDeSegmentos){
	bool comparoNombreTabla(segmento* segmentoAComparar){
		log_info(logger,"nombreTabla:%s - nombre encontrado:%s",nombreTabla,segmentoAComparar->nombreDeTabla);
		return strcmp(nombreTabla,segmentoAComparar->nombreDeTabla) == 0;
	}

	return list_find(tablaDeSegmentos,(void*)comparoNombreTabla);
}

pagina* encuentroDatoPorKey(segmento* tabla, int key){
	bool comparoKey(pagina* pag){
//		printf("key a comparar:%d - key encontrada:%d\n", key, pag->dato->key);
		return pag->dato->key == key;
	}
	return list_find(tabla->tablaDePaginas,(void*)comparoKey);

}

pagina* getPagina(t_list* tablaDeSegmentos, int key, char* nombreTabla){ //retorna un NULL si no existe la tabla o la pagina

	segmento* tabla = encuentroTablaPorNombre(nombreTabla, tablaDeSegmentos);
//	printf("tabla pedida:%p\n",tabla);
	if(tabla != NULL){
//		log_info(logger, "Encontre una tabla con el nombre: %s", tabla->nombreDeTabla);
		pagina* dato = encuentroDatoPorKey(tabla,key);
//		printf("marco pedido:%p\n", dato);
//		log_info(logger, "Encontre un dato con el value: %s", &dato->dato->value);

		return dato;
	}else return NULL;
}

void Select(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 2, " ");
	char* tabla =  parametrosEnVector[0];
	string_to_upper(tabla);
	int key = atoi(parametrosEnVector[1]);



	log_info(logger,"Select de tabla: %s - key: %d",tabla,key);

	pagina* paginaPedida=getPagina(tablaSegmentos,key,tabla);

	if(paginaPedida!=NULL)	{
		printf("Registro pedido: %s\n",&paginaPedida->dato->value);
	}
	else{
		log_info(logger,"No encontre el dato, mandando request a LFS");
		mandarSelectALFS(tabla,key);
	}

	free(parametrosEnVector[1]);
	free(parametrosEnVector[0]);
	free(parametrosEnVector);
	free(parametros);

}


void actualizoDato (pagina* pagina , char* nuevoValue){
	strcpy(&pagina->dato->value,nuevoValue);

}

void insert(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 3, " ");

	char* tabla = parametrosEnVector[0];
	string_to_upper(tabla);
	int key = atoi(parametrosEnVector[1]);
	char* value = parametrosEnVector[2]; //TODO deberia sacar las comillas
	int timestamp = 666; //TODO: asigno timestamp con el epoch

	segmento* tablaEncontrada=encuentroTablaPorNombre(tabla,tablaSegmentos);
	if(tablaEncontrada ==NULL){
		log_info(logger,"tengo que crear la tabla y el dato");

		segmento* tablaCreada= nuevaTabla(tablaSegmentos,tabla);

		log_info(logger,"se creo la tabla:%p",tablaCreada);
//		printf("tabla:%s - key:%d - timestamp:%d - value:%s\n",tabla,key,timestamp,value);
		nuevoDato(tablaCreada->tablaDePaginas,1,key,timestamp,value);
	}
	else{
		pagina* datoEncontrado = encuentroDatoPorKey(tablaEncontrada,key);
		if(datoEncontrado != NULL){
			log_info(logger,"tengo que actualizar el dato");
			actualizoDato(datoEncontrado,value);
		}
		else{
			log_info(logger,"tengo que cear el dato");
			nuevoDato(tablaEncontrada->tablaDePaginas,1,key,timestamp,value);
		}
	}
}


void create(char* parametros) {

	char** parametrosEnVector = string_n_split(parametros, 4, " ");

	char* tabla = string_duplicate(parametrosEnVector[0]);
	char* consistencia = parametrosEnVector[1];
	int particiones = atoi(parametrosEnVector[2]);
	char* tiempoCompactacion = parametrosEnVector[3];

	mandarCreateALFS(tabla,consistencia,particiones,tiempoCompactacion);

	liberarArrayDeStrings(parametrosEnVector);
	free(parametros);

}

void mandarCreateALFS(char* tabla ,char* consistencia,int particiones  ,char* tiempoCompactacion){
	//TODO
}

void mandarSelectALFS(char* tabla,int key){
	//TODO
}

void describe(char* parametro) {

	if (strcmp(parametro, " ")) //Si hay un parametro
			{

		return;
	}

}

void drop(char* parametro) {

}
