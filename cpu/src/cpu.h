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
//lista de TLB
t_list* listaTLB;
t_list* listDireccionesFisicas;
t_list* direcciones_fisicas;
int* TAM_PAGINA;

//Si la asignacion fue correcta es 0, de haber out of memory es 1
int asignacion_or_out_of_memory;

//para controlar el envio y ejecucion de instrucciones
sem_t sem_exe_a;
sem_t sem_exe_b;

//para el resize
sem_t sem_memoria_aviso_cpu;

//para el copy string
sem_t sem_lectura;
sem_t sem_escritura;

//semaforo para ver si hay interrupcion
sem_t interrupt_mutex;

//semaforo para recibir el marco de memoria
sem_t sem_mmu;

t_log* cpu_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS

t_log* cpu_logs_obligatorios; //LOG OBLIGATORIO
t_config* cpu_config;

//t_config* 
//creemos una variable global de instruccion actual
char* instruccionActual;
uint8_t* valor_leido;
int* num_marco;

char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PUERTO_ESCUCHA_DISPATCH;
char* PUERTO_ESCUCHA_INTERRUPT;
int CANTIDAD_ENTRADAS_TLB;
char* ALGORITMO_TLB;
int* any_interrupcion;

void enviar_instruccion_kernel (char* instruccion, uint32_t tam_instruccion, PCB proceso, op_code codigo_operacion )
{
    //Creamos un Buffer
    t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(PCB) + (tam_instruccion) + sizeof(uint32_t);//cambios
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset, &proceso, sizeof(PCB));
    buffer->offset += sizeof(PCB);


    // Para el nombre primero mandamos el tamaño y luego el texto en sí:
    memcpy(buffer->stream + buffer->offset, &tam_instruccion, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, instruccion, tam_instruccion);
    
	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;

    //Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(fd_kernel_dispatch, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
	//free(buffer);
    free(paquete);
}

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


//Funcion para encontar la TLB del numero_pagina y pid del proceso

TLB* buscar_en_TLB(int numero_Pagina, PCB* proceso) { 	
    // Implementación básica de búsqueda en la TLB 
    for (int i = 0; i < list_size(listaTLB); i++) { 
		TLB* entrada = list_get(listaTLB, i);

		if (entrada->PID == proceso->PID && entrada->pagina == numero_Pagina) {
				
			return entrada; // Retorna la entrada de la TLB encontrada
		}
    }
    return NULL; // Retorna NULL si no se encuentra la entrada en la TLB (TLB HIT)
}

//Funcion para agregar a lista de TLB el pid del proceso y su numero_pagina
void agregar_a_TLB(int pid, int numero_Pagina, int marco) 
{
	TLB* nueva_entrada = malloc(sizeof(TLB));
	nueva_entrada->PID = pid;
	nueva_entrada->pagina = numero_Pagina;
	nueva_entrada->marco = marco;
	
	list_add(listaTLB, nueva_entrada);
	printf("Agregué el proceso %d a la TLB\n", nueva_entrada->PID);

	//
	for(int i=0; i < list_size(listaTLB); i++){
		
		TLB* entrada_tlb = list_get(listaTLB, i);

		printf("El proceso %d tiene numero de pagina %d y marco %d\n", entrada_tlb->PID, entrada_tlb->pagina, entrada_tlb->marco);

	}
	printf("Fin de agregar entradas a la TLB\n");
}

void algoritmoSustitucion(int pid, int numero_Pagina, int marco) 
{
	if(strcmp(ALGORITMO_TLB,"FIFO")==0)    //-------------------
	{   // Implementación básica de FIFO para la TLB
		
		agregar_a_TLB(pid, numero_Pagina, marco);
		/*
		TLB* entrada_mas_antigua = malloc(sizeof(TLB));      //list_get(listaTLB, 0);

		entrada_mas_antigua->PID = pid;
		entrada_mas_antigua->pagina = numero_Pagina;
		entrada_mas_antigua->marco = marco;     
			
		// Mover la entrada más antigua al final de la lista (simulando FIFO)
		*/	
		list_remove(listaTLB, 0);
		//list_add(listaTLB, entrada_mas_antigua);
		
	}   
		
	/*if(strcmp(ALGORITMO_TLB,"LRU")==0)   //---------------
	{
		// Implementación de LRU para la TLB
        int indiceLRU;  //
		TLB* entradaLRU = buscar_en_TLB(numero_Pagina, pid, &indiceLRU);
		
		if (entradaLRU != NULL) {
			// Actualizar la entrada LRU encontrada
			entradaLRU->pid = pid;
			entradaLRU->pagina = numero_Pagina;
			entradaLRU->marco = marco;
			entradaLRU->contadorLRU = LRU_counter++;
		} else {
			// No se encontró una entrada, se debe reemplazar la menos recientemente usada
			indiceLRU = encontrar_indice_menos_recientemente_usado();
			TLB* listaTLB[indiceLRU].pid = pid;   // 
			listaTLB[indiceLRU].pagina = numero_Pagina;
			listaTLB[indiceLRU].marco = marco;
			listaTLB[indiceLRU].contadorLRU = LRU_counter++;
		}
			
	}*/ 
	//ver2
		if(strcmp(ALGORITMO_TLB, "LRU")==0)
	{
		TLB* entradaLRU = buscar_en_TLB(numero_Pagina, pid);
		int LRU_counter = 0; //

		if(entradaLRU != NULL)
		{
			entradaLRU->PID = pid;
			entradaLRU->pagina = numero_Pagina;
			entradaLRU->marco = marco;
			entradaLRU->contadorLRU = LRU_counter++;
		} else{
			TLB* entrada_menos_usada = NULL;
			for(int i = 0 ; i < list_size(listaTLB); i++){
				TLB* entrada_act = list_get(listaTLB, i);
				if(entrada_menos_usada == NULL || entrada_act->contadorLRU < entrada_menos_usada->contadorLRU){
					entrada_menos_usada = entrada_act;
				}				
			}

			if(entrada_menos_usada != NULL){ //reemplazo
				entrada_menos_usada->PID = pid;
				entrada_menos_usada->pagina = numero_Pagina;
				entrada_menos_usada->marco = marco;
				entrada_menos_usada->contadorLRU = LRU_counter++;
			}
		}
	}
	//else   
	//{
    //  	printf("Algoritmo de TLB no reconocido\n");
    //}
} 
	
// Función de serialización para enviar solicitud a la memoria para recibir el marco correspondiente al proceso
void enviar_paginaypid_a_memoria(int numero_pagina, uint32_t pid, op_code codigo_operacion) 
{
	//Creamos un Buffer
    t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	
    //Calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint32_t);//cambios
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
	memcpy(buffer->stream + buffer->offset, &numero_pagina, sizeof(int));
    buffer->offset += sizeof(int);
    memcpy(buffer->stream + buffer->offset, &pid, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = codigo_operacion;
    paquete->buffer = buffer;

    //Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    //No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

// Función MMU, traductor de lógica a física
t_list* mmu(int dir_Logica, PCB* proceso, int tamanio)  // 1 dir fisica -> uint8 / 4 dir fisica -> uint32
{  	
	listDireccionesFisicas = list_create();
	int dir_fisica=0;	 									
	for(int i=0; i < tamanio; i++)
	{
		int numero_Pagina = floor(dir_Logica/ *TAM_PAGINA); 					//config.TAM_PAGINA. necesitamos traer de memoria.config
		int desplazamiento = dir_Logica - numero_Pagina * *TAM_PAGINA;   	//dirección_lógica - número_página * tamaño_página     TAM_PAGINA    
		
		printf("El desplazamiento es: %d, numero pagina: %d\n",desplazamiento,numero_Pagina);
		
		TLB* retorno_TLB = buscar_en_TLB(numero_Pagina, proceso);			//buscar por numero de pagina y pid de proceso	
			
		if(retorno_TLB!=NULL){  											 // Si el TLB obtiene el numero_Pagina  -> TLB Hit
			log_info(cpu_logger, "TLB Hit: PID: %d- TLB HIT - Pagina: %d", proceso->PID, numero_Pagina );  
			
			dir_fisica = retorno_TLB->marco * *TAM_PAGINA + desplazamiento; //se calcula la direccion fisica TAM_PAGINA
			//return (retorno_TLB->marco) + desplazamiento;   				 //devuelve la direccion fisica
		} else{																// Si no -> Se consulta a memoria por el marco correcto a la pagina buscada
			//log_info(cpu_logger, "TLB Miss: PID: %d- TLB MISS - Pagina: %d", proceso->PID, numero_Pagina ); ---------------------------------------> chequear
			enviar_paginaypid_a_memoria(numero_Pagina, proceso->PID, MARCO); //pide a memoria  

			//printf("Llegué hasta antes del semaforo\n");
			//tiene que esperar que llegue el marco de memoria
			sem_wait(&sem_mmu);
			
			printf("Obtuve el marco de proceso %d\n",proceso->PID);
			//num_marco es global
			if(list_size(listaTLB) < CANTIDAD_ENTRADAS_TLB){ 			//Si la nueva entrada a la TLB aun no esta llena
				
				printf("Voy agregar el proceso %d  a la TLB\n", proceso->PID);
				agregar_a_TLB(proceso->PID, numero_Pagina, *num_marco);	//agregamos los datos del proceso a la TLB
			} else{														// pero si lo esta debo implementar el algoritmo
				//el algoritmo FIFO	y LRU
				algoritmoSustitucion(proceso->PID, numero_Pagina, *num_marco);
			}

			dir_fisica =  *num_marco * *TAM_PAGINA + desplazamiento;
			//return *num_marco + desplazamiento; //Devuelve la direccion fisica		
		} 
		printf("La direccion fisica %d es: %d\n",i , dir_fisica);
		list_add(listDireccionesFisicas, dir_fisica); //
		dir_Logica++;
	}

	if(list_size(listDireccionesFisicas) > 0)
	{
		return listDireccionesFisicas;
	}else{
		return -1;
	}
	list_destroy(listDireccionesFisicas);
}


void pedido_lectura_numerico(int direccion_fisica, int tamanioDato)
{
	int* entero_a_enviar = malloc(sizeof(int));
	*entero_a_enviar = direccion_fisica;
	
	int* size_a_enviar = malloc(sizeof(int));
	*size_a_enviar = tamanioDato;

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int)*2;
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset,entero_a_enviar, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,size_a_enviar, sizeof(int));

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = LECTURA;
    paquete->buffer = buffer;

	//Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);

}


void pedido_escritura_numerico (int direccion_fisica, uint8_t valor_a_escribir)
{
	int* direccion_a_enviar = malloc(sizeof(int));
	*direccion_a_enviar = direccion_fisica;
	
	uint8_t* entero_a_enviar = malloc(sizeof(uint8_t));
	*entero_a_enviar = valor_a_escribir;

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint8_t);
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset,direccion_a_enviar, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,entero_a_enviar, sizeof(uint8_t));

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = ESCRITURA_NUMERICO;
    paquete->buffer = buffer;

	//Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


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
											if(strcmp(registro,"PC") == 0)
											{
												return (void*)&(proceso->PC); // Devuelve un puntero al valor de DI
											}
											else
											{
												printf("Error, no se ha encontrado el registro\n");
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
				if( strcmp(registro, "DX") == 0 )
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
						else
						{
							if( strcmp(registro, "PC") == 0 )
							{
								to_ret = true;
							}
						}
					}
				}
			}
		}
	}
	return to_ret;
}

void enviarDirecciones(int direccion_origen, int direccion_destino)
{
	//preparamos la primera direccion
	int* d1_to_send = malloc(sizeof(int));
	*d1_to_send = direccion_origen;

	//preparamos la segunda direccion
	int* d2_to_send = malloc(sizeof(int));
	*d2_to_send = direccion_destino;

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int)*2;
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);

    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset, d1_to_send, sizeof(int));
	buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset, d2_to_send, sizeof(int));

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = COPY_STRING;
    paquete->buffer = buffer;

	//Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
    int offset = 0;
    memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(op_code));
    offset += sizeof(op_code);
    memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
    offset += sizeof(uint32_t);
    memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);
    //Por último enviamos
    send(fd_memoria, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}


void ejecutar_proceso(PCB* proceso)
{
	//enviar mensaje a memoria, debemos recibir primera interrupcion
	instruccionActual = "Goku";
	enviar_pcb_memoria(proceso->PID,proceso->PC);

	*any_interrupcion = 0;	
	sem_wait(&sem_exe_b);
	while( strcmp(instruccionActual,"") != 0 )
	{
		//obtengo instruccion actual
		char* instruccion = string_duplicate(instruccionActual);
		printf("%s\n",instruccion); //verificamos que la instruccion actual sea correcta
		char** instruccion_split = string_split (instruccion, " ");

		proceso->PC++;

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
		if(strcmp(instruccion_split[0], "IO_GEN_SLEEP") == 0 )
		{
			//Primero bloqueamos al proceso para que llegue a la cola de bloqueados antes que nada y luego ejecutamos la rutina relacionada con la IO
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);

			//debemos devolver instruccion + pcb parando el proceso actual
			uint32_t instruccion_length = strlen(instruccion)+1;
			enviar_instruccion_kernel(instruccion, instruccion_length,*proceso,GENERICA);
			
			//se bloquea el proceso, devolvemos al kernel
			free(instruccionActual);
			free(instruccion);
			instruccionActual = malloc(1);
			instruccionActual = "";
			/*
			//dormir un poco antes de enviar, para no solaparse a la hora de mandar
			//usleep(1000);
			proceso->PC++;
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			*/
			return;
		}
		//CASO DE TERNER UNA INSTRUCCION RESIZE
		if(strcmp(instruccion_split[0], "RESIZE") == 0 )
		{
			proceso->path_length = atoi(instruccion_split[1]);
			enviarPCB(proceso,fd_memoria,RESIZE);

			//esperamos a que sea reactivado
			printf("Voy a esperar a que memoria me avise\n");
			sem_wait(&sem_memoria_aviso_cpu);
			printf("Puedo continuar wiii\n");
			if( asignacion_or_out_of_memory == 1 ) //no puede seguir
			{
				enviarPCB(proceso,fd_kernel_dispatch,PROCESOFIN); //el codigo de operacion termina al proceso
				printf("ERROR, OUT OF MEMORY\n");
				return;
			}
			else
			{
				printf("Asignacion realizada");
			}
		}		
		//CASO DE TENER UNA INSTRUCCION IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
		if (strcmp(instruccion_split[0], "IO_STDIN_READ") == 0 ) 
		{
		 	//Primero bloqueamos al proceso para que llegue a la cola de bloqueados antes que nada y luego ejecutamos la rutina relacionada con la IO
		 	enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			char* instruccion_to_send;

		 	if( esRegistroUint8(instruccion_split[2])) //REGISTRO DIRECCION UNIT8
		 	{
		 		//GUARDAMOS LA DIRECCION FÍSICA
				int tamanio;
		 		uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[2], proceso);
		 		int direc_logica = (int)(*registro_uint8); // Conversión explícita a int *
		 		printf("El valor del registro encontrado es: %d\n",*registro_uint8);
		 		printf("al haberlo transformado en int quedó: %d\n",direc_logica); //no se realiza la conversion correctamente

		 		//TODAVIA NO SE IMPLEMENTÓ LA MMU A ESTA INSTRUCCIÓN
		 		//IO_STDIN_READ Int1 BX CX
		 		//IO_STDIN_READ Int1 BX CX 10 1 2 3 4 5 6 7 8 9 10
		 		//GUARDAMOS EL TAMAÑO
		 		if( esRegistroUint8(instruccion_split[3]) )
		 		{
		 			uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[3], proceso);
		 			tamanio = (int)(*registro_uint8); // Conversión explícita a int *
		 			printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint8 );
		 			printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 
		 			char tamanioToSend[20];
		 			sprintf(tamanioToSend, "%d", tamanio);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, tamanioToSend);   //CONCATENAR
					*/
		 		}
		 		else
		 		{
		 			uint32_t *registro_uint32_2 = (uint32_t*)obtener_registro(instruccion_split[3], proceso);
		 			tamanio = (int)(*registro_uint32_2); // Conversión explícita a int *
		 			printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint32_2);
		 			printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 
		 			char tamanioToSend[20];
		 			sprintf(tamanioToSend, "%d", tamanio);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, tamanioToSend);  //CONCATENAR
					*/
		 		}

		 		direcciones_fisicas = mmu (direc_logica, proceso, tamanio); //me devuelve una lista

				instruccion_to_send = malloc(strlen(instruccion) + 1 + 4*list_size(direcciones_fisicas));
				strncpy(instruccion_to_send,instruccion,strlen(instruccion)+1);

				if(list_size(direcciones_fisicas) > 0)
				{
					char tamanioToSend[20];
					sprintf(tamanioToSend, "%d", tamanio);//copia en valor del tercer parametro en el primero
						
					strcat(instruccion_to_send," ");//CONCATENAR
					strcat(instruccion_to_send, tamanioToSend);
					
					for(int i=0; i < list_size(direcciones_fisicas);i++)
					{
						int direc_fisica = list_get(direcciones_fisicas,i);
						printf("La direccion fisica %d es: %d\n",i, direc_fisica);
						//contenamos la direccion fisica
						char direccionFisica[20];
						sprintf(direccionFisica, "%d", direc_fisica);
							
						strcat(instruccion_to_send," ");//CONCATENAR
						strcat(instruccion_to_send, direccionFisica);//CONCATENAR
					}
				}else{
					printf("NO HAY ELEMENTOS EN LA LISTA\n");
				}
		 	}	
		 	else
		 	{
		 		if( esRegistroUint32(instruccion_split[2])) //REGISTRO DIRECCION UNIT32
		 		{
		 			//GUARDAMOS LA DIRECCION FÍSICA
					int tamanio;
		 			uint32_t *registro_uint32 = (uint32_t*)obtener_registro(instruccion_split[2], proceso);
		 			printf("El valor del registro encontrado es: %d\n",*registro_uint32);
		 			int direc_logica = (int)(*registro_uint32); // Conversión explícita a int *
		 			printf("al haberlo transformado en int quedó: %d\n",direc_logica);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 

		 			//contenamos la direccion fisica
		 			char direccionFisica[20];
					sprintf(direccionFisica, "%d", direc_logica);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, direccionFisica);//CONCATENAR
					*/

		 			//GUARDAMOS EL TAMAÑO
		 			if( esRegistroUint8(instruccion_split[3]) )
		 			{
		 				uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[3], proceso);	//no entinedo porque le pasamos el tamaño de registro a mmu
						tamanio = (int)(*registro_uint8); // Conversión explícita a int *
		 				printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint8 );
		 				printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 				/*int direccion_fisica = mmu (direc_logica, proceso); 
		 				char tamanioToSend[20];
		 				sprintf(tamanioToSend, "%d", tamanio);
		 				strcat(instruccion," ");//CONCATENAR
		 				strcat(instruccion, tamanioToSend);//CONCATENAR
						*/
		 			}
		 			else
		 			{
		 				uint32_t *registro_uint32_2 = (uint32_t*)obtener_registro(instruccion_split[3], proceso);	//no entinedo porque le pasamos el tamaño de registro a mmu
		 				tamanio = (int)(*registro_uint32_2); // Conversión explícita a int *
		 				printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint32_2);
		 				printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 				/*int direccion_fisica = mmu (direc_logica, proceso); 
		 				char tamanioToSend[20];
		 				sprintf(tamanioToSend, "%d", tamanio);
		 				strcat(instruccion," ");//CONCATENAR
		 				strcat(instruccion, tamanioToSend);//CONCATENAR
						*/
		 			}
					
		 			direcciones_fisicas = mmu (direc_logica, proceso, tamanio); //me devuelve una lista
					
		 			instruccion_to_send = malloc(strlen(instruccion) + 1 + 4*list_size(direcciones_fisicas));
					strncpy(instruccion_to_send,instruccion,strlen(instruccion)+1);

					if(list_size(direcciones_fisicas) > 0)
					{
						char tamanioToSend[20];
						sprintf(tamanioToSend, "%d", tamanio);//copia en valor del tercer parametro en el primero
							
						strcat(instruccion_to_send," ");//CONCATENAR
						strcat(instruccion_to_send, tamanioToSend);
						
						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							int direc_fisica = list_get(direcciones_fisicas,i);
							printf("La direccion fisica %d es: %d\n",i, direc_fisica);
							//contenamos la direccion fisica
							char direccionFisica[20];
							sprintf(direccionFisica, "%d", direc_fisica);
								
							strcat(instruccion_to_send," ");//CONCATENAR
							strcat(instruccion_to_send, direccionFisica);//CONCATENAR
						}
					}else{
						printf("NO HAY ELEMENTOS EN LA LISTA\n");
					}							
		 		}
		 		else
		 		{
		 			printf("El registro no se encontró en el proceso.\n");
		 		}	
		 	}

		 	printf("La instruccion que se envia a KERNEL es: %s\n", instruccion);

		 	//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion_to_send)+1;
		 	enviar_instruccion_kernel(instruccion_to_send, instruccion_length, *proceso, STDIN);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
			free(instruccion_to_send);
		 	return;
		}

		//CASO DE TENER UNA INSTRUCCION IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
		if (strcmp(instruccion_split[0], "IO_STDOUT_WRITE") == 0 ) 
		{
		 	//Primero bloqueamos al proceso para que llegue a la cola de bloqueados antes que nada y luego ejecutamos la rutina relacionada con la IO
		 	enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			char* instruccion_to_send;

		 	if( esRegistroUint8(instruccion_split[2])) //REGISTRO DIRECCION UNIT8
		 	{
		 		//GUARDAMOS LA DIRECCION FÍSICA
				int tamanio;
		 		uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[2], proceso);
		 		int direc_logica = (int)(*registro_uint8); // Conversión explícita a int *
		 		printf("El valor del registro encontrado es: %d\n",*registro_uint8);
		 		printf("al haberlo transformado en int quedó: %d\n",direc_logica); //no se realiza la conversion correctamente

		 		//TODAVIA NO SE IMPLEMENTÓ LA MMU A ESTA INSTRUCCIÓN
		 		//IO_STDIN_READ Int1 BX CX
		 		//IO_STDIN_READ Int1 BX CX 10 1 2 3 4 5 6 7 8 9 10
		 		//GUARDAMOS EL TAMAÑO
		 		if( esRegistroUint8(instruccion_split[3]) )
		 		{
		 			uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[3], proceso);
		 			tamanio = (int)(*registro_uint8); // Conversión explícita a int *
		 			printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint8 );
		 			printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 
		 			char tamanioToSend[20];
		 			sprintf(tamanioToSend, "%d", tamanio);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, tamanioToSend);   //CONCATENAR
					*/
		 		}
		 		else
		 		{
		 			uint32_t *registro_uint32_2 = (uint32_t*)obtener_registro(instruccion_split[3], proceso);
		 			tamanio = (int)(*registro_uint32_2); // Conversión explícita a int *
		 			printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint32_2);
		 			printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 
		 			char tamanioToSend[20];
		 			sprintf(tamanioToSend, "%d", tamanio);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, tamanioToSend);  //CONCATENAR
					*/
		 		}

		 		direcciones_fisicas = mmu (direc_logica, proceso, tamanio); //me devuelve una lista

				instruccion_to_send = malloc(strlen(instruccion) + 1 + 4*list_size(direcciones_fisicas));
				strncpy(instruccion_to_send,instruccion,strlen(instruccion)+1);

				if(list_size(direcciones_fisicas) > 0)
				{
					char tamanioToSend[20];
					sprintf(tamanioToSend, "%d", tamanio);//copia en valor del tercer parametro en el primero
						
					strcat(instruccion_to_send," ");//CONCATENAR
					strcat(instruccion_to_send, tamanioToSend);
					
					for(int i=0; i < list_size(direcciones_fisicas);i++)
					{
						int direc_fisica = list_get(direcciones_fisicas,i);
						printf("La direccion fisica %d es: %d\n",i, direc_fisica);
						//contenamos la direccion fisica
						char direccionFisica[20];
						sprintf(direccionFisica, "%d", direc_fisica);
							
						strcat(instruccion_to_send," ");//CONCATENAR
						strcat(instruccion_to_send, direccionFisica);//CONCATENAR
					}
				}else{
					printf("NO HAY ELEMENTOS EN LA LISTA\n");
				}
		 	}	
		 	else
		 	{
		 		if( esRegistroUint32(instruccion_split[2])) //REGISTRO DIRECCION UNIT32
		 		{
		 			//GUARDAMOS LA DIRECCION FÍSICA
					int tamanio;
		 			uint32_t *registro_uint32 = (uint32_t*)obtener_registro(instruccion_split[2], proceso);
		 			printf("El valor del registro encontrado es: %d\n",*registro_uint32);
		 			int direc_logica = (int)(*registro_uint32); // Conversión explícita a int *
		 			printf("al haberlo transformado en int quedó: %d\n",direc_logica);

		 			/*int direccion_fisica = mmu (direc_logica, proceso); 

		 			//contenamos la direccion fisica
		 			char direccionFisica[20];
					sprintf(direccionFisica, "%d", direc_logica);
		 			strcat(instruccion," ");//CONCATENAR
		 			strcat(instruccion, direccionFisica);//CONCATENAR
					*/

		 			//GUARDAMOS EL TAMAÑO
		 			if( esRegistroUint8(instruccion_split[3]) )
		 			{
		 				uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[3], proceso);	//no entinedo porque le pasamos el tamaño de registro a mmu
						tamanio = (int)(*registro_uint8); // Conversión explícita a int *
		 				printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint8 );
		 				printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 				/*int direccion_fisica = mmu (direc_logica, proceso); 
		 				char tamanioToSend[20];
		 				sprintf(tamanioToSend, "%d", tamanio);
		 				strcat(instruccion," ");//CONCATENAR
		 				strcat(instruccion, tamanioToSend);//CONCATENAR
						*/
		 			}
		 			else
		 			{
		 				uint32_t *registro_uint32_2 = (uint32_t*)obtener_registro(instruccion_split[3], proceso);	//no entinedo porque le pasamos el tamaño de registro a mmu
		 				tamanio = (int)(*registro_uint32_2); // Conversión explícita a int *
		 				printf("El valor del registro encontrado TAMANIO: %d\n",*registro_uint32_2);
		 				printf("al haberlo transformado en int quedó: %d\n",tamanio);

		 				/*int direccion_fisica = mmu (direc_logica, proceso); 
		 				char tamanioToSend[20];
		 				sprintf(tamanioToSend, "%d", tamanio);
		 				strcat(instruccion," ");//CONCATENAR
		 				strcat(instruccion, tamanioToSend);//CONCATENAR
						*/
		 			}
					
		 			direcciones_fisicas = mmu (direc_logica, proceso, tamanio); //me devuelve una lista
					
		 			instruccion_to_send = malloc(strlen(instruccion) + 1 + 4*list_size(direcciones_fisicas));
					strncpy(instruccion_to_send,instruccion,strlen(instruccion)+1);

					if(list_size(direcciones_fisicas) > 0)
					{
						char tamanioToSend[20];
						sprintf(tamanioToSend, "%d", tamanio);//copia en valor del tercer parametro en el primero
							
						strcat(instruccion_to_send," ");//CONCATENAR
						strcat(instruccion_to_send, tamanioToSend);
						
						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							int direc_fisica = list_get(direcciones_fisicas,i);
							printf("La direccion fisica %d es: %d\n",i, direc_fisica);
							//contenamos la direccion fisica
							char direccionFisica[20];
							sprintf(direccionFisica, "%d", direc_fisica);
								
							strcat(instruccion_to_send," ");//CONCATENAR
							strcat(instruccion_to_send, direccionFisica);//CONCATENAR
						}
					}else{
						printf("NO HAY ELEMENTOS EN LA LISTA\n");
					}
					
		 		}
		 		else
		 		{
		 			printf("El registro no se encontró en el proceso.\n");
		 		}	
		 	}

		 	printf("La instruccion que se envia a KERNEL es: %s\n", instruccion);

		 	//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion_to_send)+1;
		 	enviar_instruccion_kernel(instruccion_to_send, instruccion_length, *proceso, STDOUT);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
			free(instruccion_to_send);
		 	return;
		}

		//CASO DE TENER UNA INSTRUCCION MOV_IN (Registro Datos, Registro Dirección)
		if (strcmp(instruccion_split[0], "MOV_IN") == 0)
		{ 
		 	if( esRegistroUint8(instruccion_split[2])) //REGISTRO DIRECCION UNIT8
		 	{
		 		uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[2], proceso); //direccion
		 		int direc_logica = (int)*registro_uint8;
				
		 			//REGISTRO DATOS
		 			if ( esRegistroUint8(instruccion_split[1])){
						int direccion_fisica;
		 				uint8_t* registro_datos = (uint8_t*)obtener_registro(instruccion_split[1], proceso); //registor donde guardaremos
		 				direcciones_fisicas = mmu (direc_logica, proceso, sizeof(uint8_t));
		 				
						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							direccion_fisica = list_get(direcciones_fisicas,i);
						}

						pedido_lectura_numerico(direccion_fisica, sizeof(uint8_t));//devuelve el valor de lo que esta en esa posicion de memoria
		 				sem_wait(&sem_lectura);
					
		 				*registro_datos = *valor_leido; //asigna ese valor al registro
		 			}
		 			else 
		 			{	//REGISTRO DATOS
		 				if ( esRegistroUint32(instruccion_split[1])){
							
		 					uint32_t* registro_datos = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
		 					direcciones_fisicas = mmu(direc_logica, proceso, sizeof(uint32_t)); //fc a implementar (MMU)

							uint8_t valores_leidos[4];
							for(int i=0; i < list_size(direcciones_fisicas);i++)
							{
								int direccion_fisica = list_get(direcciones_fisicas,i);
								pedido_lectura_numerico (direccion_fisica, sizeof(uint8_t));//devuelve el valor de lo que esta en esa posicion de memoria
								
								sem_wait(&sem_lectura);

								valores_leidos[i] =  *valor_leido;
								printf("El valor leído de memoria de tipo uint8_t es: %u\n",valores_leidos[i]);
							}
		 					
							/*
							uint8_t a = ...; // Primer byte
							uint8_t b = ...; // Segundo byte
							uint8_t c = ...; // Tercer byte
							uint8_t d = ...; // Cuarto byte
							*/
						
							uint32_t valor_uint32 = ((uint32_t)valores_leidos[3] << 24) | ((uint32_t)valores_leidos[2] << 16) | ((uint32_t)valores_leidos[1] << 8) | valores_leidos[0];
							*registro_datos = valor_uint32;
							printf("El valor registro de datos ha quedado de la siguinte forma: %u\n",*registro_datos);
		 				}
		 			}
				 
		 	}
		 	else
		 	{
		 		if( esRegistroUint32(instruccion_split[2])) //REGISTRO DIRECCION UNIT32
		 		{
		 			uint32_t *registro_uint32 = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
		 			int direc_logica = (int)*registro_uint32;

		 				//REGISTRO DATOS
		 			if ( esRegistroUint8(instruccion_split[1])){
						int direccion_fisica;
		 				uint8_t* registro_datos = (uint8_t*)obtener_registro(instruccion_split[1], proceso); //registor donde guardaremos
		 				direcciones_fisicas = mmu (direc_logica, proceso, sizeof(uint8_t));
		 				
						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							direccion_fisica = list_get(direcciones_fisicas,i);
						}

						pedido_lectura_numerico(direccion_fisica, sizeof(uint8_t));//devuelve el valor de lo que esta en esa posicion de memoria
		 				sem_wait(&sem_lectura);
					
		 				*registro_datos = *valor_leido; //asigna ese valor al registro
		 			}
		 			else 
		 			{	//REGISTRO DATOS
		 				if ( esRegistroUint32(instruccion_split[1])){
							
		 					uint32_t* registro_datos = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
		 					direcciones_fisicas = mmu(direc_logica, proceso, sizeof(uint32_t)); //fc a implementar (MMU)

							uint8_t valores_leidos[4];
							for(int i=0; i < list_size(direcciones_fisicas);i++)
							{
								int direccion_fisica = list_get(direcciones_fisicas,i);
								pedido_lectura_numerico (direccion_fisica, sizeof(uint8_t));//devuelve el valor de lo que esta en esa posicion de memoria
								
								sem_wait(&sem_lectura);

								valores_leidos[i] =  *valor_leido;
								printf("El valor leído de memoria de tipo uint8_t es: %u\n",valores_leidos[i]);
							}
		 					
							/*
							uint8_t a = ...; // Primer byte
							uint8_t b = ...; // Segundo byte
							uint8_t c = ...; // Tercer byte
							uint8_t d = ...; // Cuarto byte
							*/
						
							uint32_t valor_uint32 = ((uint32_t)valores_leidos[3] << 24) | ((uint32_t)valores_leidos[2] << 16) | ((uint32_t)valores_leidos[1] << 8) | valores_leidos[0];
							*registro_datos = valor_uint32;
							printf("El valor registro de datos ha quedado de la siguinte forma: %u\n",*registro_datos);
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
		 		uint8_t *registro_uint8 = (uint8_t*)obtener_registro(instruccion_split[1],proceso); //REGISTRO DONDE SE GUARDARA
		 		int direc_logica = (int)*registro_uint8;

				//REGISTRO DATOS
				if ( esRegistroUint8(instruccion_split[2])){
					uint8_t* registro_datos = (uint8_t*)obtener_registro(instruccion_split[2],proceso); //REGISTRO QUE ESCRIBIREMOS EN LA MEMORIA
					direcciones_fisicas = mmu (direc_logica, proceso, sizeof(uint8_t)); 
					
					int direccion_fisica;
					for(int i=0; i < list_size(direcciones_fisicas);i++)
					{
						direccion_fisica = list_get(direcciones_fisicas,i);
					}

					printf("Enviaremos lo siguiente: %d\n",direccion_fisica);//para hacer pruebas
					printf("Enviaremos lo siguiente: %d\n",*registro_datos);//para hacer pruebas

					pedido_escritura_numerico (direccion_fisica, *registro_datos);//para pasarle a memoria la direccion fisica y lo que tiene que escribir en esa direccion
					sem_wait(&sem_escritura);

				}
				else 
				{	//REGISTRO DATOS
					if ( esRegistroUint32(instruccion_split[2])){
						uint32_t* registro_datos = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
						direcciones_fisicas = mmu (direc_logica, proceso,sizeof(uint32_t)); //fc a implementar (MMU)

						
						//uint32_t* v = ...; // Tu puntero uint32_t
						uint8_t* ptr_registro_datos = (uint8_t *)registro_datos;
						uint8_t byte[4];
						byte[0] = ptr_registro_datos[0]; // Primer byte
						byte[1] = ptr_registro_datos[1]; // Segundo byte
						byte[2] = ptr_registro_datos[2]; // Tercer byte
						byte[3] = ptr_registro_datos[3]; // Cuarto byte

						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							int direccion_fisica = list_get(direcciones_fisicas,i);
							printf("La dirección física donde escribiremos dicho byte es: %d\n",direccion_fisica); 
							pedido_escritura_numerico (direccion_fisica, byte[i]);//devuelve el valor de lo que esta en esa posicion de memoria		
							sem_wait(&sem_escritura);
						}

					}
				}
		 	}
		 	else
		 	{
		 		if( esRegistroUint32(instruccion_split[1])) //REGISTRO DIRECCION UNIT32
		 		{
		 			uint32_t *registro_uint32 = (uint32_t*)obtener_registro(instruccion_split[1],proceso);
		 			int direc_logica = (int)*registro_uint32;

		 			//REGISTRO DATOS UNIT8
		 			if ( esRegistroUint8(instruccion_split[2])){
		 				uint8_t* registro_datos = (uint8_t*)obtener_registro(instruccion_split[2],proceso); //REGISTRO QUE ESCRIBIREMOS EN LA MEMORIA
						direcciones_fisicas = mmu (direc_logica, proceso, sizeof(uint8_t)); 
							
						int direccion_fisica;
						for(int i=0; i < list_size(direcciones_fisicas);i++)
						{
							direccion_fisica = list_get(direcciones_fisicas,i);
						}

						printf("Enviaremos lo siguiente: %d\n",direccion_fisica);//para hacer pruebas
						printf("Enviaremos lo siguiente: %d\n",*registro_datos);//para hacer pruebas

						pedido_escritura_numerico (direccion_fisica, *registro_datos);//para pasarle a memoria la direccion fisica y lo que tiene que escribir en esa direccion
						sem_wait(&sem_escritura);

		 			}
		 			else 
		 			{	//REGISTRO DATO UINT32
		 				if ( esRegistroUint32(instruccion_split[2])){
							uint32_t* registro_datos = (uint32_t*)obtener_registro(instruccion_split[2],proceso);
							direcciones_fisicas = mmu (direc_logica, proceso,sizeof(uint32_t)); //fc a implementar (MMU)

							
							//uint32_t* v = ...; // Tu puntero uint32_t
							uint8_t* ptr_registro_datos = (uint8_t *)registro_datos;
							uint8_t byte[4];
							byte[0] = ptr_registro_datos[0]; // Primer byte
							byte[1] = ptr_registro_datos[1]; // Segundo byte
							byte[2] = ptr_registro_datos[2]; // Tercer byte
							byte[3] = ptr_registro_datos[3]; // Cuarto byte

							for(int i=0; i < list_size(direcciones_fisicas);i++)
							{
								int direccion_fisica = list_get(direcciones_fisicas,i);
								printf("La dirección física donde escribiremos dicho byte es: %d\n",direccion_fisica); 
								pedido_escritura_numerico (direccion_fisica, byte[i]);//devuelve el valor de lo que esta en esa posicion de memoria		
								sem_wait(&sem_escritura);
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


		//rehacemos en esta parte copy string
		 if ( strcmp(instruccion_split[0], "COPY_STRING") == 0)
		{
			//debemos obtener la cantidad de bytes
			int bytes_a_copiar = atoi(instruccion_split[1]);
			direcciones_fisicas = mmu(proceso->registro.SI, proceso, bytes_a_copiar);
			t_list* direcciones_fisicas_destino = mmu(proceso->registro.DI, proceso, bytes_a_copiar);
			
			//vamos enviando ambas direcciones y memoria las irá copiando
			for(int i = 0; i < bytes_a_copiar; i++)
			{
				//enviamos a memoria dos enteros
				int d1 = list_get(direcciones_fisicas,i);
				printf("El destino a enviar es: %d\n",d1);
				int d2 = list_get(direcciones_fisicas_destino,i);
				printf("El destino a enviar es: %d\n",d2);
				enviarDirecciones(d1, d2);
				//esperamos a recibir la señal para continuar
				sem_wait(&sem_escritura);
			}
			
			printf("Copia finalizada\n");
		} 


		//CASO DE TENER UNA INSTRUCCION IO_FS_CREATE (Interfaz, Nombre Archivo): 
		 //Esta instrucción solicita al Kernel que, mediante la interfaz seleccionada, se cree un archivo en el FS montado en dicha interfaz.
		 if (strcmp(instruccion_split[0], "IO_FS_CREATE") == 0)
		 {
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length, *proceso, DIALFS);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);

		 	return;
		 }

		//CASO DE TENER UNA INSTRUCCION IO_FS_DELETE (Interfaz, Nombre Archivo)
		if (strcmp(instruccion_split[0], "IO_FS_DELETE") == 0){
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length, *proceso, DIALFS);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);

		 	return;
		}

		//Tendremos que obtener el registro
		//CASO DE TENER UNA INSTRUCCION IO_FS_TRUNCATE(Interfaz, Nombre Archivo, Registro Tamaño)
		if (strcmp(instruccion_split[0], "IO_FS_TRUNCATE") == 0){
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length, *proceso, DIALFS);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
		 	return;
		}	

		//CASO DE TENER UNA INSTRUCCION IO_FS_WRITE(Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
		if (strcmp(instruccion_split[0], "IO_FS_WRITE") == 0){
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);

			//DEBEMOS OBTENER EL VALOR DEL REGISTRO
			uint32_t* d_logica = malloc(sizeof(uint32_t));
			if( esRegistroUint32(instruccion_split[3]) )
			{
				*d_logica = *(uint32_t*) obtener_registro(instruccion_split[3],proceso);
			}
			else
			{
				uint8_t* dt = (uint8_t*) obtener_registro(instruccion_split[3],proceso);
				printf("el valor de %d\n",*dt);
				*d_logica = *dt;
			}
			printf("El valor que nos ha quedado es: %d\n",*d_logica);

			uint32_t* tamanio = malloc(sizeof(uint32_t));
			if( esRegistroUint32(instruccion_split[4]) )
			{
				*tamanio = *(uint32_t*) obtener_registro(instruccion_split[4],proceso);
			}
			else
			{
				uint8_t* dt = (uint8_t*) obtener_registro(instruccion_split[4],proceso);
				printf("el valor de %d\n",*dt);
				*tamanio = *dt;
			}
			printf("El valor que nos ha quedado es: %d\n",*tamanio);

			uint32_t* pointer_archivo = malloc(sizeof(uint32_t));
			if( esRegistroUint32(instruccion_split[5]) )
			{
				*pointer_archivo = *(uint32_t*) obtener_registro(instruccion_split[5],proceso);
			}
			else
			{
				uint8_t* dt = (uint8_t*) obtener_registro(instruccion_split[5],proceso);
				printf("el valor de %d\n",*dt);
				*pointer_archivo = *dt;
			}
			printf("El valor que nos ha quedado es: %d\n",*pointer_archivo);

			direcciones_fisicas = mmu(*d_logica, proceso, *tamanio);
			char* instruccion_to_send = malloc( strlen(instruccion) + 1 + 4*list_size(direcciones_fisicas));
			strncpy(instruccion_to_send,instruccion, strlen(instruccion)+1);
			if(list_size(direcciones_fisicas) > 0)
			{
				char pointer_to_send[20];
				sprintf(pointer_to_send, "%u", *pointer_archivo);//copia en valor del tercer parametro en el primero
				strcat(instruccion_to_send," ");//CONCATENAR
				strcat(instruccion_to_send, pointer_to_send);

				char tamanioToSend[20];
				sprintf(tamanioToSend, "%u", *tamanio);//copia en valor del tercer parametro en el primero
					
				strcat(instruccion_to_send," ");//CONCATENAR
				strcat(instruccion_to_send, tamanioToSend);
				
				for(int i=0; i < list_size(direcciones_fisicas);i++)
				{
					int direc_fisica = list_get(direcciones_fisicas,i);
					printf("La direccion fisica %d es: %d\n",i, direc_fisica);
					//contenamos la direccion fisica
					char direccionFisica[20];
					sprintf(direccionFisica, "%d", direc_fisica);
						
					strcat(instruccion_to_send," ");//CONCATENAR
					strcat(instruccion_to_send, direccionFisica);//CONCATENAR
				}
			}
			else
			{
				printf("NO HAY ELEMENTOS EN LA LISTA\n");
			}



			//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion_to_send)+1;
		 	enviar_instruccion_kernel(instruccion_to_send, instruccion_length, *proceso, DIALFS);
			
		 	//se bloquea el proceso, devolvemos al kernel
			free(d_logica);
			free(tamanio);
			free(pointer_archivo);
			
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
			free(instruccion_to_send);
		 	return;
		}

		//CASO DE TENER UNA INSTRUCCION IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
		if (strcmp(instruccion_split[0], "IO_FS_READ") == 0){
			enviarPCB(proceso,fd_kernel_dispatch,PROCESOIO);
			//debemos devolver instruccion + pcb parando el proceso actual
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length, *proceso, DIALFS);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";

		 	return;
		}



		//CASO DE TENER UNA INSTRUCCION WAIT (Recurso): 
		 if(strcmp(instruccion_split[0], "WAIT") == 0)
		 {
		 	//debemos devolver el proceso al kernel
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length,*proceso,WAIT);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
		 	return;
		 }
		//CASO DE TENER UNA INSTRUCCION SIGNAL (Recurso): 
		 if(strcmp(instruccion_split[0], "SIGNAL") == 0)
		 {
		 	//debemos devolver el proceso al kernel
		 	uint32_t instruccion_length = strlen(instruccion)+1;
		 	enviar_instruccion_kernel(instruccion, instruccion_length,*proceso,SIGNAL);
			
		 	//se bloquea el proceso, devolvemos al kernel
		 	free(instruccionActual);
		 	instruccionActual = malloc(1);
		 	instruccionActual = "";
			free(instruccion);
		 	return;
		 }
		
		//preguntamos por el valor de la variable interrupcion
		sem_wait(&interrupt_mutex);
		if( *any_interrupcion == (-1) )//fin de quantum
		{
			printf("AL PROCESO SE LE HA ACABADO EL QUANTUM\n");
			enviarPCB(proceso,fd_kernel_dispatch,FIN_DE_QUANTUM);
			*any_interrupcion = 0;
			sem_post(&interrupt_mutex);
			return;
		}
		else
		{
			//si la variable tiene el pid del proceso, se debe finalizar.
			if( *any_interrupcion == proceso->PID)
			{
				enviarPCB(proceso,fd_kernel_dispatch,PROCESOFIN);
				*any_interrupcion = 0;
				sem_post(&interrupt_mutex);
				return;
			}
		}
		sem_post(&interrupt_mutex);

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
				printf("/////////////\n");
				printf("Recibi el siguiente proceso:\n");
				printf("Su PID es: %d\n",proceso->PID);
				printf("El quamtum actual es de: %d\n",proceso->quantum);
				printf("Su AX es: %d\n",proceso->registro.AX);//cambios
				printf("Su BX es: %d\n",proceso->registro.BX);
				printf("Su CX es: %d\n",proceso->registro.CX);//cambios
				printf("Su DX es: %d\n",proceso->registro.DX);
				printf("Su DI es: %d\n",proceso->registro.DI);//cambios
				printf("Su EAX es: %d\n",proceso->registro.EAX);
				printf("/////////////\n");
				ejecutar_proceso(proceso);

				free(proceso->path);
				free(proceso);
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
			//printf("RECIBI ALGO\n");

		    switch (cod_op) {
			case FIN_DE_QUANTUM:

				//printf("RECIBI FIN DE QUANTUM\n");
				//si es fin de quantum es un 1
				sem_wait(&interrupt_mutex);
				*any_interrupcion = (-1);
				sem_post(&interrupt_mutex);
				break;
			case FINALIZAR_PROCESO:
				//
				int* pid_a_finalizar = paquete->buffer->stream;
				sem_wait(&interrupt_mutex);
				*any_interrupcion = *pid_a_finalizar;
				sem_post(&interrupt_mutex);
				break;
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

			printf("RECIBI EL CÓDIGO DE OPERACIÓN: %d\n",cod_op);

            //deserializamos el mensaje, primero el tamaño del buffer, luego el offset
			recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);		
			paquete->buffer->stream = malloc(paquete->buffer->size);
			
			if( cod_op != 8 && cod_op != 20 && cod_op != 21 && cod_op != 28 && cod_op != 29) 		//los cod_op reciben datos de tipo ENTERO
			{
				recv(fd_memoria,&(paquete->buffer->offset), sizeof(uint32_t),0);
			}
			
			recv(fd_memoria,paquete->buffer->stream,paquete->buffer->size,0);

			switch (cod_op) {
			case MENSAJE:
			    sem_wait(&sem_exe_a);
				//variable global con 
				instruccionActual = malloc(paquete->buffer->size);
				char* instruccionQueLlego = paquete->buffer->stream;
				instruccionActual = string_duplicate(instruccionQueLlego);
				//printf("La instruccion que llego fue: %s\n",instruccionActual);
				//instruccionActual = paquete->buffer->stream;
				sem_post(&sem_exe_b);
				break;
			case ASIGNACION_CORRECTA:
				printf("Voy a avisarle a ejecutar proceso que puede seguir\n");
				asignacion_or_out_of_memory = 0;
				sem_post(&sem_memoria_aviso_cpu);
				break;
			case OUT_OF_MEMORY:
				printf("Voy a avisarle a ejecutar proceso NO puede seguir\n");
				asignacion_or_out_of_memory = 1;
				sem_post(&sem_memoria_aviso_cpu);
				break;
			case LECTURA:
				valor_leido = malloc(sizeof(uint8_t));
				uint8_t* leidoQueLlego = paquete->buffer->stream;
				memcpy(valor_leido, leidoQueLlego, sizeof(uint8_t));  
				printf("El valor leido que llego fue: %u\n",*valor_leido); //no imprime el valor leido
				sem_post(&sem_lectura);
				break;
			case ESCRITO:
				printf("Teoricamente se ha escrito el valor en la direccion fisica\n");
				sem_post(&sem_escritura);
				//
				break;
			case COPY_STRING:
				printf("Teoricamente el valor se ha copiado correctamente\n");
				sem_post(&sem_escritura);
				//
				break;
			case MARCO:

				//llega el marco de memoria
				printf("Me encuentro en la parte de marco\n");
				num_marco = malloc(sizeof(int));
				int* marcoLeido = paquete->buffer->stream;
				memcpy(num_marco, marcoLeido, sizeof(int));
				printf("El marco es: %d\n", *num_marco);
				sem_post(&sem_mmu);
				break;
			case PAQUETE:
				//pop aca se enviará el num_pagina
				/*
			
				*/
				break;
			case TAMPAGINA:
				
				TAM_PAGINA = malloc(sizeof(int));

				int* tamaño = paquete->buffer->stream;
				memcpy(TAM_PAGINA, tamaño, sizeof(int));

				printf("El tamaño de la pagina de memoria es: %d\n", *TAM_PAGINA);
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
		printf("Llegué hasta antes el final de escuchar memoria\n");
}


#endif /* CPU_H_ */