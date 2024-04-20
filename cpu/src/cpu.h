#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>
#include <pthread.h>

//file descriptors de CPU y los modulos que se conectaran con el
int fd_cpu; //luego se dividira en dos fd un dispach y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_kernel;


t_log* cpu_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* cpu_config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int* CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;



void cpu_escuchar_kernel (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(cpu_logger, "El cliente Kernel se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}
}



void cpu_escuchar_memoria (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_memoria);
			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(cpu_logger, "Desconexion de memoria.");
				control_key = 0;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}


#endif /* CPU_H_ */