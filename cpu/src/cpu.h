#ifndef CPU_H_
#define CPU_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//file descriptors de CPU y los modulos que se conectaran con el
int fd_cpu_dispatch;
int fd_cpu_interrupt; //luego se dividira en dos fd un dispatch y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_kernel_dispatch;
int fd_kernel_interrupt;

sem_t sem_exe_a;
sem_t sem_exe_b;


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

void enviar_pcb_memoria(int PID, int PC)
{ 
	//enviar pcb
	PCB* to_send = malloc(sizeof(PCB));
	to_send->PC = PC;
	to_send->PID = PID;
	to_send->quantum = 0;
	to_send->AX = 0;
	to_send->BX = 0;
	to_send->CX = 0;
	to_send->DX = 0;
	to_send->estado = NEW;
	to_send->path_length = 1;
	to_send->path = "";
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

//UNA POSIBLE SOLUCION Y LOS CASOS.
void* obtener_registro(char* registro, PCB* proceso)
{
	if( strcmp(registro,"AX") == 0 )
	{
		return proceso->AX;
	}
	else
	{

	}
}

void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion
	instruccionActual = "Goku";
	enviar_pcb_memoria(proceso->PID,proceso->PC);
	//POSIBLES PROBLEMAS
	//int i = 7;
	sem_wait(&sem_exe_b);
	while( strcmp(instruccionActual,"") != 0 )
	{
		//necesita esperar un semaforo
		
		//obtengo instruccion actual
		char* instruccion = string_duplicate(instruccionActual);
		printf("%s\n",instruccion); //verificamos que la instruccion actual sea correcta
		char** instruccion_split = string_split (instruccion, " ");

		//CÓMO HACEMOS ESTO MÁS EFICIENTEMENTE
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
					
					proceso->AX += proceso->BX;

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
					proceso->BX += proceso->AX;

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
					proceso->BX -= proceso->AX;

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
        //i--;
		proceso->PC++;
		//pido de vuelta
		enviar_pcb_memoria(proceso->PID,proceso->PC);
		printf("------------------------------\n");
		sem_post(&sem_exe_a);
		sem_wait(&sem_exe_b);
		//sem_wait(&sem_exe);
	}
	//reiniciamos el semaforo
	//debemos devolver la pcb al kernel, llegado a este punto el proceso terminó
	enviarPCB(proceso,fd_kernel_dispatch,PROCESOFIN);
}


void cpu_escuchar_kernel_dispatch (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel_dispatch);

			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel_dispatch,&(paquete->buffer->size),sizeof(uint32_t),0);
				
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel_dispatch,paquete->buffer->stream, paquete->buffer->size,0);
			
		    switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case PROCESO:
				PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
				printf("Recibi el siguiente proceso:\n");
				printf("Su PID es: %d\n",proceso->PID);
				printf("Su AX es: %d\n",proceso->AX);
				printf("Su BX es: %d\n",proceso->BX);
				printf("Su CX es: %d\n",proceso->CX);
				printf("Su DX es: %d\n",proceso->DX);
				ejecutar_proceso(proceso);
				sem_init(&sem_exe_a,0,1);
                sem_init(&sem_exe_b,0,0);
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

void cpu_escuchar_kernel_interrupt (){
		bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel_interrupt);

			//debemos extraer el resto, primero el tamaño y luego el contenido
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel_interrupt,&(paquete->buffer->size),sizeof(uint32_t),0);
				
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel_interrupt,paquete->buffer->stream, paquete->buffer->size,0);
			
		    switch (cod_op) {
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case PROCESO:
				printf("Hola recibi algo por interrupt\n");
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
			    sem_wait(&sem_exe_a);
				printf("Instruccion de la memoria recibida con exito\n");
				//variable global con 
				instruccionActual = malloc(paquete->buffer->size);
				char* instruccionQueLlego = paquete->buffer->stream;
				instruccionActual = string_duplicate(instruccionQueLlego);
				printf("La instruccion que llego fue: %s\n",instruccionActual);
				//instruccionActual = paquete->buffer->stream;
				sem_post(&sem_exe_b);
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
			//liberamos todo para evitar los errores pasados
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}


#endif /* CPU_H_ */