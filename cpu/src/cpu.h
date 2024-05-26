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

char* recibir_instruccion_cpu(int PID, int PC)
{ 
	printf("Llegue a la instruccion recibir instruccion cpu\n");
	//enviar pcb
	PCB* to_send = malloc(sizeof(PCB));
	to_send->PC = PC;
	to_send->PID = PID;
	enviarPCB (to_send, fd_memoria);
	printf("pude enviar la pcb\n");

	//esperar recibir mensaje de memoria
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
	paquete->buffer = malloc(sizeof(t_newBuffer));
	recibir_operacion(fd_memoria);
	recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);		
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(fd_memoria,paquete->buffer->stream, paquete->buffer->size,0);
	printf("recibi el mensaje\n");

	void *stream = paquete->buffer->stream;
	memcpy(&(paquete->buffer->size), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	// deserailizamos el path como tal
	char* ret = malloc(paquete->buffer->size);
	memcpy(ret, stream, paquete->buffer->size);
	printf("deserialice el paquete correctamente\n");

	return ret;
}

void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion

	char* instruccion = recibir_instruccion_cpu(proceso->PID,proceso->PC);
	
		char** instruccion_split = string_split (instruccion, " ");
			if (instruccion_split[0] == "SET"){ 
				if(instruccion_split [1] == "AX"){
					int dato = atoi(instruccion_split[2]); 
					proceso->AX = dato; 
					printf("Ejecuta instruccion SET PARA AX, $d \n", proceso->AX);
				} else if(instruccion_split [1] == "BX"){
					int dato = atoi(&instruccion_split [2]); 
					proceso->BX = dato;
					printf("Ejecuta instruccion SET PARA BX, $d \n", proceso->BX);
				} else{
					printf("error en la instruccion SET\n");
				}
			} else if (instruccion_split[0] == "SUM"){ 

			} else if (instruccion_split[0] == "SUB"){
				
			} else if (instruccion_split[0] == "JNZ"){
				
			} else if (instruccion_split[0] == "IO_GEN_SLEEP"){
				
			} 
			

			/*switch (instruccion_split[0]) {
			case SET:
			printf("El valor de AX %d ", )

				instruccion_split [1] = instruccion_split [2];
				break; 
			case SUM
				break;
			case SUB
				break;
			case JNZ
				break;
			case IO_GEN_SLEEP
				break;
			}*/
		//ejecuto la instruccion

		proceso->PC++;
		//pido de vuelta
}


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
				ejecutar_proceso(proceso);
				if(proceso != NULL){
					printf("------------------------\n");
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
					printf("------------------------\n");
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
				printf("Instruccion de la memoria recibida con exito\n");
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