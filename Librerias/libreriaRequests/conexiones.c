#include "requests.h"

int seRecibioBien(int respuesta, t_log* logger) {
	if (respuesta < 0) {

		log_error(logger, "Hubo un problema en el recibo de algo.");

		return 0;
	}
	return 1;
}

int conectarseAServidor(char* ip, int puerto) {

	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = inet_addr(ip);
	direccionServidor.sin_port = htons(puerto);

	int conexion = socket(AF_INET, SOCK_STREAM, 0);
	if (connect(conexion, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("No me pude conectar");
		return -1;
	}
	return conexion;
}

int crearServidor(int puerto) {
	struct sockaddr_in direccionServidor;
	direccionServidor.sin_family = AF_INET;
	direccionServidor.sin_addr.s_addr = INADDR_ANY;
	direccionServidor.sin_port = htons(puerto);

	int servidor = socket(AF_INET, SOCK_STREAM, 0);

	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if (bind(servidor, (void*) &direccionServidor, sizeof(direccionServidor))
			!= 0) {
		perror("Fallo el bind");
		return -1;
	}

	listen(servidor, 100);

	return servidor;
}

void enviarIntConHeader(int aQuien, int intAEnviar, int header) {
	void* paquete = malloc(sizeof(int) + sizeof(int));
	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &intAEnviar, sizeof(int));
	send(aQuien, paquete, sizeof(int) + sizeof(int), 0);
	free(paquete);
}

void enviarVariosIntsConHeader(int aQuien, t_list* intsAEnviar, int header) {
	int cantidadInts = list_size(intsAEnviar);

	int tamanioPaquete = cantidadInts * sizeof(int) + sizeof(int);

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));

	int ultimaPosicionPaquete = sizeof(int);

	for (int i = 0; i < cantidadInts; i++) {
		int* unInt = list_get(intsAEnviar, i);
		memcpy(paquete + ultimaPosicionPaquete, unInt, sizeof(int));
		ultimaPosicionPaquete += sizeof(int);
	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);
}

void enviarStringConHeader(int aQuien, char* string, int header) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), string, tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarStringConHeaderEId(int aQuien, char* string, int header, int id) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &id, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int) + sizeof(int), string,
			tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarRequestConHeader(int aQuien, request* requestAEnviar, int header) {
	char* stringAEnviar = requestStructAString(requestAEnviar);

	enviarStringConHeader(aQuien, stringAEnviar, header);

	free(stringAEnviar);

}

void enviarRequestConHeaderEId(int aQuien, request* requestAEnviar, int header,
		int id) {

	char* stringAEnviar = requestStructAString(requestAEnviar);

	void* paquete;

	int tamanioString = (strlen(stringAEnviar) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + sizeof(int) + sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &id, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int) + sizeof(int), stringAEnviar,
			tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(stringAEnviar);
	free(paquete);

}

void enviarMetadatasConHeaderEId(int aQuien, t_list* metadatas, int header,
		int id) {

	int cantidadMetadatas = list_size(metadatas);

	if (cantidadMetadatas == 0) {
		enviarIntConHeader(aQuien, cantidadMetadatas, header);
		return;
	}

	int tamanioPaquete = sizeof(int) + sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas, el header e id que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &id, sizeof(int));
	memcpy(paquete + sizeof(int) + sizeof(int), &cantidadMetadatas,
			sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);
}

void enviarMetadatasConHeader(int aQuien, t_list* metadatas, int header) {

	int cantidadMetadatas = list_size(metadatas);

	if (cantidadMetadatas == 0) {
		enviarIntConHeader(aQuien, cantidadMetadatas, header);
		return;
	}
	int tamanioPaquete = sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas y el header que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &cantidadMetadatas, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarSeedsConHeader(int aQuien, t_list* seeds, int header) {

	int cantidadSeeds = list_size(seeds);

	if (cantidadSeeds == 0)
	{
		enviarIntConHeader(aQuien, cantidadSeeds, header);
		return;
	}

	int tamanioPaquete = sizeof(int) + sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadSeeds y el header que va a ir al principio

	for (int i = 0; i < list_size(seeds); i++) {

		seed* elemento = list_get(seeds, i);

		int tamanioIp = (strlen(elemento->ip) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioIp + sizeof(int);
	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));
	memcpy(paquete + sizeof(int), &cantidadSeeds, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int) + sizeof(int);

	for (int i = 0; i < list_size(seeds); i++) {

		seed* elemento = list_get(seeds, i);

		int tamanioIp = (strlen(elemento->ip) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioIp, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int), elemento->ip,
				tamanioIp);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioIp,
				&elemento->puerto, sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioIp + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarRegistroConHeaderEId(int aQuien, registro* unRegistro, int header,
		int id) {

	int tamanioValue = strlen(unRegistro->value) + 1;
	int tamanioTimestamp = sizeof(typeof(unRegistro->timestamp));
	int tamanioKey = sizeof(typeof(unRegistro->key));

	int tamanioPaquete = sizeof(int) + sizeof(int) + tamanioTimestamp
			+ tamanioKey + sizeof(int) + tamanioValue;

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &id, sizeof(int));

	memcpy(paquete + sizeof(int) + sizeof(int), &unRegistro->timestamp,
			tamanioTimestamp);

	memcpy(paquete + sizeof(int) + sizeof(int) + tamanioTimestamp,
			&unRegistro->key, tamanioKey);

	memcpy(paquete + sizeof(int) + sizeof(int) + tamanioTimestamp + tamanioKey,
			&tamanioValue, sizeof(int));

	memcpy(
			paquete + sizeof(int) + sizeof(int) + tamanioTimestamp + tamanioKey
					+ sizeof(int), unRegistro->value, tamanioValue);

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarRegistroConHeader(int aQuien, registro* unRegistro, int header) {

	int tamanioValue = strlen(unRegistro->value) + 1;
	int tamanioTimestamp = sizeof(typeof(unRegistro->timestamp));
	int tamanioKey = sizeof(typeof(unRegistro->key));

	int tamanioPaquete = sizeof(int) + tamanioTimestamp + tamanioKey
			+ sizeof(int) + tamanioValue;

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &header, sizeof(int));

	memcpy(paquete + sizeof(int), &unRegistro->timestamp, tamanioTimestamp);

	memcpy(paquete + sizeof(int) + tamanioTimestamp, &unRegistro->key,
			tamanioKey);

	memcpy(paquete + sizeof(int) + tamanioTimestamp + tamanioKey, &tamanioValue,
			sizeof(int));

	memcpy(paquete + sizeof(int) + tamanioTimestamp + tamanioKey + sizeof(int),
			unRegistro->value, tamanioValue);

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarListaDeRequestsConHeader(int aQuien, t_list* requests, int header)
{
	int cantidadRequests = list_size(requests);

	if(list_size(requests) == 0)
	{
		enviarIntConHeader(aQuien,cantidadRequests,header);
		return;
	}

	t_list* requestsEnString = list_map(requests,(void*) requestStructAString);

	int tamanioPaquete = sizeof(int); //Header

	for(int i = 0; i<cantidadRequests; i++)
	{
		char* unaRequest = list_get(requestsEnString,i);
		int tamanioString = strlen(unaRequest)+1;

		tamanioPaquete += sizeof(int) + tamanioString;
	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete,&header,sizeof(int));

	memcpy(paquete+sizeof(int),&cantidadRequests,sizeof(int));

	int ultimaPosicionPaquete = sizeof(int) + sizeof(int);

	for(int i = 0; i<cantidadRequests;i++)
	{

		char* unaRequest = list_get(requestsEnString,i);

		int tamanioString =strlen(unaRequest) + 1;

		memcpy(paquete + ultimaPosicionPaquete,&tamanioString,sizeof(int));
		memcpy(paquete + ultimaPosicionPaquete+sizeof(int),unaRequest,tamanioString);

		ultimaPosicionPaquete += sizeof(int)+tamanioString;
	}

	send(aQuien,paquete,tamanioPaquete,0);

	free(paquete);
}

t_list* recibirRequests (int deQuien, t_log* logger) //Si hubo error: primer elemento de lista tiene requestEnInt == -1
{
	request* requestFallida = malloc(sizeof(request));
	requestFallida->parametros = NULL;
	requestFallida->requestEnInt = -1;

	int cantidadRequests = recibirInt(deQuien,logger);

	t_list* requests = list_create();

	if (cantidadRequests == 0)
	{
		free(requestFallida);
		return requests; //Lista vacia
	}
	else if(cantidadRequests == -1)
	{
		list_add(requests,requestFallida);
		return requests;
	}

	for(int i = 0;i<cantidadRequests;i++)
	{
		request* unaRequest = recibirRequest(deQuien,logger);

		if (unaRequest->requestEnInt == -1)
		{
			free(unaRequest);

			while(list_size(requests) != 0)
			{
				request* requestARemover = list_remove(requests,0);
				free(requestARemover->parametros);
				free(requestARemover);
			}

			list_add(requests,requestFallida);
			return requests;
		}
		list_add(requests,unaRequest);
	}

	free(requestFallida);
	return requests;

}

registro* recibirRegistro(int deQuien, t_log* logger) { //Si hubo error: registro->value = " "

	registro* elRegistro = malloc(sizeof(registro));

	int tamanioTimestamp = sizeof(typeof(elRegistro->timestamp));

	void* bufferTimestamp = malloc(tamanioTimestamp);

	int respuesta = recv(deQuien, bufferTimestamp, tamanioTimestamp,
	MSG_WAITALL);

	if (respuesta == -1) {
		elRegistro->value = string_duplicate(" ");
		return elRegistro;
	}

	memcpy(&elRegistro->timestamp, bufferTimestamp, tamanioTimestamp);

	free(bufferTimestamp);

	int tamanioKey = sizeof(typeof(elRegistro->timestamp));

	void* bufferKey = malloc(tamanioKey);

	respuesta = recv(deQuien, bufferKey, tamanioKey, MSG_WAITALL);

	if (respuesta == -1) {
		elRegistro->value = string_duplicate(" ");
		return elRegistro;
	}

	memcpy(&elRegistro->key, bufferKey, tamanioKey);

	free(bufferKey);

	elRegistro->value = recibirString(deQuien, logger);

	return elRegistro;

}

request* recibirRequest(int deQuien, t_log* logger) { //Si hubo error: request->requestEnInt = -1
	char* requestEnString = recibirString(deQuien, logger);

	request* requestNuevo;

	if (!strcmp(requestEnString, " ")) {
		requestNuevo = malloc(sizeof(request));
		requestNuevo->requestEnInt = -1;
		requestNuevo->parametros = NULL;
	} else {
		requestNuevo = crearStructRequest(requestEnString);
	}

	free(requestEnString);

	return requestNuevo;

}

char* recibirString(int deQuien, t_log* logger) { //Si hubo error: string = " "

	int tamanioString = recibirInt(deQuien, logger);

	if (tamanioString == -1) {
		return " ";
	}

	void* bufferString = malloc(tamanioString);

	int respuesta = recv(deQuien, bufferString, tamanioString,
	MSG_WAITALL);

	if (!seRecibioBien(respuesta, logger)) {
		return " ";
	}

	char* stringRecibido = malloc(tamanioString);

	memcpy(stringRecibido, bufferString, tamanioString);

	free(bufferString);

	return stringRecibido;
}

int recibirInt(int deQuien, t_log* logger) { //Si hubo error: elInt = -1
	void* bufferInt = malloc(sizeof(int));
	int respuesta = recv(deQuien, bufferInt, sizeof(int),
	MSG_WAITALL);

	if (!seRecibioBien(respuesta, logger)) {

		free(bufferInt);
		return -1;
	}

	int intRecibido;
	memcpy(&intRecibido, bufferInt, sizeof(int));

	free(bufferInt);
	return intRecibido;
}

t_list* recibirMetadatas(int deQuien, t_log* logger) { //Si hubo error: lista de una metadata, esa metadata tiene consistencia,particiones y compactTime = -1

	int cantidadMetadatas = recibirInt(deQuien, logger);

	t_list* metadatas = list_create();

	if (cantidadMetadatas == -1) {
		metadataTablaLFS* metadata = malloc(sizeof(metadataTablaLFS));
		metadata->nombre = NULL;
		metadata->consistencia = -1;
		metadata->particiones = -1;
		metadata->compactTime = -1;

		list_add(metadatas, metadata);

		return metadatas;
	}

	if (cantidadMetadatas == 0) {
		return metadatas; //Lista vacia
	}

	for (int i = 0; i < cantidadMetadatas; i++) {
		metadataTablaLFS* metadata = malloc(sizeof(metadataTablaLFS));

		metadata->nombre = recibirString(deQuien, logger);
		metadata->consistencia = recibirInt(deQuien, logger);
		metadata->particiones = recibirInt(deQuien, logger);
		metadata->compactTime = recibirInt(deQuien, logger);

		if (!strcmp(metadata->nombre, " ") || metadata->consistencia == -1
				|| metadata->particiones == -1 || metadata->compactTime == -1) {

			free(metadata->nombre);
			free(metadata);

			while (list_size(metadatas) != 0) {
				metadataTablaLFS* metadataABorrar = list_remove(metadatas, 0);
				free(metadataABorrar->nombre);
				free(metadataABorrar);
			}

			metadataTablaLFS* metadata = malloc(sizeof(metadataTablaLFS));
			metadata->nombre = NULL;
			metadata->consistencia = -1;
			metadata->particiones = -1;
			metadata->compactTime = -1;

			list_add(metadatas, metadata);

			return metadatas;
		}

		list_add(metadatas, metadata);
	}

	return metadatas;
}

t_list* recibirSeeds(int deQuien, t_log* logger) { //Si hubo error: primer elemento tiene puerto == -1
	int cantidadSeeds = recibirInt(deQuien, logger);

	t_list* seeds = list_create();

	if (cantidadSeeds == -1) {
		seed* seedFallida = malloc(sizeof(seed));
		seedFallida->ip = NULL;
		seedFallida->puerto = -1;
		list_add(seeds, seedFallida);
		return seeds;
	}

	if (cantidadSeeds == 0) {
		return seeds; //Lista vacia
	}

	for (int i = 0; i < cantidadSeeds; i++) {

		seed* unaSeed = malloc(sizeof(metadataTablaLFS));

		unaSeed->ip = recibirString(deQuien, logger);

		unaSeed->puerto = recibirInt(deQuien, logger);

		if (!strcmp(unaSeed->ip, " ") || unaSeed->puerto == -1) {

			free(unaSeed->ip);
			free(unaSeed);

			while (list_size(seeds) != 0) {
				seed* seedABorrar = list_remove(seeds, 0);
				free(seedABorrar->ip);
				free(seedABorrar);
			}

			seed* seedFallida = malloc(sizeof(seed));
			seedFallida->ip = NULL;
			seedFallida->puerto = -1;
			list_add(seeds, seedFallida);

			return seeds;

		}

		list_add(seeds, unaSeed);
	}

	return seeds;
}

//----------------------------------------------------------------------
//Funciones sin header, por si alguna razon son necesarias:

void enviarInt(int aQuien, int intAEnviar) {
	void* paquete = malloc(sizeof(int));
	memcpy(paquete, &intAEnviar, sizeof(int));
	send(aQuien, paquete, sizeof(int), 0);
	free(paquete);
}

void enviarString(char* string, int aQuien) {
	void* paquete;

	int tamanioString = (strlen(string) + 1) * sizeof(char);

	int tamanioEnvio = sizeof(int) + tamanioString;

	paquete = malloc(tamanioEnvio);

	memcpy(paquete, &tamanioString, sizeof(int));

	memcpy(paquete + sizeof(int), string, tamanioString);

	send(aQuien, paquete, tamanioEnvio, 0);

	free(paquete);
}

void enviarMetadatas(t_list* metadatas, int aQuien) {

	int cantidadMetadatas = list_size(metadatas);

	int tamanioPaquete = sizeof(int); //Se pone "= sizeof(int)" por el int de cantidadMetadatas que va a ir al principio

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		tamanioPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	void* paquete = malloc(tamanioPaquete);

	memcpy(paquete, &cantidadMetadatas, sizeof(int));

	int ultimaPosicionDelPaquete = sizeof(int);

	for (int i = 0; i < list_size(metadatas); i++) {
		metadataTablaLFS* elemento = list_get(metadatas, i);

		int tamanioNombre = (strlen(elemento->nombre) + 1) * sizeof(char);

		memcpy(paquete + ultimaPosicionDelPaquete, &tamanioNombre, sizeof(int));

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int),
				elemento->nombre, tamanioNombre);

		memcpy(paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre,
				&elemento->consistencia, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int), &elemento->particiones, sizeof(int));

		memcpy(
				paquete + ultimaPosicionDelPaquete + sizeof(int) + tamanioNombre
						+ sizeof(int) + sizeof(int), &elemento->compactTime,
				sizeof(int));

		ultimaPosicionDelPaquete += sizeof(int) + tamanioNombre + sizeof(int)
				+ sizeof(int) + sizeof(int);

	}

	send(aQuien, paquete, tamanioPaquete, 0);

	free(paquete);

}

void enviarRequest(int aQuien, request* requestAEnviar) {
	char* stringAEnviar = requestStructAString(requestAEnviar);

	enviarString(stringAEnviar, aQuien);

	free(stringAEnviar);
}
