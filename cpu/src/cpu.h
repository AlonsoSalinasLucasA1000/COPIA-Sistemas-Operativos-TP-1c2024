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
	to_send->registro.AX  = 0;
	to_send->registro.BX = 0;
	to_send->registro.CX = 0;
	to_send->registro.DX = 0;
	to_send->registro.EAX = 0;//cambios
	to_send->registro.EBX = 0;//cambios
	to_send->registro.ECX = 0;
	to_send->registro.EDX = 0;//cambios
	to_send->registro.DI = 0;
	to_send->registro.SI = 0;//cambios

	to_send->estado = NEW;
	to_send->path_length = 1;
	to_send->path = "";
	enviarPCB (to_send, fd_memoria,PROCESO);

}


//Funcion para encontar la TLB del numero_pagina
/*	
	TLB* buscar_en_TLB(int numero_Pagina, PCB* proceso) {
    	// Implementación básica de búsqueda en la TLB
    	for (int i = 0; i < list_size(listaTLB); i++) {
			TLB* entrada = list_get(listaTLB, i);
			if (entrada->PID == proceso->PID && entrada->pagina == numero_Pagina) {
				return entrada; // Retorna la entrada de la TLB si se encuentra
			}
    	}
    	return NULL; // Retorna NULL si no se encuentra la entrada en la TLB (TLB Miss)
	}
*/
/*
	void agregar_a_TLB(int pid, int numero_Pagina, int marco) 
	{
		TLB* nueva_entrada = malloc(sizeof(TLB));
		nueva_entrada->pid = pid;
		nueva_entrada->pagina = numero_Pagina;
		nueva_entrada->marco = marco;
		
		list_add(listaTLB, nueva_entrada); // Agrega la nueva entrada a la lista de TLB
	}
*/
/*
	void algoritmoSustitucion(int pid, int numero_Pagina, int marco) 
	{
		// Implementación básica de FIFO para la TLB
		TLB* entrada_mas_antigua = list_get(listaTLB, 0);
		entrada_mas_antigua->pid = pid;
		entrada_mas_antigua->pagina = numero_Pagina;
		entrada_mas_antigua->marco = marco;
		
		// Mover la entrada más antigua al final de la lista (simulando FIFO)
		list_remove(listaTLB, 0);
		list_add(listaTLB, entrada_mas_antigua);
	}

*/
/*
	// Función para enviar solicitud a la memoria y recibir el número actualizado
	void obtener_marco_desde_memoria(int fd_memoria, int numero_Pagina, op_code codigo_operacion) {
		// Aquí se implementaría la lógica para enviar la solicitud a la memoria
		// usando el descriptor de archivo fd_memoria y recibir la respuesta.
		// Esto puede variar dependiendo de cómo se implemente la comunicación
		// con el subsistema de memoria en tu aplicación.
	}

	// Función para recibir el número desde la memoria
	int recibir_numero(int fd_memoria) 
	{
		// Implementación de recepción de número desde la memoria.
		// Esto puede variar dependiendo de la estructura de los datos que
		// recibes desde el subsistema de memoria.
		int numero_recibido = 0; // Ejemplo básico, debes adaptarlo según tu implementación real
		// Ejemplo básico:
		read(fd_memoria, &numero_recibido, sizeof(int));
		return numero_recibido;
	}

*/
// Función MMU, traductor de lógica a física
/*
	int mmu(int dir_Logica, PCB* proceso){  			 					//faltaria pasar el proceso								
		int numero_Pagina = floor(dirLogica/TAM_MEMORIA); 					//config.tam_pag_memoria= TAM_MEMORIA=4096,   floor(dirección_lógica / tamaño_página)
		int desplazamiento = dir_Logica - numero_Pagina * TAM_MEMORIA;   	//dirección_lógica - número_página * tamaño_página          ,TAM_PAGINA=32
		
		TLB* retorno_TLB = buscar_en_TLB(numero_Pagina);					                           //
		
		if(retorno_TLB!=NULL){  											                          // Si el TLB obtiene el numero_Pagina  ->TLB Hit
			log_info(cpu_logger, "TLB Hit: PID: %d- TLB HIT - Pagina: %d",proceso->pid,numero_Pagina );  
			return (retorno_TLB->marco) + desplazamiento;   				                         //devuelve la direccion fisica
		} else{																                        // Si no -> Se consulta a memoria por el marco correcto a la pagina buscada
			log_info(logger, "TLB Miss: PID: %d- TLB MISS - Pagina: %d", proceso->pid, numero_Pagina );
			int marco = obtener_marco_desde_memoria(fd_memoria, numero_Pagina, MARCO); 				//pide a memoria  
			op_code codigo_operacion = recibir_operacion(fd_memoria);

			if(codigo_operacion == NUMERO){
				log_debug(cpu_logger,"Entre a numero bien");
				int marco = recibir_numero(fd_memoria);

				if(list_size(listaTLB) < CANTIDAD_ENTRADAS_TLB){ 			//Si la nueva entrada a la TLB aun no esta llena
					agregar_a_TLB(proceso->PID, numero_Pagina, marco);		//datos de la TLB
				} else{														// pero si lo esta debo implementar el algoritmo
					//el algoritmo FIFO	
					algoritmoSustitucion(proceso->PID, numero_Pagina, marco);
				}
				return marco + desplazamiento;//Devuelve la direccion fisica
			}			
		}
		return -1;
	}

*/
void* obtener_registro(char* registro, PCB* proceso)
{
	if( strcmp(registro,"AX") == 0 )
	{
		return (void*)&(proceso->registro.AX); // Devuelve un puntero al valor de AX
	}
	else
	{
		if( strcmp(registro,"BX") == 0 )
		{
			return (void*)&(proceso->registro.BX); // Devuelve un puntero al valor de BX
		}
		else
		{
			if( strcmp(registro,"CX") == 0 )
			{
				return (void*)&(proceso->registro.CX); // Devuelve un puntero al valor de CX
			}
			else
			{
				if( strcmp(registro,"DX") == 0 )
				{
					return (void*)&(proceso->registro.DX); // Devuelve un puntero al valor de DX
				}
				else
				{
					printf("REGISTRO uint32\n");
					if( strcmp(registro,"EAX") == 0 )
					{
						return (void*)&(proceso->registro.EAX); // Devuelve un puntero al valor de EAX
					}
					else
					{
						if( strcmp(registro,"EBX") == 0 )
						{
							return (void*)&(proceso->registro.EBX); // Devuelve un puntero al valor de EBX
						}
						else{
							if( strcmp(registro,"ECX") == 0 )
							{
								return (void*)&(proceso->registro.ECX); // Devuelve un puntero al valor de ECX
							}
							else
							{
								if( strcmp(registro,"EDX") == 0 )
								{
									return (void*)&(proceso->registro.EDX); // Devuelve un puntero al valor de EDX
								}
								else
								{
									if( strcmp(registro,"SI") == 0 )
									{
										return (void*)&(proceso->registro.SI); // Devuelve un puntero al valor de SI
									}
									else
									{
										if( strcmp(registro,"DI") == 0 )
										{
											return (void*)&(proceso->registro.DI); // Devuelve un puntero al valor de DI
										}
										else
										{
											printf("ERROR, NO SE ENCONTRO REGISTRO\n");
										}
									}
								}
							}
						}

					}

				}
			}
		}
	}
	return NULL;
}

bool esRegistroUint8(char* registro)
{
	bool to_ret = false;
	if( strcmp(registro, "AX") == 0 )
	{
		to_ret = true;
	}
	else
	{
		if( strcmp(registro, "BX") == 0 )
		{
			to_ret = true;
		}
		else
		{
			if( strcmp(registro, "CX") == 0 )
			{
				to_ret = true;
			}
			else
			{
				if( strcmp(registro, "CX") == 0 )
				{
					to_ret = true;
				}
			}
		}
	}
	return to_ret;
}

bool esRegistroUint32(char* registro)
{
	bool to_ret = false;
	if( strcmp(registro, "EAX") == 0 )
	{
		to_ret = true;
	}
	else
	{
		if( strcmp(registro, "EBX") == 0 )
		{
			to_ret = true;
		}
		else
		{
			if( strcmp(registro, "ECX") == 0 )
			{
				to_ret = true;
			}
			else
			{
				if( strcmp(registro, "EDX") == 0 )
				{
					to_ret = true;
				}
				else
				{
					if( strcmp(registro, "SI") == 0 )
					{
						to_ret = true;
					}
					else
					{
						if( strcmp(registro, "DI") == 0 )
						{
							to_ret = true;
						}
					}
				}
			}
		}
	}
	return to_ret;
}

/*
void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion
	instruccionActual = "Goku";
	enviar_pcb_memoria(proceso->PID,proceso->PC);
	
	sem_wait(&sem_exe_b);
	while( strcmp(instruccionActual,"") != 0 )
	{
		//obtengo instruccion actual
		char* instruccion = string_duplicate(instruccionActual);
		printf("%s\n",instruccion); //verificamos que la instruccion actual sea correcta
		char** instruccion_split = string_split (instruccion, " ");

		//CASO DE TENER UNA INSTRUCCION SET
		if(strcmp(instruccion_split[0], "SET") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]))
			{
				uint8_t* valor_registro = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				if( valor_registro != NULL )
				{
					int dato = atoi(instruccion_split[2]);
					*valor_registro = dato;
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro);
				}
				else
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			/*
			int *valor_registro = (int *)obtener_registro(instruccion_split[1], proceso);
			if (valor_registro != NULL) 
			{
				int dato = atoi(instruccion_split[2]);
				*valor_registro = dato; // Asigna el valor a través del puntero
				printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro);
			} 
			else 
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			
		}
		//CASO DE TENER UNA INSTRUCCION SUM
		if (strcmp(instruccion_split[0], "SUM") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]) )
			{
				//si es un registro de 8 bits, tenemos que interpretarlo como tal
				uint8_t* valor_registro1 = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				uint8_t* valor_registro2 = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if (valor_registro1 != NULL && valor_registro2 != NULL) 
				{
					*valor_registro1 = *valor_registro1 + *valor_registro2; // Asigna el valor a través del puntero
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
				} 
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				printf("Todavia no implementado registros de 32 bits o tuviste algún error");
			}
			/*
			int *valor_registro1 = (int *)obtener_registro(instruccion_split[1], proceso);
			int *valor_registro2 = (int *)obtener_registro(instruccion_split[2], proceso);
			if (valor_registro1 != NULL && valor_registro2 != NULL) 
			{
				*valor_registro1 = *valor_registro1 + *valor_registro2; // Asigna el valor a través del puntero
				printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
			} 
			else 
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			/
		}
		//CASO DE TENER UNA INSTRUCCION SUB
		if (strcmp(instruccion_split[0], "SUB") == 0)
		{
			//TENEMOS QUE APRECIAR LOS CASOS DONDE OBTENGAMOS AX, BX, CX o DX y los casos donde tengamos EAX, EBX, ECX, EDX, SI o DI. ASUMIENDO QUE NO SE INTENTA SUMAR UN AX con EAX
			if( esRegistroUint8(instruccion_split[1]) )
			{
				//si es un registro de 8 bits, tenemos que interpretarlo como tal
				uint8_t* valor_registro1 = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				uint8_t* valor_registro2 = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if (valor_registro1 != NULL && valor_registro2 != NULL) 
				{
					*valor_registro1 = *valor_registro1 - *valor_registro2; // Asigna el valor a través del puntero
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
				} 
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				printf("Todavia no implementado registros de 32 bits o tuviste algún error");
			}
			/*
			int *valor_registro1 = (int *)obtener_registro(instruccion_split[1], proceso);
			int *valor_registro2 = (int *)obtener_registro(instruccion_split[2], proceso);
			if (valor_registro1 != NULL && valor_registro2 != NULL) 
			{
				*valor_registro1 = *valor_registro1 - *valor_registro2; // Asigna el valor a través del puntero
				printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
			} 
			else 
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			
		}
		//CASO DE TENER UNA INSTRUCCION JNZ
		if (strcmp(instruccion_split[0], "JNZ") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]))
			{
				uint8_t* valor_registro = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				if( valor_registro != NULL )
				{
					if( *valor_registro != 0 )
					{
						proceso->PC = atoi(instruccion_split[2]);
						printf("El registro PC ha sido modificado a: %d", proceso->PC);
					}
				}
				else
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			/*
			int *valor_registro = (int *)obtener_registro(instruccion_split[1], proceso);
			if (valor_registro != NULL) 
			{
				if( *valor_registro != 0 )
				{
					proceso->PC = atoi(instruccion_split[2]);
				}
			} 
			else 
			{
				printf("El registro no se encontró en el proceso.\n");
			}
			
		}
		//AUMENTAMOS EL PC Y PEDIMOS NUEVAMENTE
		proceso->PC++;
		enviar_pcb_memoria(proceso->PID,proceso->PC);
		printf("------------------------------\n");
		sem_post(&sem_exe_a);
		sem_wait(&sem_exe_b);
	}
	enviarPCB(proceso,fd_kernel_dispatch,PROCESOFIN);
} */

void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion
	instruccionActual = "Goku";
	enviar_pcb_memoria(proceso->PID,proceso->PC);

	sem_wait(&sem_exe_b);
	while( strcmp(instruccionActual,"") != 0 )
	{
		//obtengo instruccion actual
		char* instruccion = string_duplicate(instruccionActual);
		printf("%s\n",instruccion); //verificamos que la instruccion actual sea correcta
		char** instruccion_split = string_split (instruccion, " ");

		//CASO DE TENER UNA INSTRUCCION SET
		if(strcmp(instruccion_split[0], "SET") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]))
			{
				uint8_t* valor_registro = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				if( valor_registro != NULL )
				{
					int dato = atoi(instruccion_split[2]);
					*valor_registro = dato;
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro);
				}
				else
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				if( esRegistroUint32(instruccion_split[1]))
				{
					uint32_t* valor_registro = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
					if( valor_registro != NULL )
					{
						int dato = atoi(instruccion_split[2]);
						*valor_registro = dato;
						printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro);
					}
					else
					{
						printf("El registro no se encontró en el proceso.\n");
					}
				}
			}
		}
		//CASO DE TENER UNA INSTRUCCION SUM
		if (strcmp(instruccion_split[0], "SUM") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]) )
			{
				//si es un registro de 8 bits, tenemos que interpretarlo como tal
				uint8_t* valor_registro1 = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				uint8_t* valor_registro2 = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if (valor_registro1 != NULL && valor_registro2 != NULL) 
				{
					*valor_registro1 = *valor_registro1 + *valor_registro2; // Asigna el valor a través del puntero
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
				} 
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				if( esRegistroUint32(instruccion_split[1]) )
				{
					uint32_t* valor_registro1 = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
					uint32_t* valor_registro2 = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
					if (valor_registro1 != NULL && valor_registro2 != NULL) 
					{
						*valor_registro1 = *valor_registro1 + *valor_registro2;
						printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
					} 
					else 
					{
						printf("El registro no se encontró en el proceso.\n");
					}
				}
			}
		}
		//CASO DE TENER UNA INSTRUCCION SUB
		if (strcmp(instruccion_split[0], "SUB") == 0)
		{
			//TENEMOS QUE APRECIAR LOS CASOS DONDE OBTENGAMOS AX, BX, CX o DX y los casos donde tengamos EAX, EBX, ECX, EDX, SI o DI. ASUMIENDO QUE NO SE INTENTA SUMAR UN AX con EAX
			if( esRegistroUint8(instruccion_split[1]) )
			{
				//si es un registro de 8 bits, tenemos que interpretarlo como tal
				uint8_t* valor_registro1 = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				uint8_t* valor_registro2 = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if (valor_registro1 != NULL && valor_registro2 != NULL) 
				{
					*valor_registro1 = *valor_registro1 - *valor_registro2; // Asigna el valor a través del puntero
					printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
				} 
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				if( esRegistroUint32(instruccion_split[1]) )
				{
					//si es un registro de 8 bits, tenemos que interpretarlo como tal
					uint32_t* valor_registro1 = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
					uint32_t* valor_registro2 = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
					if (valor_registro1 != NULL && valor_registro2 != NULL) 
					{
						*valor_registro1 = *valor_registro1 - *valor_registro2; // Asigna el valor a través del puntero
						printf("El valor de %s es: %d\n", instruccion_split[1], *valor_registro1);
					} 
					else 
					{
						printf("El registro no se encontró en el proceso.\n");
					}
				}
			}
		}
		//CASO DE TENER UNA INSTRUCCION JNZ
		if (strcmp(instruccion_split[0], "JNZ") == 0)
		{
			if( esRegistroUint8(instruccion_split[1]))
			{
				uint8_t* valor_registro = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				if( valor_registro != NULL )
				{
					if( *valor_registro != 0 )
					{
						proceso->PC = atoi(instruccion_split[2]);
						printf("El registro PC ha sido modificado a: %d", proceso->PC);
					}
				}
				else
				{
					printf("El registro no se encontró en el proceso.\n");
				}
			}
			else
			{
				if( esRegistroUint32(instruccion_split[1]))
				{
					uint32_t* valor_registro = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
					if( valor_registro != NULL )
					{
						if( *valor_registro != 0 )
						{
							proceso->PC = atoi(instruccion_split[2]);
							printf("El registro PC ha sido modificado a: %d", proceso->PC);
						}
					}
				}
				else
				{
					printf("El registro no se encontró en el proceso.\n");
				}
				
			}
		}
	
		//CASO DE TENER UNA INSTRUCCION IO_GEN_SLEEP
		//CASO DE TENER UNA INSTRUCCION IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
		//CASO DE TENER UNA INSTRUCCION IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)
		/*
		if (strcmp(instruccion_split[0], "IO_STDIN_READ") == 0 ) || strcmp(instruccion_split[0], "IO_STDOUT_WRITE") == 0 || ) 
		{
			if( esRegistroUint8(instruccion_split[2])) //REGISTRO DIRECCION UNIT8
			{
				int* direc_logica = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if( direc_logica != NULL )
				{
					//REGISTRO TAMAÑO UNIT8
					if ( esRegistroUint8(instruccion_split[3])){
						int* tam_registro = (uint8_t*)obtener_registro(instruccion_split[3],proceso);
						mmu (direc_logica,proceso); //fc a implementar (MMU)
					}
					else 
					{	//REGISTRO TAMAÑO UINT32
						if ( esRegistroUint32(instruccion_split[3])){
							int* tam_registro = (uint32_t*)obtener_registro(instruccion_split[3],proceso);
							int direccion_fisica= mmu (direc_logica, proceso); //fc a implementar (MMU)
							//mandar a kerner direccion_fisica y tam_registro
						}
						else 
						{
							printf("El registro no se encontró en el proceso.\n");
						}
					}
					
				}	
			}
			else
			{
				if( esRegistroUint32(instruccion_split[2])) //REGISTRO DIRECCION UNIT32
				{
					int* direc_logica = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
					if( direc_logica != NULL )
					{
						//REGISTRO TAMAÑO UNIT8
						if ( esRegistroUint8(instruccion_split[3])){
							int* tam_registro = (uint8_t*)obtener_registro(instruccion_split[3],proceso);
							mmu (direc_logica, proceso); //fc a implementar (MMU)
						}
						else 
						{	//REGISTRO TAMAÑO UINT32
							if ( esRegistroUint32(instruccion_split[3])){
								int* tam_registro = (uint32_t*)obtener_registro(instruccion_split[3],proceso);
								mmu (direc_logica, proceso); //fc a implementar (MMU)
							}
							else 
							{
								printf("El registro no se encontró en el proceso.\n");
							}
						}
					
					}	
				}
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
				
			}
		}
		//CASO DE TENER UNA INSTRUCCION MOV_IN (Registro Datos, Registro Dirección)
		if (strcmp(instruccion_split[0], "MOV_IN") == 0)
		{
			if( esRegistroUint8(instruccion_split[2])) //REGISTRO DIRECCION UNIT8
			{
				int* direc_logica = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
				if( direc_logica != NULL )
				{
					//REGISTRO DATOS
					if ( esRegistroUint8(instruccion_split[1])){
						int* registro_datos = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
						mmu (direc_logica, proceso); 
						int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
						*registro_datos = ;//resultado de buscar_tlb
					}
					else 
					{	//REGISTRO DATOS
						if ( esRegistroUint32(instruccion_split[1])){
							int* registro_datos = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
							mmu(direc_logica, proceso); //fc a implementar (MMU)
							int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
							*registro_datos = ;//resultado de buscar_tlb
		
						}
					}
				}	
			}
			else
			{
				if( esRegistroUint32(instruccion_split[2])) //REGISTRO DIRECCION UNIT32
				{
					int* direc_logica = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
					if( direc_logica != NULL )
					{
						//REGISTRO TAMAÑO UNIT8
						if ( esRegistroUint8(instruccion_split[3])){
							int* registro_datos = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
							mmu (direc_logica, proceso); 
							int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
							*registro_datos = ;//resultado de buscar_tlb
						}
						else 
						{	//REGISTRO TAMAÑO UINT32
							if ( esRegistroUint32(instruccion_split[3])){
								int* registro_datos = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
								mmu (direc_logica, proceso);
								int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
								*registro_datos = ;//resultado de buscar_tlb
							}
						}
					}	
				}
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
				
			}
		}

		//CASO DE TENER UNA INSTRUCCION MOV_OUT (Registro Direccion, Registro Datos)
		if (strcmp(instruccion_split[0], "MOV_OUT") == 0)
		{
			if( esRegistroUint8(instruccion_split[1])) //REGISTRO DIRECCION UNIT8
			{
				int* direc_logica = (uint8_t*)obtener_registro(instruccion_split[1],proceso);
				if( direc_logica != NULL )
				{
					//REGISTRO DATOS
					if ( esRegistroUint8(instruccion_split[2])){
						int* registro_datos = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
						mmu (direc_logica, proceso); 
						int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
						*registro_datos = ;//resultado de buscar_tlb
					}
					else 
					{	//REGISTRO DATOS
						if ( esRegistroUint32(instruccion_split[2])){
							int* registro_datos = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
							mmu (direc_logica, proceso); //fc a implementar (MMU)
							int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
							*registro_datos = ;//resultado de buscar_tlb
		
						}
					}
				}	
			}
			else
			{
				if( esRegistroUint32(instruccion_split[1])) //REGISTRO DIRECCION UNIT32
				{
					int* direc_logica = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
					if( direc_logica != NULL )
					{
						//REGISTRO TAMAÑO UNIT8
						if ( esRegistroUint8(instruccion_split[2])){
							int* registro_datos = (uint8_t*)obtener_registro(instruccion_split[2],proceso);
							mmu (direc_logica, proceso); 
							int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
							*registro_datos = ;//resultado de buscar_tlb
						}
						else 
						{	//REGISTRO TAMAÑO UINT32
							if ( esRegistroUint32(instruccion_split[2])){
								int* registro_datos = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
								mmu (direc_logica, proceso);
								int reemplazo_dato = buscar_tlb ;//fc buscar un  la tlb o en su caso pedir a memoria lo que hay en el la direc fisica que nos paso la mmu
								*registro_datos = ;//resultado de buscar_tlb
							}
						}
					}	
				}
				else 
				{
					printf("El registro no se encontró en el proceso.\n");
				}
				
			}
		}

		//CASO DE TENER UNA INSTRUCCION COPY_STRING (Tamaño) tamaño = cantidad de bytes a copiar
		if (strcmp(instruccion_split[0], "COPY_STRING") == 0)
		{
			int* direc_logica_si = (uint32_t*)obtener_registro("SI",proceso);
			int* direc_logica_di = (uint32_t*)obtener_registro("DI",proceso);
			if( direc_logica_si != NULL || direc_logica_di != NULL) //DIRECCION SI y DIRECCION DI
			{
				int tamanio = atoi (instruccion_split[1]);
				mmu (direc_logica_si, proceso); 
				mmu (direc_logica_di, proceso); 
				//copiar el contenido de la direccion contenida en si y lo pone en la direccion contenida en di
			}
			else
			{
				printf("El registro no se encontró en el proceso.\n");				
			}
		}



		*/
		
		//AUMENTAMOS EL PC Y PEDIMOS NUEVAMENTE
			proceso->PC++;
			enviar_pcb_memoria(proceso->PID,proceso->PC);
			printf("------------------------------\n");
			sem_post(&sem_exe_a);
			sem_wait(&sem_exe_b);
		
	}
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
				printf("Su AX es: %d\n",proceso->registro.AX);//cambios
				printf("Su BX es: %d\n",proceso->registro.BX);
				printf("Su CX es: %d\n",proceso->registro.CX);//cambios
				printf("Su DX es: %d\n",proceso->registro.DX);
				printf("Su DI es: %d\n",proceso->registro.DI);//cambios
				printf("Su EAX es: %d\n",proceso->registro.EAX);

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