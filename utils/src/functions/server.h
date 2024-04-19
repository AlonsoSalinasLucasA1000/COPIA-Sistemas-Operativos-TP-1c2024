#ifndef SERVER_H_
#define SERVER_H_

#include<commons/collections/list.h>
#include<assert.h>

#include <./functions/generals.h>

//#define PUERTO "45007"
// Hay que definir el puerto en c/u de los servers sv


extern t_log* logger;


int iniciar_socket(int);
int esperar_cliente(int);
void* recibir_buffer(int*, int);
void recibir_mensaje(int);
int recibir_operacion(int);

//t_list* recibir_paquete(int);

#endif /* HEADERFUNCTIONS_H_ */
