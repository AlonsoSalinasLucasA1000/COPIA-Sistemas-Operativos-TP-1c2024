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

sem_t sem_mutex;
sem_t sem_exe;


t_log* cpu_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* cpu_config;
//creamos una cola para guardar las instrucciones (problema productor-consumidor)
t_queue* instrucciones;

//creemos una variable global de instruccion actual
char* instruccionActual;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;

void recibir_instruccion_cpu(int PID, int PC)
{ 
	//enviar pcb
	PCB* to_send = malloc(sizeof(PCB));
	to_send->PC = PC;
	to_send->PID = PID;
	enviarPCB (to_send, fd_memoria);
/*
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
*/
}

void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion
	int i = 5;

	recibir_instruccion_cpu(proceso->PID,proceso->PC);
	while( i > 0)
	{
			//necesita esperar un semaforo
			//sleep(2);
			sem_wait(&sem_exe);//1   
			printf("EL program counter ANTES en CPU ES: %d\n",proceso->PC);
			/*
			sem_wait(&sem_mutex);
			char* instruccion = queue_pop(instrucciones);
			sem_post(&sem_mutex);
			//necesito saber que la instrucción recibida es la correcta
			printf("%s\n",instruccion);
			
			char** instruccion_split = string_split (instruccion, " ");
			printf("%s\n",instruccion_split[0]);
			
			if(strcmp(instruccion_split[0], "SET") == 0)
			{
				if(strcmp(instruccion_split[1], "AX") == 0)
				{
					int dato = atoi(instruccion_split[2]); 
					proceso->AX = dato; 
					printf("Ejecuta instruccion SET PARA AX, el AX = %d \n", proceso->AX);
				} 
				else
				{
					if(strcmp(instruccion_split[1], "BX") == 0)
					{
						printf("llegue hasta el verificar el bx if\n");
						int dato = atoi(instruccion_split [2]); 
						proceso->BX = dato;
						printf("Ejecuta instruccion SET PARA BX, el BX = %d \n", proceso->BX);
					}
					else
					{
						printf("Nada, por ahora\n");
					}
				}
			}
			i--;
			proceso->PC++;
			//pido de vuelta
			*/
		    proceso->PC++;
			printf("EL program counter DESPUES en CPU ES: %d\n",proceso->PC);
			recibir_instruccion_cpu(proceso->PID,proceso->PC);
	}
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
				//
				ejecutar_proceso(proceso);
				/*
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
				*/
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

 //recibimos operacion y mensaje
			int cod_op = recibir_operacion(fd_memoria);

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));

			recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);	
			printf("Recibimos el siguiente tamaño: %d\n",paquete->buffer->size);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_memoria,&(paquete->buffer->offset),sizeof(uint32_t),0);  // se agrego junto con util.c linea 300 
			recv(fd_memoria,paquete->buffer->stream, paquete->buffer->size,0);
			printf("Tenemos la siguiente instruccion %s\n",paquete->buffer->stream);
            //asignamos el stream al paquete
			//void *stream = paquete->buffer->stream;
			//memcpy(stream, &(paquete->buffer->size), sizeof(uint32_t));
			//stream += sizeof(uint32_t);
			// deserializamos el path como tal
			//char* ret = malloc(paquete->buffer->size);
			//memcpy(ret, stream, paquete->buffer->size);

			switch (cod_op) {
			case MENSAJE:
				printf("Instruccion de la memoria recibida con exito\n");
				
				/*
				instruccionActual = paquete->buffer->stream;

				//guardemos la instruccion en la cola
				char* instruction = malloc(paquete->buffer->size);
				instruction = paquete->buffer->stream;
				//semaforos
				sem_wait(&sem_mutex);
				queue_push(instrucciones,instruction);
                sem_post(&sem_mutex);
				*/			
				sem_post(&sem_exe);//2
				
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
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}


#endif /* CPU_H_ */