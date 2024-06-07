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

sem_t sem_exe;


t_log* cpu_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* cpu_config;
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
	enviarPCB (to_send, fd_memoria,PROCESO);

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

	recibir_instruccion_cpu(proceso->PID,proceso->PC);
	//POSIBLES PROBLEMAS
	int i = 6;
	while( i > 0 )
	{
		//necesita esperar un semaforo
		sem_wait(&sem_exe);
		//obtengo instruccion actual
		char* instruccion = string_duplicate(instruccionActual);
		printf("%s\n",instruccion); //verificamos que la instruccion actual sea correcta
		char** instruccion_split = string_split (instruccion, " ");
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
			//
		}else if (strcmp(instruccion_split[0], "SUM") == 0){

			if(strcmp(instruccion_split[1], "AX") == 0)
			{
				
				if(strcmp(instruccion_split[2], "BX") == 0)
				{
					
					proceso->AX +=+ proceso->BX;

					printf("Ejecuta instruccion SUM PARA AX + BX, el AX = %d \n", proceso->AX);
				}
				else
				{
					printf("Error, registro no implementado.\n");
				}
			} 
			else
			{
				if(strcmp(instruccion_split[1], "BX") == 0)
				{
					if(strcmp(instruccion_split[2], "AX") == 0)
					{
					proceso->BX += proceso->BX;

					printf("Ejecuta instruccion SUM PARA BX + AX, el BX = %d \n", proceso->BX);
					}else{
					printf("Error, registro no implementado.\n");
					}
				}
			}
		}else if (strcmp(instruccion_split[0], "SUB") == 0){

			if(strcmp(instruccion_split[1], "AX") == 0)
			{
				
				if(strcmp(instruccion_split[2], "BX") == 0)
				{
					
					proceso->AX -= proceso->BX;

					printf("Ejecuta instruccion SUB PARA AX - BX, el AX = %d \n", proceso->AX);
				}
				else
				{
					printf("Error, registro no implementado.\n");
				}
			} 
			else
			{
				if(strcmp(instruccion_split[1], "BX") == 0)
				{
					if(strcmp(instruccion_split[2], "AX") == 0)
					{
					proceso->BX -= proceso->BX;

					printf("Ejecuta instruccion SUM PARA BX - AX, el BX = %d \n", proceso->BX);
					}else{
					printf("Error, registro no implementado.\n");
					}
				}
			}
		}else if (strcmp(instruccion_split[0], "JNZ") == 0){

			if(strcmp(instruccion_split[1], "AX") == 0)
			{
				if (proceso->AX != 0)
				{
					
					proceso->PC = atoi(instruccion_split [2])-1;
					printf("Ejecuta instruccion JNZ PARA AX, program counter = %d \n", proceso->PC);
				}else{
					printf("Error en la ejecucion de JNZ\n");
				}
			}else
			{
				if(strcmp(instruccion_split[1], "BX") == 0)
				{
					if (proceso->BX != 0)
					{
						proceso->PC = atoi(instruccion_split [2])-1;
						printf("Ejecuta instruccion JNZ PARA BX, program counter = %d \n", proceso->PC);

					}else{
						printf("Error en la ejecucion de JNZ\n");
					}
				}
			}
		}
        i--;
		proceso->PC++;
			//pido de vuelta
		recibir_instruccion_cpu(proceso->PID,proceso->PC);
	}
	//debemos devolver la pcb al kernel, llegado a este punto el proceso terminó
	enviarPCB(proceso,fd_kernel,PROCESOFIN);
}


void cpu_escuchar_kernel (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);
				
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);
			
		    switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case PROCESO:
				PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
				ejecutar_proceso(proceso);
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
            //deserializamos el mensaje, primero el tamaño del buffer, luego el offset
			recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);		
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_memoria,&(paquete->buffer->offset), sizeof(uint32_t),0);
			recv(fd_memoria,paquete->buffer->stream,paquete->buffer->size,0);
			

			switch (cod_op) {
			case MENSAJE:
				printf("Instruccion de la memoria recibida con exito\n");
				//variable global con 
				instruccionActual = malloc(paquete->buffer->size);
				instruccionActual = paquete->buffer->stream;
				printf("------------------------------\n");
				printf("La instruccion que llego fue: %s\n",instruccionActual);
				//instruccionActual = paquete->buffer->stream;
				sem_post(&sem_exe);
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(cpu_logger, "Desconexion de memoria.\n");
				control_key = 0;
			default:
				log_warning(cpu_logger,"Operacion desconocida. No quieras meter la pata\n");
				break;
			}
		}	
}


#endif /* CPU_H_ */