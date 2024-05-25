#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//file descriptors de CPU y los modulos que se conectaran con el
int fd_cpu_dispach;
int fd_cpu_interrupt; //luego se dividira en dos fd un dispach y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_kernel;


t_log* cpu_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* cpu_config;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;



void cpu_escuchar_kernel (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			printf("Recibi codigo de op. del kernel\n");
			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));

			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);
			printf("Recibimos tamaño\n");
			
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);
			printf("Recibi stream\n");

			switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case PROCESO:

			    printf("Voy a atender un proceso\n");
				PCB* proceso = deserializar_proceso_cpu(paquete->buffer);

				if(proceso != NULL){ 
					printf("El PID que recibi es: %d\n", proceso->PID);
					printf("El PC que recibi es: %d\n", proceso->PC);
					printf("El QUANTUM que recibi es: %d\n", proceso->quantum);
					printf("El AX que recibi es: %d\n", proceso->AX);
					printf("El BX que recibi es: %d\n", proceso->BX);
					printf("El CX que recibi es: %d\n", proceso->CX);
					printf("El DX que recibi es: %d\n", proceso->DX);
					printf("EL ESTADO que recibi: %d\n",proceso->estado);
					printf("EL tamaño de path que recibi: %d\n",proceso->path_length);
					printf("EL path que recibi: %s\n",proceso->path);
				} else{
					printf("No se pudo deserializar\n");
				}
				free(proceso);
			    break;
			case -1:
				log_error(cpu_logger, "El cliente Kernel se desconecto. Terminando servidor\n");
				control_key = 0;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);

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