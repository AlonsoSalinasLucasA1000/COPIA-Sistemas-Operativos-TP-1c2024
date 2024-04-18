
#ifndef HEADERSERVER_H_
#define HEADERSERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <string.h>
#include <assert.h>

#define PUERTO "45007"

typedef enum
{
	MENSAJE,
	PAQUETE
}op_code;

extern t_log* logger;

void* recibir_buffer(int*, int);

int iniciar_servidor(void);
int esperar_cliente(int);
void recibir_mensaje(int);
int recibir_operacion(int);
//t_list* recibir_paquete(int);

void iterator(char* value);

#endif /* HEADERSERVER_H_ */
