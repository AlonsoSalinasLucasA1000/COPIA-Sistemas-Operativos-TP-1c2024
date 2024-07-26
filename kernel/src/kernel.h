#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdio.h>
#include <stdlib.h>
#include <utils/utils.h>
#include <utils/utils.c>

//file descriptors de kernel y los modulos que se conectaran con ella
int fd_kernel;
int fd_cpu_interrupt; 
int fd_cpu_dispatch; //luego se dividira en dos fd, un dispach y un interrupt, por ahora nos es suficiente con este
int fd_memoria;
int fd_entradasalida;
bool cpu_ocupada; //0 si no, 1 si sí
int grado_multiprogramacion;
bool iniciar_planificacion;
bool iniciar_planificacion = true;

//colas y pid
int procesos_en_new = 0;
int procesos_fin = 0;

sem_t sem; //semaforo mutex region critica cola new
sem_t sem_ready; 
sem_t sem_cant; //semaforo cant de elementos en la cola
sem_t sem_cant_ready;
sem_t sem_ready_prio;
sem_t sem_mutex_plani_corto; //semaforo oara planificacion FIFO
sem_t sem_mutex_cpu_ocupada; //semaforo para indicar si la cpu esta ocupada
sem_t sem_blocked;
sem_t sem_procesos;
sem_t sem_grado_mult;


//semaforos para garantizar exclusión mutua al acceso de las listas de ios
sem_t sem_mutex_lists_io; //un unico semaforo para el acceso a la listas de ios

//lista de recursos
t_list* listRecursos;

//TODAS LAS QUEUES
int pid = 0;
t_queue* cola_new;
t_queue* cola_ready;
t_queue* cola_blocked;
t_queue* cola_ready_prioridad;

//LISTA DE PROCESOS
t_list* lista_procesos;

//LISTA DE ENTRADAS Y SALIDAS
t_list* listGenericas;
t_list* listStdin;
t_list* listStdout;
t_list* listDialfs;


//CONTADOR PARA EL VRR
t_temporal* cronometro;

//su semaforo
sem_t sem_mutex_cronometro;

t_log *kernel_logger; // LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config *kernel_config;
t_log *kernel_logs_obligatorios;// LOGS MINIMOS Y OBLIGATORIOS

char *PUERTO_ESCUCHA;
char *IP_MEMORIA;
char *PUERTO_MEMORIA;
char *IP_CPU;
char *PUERTO_CPU_DISPATCH; 
char *PUERTO_CPU_INTERRUPT;
char *ALGORITMO_PLANIFICACION;
char *QUANTUM; //Da segmentation fault si lo defino como int
char* RECURSOS;
char* INSTANCIAS_RECURSOS;
char* GRADO_MULTIPROGRAMACION; //Da segmentation fault si lo defino como int

PCB* encontrarProceso(t_list* lista, uint32_t pid)
{
	printf("Holisasdjadsj\n");
	PCB* ret;
	int i = 0;
	while( i < list_size(lista) )
	{
		PCB* got = list_get(lista,i);
		if( got->PID == pid)
		{
			ret = got;
		}
		i++;
	}
	return ret;
}


char* estado_proceso_to_string(estado_proceso estado) {
    switch (estado) {
        case NEW: return "NEW";
        case READY: return "READY";
        case BLOCKED: return "BLOCKED";
        case EXEC: return "EXEC";
        case EXIT: return "EXIT";
        default: return "UNKNOWN";
    }
}


void removerCorchetes(char* str) {
    int len = strlen(str);
    int j = 0;

    for (int i = 0; i < len; i++) {
        if (str[i] != '[' && str[i] != ']') {
            str[j++] = str[i];
        }
    }
    str[j] = '\0'; // Añadir el carácter nulo al final de la cadena resultante
}

//Estas dos funciones a continuación son para la administración de recursos. Se busca eliminar los pid que tienen instancias del recurso en el momento.
bool es_elemento_buscado(void* elemento, void* valor_buscado) {
    return *((uint32_t*)elemento) == *((uint32_t*)valor_buscado);
}

// Función para eliminar un elemento de la lista
bool eliminar_elemento(t_list* lista, uint32_t valor) 
{
	bool to_ret;
    uint32_t* valor_buscado = malloc(sizeof(uint32_t));
    *valor_buscado = valor;

    // Iterar sobre la lista para encontrar el elemento
    for (int i = 0; i < list_size(lista); i++) {
        uint32_t* elemento = list_get(lista, i);
        if (es_elemento_buscado(elemento, valor_buscado)) {
            list_remove(lista, i);
			to_ret = true;
            return to_ret;
        }
    }
    free(valor_buscado);
	to_ret = false;
	return to_ret;
}

//Función para generar los recursos a partir del archivo de configuración
t_list* generarRecursos(char* recursos, char* instancias_recursos)
{
	//creamos la lista a retornar
	t_list* ret = list_create();
	//sacamos los corchetes
	removerCorchetes(recursos);
	removerCorchetes(instancias_recursos);
	//los separamos por las comas
	char** recursos_separados = string_split(recursos,",");   //
	char** instancias_recursos_separados = string_split(instancias_recursos,","); //
	//vamos creando los Recurso y añadimos a la lista
	int i = 0;
	while( recursos_separados[i] != NULL )
	{
		Recurso* to_add = malloc(sizeof(Recurso));
		strncpy(to_add->name, recursos_separados[i], sizeof(to_add->name) - 1);
		to_add->instancias = atoi(instancias_recursos_separados[i]);
		to_add->listBloqueados = list_create();
		to_add->pid_procesos = list_create();
		list_add(ret,to_add);
		i++;
	}
	return ret;
}

//Se ejecuta la interfaz genérica
void ejecutar_interfaz_generica(char* instruccion, op_code tipoDeInterfaz, int fd_io, int pid_actual)
{
	char** instruccion_split = string_split(instruccion," ");
	int* unidadesDeTrabajo = malloc(sizeof(int));
	*unidadesDeTrabajo = atoi(instruccion_split[2]);

	int* pid_io = malloc(sizeof(int));
	*pid_io = pid_actual;

	enviarEntero(pid_io,fd_io,NUEVOPID);
	enviarEntero(unidadesDeTrabajo,fd_io,GENERICA);
}

//Ejecuta tanto stdin como stdout
void ejecutar_interfaz_stdinstdout(char* instruccion, op_code tipoDeInterfaz, int fd_io, int pid_actual)
{
	char** instruccion_split = string_split(instruccion," ");
	//IO_STDIN_READ (Interfaz, Registro Direccion, Registro Tamaño)
	//Debemos enviar la direccion y el tamanio a escribir
	int* direccionFisica = malloc(sizeof(int));
	int* tamanio = malloc(sizeof(int));
	*direccionFisica = atoi(instruccion_split[4]);
	*tamanio = atoi(instruccion_split[5]);

	//antes que todo enviamos del pid del proceso que actualmente lo usa
	int* pid = malloc(sizeof(int));
	*pid = pid_actual;
	enviarEntero(pid,fd_io,NUEVOPID);

	//mandaremos
	printf("mandaremos la siguiente direccion fisica: %d\n",*direccionFisica);
	printf("mandaremos el siguiente tamanio: %d\n",*tamanio);

	printf("Voy a mandar algo para el stdin\n");
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int)*2;
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = tipoDeInterfaz;
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
    send(fd_io, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);
	printf("Lo mande\n");
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
	
}

void new_ejecutar_interfaz_stdin_stdout(char* instruccion, op_code tipoDeInterfaz, int fd_io, int pid_actual)
{
	//debemos enviar la instrucción entero + su tamaño
	char* to_send = string_duplicate(instruccion);
	int* tamanio = malloc(sizeof(int));
	*tamanio = strlen(to_send)+1;

	//antes que todo enviamos del pid del proceso que actualmente lo usa
	int* pid = malloc(sizeof(int));
	*pid = pid_actual;
	enviarEntero(pid,fd_io,NUEVOPID);
	
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
	buffer->size = sizeof(int) + *tamanio;
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,to_send, *tamanio);

	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = tipoDeInterfaz;
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
    send(fd_io, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);
	printf("Lo mande\n");
	
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

//Se buscan las IO dentro de la lista
EntradaSalida* encontrar_io(t_list* lista, const char* nombre_buscado) {
    for (int i = 0; i < list_size(lista); i++) 
	{
        EntradaSalida* entrada = list_get(lista, i);
        if (strcmp(entrada->nombre, nombre_buscado) == 0) {
            return entrada;
        }
    }
    return NULL; // Si no se encuentra el elemento
}

//Busca una PCB en base a su pid en una lista. La usamos para eliminar procesos de la cola de bloqueados.
PCB* encontrar_por_pid(t_list* lista, uint32_t pid_buscado) {
	printf("kakaroto\n");
    for (int i = 0; i < list_size(lista); i++) 
	{
        PCB* pcb = list_get(lista, i);
        if (pcb->PID == pid_buscado) 
		{
			pcb = list_remove(lista, i);
            return pcb;
        }
    }
    return NULL; // Si no se encuentra el elemento
}

void encolar_procesos_vrr(PCB* proceso)
{
	printf("[CHECKPOINT VRR] En este instante el quantum es %d\n",proceso->quantum);
	if( proceso->quantum > 0 )
	{
		printf("A la cola de máxima prioridad llegó el proceso de PID %d\n",proceso->PID);
		sem_wait(&sem_ready_prio);
		queue_push(cola_ready_prioridad,proceso);
		sem_post(&sem_ready_prio);
	}
	else
	{
		//reiniciamos el quantum
		proceso->quantum = atoi(QUANTUM);
		sem_wait(&sem_ready);   // mutex hace wait
		queue_push(cola_ready, proceso); // agrega el proceso a la cola de ready
		sem_post(&sem_ready);
		sem_post(&sem_cant_ready);  // mutex hace wait
	}
}

//Se liberan los recursos una vez un proceso ha sido eliminado
void liberar_recursos(int pid)
{
	for(int i = 0; i < list_size(listRecursos); i++)
	{
		Recurso* r = list_get(listRecursos,i);
		if( eliminar_elemento(r->listBloqueados,pid) )
		{
			r->instancias++;
		}
		bool borrado = true;
		while( borrado )
		{
			borrado = eliminar_elemento(r->pid_procesos,pid);
			if( borrado )
			{
				r->instancias++;
				if (r->instancias <= 0) 
				{
					//PCB* to_ready = malloc(sizeof(PCB));
					PCB* proceso_desbloqueado = list_remove(r->listBloqueados, 0);
					//*to_ready = *proceso_desbloqueado;

					printf("El proceso desbloqueado es el siguiente: \n");
					printf("PID: %d\n", proceso_desbloqueado->PID);
					printf("QUANTUM: %d\n",proceso_desbloqueado->quantum);
					//printf("PATH: %s\n",proceso_desbloqueado->path);
					//el proceso ya no queda bloquedo, lo sacamos de la cola general de bloqueados
					sem_wait(&sem_blocked);
					eliminar_elemento(cola_blocked->elements,proceso_desbloqueado->PID);
					sem_post(&sem_blocked);
					if (proceso_desbloqueado != NULL) 
					{
						//añadimos que dicho proceso está usando una instancia
						uint32_t* pid = malloc(sizeof(uint32_t));
						*pid = proceso_desbloqueado->PID;
						list_add(r->pid_procesos,pid);

						//añadimos a una cola dependiendo si le queda quantum o no
						// Enviamos el proceso a la cola de ready

						if( strcmp(ALGORITMO_PLANIFICACION,"VRR") == 0 )
						{
							encolar_procesos_vrr(proceso_desbloqueado);
						}
						else
						{
							sem_wait(&sem_ready);   // mutex hace wait
							printf("En este momento en la cola de ready se ha pusheado al pid: %d\n", proceso_desbloqueado->PID);
							queue_push(cola_ready, proceso_desbloqueado); // agrega el proceso a la cola de ready
							sem_post(&sem_ready);
							sem_post(&sem_cant_ready);  // mutex hace wait
						}
					}
				}
			}
		}
	}
}

void kernel_escuchar_cpu ()
{
	bool control_key = 1;
	while (control_key) {
	//recibimos operacion y guardamos todo en un paquete
			int cod_op = recibir_operacion(fd_cpu_dispatch);

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_cpu_dispatch,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_cpu_dispatch,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case PROCESOFIN:
			//recibo el proceso de la CPU
			    PCB* proceso = deserializar_proceso_cpu(paquete->buffer);
				char* estado_anterior = estado_proceso_to_string(proceso->estado);
				proceso->estado = EXIT;
				char* estado_actual = estado_proceso_to_string(proceso->estado);
				printf("Recibimos el proceso con el pid: %d\n",proceso->PID);
				printf("Recibimos el proceso con el quantum en: %d\n",proceso->quantum);
				printf("Recibimos el proceso con el AX: %d\n",proceso->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso->registro.DI);//cambios
				printf("/////////////-----[EL PROCESO DE PID %d ha FINALIZADO]-----////////////\n",proceso->PID);
				log_info (kernel_logs_obligatorios, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", proceso->PID, estado_anterior, estado_actual);

				//ACTUALIZAMOS EN LA LISTA GENERAL
				sem_wait(&sem_procesos);
				PCB* actualizado_fin = encontrarProceso(lista_procesos,proceso->PID);
				actualizado_fin->estado = EXIT;
				sem_post(&sem_procesos);

				liberar_recursos(proceso->PID);
				enviarPCB(proceso,fd_memoria,PROCESOFIN);

				free(proceso->path);
				free(proceso);
				
				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);

				printf("Los recursos han quedado de la siguiente forma:\n");
				for(int i = 0; i < list_size(listRecursos); i++)
				{
					Recurso* got = list_get(listRecursos,i);
					printf("El nombre del recurso es %s y tiene %d instancias\n",got->name,got->instancias);
				}				
			//	
                break;
			case PROCESOIO:

				PCB* proceso_io = deserializar_proceso_cpu(paquete->buffer);
				//char* estado_anterior_1 = estado_proceso_to_string(proceso->estado);
				proceso_io->estado = BLOCKED;
				printf("Recibimos el proceso con el pid: %d\n",proceso_io->PID);
				printf("Recibimos el proceso con el siguiente quantum: %d\n",proceso_io->quantum);
				printf("Recibimos el proceso con el AX: %d\n",proceso_io->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso_io->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso_io->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso_io->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso_io->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso_io->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso_io->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso_io->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso_io->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso_io->registro.DI);//cambios
				printf("Este SE HA BLOQUEADO\n");

				//ACTUALIZAMOS EN LA LISTA GENERAL
				sem_wait(&sem_procesos);
				PCB* actualizado_io = encontrarProceso(lista_procesos,proceso_io->PID);
				actualizado_io->estado = BLOCKED;
				sem_post(&sem_procesos);

				//PARA VIRTUAL ROUND ROBIN
				//En caso de darse esto, debemos actualizar los valores del quantum
				if( strcmp(ALGORITMO_PLANIFICACION,"VRR") == 0)
				{
					sem_wait(&sem_mutex_cronometro);
					int64_t quantum_transcurrido= temporal_gettime(cronometro);
					printf("La cantidad de tiempo que ha transcurrido: %d\n",quantum_transcurrido);
					sem_post(&sem_mutex_cronometro);
					proceso_io->quantum -= quantum_transcurrido;
					printf("[ACTUALIZACIÓN] Hemos actualizado el quantum y ha quedado de la siguiente forma: %d\n",proceso_io->quantum);
				}

				//lo ideal seria tambien agregarlo a una cola de la interfaz de cada proceso
				sem_wait(&sem_blocked);
				queue_push(cola_blocked, proceso_io);
				sem_post(&sem_blocked);


				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);		

				break;
			case FIN_DE_QUANTUM:

				PCB* proceso_fin_de_quantum = deserializar_proceso_cpu(paquete->buffer);
				proceso_fin_de_quantum->estado = READY;
				printf("Recibimos el proceso con el pid: %d\n",proceso_fin_de_quantum->PID);
				printf("Recibimos el proceso con el siguiente quantum: %d\n",proceso_fin_de_quantum->quantum);
				printf("Recibimos el proceso con el AX: %d\n",proceso_fin_de_quantum->registro.AX);//cambios
				printf("Recibimos el proceso con el BX: %d\n",proceso_fin_de_quantum->registro.BX);//cambios
				printf("Recibimos el proceso con el CX: %d\n",proceso_fin_de_quantum->registro.CX);//cambios
				printf("Recibimos el proceso con el DX: %d\n",proceso_fin_de_quantum->registro.DX);//cambios
				printf("Recibimos el proceso con el EAX: %d\n",proceso_fin_de_quantum->registro.EAX);//cambios
				printf("Recibimos el proceso con el EBX: %d\n",proceso_fin_de_quantum->registro.EBX);//cambios
				printf("Recibimos el proceso con el ECX: %d\n",proceso_fin_de_quantum->registro.ECX);//cambios
				printf("Recibimos el proceso con el EDX: %d\n",proceso_fin_de_quantum->registro.EDX);//cambios
				printf("Recibimos el proceso con el SI: %d\n",proceso_fin_de_quantum->registro.SI);//cambios
				printf("Recibimos el proceso con el DI: %d\n",proceso_fin_de_quantum->registro.DI);//cambios
				printf("SE HA ACABADO EL QUANTUM DE ESTE PROCESO\n");

				sem_wait(&sem_procesos);
				PCB* actualizado_quantum = encontrarProceso(lista_procesos,proceso_fin_de_quantum->PID);
				actualizado_quantum->estado = READY;
				sem_post(&sem_procesos);

				
				log_info (kernel_logs_obligatorios, "PID: <%d> - Desalojado por fin de Quantum", proceso_fin_de_quantum->PID);

				//el quantum vuelve al valor original
				proceso_fin_de_quantum->quantum = atoi(QUANTUM);
				printf("Hemos reiniciado el quantum: %d\n",proceso_fin_de_quantum->quantum);

				//lo ideal seria tambien agregarlo a una cola de la interfaz de cada proceso
				sem_wait(&sem_ready);   // mutex hace wait
				queue_push(cola_ready,proceso_fin_de_quantum);	//agrega el proceso a la cola de ready
  				sem_post(&sem_ready); 
				sem_post(&sem_cant_ready);  // mutex hace wait
				
				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);	
				break;
			case GENERICA:
				
				//Deserializamos
				Instruccion_io* instruccion_io_gen = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_io_gen->instruccion);
				printf("El PID del proceso es: %d\n",instruccion_io_gen->proceso.PID);

				//Debemos obtener el IO específico de la lista
				sem_wait(&sem_mutex_lists_io);
				EntradaSalida* io_gen = encontrar_io(listGenericas,string_split(instruccion_io_gen->instruccion," ")[1]);
				sem_post(&sem_mutex_lists_io);

				//verificamos que exista
				if( io_gen != NULL )
				{
					//una vez encontrada la io, veo si está ocupado
					printf("El io encontrado tiene como nombre %s\n",io_gen->nombre);
					if( io_gen->ocupado )
					{
						//de estar ocupado, añado el proceso a la lista de este (al proceso con la instruccion y todo)
						printf("La IO está ocupada, se bloqueará en su lista propia\n");
						printf("En la lista se guarda la instruccion %s\n",instruccion_io_gen->instruccion);
						printf("En la lista se guarda el PID %d\n",instruccion_io_gen->proceso.PID);
						list_add(io_gen->procesos_bloqueados, instruccion_io_gen);
					}
					else
					{
						//marcamos la io como ocupada y ejecutamos
						io_gen->ocupado = true;
						ejecutar_interfaz_generica(instruccion_io_gen->instruccion,GENERICA,io_gen->fd_cliente,instruccion_io_gen->proceso.PID);
					}
				}
				else
				{
					//de no existir debemos terminar el proceso	
					sem_wait(&sem_blocked);				
					PCB* proceso_to_end = encontrar_por_pid(cola_blocked->elements,instruccion_io_gen->proceso.PID);
					sem_post(&sem_blocked);	

					sem_wait(&sem_procesos);
					PCB* actualizado_gen = encontrarProceso(lista_procesos,instruccion_io_gen->proceso.PID);
					actualizado_gen->estado = EXIT;
					sem_post(&sem_procesos);

					
					printf("Este proceso ha terminado\n");
					liberar_recursos(proceso_to_end->PID);
					enviarPCB(proceso_to_end,fd_memoria,PROCESOFIN);
					free(proceso_to_end->path);
					free(proceso_to_end);
				}
					//CPU desocupada
					sem_wait(&sem_mutex_cpu_ocupada);
					cpu_ocupada = false;
					sem_post(&sem_mutex_cpu_ocupada);
				break;
			case STDIN:
				//Deserializamos
				Instruccion_io* instruccion_io_stdin = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_io_stdin->instruccion);
				printf("El PID del proceso es: %d\n",instruccion_io_stdin->proceso.PID);

				//debemos obtener el io específico de la lista
				sem_wait(&sem_mutex_lists_io);
				EntradaSalida* io_stdin = encontrar_io(listStdin,string_split(instruccion_io_stdin->instruccion," ")[1]);
				sem_post(&sem_mutex_lists_io);

				//verificamos que exista
				if( io_stdin != NULL)
				{
					printf("Entré acá first\n");
					//Una vez encontrada la io, vemos si está ocupado
					if( io_stdin->ocupado )
					{
						//de estar ocupado añado el proceso a la lista de este
						printf("La IO está ocupada, se bloqueará en su lista propia\n");
						Instruccion_io* to_block = malloc(sizeof(Instruccion_io));
						to_block->proceso = instruccion_io_stdin->proceso;
						to_block->tam_instruccion = instruccion_io_stdin->tam_instruccion;
						to_block->instruccion = malloc(to_block->tam_instruccion);
						strcpy(to_block->instruccion,instruccion_io_stdin->instruccion);
						list_add(io_stdin->procesos_bloqueados,to_block);
					}
					else
					{
						io_stdin->ocupado = true;
						printf("El fd de este IO es %d\n",io_stdin->fd_cliente);
						new_ejecutar_interfaz_stdin_stdout(instruccion_io_stdin->instruccion,STDIN,io_stdin->fd_cliente,instruccion_io_stdin->proceso.PID);
					}
				}
				else
				{
					//de no existir debemos terminar el proceso
					sem_wait(&sem_blocked);				
					PCB* proceso_to_end = encontrar_por_pid(cola_blocked->elements,instruccion_io_stdin->proceso.PID);
					sem_post(&sem_blocked);	

					printf("1Another checkpoint\n");
					sem_wait(&sem_procesos);
					PCB* actualizado_in = encontrarProceso(lista_procesos,instruccion_io_stdin->proceso.PID);
					actualizado_in->estado = EXIT;
					//log_info (kernel_logs_obligatorios,"Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>",
					//sem_post(&PID);sem_procesos);
					sem_post(&sem_procesos);
					printf("2Another checkpoint\n");

					printf("Este proceso ha terminado\n");
					liberar_recursos(proceso_to_end->PID);
					enviarPCB(proceso_to_end,fd_memoria,PROCESOFIN);
					free(proceso_to_end->path);
					free(proceso_to_end);
				}

				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);
				free(instruccion_io_stdin); //ESTE FREE ES PELIGROSÍSIMO, ESTAMOS BORRANDO UN DATO DE LA LISTA DE BLOQUEADOS DEL PROCESO
				break;
			//case FS_CREATE:
				//
			//	break;
			case STDOUT:
				//deserializamos
				Instruccion_io* instruccion_io_stdout = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_io_stdout->instruccion);
				printf("El PID del proceso es: %d\n",instruccion_io_stdout->proceso.PID);

				//debemos obtener el io específico de la lista
				sem_wait(&sem_mutex_lists_io);
				EntradaSalida* io_stdout = encontrar_io(listStdout,string_split(instruccion_io_stdout->instruccion," ")[1]);
				sem_post(&sem_mutex_lists_io);

				//verificamos que exista
				if( io_stdout != NULL)
				{
					//existe
					//una vez encontrada la io, vemos si está ocupado
					if( io_stdout->ocupado )
					{
						//de estar ocupado añado el proceso a la lista de este
						printf("La IO está ocupada, se bloqueará en su lista propia\n");
						list_add(io_stdout->procesos_bloqueados,instruccion_io_stdout);
					}
					else
					{
						io_stdout->ocupado = true;
						printf("El fd de este IO es %d\n",io_stdout->fd_cliente);
						new_ejecutar_interfaz_stdin_stdout(instruccion_io_stdout->instruccion,STDOUT,io_stdout->fd_cliente,instruccion_io_stdout->proceso.PID);
					}
				}
				else
				{
					//de no existir debemos terminar el proceso
					sem_wait(&sem_blocked);				
					PCB* proceso_to_end = encontrar_por_pid(cola_blocked->elements,instruccion_io_stdout->proceso.PID);
					sem_post(&sem_blocked);

					sem_wait(&sem_procesos);
					PCB* actualizado_out = encontrarProceso(lista_procesos,instruccion_io_stdout->proceso.PC);
					actualizado_out->estado = EXIT;
					sem_post(&sem_procesos);

					printf("Este proceso ha terminado\n");
					liberar_recursos(proceso_to_end->PC);
					enviarPCB(proceso_to_end,fd_memoria,PROCESOFIN);
					free(proceso_to_end->path);
					free(proceso_to_end);
				}
				sem_wait(&sem_mutex_cpu_ocupada);
				cpu_ocupada = false;
				sem_post(&sem_mutex_cpu_ocupada);
				free(instruccion_io_stdout);
				break;
			case WAIT:
				//Deserializamos el recurso
				Instruccion_io* instruccion_recurso_wait = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n",instruccion_recurso_wait->instruccion);
				printf("El PID del proceso es: %d\n",instruccion_recurso_wait->proceso.PID);
				char** instruccion_partida_wait = string_split(instruccion_recurso_wait->instruccion," ");

				//verificamos que el recurso exista
				bool existe_wait = false;
				int i_wait = 0;
				Recurso* recurso_wait;
				while( i_wait < list_size(listRecursos) && existe_wait != true )
				{
					recurso_wait = list_get(listRecursos,i_wait);
					if( strcmp(recurso_wait->name,instruccion_partida_wait[1]) == 0 )
					{
						existe_wait = true;
					}
					i_wait++;
				}

				//si existe
				if( existe_wait )
				{
					//reducimos las instancias
					recurso_wait->instancias--;

					//si es menor a cero, se bloquea
					if( recurso_wait->instancias < 0 )
					{
						//de lo contrario lo añadimos a la cola de bloqueados del recurso y también a la cola de bloqueados general
						PCB* proceso_recurso = malloc(sizeof(PCB));
						*proceso_recurso = instruccion_recurso_wait->proceso;
						list_add(recurso_wait->listBloqueados,proceso_recurso);
						
						sem_wait(&sem_blocked);
						queue_push(cola_blocked,proceso_recurso);
						sem_post(&sem_blocked);

						sem_wait(&sem_procesos);
						PCB* actualizado_wait = encontrarProceso(lista_procesos,instruccion_recurso_wait->proceso.PID);
						actualizado_wait->estado = BLOCKED;
						sem_post(&sem_procesos);

						printf("El proceso se ha bloqueado porque no hay instancias disponibles\n");

						//CPU desocupada
						sem_wait(&sem_mutex_cpu_ocupada);
						cpu_ocupada = false;
						sem_post(&sem_mutex_cpu_ocupada);
						//printf("En este momento la cola de ready consta de %d procesos\n",queue_size(cola_ready));
					}
					else
					{
						//SI EL PROCESO TOMÓ LA INSTANCIA, DEBE VOLVER A EJECUTARSE.
						//indicamos que hay un proceso que tomó una instancia
						uint32_t* pid = malloc(sizeof(uint32_t));
						*pid = instruccion_recurso_wait->proceso.PID;
						list_add(recurso_wait->pid_procesos, pid);
						printf("En este momento el tamaño de la lista del recurso es de %d\n",list_size(recurso_wait->pid_procesos));

						//enviamos el proceso a la cola de ready
						PCB* proceso_recurso = malloc(sizeof(PCB));
						*proceso_recurso = instruccion_recurso_wait->proceso;

						/*
						if( strcmp(ALGORITMO_PLANIFICACION,"VRR") == 0 )
						{
							encolar_procesos_vrr(proceso_recurso);
						}
						else
						{
							sem_wait(&sem_ready);   // mutex hace wait
							queue_push(cola_ready,proceso_recurso);	//agrega el proceso a la cola de ready
							sem_post(&sem_ready); 
							sem_post(&sem_cant_ready);  // mutex hace wait
						}
						*/
						printf("Enviaremos un proceso\n");
						printf("Enviaremos el proceso cuyo pid es %d\n",proceso_recurso->PID);
						enviarPCB(proceso_recurso, fd_cpu_dispatch,PROCESO);
						sem_wait(&sem_mutex_cpu_ocupada);
						cpu_ocupada = true;
						sem_post(&sem_mutex_cpu_ocupada);

						/*
						sem_wait(&sem_mutex_cpu_ocupada);
						cpu_ocupada = false;
						sem_post(&sem_mutex_cpu_ocupada);
						*/
						printf("Los recursos han quedado de la siguiente forma:\n");
						for(int i = 0; i < list_size(listRecursos); i++)
						{
							Recurso* got = list_get(listRecursos,i);
							printf("El nombre del recurso es %s y tiene %d instancias\n",got->name,got->instancias);
						}
					}
				}
				else
				{
					//el recurso no existe, debemos terminarlo
					printf("El recurso no existe\n");
					printf("Este proceso ha terminado\n");

					sem_wait(&sem_procesos);
					PCB* actualizado_wait = encontrarProceso(lista_procesos,instruccion_recurso_wait->proceso.PID);
					actualizado_wait->estado = EXIT;
					sem_post(&sem_procesos);

					//printf("En este momento la cola de ready consta de %d procesos\n",queue_size(cola_ready));
					PCB* proceso_recurso = malloc(sizeof(PCB));
					*proceso_recurso = instruccion_recurso_wait->proceso;
					liberar_recursos(proceso_recurso->PID);
					enviarPCB(proceso_recurso,fd_memoria,PROCESOFIN);

					//EN ESTA PARTE, LO IDEAL SERÍA TENER UNA LISTA GLOBAL DE PROCESOS CON LA CUAL PODAMOS TENER SEGUIMIENTO TOTAL DE ELLOS.
					//De darse este caso, en dicha lista diríamos que el proceso ha finalizado.
					free(proceso_recurso);
					sem_wait(&sem_mutex_cpu_ocupada);
					cpu_ocupada = false;
					sem_post(&sem_mutex_cpu_ocupada);
				}
				
				break;
			case SIGNAL:
				// Deserializamos el recurso
				Instruccion_io* instruccion_recurso_signal = deserializar_instruccion_io(paquete->buffer);
				printf("La instruccion es: %s\n", instruccion_recurso_signal->instruccion);
				printf("El PID del proceso es: %d\n", instruccion_recurso_signal->proceso.PID);
				char** instruccion_partida_signal = string_split(instruccion_recurso_signal->instruccion, " ");

				// Verificamos que el recurso exista
				bool existe_signal = false;
				int i_signal = 0;
				Recurso* recurso_signal;
				while (i_signal < list_size(listRecursos) && existe_signal != true) {
					recurso_signal = list_get(listRecursos, i_signal);
					if (strcmp(recurso_signal->name, instruccion_partida_signal[1]) == 0) {
						existe_signal = true;
					}
					i_signal++;
				}

				// Si existe
				if (existe_signal) 
				{
					// Aumentamos las instancias
					recurso_signal->instancias++;

					// Si hay procesos bloqueados, los desbloqueamos
					if (recurso_signal->instancias <= 0) 
					{
						PCB* proceso_desbloqueado = list_remove(recurso_signal->listBloqueados, 0);
						sem_wait(&sem_blocked);
						eliminar_elemento(cola_blocked->elements,proceso_desbloqueado->PID);
						sem_post(&sem_blocked);
						if (proceso_desbloqueado != NULL) {
							//añadimos que dicho proceso está usando una instancia
							uint32_t* pid = malloc(sizeof(uint32_t));
							*pid = proceso_desbloqueado->PID;
							list_add(recurso_signal->pid_procesos,pid);

							sem_wait(&sem_procesos);
							PCB* actualizado_signal = encontrarProceso(lista_procesos,instruccion_recurso_signal->proceso.PID);
							actualizado_signal->estado = READY;
							sem_post(&sem_procesos);

							if( strcmp(ALGORITMO_PLANIFICACION,"VRR") == 0 )
							{
								encolar_procesos_vrr(proceso_desbloqueado);
							}
							else
							{
								// Enviamos el proceso a la cola de ready
								sem_wait(&sem_ready);   // mutex hace wait
								printf("En este momento en la cola de ready se ha pusheado al pid: %d\n", proceso_desbloqueado->PID);
								queue_push(cola_ready, proceso_desbloqueado); // agrega el proceso a la cola de ready
								sem_post(&sem_ready);
								sem_post(&sem_cant_ready);  // mutex hace wait
							}
						}
					}
					
					//Tenemos que sacar el pid de la lista del recurso
					eliminar_elemento(recurso_signal->pid_procesos, instruccion_recurso_signal->proceso.PID);

					PCB* proceso_recurso = malloc(sizeof(PCB));
					*proceso_recurso = instruccion_recurso_signal->proceso;

					//debe volver a ejecutarse
					printf("Enviaremos un proceso\n");
					printf("Enviaremos el proceso cuyo pid es %d\n",proceso_recurso->PID);
					enviarPCB(proceso_recurso, fd_cpu_dispatch,PROCESO);
					sem_wait(&sem_mutex_cpu_ocupada);
					cpu_ocupada = true;
					sem_post(&sem_mutex_cpu_ocupada);
					/*
					if( strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0 )
					{
						encolar_procesos_vrr(proceso_recurso);
					}
					else
					{
						sem_wait(&sem_ready);   // mutex hace wait
						queue_push(cola_ready,proceso_recurso);	//agrega el proceso a la cola de ready
						sem_post(&sem_ready); 
						sem_post(&sem_cant_ready);  // mutex hace wait
						printf("(EXISTE SIGNAL)Se ha añadido un proceso con PID %d\n",proceso_recurso->PID);

						sem_wait(&sem_mutex_cpu_ocupada);
						cpu_ocupada = false;
						sem_post(&sem_mutex_cpu_ocupada);
					}
					*/

				} 
				else 
				{
					// El recurso no existe, debemos terminarlo
					printf("El recurso no existe\n");
					printf("Este proceso ha terminado\n");
					sem_wait(&sem_procesos);
					PCB* actualizado_signal = encontrarProceso(lista_procesos,instruccion_recurso_signal->proceso.PID);
					
					actualizado_signal->estado = EXIT;
					
					
					sem_post(&sem_procesos);

					PCB* proceso_recurso = malloc(sizeof(PCB));
					*proceso_recurso = instruccion_recurso_signal->proceso;
					liberar_recursos(proceso_recurso->PID);
					enviarPCB(proceso_recurso, fd_memoria, PROCESOFIN);

					// EN ESTA PARTE, LO IDEAL SERÍA TENER UNA LISTA GLOBAL DE PROCESOS CON LA CUAL PODAMOS TENER SEGUIMIENTO TOTAL DE ELLOS.
					// De darse este caso, en dicha lista diríamos que el proceso ha finalizado.
					free(proceso_recurso);
					sem_wait(&sem_mutex_cpu_ocupada);
					cpu_ocupada = false;
					sem_post(&sem_mutex_cpu_ocupada);
				}

				break;
				
			case DIALFS:
				
				// Deserializamos
					Instruccion_io* instruccion_io_fs = deserializar_instruccion_io(paquete->buffer);
					printf("La instrucción es: %s\n", instruccion_io_fs->instruccion);
					printf("El PID del proceso es: %d\n", instruccion_io_fs->proceso.PID);

					// Debemos obtener la IO específica de la lista
					sem_wait(&sem_mutex_lists_io);
					EntradaSalida* io_fs = encontrar_io(listDialfs, string_split(instruccion_io_fs->instruccion, " ")[1]);
					sem_post(&sem_mutex_lists_io);

					// Verificamos que exista
					if (io_fs != NULL)
					{
						// Existe
						// Una vez encontrada la IO, vemos si está ocupada
						if (io_fs->ocupado)
						{
							// Si está ocupada, añadimos el proceso a la lista de bloqueados
							printf("La IO está ocupada, se bloqueará en su lista propia\n");
							list_add(io_fs->procesos_bloqueados, instruccion_io_fs);
						}
						else
						{
							io_fs->ocupado = true;
							printf("El fd de esta IO es %d\n", io_fs->fd_cliente);
							char** instruccion_partida_dialfs = string_split(instruccion_io_fs->instruccion," ");
							if( strcmp(instruccion_partida_dialfs[0], "IO_FS_CREATE") == 0 )
							{
								//crear archivo 
								new_ejecutar_interfaz_stdin_stdout(instruccion_io_fs->instruccion, IO_FS_CREATE, io_fs->fd_cliente, instruccion_io_fs->proceso.PID);
							}
							else
							{
								if( strcmp(instruccion_partida_dialfs[0], "IO_FS_DELETE") == 0 )
								{
									//archivo borrar
									new_ejecutar_interfaz_stdin_stdout(instruccion_io_fs->instruccion, IO_FS_DELETE, io_fs->fd_cliente, instruccion_io_fs->proceso.PID);
								}
								else
								{
									if( strcmp(instruccion_partida_dialfs[0], "IO_FS_TRUNCATE") == 0 )
									{
										//archivo truncar
										new_ejecutar_interfaz_stdin_stdout(instruccion_io_fs->instruccion, IO_FS_TRUNCATE, io_fs->fd_cliente, instruccion_io_fs->proceso.PID);
									}
									else
									{
										if( strcmp(instruccion_partida_dialfs[0], "IO_FS_WRITE") == 0 )
										{
											//archivo escribir
											new_ejecutar_interfaz_stdin_stdout(instruccion_io_fs->instruccion, IO_FS_WRITE, io_fs->fd_cliente, instruccion_io_fs->proceso.PID);
										}
										else
										{
											//archivo leer	
											new_ejecutar_interfaz_stdin_stdout(instruccion_io_fs->instruccion, IO_FS_READ, io_fs->fd_cliente, instruccion_io_fs->proceso.PID);
										}
									}
								}
							}
						}
					}
					else
					{
						// Si no existe, debemos terminar el proceso
						sem_wait(&sem_blocked);
						PCB* proceso_to_end = encontrar_por_pid(cola_blocked->elements, instruccion_io_fs->proceso.PID);
						sem_post(&sem_blocked);

						sem_wait(&sem_procesos);
						PCB* actualizado_fs = encontrarProceso(lista_procesos, instruccion_io_fs->proceso.PID);
						actualizado_fs->estado = EXIT;
						sem_post(&sem_procesos);

						printf("Este proceso ha terminado\n");
						liberar_recursos(proceso_to_end->PC);
						enviarPCB(proceso_to_end, fd_memoria, PROCESOFIN);
						free(proceso_to_end->path);
						free(proceso_to_end);
					}

					sem_wait(&sem_mutex_cpu_ocupada);
					cpu_ocupada = false;
					sem_post(&sem_mutex_cpu_ocupada);
					free(instruccion_io_fs);
					break;
   
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(kernel_logger, "Desconexion de cpu.");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

EntradaSalida* encontrar_por_fd_cliente(t_list* lista, int fd_cliente_buscado) {
    for (int i = 0; i < list_size(lista); i++) 
	{
        EntradaSalida* entrada = list_get(lista, i);
        if (entrada->fd_cliente == fd_cliente_buscado) {
            return entrada;
        }
    }
    return NULL; // Si no se encuentra el elemento
}

//encuentra la io en alguna de las listas, indica que ya no está ocupada y envía a un proceso si es que está ahí esperando
void modificar_io_en_listas(int fd_io)
{
	EntradaSalida* to_ret;
	to_ret = encontrar_por_fd_cliente(listGenericas, fd_io);
	if( to_ret != NULL )
	{
		to_ret->ocupado = false;
		if( list_size(to_ret->procesos_bloqueados) > 0 )
		{
			Instruccion_io* proceso_bloqueado_io = list_remove(to_ret->procesos_bloqueados,0);
			printf("De la lista hemos extraído la instrucción %s\n",proceso_bloqueado_io->instruccion);
			printf("De la lista hemos extraído el pid %d\n",proceso_bloqueado_io->proceso.PID);
			ejecutar_interfaz_generica(proceso_bloqueado_io->instruccion,GENERICA,fd_io,proceso_bloqueado_io->proceso.PID);
		}
	}
	else
	{
		to_ret = encontrar_por_fd_cliente(listStdin, fd_io);
		if( to_ret != NULL )
		{
			to_ret->ocupado = false;
			if( list_size(to_ret->procesos_bloqueados) > 0 )
			{
				Instruccion_io* proceso_bloqueado_io = list_remove(to_ret->procesos_bloqueados,0);
				ejecutar_interfaz_stdinstdout(proceso_bloqueado_io->instruccion,STDIN,fd_io,proceso_bloqueado_io->proceso.PID);
			}
		}
		else
		{
			to_ret = encontrar_por_fd_cliente(listStdout, fd_io);
			if( to_ret != NULL )
			{
				to_ret->ocupado = false;
				if( list_size(to_ret->procesos_bloqueados) > 0 )
				{
					Instruccion_io* proceso_bloqueado_io = list_remove(to_ret->procesos_bloqueados,0);
					ejecutar_interfaz_stdinstdout(proceso_bloqueado_io->instruccion,STDOUT,fd_io,proceso_bloqueado_io->proceso.PID);
				}
			}
			else
			{
				to_ret = encontrar_por_fd_cliente(listDialfs, fd_io);
				to_ret->ocupado = false;
				if( list_size(to_ret->procesos_bloqueados) > 0 )
				{
					Instruccion_io* proceso_bloqueado_io = list_remove(to_ret->procesos_bloqueados,0);
				}
			}
		}
	}
}

//Al escuchar las entradas salidas
void kernel_escuchar_entradasalida_mult(int* fd_io)
{
	bool control_key = 1;
	while (control_key) {

			int cod_op = recibir_operacion(*fd_io);
			printf("El fd de este IO es %d\n",*fd_io);
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(*fd_io,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(*fd_io,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case GENERICA:
				//creamos la interfaz genérica
				EntradaSalida* new_io_generica = deserializar_entrada_salida(paquete->buffer);
				new_io_generica->fd_cliente = *fd_io;
				printf("Llego una IO cuyo nombre es: %s\n",new_io_generica->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_generica->path);
				
				//lo añadimos a la lista de ios genéricas.
				sem_wait(&sem_mutex_lists_io);
				list_add(listGenericas,new_io_generica);
				sem_post(&sem_mutex_lists_io);
			    break;
			case STDIN:
				//creamos la interfaz genérica
				EntradaSalida* new_io_stdin = deserializar_entrada_salida(paquete->buffer);
				new_io_stdin->fd_cliente = *fd_io;
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdin->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdin->path);
				
				//lo añadimos a la lista de ios genéricas.
				sem_wait(&sem_mutex_lists_io);
				list_add(listStdin,new_io_stdin);
				sem_post(&sem_mutex_lists_io);		
			    break;
			case STDOUT:
				//creamos la interfaz genérica
				EntradaSalida* new_io_stdout = deserializar_entrada_salida(paquete->buffer);
				new_io_stdout->fd_cliente = *fd_io;
				printf("Llego una IO cuyo nombre es: %s\n",new_io_stdout->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_stdout->path);

				//lo añadimos a la lista de ios genéricas.
				sem_wait(&sem_mutex_lists_io);
				list_add(listStdout,new_io_stdout);
				sem_post(&sem_mutex_lists_io);			
			    break;
			case DIALFS:
				//creamos la interfaz genérica
				EntradaSalida* new_io_dialfs = deserializar_entrada_salida(paquete->buffer);
				new_io_dialfs->fd_cliente = *fd_io;
				printf("Llego una IO cuyo nombre es: %s\n",new_io_dialfs->nombre);
		   		printf("Llego una IO cuyo path es: %s\n",new_io_dialfs->path);
				
				//lo añadimos a la lista de ios genéricas.
				sem_wait(&sem_mutex_lists_io);
				list_add(listDialfs,new_io_dialfs);
				sem_post(&sem_mutex_lists_io);
			    break;
			case DESPERTAR:
				//sacamos al proceso de la cola de blocked y lo añadimos a IO
				printf("La IO ha despertado :O\n");
				int* pid_io = paquete->buffer->stream;
				printf("El pid que ha llegado de la IO es %d\n",*pid_io);

				//PCB* proceso_awaken = queue_pop(cola_blocked);
				sem_wait(&sem_blocked);
				PCB* proceso_awaken = encontrar_por_pid(cola_blocked->elements,(uint32_t)*pid_io);
				sem_post(&sem_blocked);
				
				//ACÁ METERÉ EN READY DEPENDIENDO DEL QUANTUM DEL PROCESO
				proceso_awaken->estado = READY;

				sem_wait(&sem_procesos);
				PCB* actualizado = encontrarProceso(lista_procesos,(uint32_t)*pid_io);
				actualizado->estado = READY;
				sem_post(&sem_procesos);

				//si el quantum del proceso es mayor a cero, debe ir a la cola de mayor prioridad
				//meter en ready
				if( strcmp(ALGORITMO_PLANIFICACION, "VRR") == 0 )
				{
					encolar_procesos_vrr(proceso_awaken);
				}
	 			else
				{
					sem_wait(&sem_ready);   // mutex hace wait
					queue_push(cola_ready,proceso_awaken);	//agrega el proceso a la cola de ready
					sem_post(&sem_ready); 
					sem_post(&sem_cant_ready);
				}

				//también debemos marcar la io como desocupada, buscamos por fd
				sem_wait(&sem_mutex_lists_io);
				modificar_io_en_listas(*fd_io);
				sem_post(&sem_mutex_lists_io);
			    break;
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(kernel_logger, "El cliente EntradaSalida se desconecto. Terminando servidor");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void escuchar_io()
{
	while (1) 
	{
		pthread_t thread;
		int *fd_conexion_ptr = malloc(sizeof(int));
		*fd_conexion_ptr = accept(fd_kernel, NULL, NULL);
		handshakeServer(*fd_conexion_ptr);
		printf("Se ha conectado un cliente de tipo IO\n");
		pthread_create(&thread, NULL, (void*) kernel_escuchar_entradasalida_mult, fd_conexion_ptr);
		pthread_detach(thread);
	}
}

void kernel_escuchar_memoria ()
{
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
				log_error(kernel_logger, "Desconexion de memoria.");
				control_key = 0;
			default:
				log_warning(kernel_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
		}	
}

void enviarProcesoMemoria (ProcesoMemoria* proceso, int socket_servidor)
{
    //Creamos un Buffer
    t_newBuffer* buffer = malloc(sizeof(t_newBuffer));

    //Calculamos su tamaño
    buffer->size = sizeof(uint32_t)*2 + proceso->path_length +1;//MODIFCADO
    buffer->offset = 0;
    buffer->stream = malloc(buffer->size);
	
    //Movemos los valores al buffer
    memcpy(buffer->stream + buffer->offset, &proceso->PID, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);

    // Para el nombre primero mandamos el tamaño y luego el texto en sí:
    memcpy(buffer->stream + buffer->offset, &proceso->path_length, sizeof(uint32_t));
    buffer->offset += sizeof(uint32_t);
    memcpy(buffer->stream + buffer->offset, proceso->path, proceso->path_length);
    
	//Creamos un Paquete
    t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = PAQUETE;
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
    send(socket_servidor, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void iniciar_proceso(char* path)
{
	PCB* pcb = malloc(sizeof(PCB));
	if ( pcb == NULL )
	{
		printf("Error al crear pcb");
	}

	//inicializo el PCB del proceso
	pid++;
	pcb->PID = pid;
	pcb->PC = 0;
	pcb->quantum = atoi(QUANTUM);
	pcb->registro.AX = 0;//cambios
	pcb->registro.BX = 0;//cambios
	pcb->registro.CX = 0;//cambios
	pcb->registro.DX = 0;//cambios
	pcb->registro.EAX = 0;//cambios
	pcb->registro.EBX = 0;//cambios
	pcb->registro.ECX = 0;//cambios
	pcb->registro.EDX = 0;//cambios
	pcb->registro.DI = 0;//cambios
	pcb->registro.SI = 0;//cambios
	pcb->estado = NEW;
	pcb->path_length = strlen(path)+1;
	pcb->path = string_duplicate(path); //consejo

	//en este punto aprovecho para enviarlo a memoria
	ProcesoMemoria* proceso = malloc(sizeof(ProcesoMemoria));
	proceso->path = malloc(strlen(pcb->path)+1);
	//proceso->path = pcb->path;
	strcpy(proceso->path, pcb->path);
	proceso->PID = pcb->PID;
	proceso->path_length = strlen(pcb->path)+1;
	proceso->TablaDePaginas = list_create();

	enviarProcesoMemoria(proceso,fd_memoria);

	//agrego el pcb a la cola new
	
	sem_wait(&sem_procesos);
	list_add(lista_procesos,pcb);
	printf("Agregué el proceso %d a la lista de procesos\n", pcb->PID);
	sem_post(&sem_procesos);
	
	
	sem_wait(&sem);
	queue_push(cola_new,pcb);	
    // Señalar (incrementar) el semáforo
    sem_post(&sem);
	sem_post(&sem_cant);
	
	//procesos_en_new++;
	
	log_info (kernel_logs_obligatorios, "Se crea el proceso <%d> en NEW, funcion iniciar proceso\n", pcb->PID);
}

void finalizar_proceso (char* pid) {
    uint32_t pid_to_eliminar = (uint32_t)atoi(pid);
    printf("El pid solicitado a eliminar es %d\n", pid_to_eliminar);

    sem_wait(&sem_procesos);
    PCB* proceso_a_terminar = encontrarProceso(lista_procesos, pid_to_eliminar);
    sem_post(&sem_procesos);

    printf("El proceso a terminar luce de la siguiente forma: \n");
    printf("PID: %d\n", proceso_a_terminar->PID);
    printf("Length del path: %d\n", proceso_a_terminar->path_length);
    printf("Path: %s\n", proceso_a_terminar->path);

    char* estado_anterior = estado_proceso_to_string(proceso_a_terminar->estado);

    switch (proceso_a_terminar->estado) {
        case NEW:
            printf("El proceso de pid %d es eliminado estando en NEW\n", proceso_a_terminar->PID);
            liberar_recursos(pid_to_eliminar);
            enviarPCB(proceso_a_terminar, fd_memoria, PROCESOFIN);
            sem_wait(&sem);
            eliminar_elemento(cola_new->elements, pid_to_eliminar);
            sem_post(&sem);
            break;
        case READY:
            printf("El proceso de pid %d es eliminado estando en READY\n", proceso_a_terminar->PID);
            liberar_recursos(pid_to_eliminar);
            enviarPCB(proceso_a_terminar, fd_memoria, PROCESOFIN);
            sem_wait(&sem_ready);
            eliminar_elemento(cola_ready->elements, pid_to_eliminar);
            sem_post(&sem_ready);
            break;
        case EXEC:
            // FALTA VERIFICAR EL CASO DE QUE SE ENCUENTRE EN EJECUCIÓN.
            int* pid_del_proceso = malloc(sizeof(int));
            *pid_del_proceso = proceso_a_terminar->PID;
            enviarEntero(pid_del_proceso, fd_cpu_interrupt, FINALIZAR_PROCESO);
            break;
        case BLOCKED:
            printf("El proceso de pid %d es eliminado estando en BLOCKED\n", proceso_a_terminar->PID);
			liberar_recursos(pid_to_eliminar);
            enviarPCB(proceso_a_terminar, fd_memoria, PROCESOFIN);
            sem_wait(&sem_blocked);
            eliminar_elemento(cola_blocked->elements, pid_to_eliminar);
            sem_post(&sem_blocked);
            break;
    }
    proceso_a_terminar->estado = EXIT;
    char* estado_actual = estado_proceso_to_string(proceso_a_terminar->estado);

    log_info(kernel_logs_obligatorios, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>\n", proceso_a_terminar->PID, estado_anterior, estado_actual);
}



void proceso_estado(){
	sem_wait(&sem_procesos);
	for(int i = 0; i < list_size(lista_procesos); i++)
	{

		PCB* pcb = list_get(lista_procesos,i);
		char* estado = estado_proceso_to_string (pcb->estado);
		printf("El proceso %d esta en estado <%s>\n",pcb->PID,estado);
		
	}
	sem_post(&sem_procesos);
}

void cambio_multiprogramacion (char* valor){
	int valor_nuevo = atoi (valor);
	printf ("Grado de multiprogramacion ANTES: %d\n",grado_multiprogramacion);
	sem_wait(&sem_grado_mult);
	grado_multiprogramacion = valor_nuevo;
	printf ("Grado de multiprogramacion DESPUES: %d\n",grado_multiprogramacion);
	sem_post(&sem_grado_mult);
}



void atender_instruccion (char* leido)
{
    char** comando_consola = string_split(leido, " ");
	//printf("%s\n",comando_consola[0]);

    if((strcmp(comando_consola[0], "INICIAR_PROCESO") == 0))
	{ 
        iniciar_proceso(comando_consola[1]);  
    }else if(strcmp(comando_consola [0], "EJECUTAR_SCRIPT") == 0){
		ejecutar_script(comando_consola[1]);
		
    }else if(strcmp(comando_consola [0], "FINALIZAR_PROCESO") == 0){
		finalizar_proceso(comando_consola[1]);

    }else if(strcmp(comando_consola [0], "DETENER_PLANIFICACION") == 0){
		iniciar_planificacion = false;

    }else if(strcmp(comando_consola [0], "INICIAR_PLANIFICACION") == 0){
		iniciar_planificacion = true;

    }else if(strcmp(comando_consola [0], "MULTIPROGRAMACION") == 0){
		cambio_multiprogramacion(comando_consola[1]);
		 
    }else if(strcmp(comando_consola [0], "PROCESO_ESTADO") == 0){
		proceso_estado();
		
    }else if(strcmp(comando_consola [0], "HELP") == 0){
    }else if(strcmp(comando_consola [0], "PRINT") == 0){
    }else{   
        log_error(kernel_logger, "Comando no reconocido, pero que paso el filtro ???");  
        exit(EXIT_FAILURE);
    }
    string_array_destroy(comando_consola); 
}

void validarFuncionesConsola(char* leido)
{
	 char** valorLeido = string_split(leido, " ");
	 //printf("%s\n",valorLeido[0]);

	 if(strcmp(valorLeido[0], "EJECUTAR_SCRIPT") == 0)
	 {
		printf("Comando válido\n");
		atender_instruccion(leido);
	 }
	 else
	 {
		if(strcmp(valorLeido[0], "INICIAR_PROCESO") == 0)
	    {
		    printf("Comando válido\n");
			atender_instruccion(leido);
	    }
		else
		{
			if(strcmp(valorLeido[0], "FINALIZAR_PROCESO") == 0)
	        {  
		        printf("Comando válido\n");
				atender_instruccion(leido);
	        }
			else
			{
				if(strcmp(valorLeido[0], "DETENER_PLANIFICACION") == 0)
	            {  
		            printf("Comando válido\n");
					atender_instruccion(leido);
	            }
				else
				{
					if(strcmp(valorLeido[0], "INICIAR_PLANIFICACION") == 0)
	                {  
		                printf("Comando válido\n");
						atender_instruccion(leido);
	                }
					else
					{
						if(strcmp(valorLeido[0], "MULTIPROGRAMACION") == 0)
	                    {  
		                       printf("Comando válido\n");
							   atender_instruccion(leido);
	                    }
						else
						{
							if(strcmp(valorLeido[0], "PROCESO_ESTADO") == 0)
	                        {  
		                       printf("Comando válido\n");
							   atender_instruccion(leido);
	                        }
						}
					}
				}
			}
		}
	 }
	 string_array_destroy(valorLeido);
}

char* remove_last_two_chars(const char* str) {
    int len = strlen(str);
    if (len <= 2) {
        return strdup(""); // Devuelve una cadena vacía si la longitud es menor o igual a 2
    }

    char* new_str = (char*)malloc((len - 1) * sizeof(char)); // Reserva memoria para la nueva cadena
    if (new_str == NULL) {
        return NULL; // Manejo de error en caso de que malloc falle
    }

    strncpy(new_str, str, len - 2); // Copia la cadena original sin los dos últimos caracteres
    new_str[len - 2] = '\0'; // Añade el carácter nulo al final

    return new_str;
}

void ejecutar_script (char* path)
{
	FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("No se pudo abrir el archivo");
        return;
    }
	
	char linea[256];
    while (fgets(linea, sizeof(linea), file)) {
		printf("La linea leida es: %s", linea);
		char* linea_modificada = remove_last_two_chars(linea);
		printf("La linea modificada leida es: %s\n", linea_modificada);
		validarFuncionesConsola(linea_modificada);
		free(linea_modificada);
		sleep (1);
    }
	fclose(file);
}

void consolaInteractiva()
{
	char* leido;
	leido = readline("> ");
	
	while( strcmp(leido,"--------") != 0 )
	{
		validarFuncionesConsola(leido);
		free(leido);
		leido = readline("> ");
		/*
		sem_wait(&sem_blocked);
		sem_wait(&sem_ready);
		printf("Ahora la cola de READY tiene %d procesos\n",queue_size(cola_ready));
		printf("Ahora la cola de BLOCKED tiene %d procesos\n",queue_size(cola_blocked));
		sem_post(&sem_blocked);
		sem_post(&sem_ready);
		*/
	}
}

void mover_procesos_a_ready()
{
	bool added = false;
	while( added != true )
	{
		sem_wait(&sem_cant);   // mutex hace wait
		sem_wait(&sem);   // mutex hace wait
		if (iniciar_planificacion == true){
			//pido los semaforos de cada cola
			//printf("Ingresamos a mover procesos a ready\n");
			sem_wait(&sem_grado_mult);
			sem_wait(&sem_blocked);
			sem_wait(&sem_ready);
			if( (queue_size(cola_ready) + queue_size(cola_blocked) + 1) < grado_multiprogramacion )
			{
				printf("[NICE] EL GRADO DE MULTIPROGRAMACIÓN NO HA SIDO SUPERADO, UN NUEVO PROCESO ES ADMITIDO\n");
				//si se cumple, significa que podemos incluir un nuevo proceso en ready
				PCB* pcb = queue_pop(cola_new);
				sem_post(&sem_blocked);
				sem_post(&sem_grado_mult);

				sem_wait(&sem_procesos);
				PCB* actualizado = encontrarProceso(lista_procesos,pcb->PID);
				char* estado_anterior = estado_proceso_to_string (actualizado->estado);
				actualizado->estado = READY;
				char* estado_actual = estado_proceso_to_string (actualizado->estado);
				sem_post(&sem_procesos);

				queue_push(cola_ready,pcb);	//agrega el proceso a la cola de ready
				printf("El proceso de PID %d ha ingresado a la cola de ready desde el planificador de largo plazo\n",pcb->PID);
				log_info (kernel_logs_obligatorios, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", pcb->PID, estado_anterior, estado_actual);
				
				sem_post(&sem_ready); 
				sem_post(&sem_cant_ready);  // mutex hace wait
				added = true;
			}
			else
			{
				printf("[BAD] EL GRADO DE MULTIPROGRAMACIÓN HA SIDO SUPERADO, EL PROCESO NO ES ADMITIDO\n");
				sem_post(&sem_grado_mult);
				sem_post(&sem_blocked);
				sem_post(&sem_ready);
			}
		}
		sem_post(&sem);   // mutex hace signal
		sleep(1);
	}

}

void planificador_de_largo_plazo()
{
	
	while(1)
	{	
		if (iniciar_planificacion == true){
		
			//sem_wait(&sem_cant);   // mutex hace wait
			//sem_wait(&sem);   // mutex hace wait

			mover_procesos_a_ready();
			
			//sem_post(&sem);   // mutex hace signal
		}
	}	
}

//FUNCIÓN OBSOLETA
void informar_memoria_nuevo_procesoNEW()
{
	//CREAMOS BUFFER
	//POR EL MOMENTO ESTAMOS HARCODEANDO para intentar mandar algo a memoria
	ProcesoMemoria* proceso = malloc(sizeof(ProcesoMemoria));

	PCB* pcb = queue_pop(cola_new);//
	proceso->path = malloc(strlen(pcb->path)+1);
	//proceso->path = pcb->path;
	strcpy(proceso->path, pcb->path);
	proceso->PID = pcb->PID;
	proceso->path_length = strlen(pcb->path)+1;
	proceso->TablaDePaginas = list_create();
	
	enviarProcesoMemoria(proceso,fd_memoria);
	
	sem_wait(&sem_ready);   // mutex hace wait
	queue_push(cola_ready,pcb);	//agrega el proceso a la cola de ready
    sem_post(&sem_ready); 
	sem_post(&sem_cant_ready);  // mutex hace wait
	
}

//FUNCIÓN OBSOLETA
void VIEJO_planificador_largo_plazo()
{
    // //Obtenemos el grado de multiprogramación especificado por el archivo de configuración
    // //int grado_multiprogramacion = config_get_int_value(kernel_config, "GRADO_MULTIPROGRAMACION");
	// int grado_multiprogramacion = 20;
	// //pregunto constantemente
	// while(1)
	// {
	// 	//hay nuevos procesos en new?
	// 	if( procesos_en_new > 0 )
	// 	{
	// 		//avisar a memoria
	// 		//USAR QUEUE Pop
	while(1)
	{
		//SEMAFOROS (PRODUCTOR-CONSUMIDOR)  consumidor de la cola de NEW

		//if( queue_size(cola_new) > 0)
		//{	
		    sem_wait(&sem_cant);   // mutex hace wait
		    sem_wait(&sem);   // mutex hace wait

			informar_memoria_nuevo_procesoNEW();
			
			sem_post(&sem);   // mutex hace signal

			//sem_post(&sem_cant_ready); //avisamos al planificador que hay un nuevo proceso listo
			
		//}

	}			
	// 	}
	// 	else
	// 	{
	// 		//algun proceso terminó?
	// 		if(procesos_fin > 0 )
	// 		{
	// 			//
	// 			printf("esta parte es la del fin del proceso");
	// 		}
	// 	}
	// 	//luego de esto muevo los procesos a ready mientras pueda
	// 	//mover_procesos_ready(grado_multiprogramacion);
	// 	sleep(1);
	//}	
}

void interrumpir_por_quantum_vrr(int proceso_quantum)
{
	//iniciamos el temporaizador
	sem_wait(&sem_mutex_cronometro);
	cronometro = temporal_create();
	sem_post(&sem_mutex_cronometro);

	//esperamos el tiempo
	int quantum = atoi(QUANTUM);
	if( proceso_quantum != quantum )
	{
		//si es distinto, le queda quantum restande
		usleep(proceso_quantum*1000);
	}
	else
	{
		//de lo contrario esperamos el quantum normal
		usleep(quantum*1000);
	}

	//Una vez el tiempo haya pasado interrumpimos
	int* enteroRandom = malloc(sizeof(int));
	*enteroRandom = 0;
	enviarEntero(enteroRandom,fd_cpu_interrupt,FIN_DE_QUANTUM);
}

void interrumpir_por_quantum()
{
	int quantumAUsar = atoi(QUANTUM);
	unsigned int quantum = quantumAUsar;
	printf("El quantum sera: %d\n",quantumAUsar);
	usleep(quantum*1000);
	//MANDAR A MEMORIA FIN DE QUANTUM POR DISPATCH
	int* enteroRandom = malloc(sizeof(int));
	*enteroRandom = 0;
 	printf("Me desperte uwu\n");
	enviarEntero(enteroRandom,fd_cpu_interrupt,FIN_DE_QUANTUM);
}

void enviar_pcb_a_cpu()
{
	sem_wait(&sem_mutex_cpu_ocupada);
	while( cpu_ocupada != false )
	{
		sem_post(&sem_mutex_cpu_ocupada);
		sleep(1);
		//usleep(500000);

		sem_wait(&sem_mutex_cpu_ocupada);
	}
	sem_post(&sem_mutex_cpu_ocupada);
	//Reservo memoria para enviarla
	PCB* to_send = malloc(sizeof(PCB));
	

	sem_wait(&sem_cant_ready);   // mutex hace wait
	sem_wait(&sem_ready);   // mutex hace wait

	PCB* pcb_cola = queue_pop(cola_ready); //saca el proceso de la cola de ready
	printf("A LA CPU SE LE ENVIARÁ: %d\n",pcb_cola->PID);
	printf("A LA CPU SE LE ENVIARÁ: %d\n",pcb_cola->quantum);
	printf("A LA CPU SE LE ENVIARÁ: %d\n",pcb_cola->path_length);
			
	sem_post(&sem_ready); // mutex hace signal
	

	to_send->PID = pcb_cola->PID;
	to_send->PC = pcb_cola->PC;
	to_send->quantum = pcb_cola->quantum;
	to_send->estado = EXEC;
	to_send->registro = pcb_cola->registro;//cambios
	/*to_send->registro.BX = pcb_cola->registro.BX;//cambios
	to_send->registro.CX = pcb_cola->registro.CX;//cambios
	to_send->registro.DX = pcb_cola->registro.DX;//cambios*/

	to_send->path = string_duplicate( pcb_cola->path);

	//ACTUALIZAMOS EN LA LISTA GENERAL
	sem_wait(&sem_procesos);
	PCB* actualizado = encontrarProceso(lista_procesos,pcb_cola->PID);
	actualizado->estado = EXEC;
	sem_post(&sem_procesos);


	printf("Enviaremos un proceso\n");
	printf("Enviaremos el proceso cuyo pid es %d\n",to_send->PID);
	enviarPCB(to_send, fd_cpu_dispatch,PROCESO);
	sem_wait(&sem_mutex_cpu_ocupada);
	cpu_ocupada = true;
	sem_post(&sem_mutex_cpu_ocupada);
	if( strcmp(ALGORITMO_PLANIFICACION,"RR") == 0 )
	{
		interrumpir_por_quantum();
	}
}

void enviar_pcb_a_cpu_vrr()
{
	sem_wait(&sem_mutex_cpu_ocupada);
	while( cpu_ocupada != false )
	{
		sem_post(&sem_mutex_cpu_ocupada);
		sleep(1);
		sem_wait(&sem_mutex_cpu_ocupada);
	}
	sem_post(&sem_mutex_cpu_ocupada);
	//Reservo memoria para enviarla
	PCB* to_send = malloc(sizeof(PCB));

	PCB* pcb_cola;
	
	//Priorizaremos a aquellos procesos con mayor prioridad
	sem_wait(&sem_ready_prio);
	if( queue_size(cola_ready_prioridad) > 0 )
	{
		printf("Entré en esta guarda\n");

		pcb_cola = queue_pop(cola_ready_prioridad); //saca el proceso de la cola de ready
	}
	else
	{
		printf("Entré en esta guarda\n");
		sem_wait(&sem_cant_ready);   // mutex hace wait
		sem_wait(&sem_ready);   // mutex hace wait
		pcb_cola = queue_pop(cola_ready); //saca el proceso de la cola de ready
		sem_post(&sem_ready); // mutex hace signal
	}
	sem_post(&sem_ready_prio);
		

	to_send->PID = pcb_cola->PID;
	to_send->PC = pcb_cola->PC;
	to_send->quantum = pcb_cola->quantum;
	to_send->estado = EXEC;
	to_send->registro = pcb_cola->registro;//cambios
	/*to_send->registro.BX = pcb_cola->registro.BX;//cambios
	to_send->registro.CX = pcb_cola->registro.CX;//cambios
	to_send->registro.DX = pcb_cola->registro.DX;//cambios*/

	to_send->path = string_duplicate( pcb_cola->path);

	//ACTUALIZAMOS EN LA LISTA GENERAL
	sem_wait(&sem_procesos);
	PCB* actualizado = encontrarProceso(lista_procesos,pcb_cola->PID);
	actualizado->estado = EXEC;
	sem_post(&sem_procesos);

	printf("Enviaremos un proceso\n");
	printf("Enviaremos el proceso cuyo pid es %d\n",to_send->PID);
	enviarPCB(to_send, fd_cpu_dispatch,PROCESO);
	sem_wait(&sem_mutex_cpu_ocupada);
	cpu_ocupada = true;
	sem_post(&sem_mutex_cpu_ocupada);
	if(strcmp(ALGORITMO_PLANIFICACION,"VRR")==0)
	{
		interrumpir_por_quantum_vrr(pcb_cola->quantum);
	}
}

void planificador_corto_plazo()
{
	if (iniciar_planificacion == true){
	
		if(strcmp(ALGORITMO_PLANIFICACION,"FIFO")==0)
		{
			printf("Planificare por FIFO\n");
			while(1)
			{
				if (iniciar_planificacion == true){

					//PLANIFICAR POR FIFO
					sem_wait(&sem_mutex_plani_corto);
					enviar_pcb_a_cpu();
					sem_post(&sem_mutex_plani_corto);
				}
			}
		}
		
		if(strcmp(ALGORITMO_PLANIFICACION,"RR")==0)
		{
			printf("Planificare por RR\n");
			//PLANIFICAR POR RR
			while(1)
			{
				if (iniciar_planificacion == true){

					sem_wait(&sem_mutex_plani_corto);
					enviar_pcb_a_cpu();
					sem_post(&sem_mutex_plani_corto);
					//interrumpir_por_quantum();
				}
			}
		}
		if(strcmp(ALGORITMO_PLANIFICACION,"VRR")==0)
		{
			//PLANIFICAR POR VRR
			printf("Planificare por VRR\n");
			//PLANIFICAR POR VRR
			while(1)
			{
				if (iniciar_planificacion == true){

				sem_wait(&sem_mutex_plani_corto);
				enviar_pcb_a_cpu_vrr();
				sem_post(&sem_mutex_plani_corto);
				}
			}
		}
	}
}
	

#endif /* KERNEL_H_ */
