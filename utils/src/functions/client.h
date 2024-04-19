#ifndef CLIENT_H_
#define CLIENT_H_

#include<signal.h>
#include<commons/config.h>
#include<readline/readline.h>

#include <./functions/generals.h>

int crear_conexion(int ip, int puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
void liberar_conexion(int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void eliminar_paquete(t_paquete* paquete);


/****FUNCIONES QUE HABIA EN EL CLIENT_TO*****/

void leer_consola(t_log*);
void paquete(int);
void terminar_programa(int, t_log*, t_config*);


#endif /* CLIENT_H_ */