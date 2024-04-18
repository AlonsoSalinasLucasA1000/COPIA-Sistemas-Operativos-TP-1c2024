
#ifndef HEADERFUNCTIONS_H_
#define HEADERFUNCTIONS_H_

#include<stdio.h>
#include<stdlib.h>
#include<commons/log.h>
#include<string.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/collections/list.h>
#include<assert.h>

#define PUERTO "45014"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;


int iniciar_servidorMR(void);
int esperar_clienteMR(int);
void* recibir_bufferMR(int*, int);
void recibir_mensajeMR(int);
int recibir_operacionMR(int);

//t_list* recibir_paquete(int);

#endif /* HEADERFUNCTIONS_H_ */
