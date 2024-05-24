#ifndef MEMORIA_H_
#define MEMORIA_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//file descriptors de memoria y los modulos que se conectaran con ella
int fd_memoria;
int fd_kernel;
int fd_cpu;
int fd_entradasalida;


t_log* memoria_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* memoria_config;

char* PUERTO_ESCUCHA;
int TAM_MEMORIA;
int TAM_PAGINA;
char* PATH_INSTRUCCIONES;
int RETARDO_RESPUESTA;



void memoria_escuchar_cpu (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_cpu);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(memoria_logger, "El cliente cpu se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}


void memoria_escuchar_entradasalida (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_entradasalida);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(memoria_logger, "El cliente EntradaSalida se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}


void memoria_escuchar_kernel (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
			//FALTA OPERACION DE RECIBIR PAQUETE.
				printf("Recibi el paquete nice");
				break;
			case -1:
				log_error(memoria_logger, "El cliente kernel se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(memoria_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}

#endif /* MEMORIA_H_ */