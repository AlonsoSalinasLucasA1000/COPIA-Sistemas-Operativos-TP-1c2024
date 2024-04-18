
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

#define PUERTO "45007"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;


int iniciar_servidor(void);
int esperar_cliente(int);
void* recibir_buffer(int*, int);
void recibir_mensaje(int);
int recibir_operacion(int);

//t_list* recibir_paquete(int);

#endif /* HEADERFUNCTIONS_H_ */
