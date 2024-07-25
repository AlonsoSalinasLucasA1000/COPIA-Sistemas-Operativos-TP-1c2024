#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//semaforo de activacion
sem_t sem_activacion;

//lista de archivos
t_list* lista_archivos;

//file descriptors de entradasalida y los modulos que se conectaran con el
int fd_entradasalida;
int fd_memoria;
int fd_kernel;
int fd_bloque;
int fd_bitmap;

//implementamos pid del proceso que se encuentra utilizando la io
int* pid_actual;

t_log* entradasalida_logger; //LOG ADICIONAL A LOS MINIMOS Y OBLIGATORIOS
t_config* entradasalida_config;

char* TIPO_INTERFAZ;
int TIEMPO_UNIDAD_TRABAJO;
char* IP_KERNEL;
char* PUERTO_KERNEL;
char* IP_MEMORIA;
char* PUERTO_MEMORIA;
char* PATH_BASE_DIALFS;
int BLOCK_SIZE;
int BLOCK_COUNT;
int RETRASO_COMPACTACION;

//en caso de tener un FS
void* bloques;
void* espacio_bit_map;

t_bitarray* bit_map;

void levantarArchivoDeBloques() {

	char* path_copia = malloc(strlen(PATH_BASE_DIALFS));
	strcpy(path_copia,PATH_BASE_DIALFS);
    strcat(path_copia, "/bloques.dat");

    printf("%s\n", path_copia);
    printf("Hola, entré acá\n");

    // Abre el archivo en modo lectura/escritura
	fd_bloque = open(path_copia, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
	// Aseguramos que el archivo tenga el tamaño adecuado
    if (ftruncate(fd_bloque, BLOCK_COUNT * BLOCK_SIZE) != 0) {
        perror("Error ajustando el tamaño del archivo");
        close(fd_bloque);
        exit(EXIT_FAILURE);
    }
	// Mapeamos el archivo en memoria
    bloques = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd_bloque, 0);
    if (bloques == MAP_FAILED) {
        perror("Error en mmap");
        close(fd_bloque);
        exit(EXIT_FAILURE);
    }
	
    printf("Archivo mapeado correctamente\n");
}

void levantarArchivoBitMap()
{
	char* path_copia = malloc(strlen(PATH_BASE_DIALFS));
	strcpy(path_copia,PATH_BASE_DIALFS);
    strcat(path_copia, "/bitmap.dat");

    printf("%s\n", path_copia);
    printf("Hola, entré acá\n");

    // Abre el archivo en modo lectura/escritura
	fd_bitmap = open(path_copia, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
	// Aseguramos que el archivo tenga el tamaño adecuado
    if (ftruncate(fd_bitmap, BLOCK_COUNT/8) != 0) {
        perror("Error ajustando el tamaño del archivo");
        exit(EXIT_FAILURE);
    }

	int tamanio_bit_map = BLOCK_COUNT/8;
	int* random = malloc(sizeof(int));

	// Mapeamos el archivo en memoria
    espacio_bit_map = mmap(NULL, tamanio_bit_map, PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (espacio_bit_map == MAP_FAILED) {
        perror("Error en mmap");
        exit(EXIT_FAILURE);
    }


	bit_map = bitarray_create_with_mode(random,tamanio_bit_map,LSB_FIRST);
    // Liberamos el mapeo y cerramos el archivo
    printf("Archivo mapeado correctamente\n");
}

void crear_archivo(char* nombre)
{
	char* path_copia = malloc(strlen(PATH_BASE_DIALFS));
	strcpy(path_copia,PATH_BASE_DIALFS);
	strcat(path_copia, "/");
    strcat(path_copia, nombre);

	int fd_random = open(path_copia, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	
	//Creamos el archivo y lo añadimos a la lista
	Archivo* archivo = malloc(sizeof(Archivo));
	archivo->path_length = strlen(path_copia)+1;
	archivo->path = path_copia;
	archivo->fd_archivo = fd_random;
	list_add(lista_archivos,archivo);

	//le asignamos un bloque:
	int i = 0;
	while( i < BLOCK_COUNT)
	{
		int value = bitarray_test_bit(bit_map,i); //obtenemos el valor actual de dicho bit array en la posición i
		if( value == 0 )
		{
			//si es cero, está libre, así que lo ocupamos
			bitarray_set_bit(bit_map, i);
		}
		i++;
	}

	//escribimos en el archivo los datos obtenidos
	char* to_write = "BLOQUE_INICIAL=\nTAMANIO_ARCHIVO=";
	void* mapeo = mmap(NULL,strlen(to_write)+1, PROT_WRITE, MAP_SHARED, fd_random, 0);
	ftruncate(fd_random, strlen(to_write)+1);
	memcpy(mapeo,to_write,strlen(mapeo)+1);
	msync(mapeo,strlen(mapeo)+1,MS_SYNC);
	//msync (void *__addr, size_t __len, int __flags);

	t_config* metadata = config_create(path_copia);
	char* first_block;
	sprintf(first_block,"%d",i);
	config_set_value(metadata,"BLOQUE_INICIAL",first_block);
	config_set_value(metadata,"TAMANIO_ARCHIVO","0");

	config_destroy(metadata);
	munmap(mapeo,strlen(to_write)+1);
}

void avisar_despertar_kernel()
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = 1;
	buffer->stream = malloc(buffer->size);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = DESPERTAR;
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
    send(fd_kernel, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void entradasalida_escuchar_memoria (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_memoria);
			printf("Codigo de operación de memoria recibido\n");

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			
			recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			if( cod_op == 14 ) //ojo con este hardcodeo, de añadir mas codigos de operacion pueden modificarse
			{
				recv(fd_memoria,&(paquete->buffer->offset), sizeof(uint32_t),0);
			}
			recv(fd_memoria,paquete->buffer->stream, paquete->buffer->size,0);
			switch (cod_op) {
			case STDOUT_TOPRINT:
				printf("Entramos acá adentro reyes\n");
				char* text_to_print = malloc(paquete->buffer->size);
				text_to_print = paquete->buffer->stream;
				printf("%s\n",text_to_print);
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				break;
			case DESPERTAR:
			//
				printf("Hola, entré a despertar\n");
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
			break;
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(entradasalida_logger, "Desconexion de memoria.");
				control_key = 0;
			default:
				log_warning(entradasalida_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void enviar_stdin_to_write_memoria(int* direccionFisica, int* tamanio,char* text)
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int)*2 + *tamanio + 1;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,text,*tamanio+1);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDIN_TOWRITE;
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

void new_enviar_stdin_to_write_memoria(int* direccionFisica, char* caracter)
{
	printf("ONE\n");
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int) + sizeof(char) + 1;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,caracter, sizeof(char));

	printf("TWO\n");
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDIN_TOWRITE;
	paquete->buffer = buffer;

	printf("THREE\n");
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

	printf("FOUR\n");
    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

void enviar_stdout_to_print_memoria(int* direccionFisica, int* tamanio)
{
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int)*2;
	buffer->stream = malloc(buffer->size);

	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDOUT_TOPRINT;
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

void new_enviar_stdout_to_print_memoria(t_list* direcciones_fisicas, int* tamanio)
{

	//Preparamos el buffer
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	buffer->offset = 0;
	buffer->size = sizeof(int)*(list_size(direcciones_fisicas)+1);
	buffer->stream = malloc(buffer->size);

	//vamos reservando la memoria
	memcpy(buffer->stream + buffer->offset,tamanio, sizeof(int));
    buffer->offset += sizeof(int);

	for(int i = 0; i < list_size(direcciones_fisicas); i++)
	{
		memcpy(buffer->stream + buffer->offset,list_get(direcciones_fisicas,i), sizeof(int));
    	buffer->offset += sizeof(int);
		int* df = list_get(direcciones_fisicas,i);
		printf("Mandaremos la siguiente direccion física: %d\n",*df);
	}

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDOUT_TOPRINT;
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

void entradasalida_escuchar_kernel (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			printf("Checkpoin1 \n");
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			paquete->buffer = malloc(sizeof(t_newBuffer));
			recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);	
			paquete->buffer->stream = malloc(paquete->buffer->size);
			recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);

			switch (cod_op) {
			case GENERICA:

				printf("Checkpoin2 \n");
				//hay que deserializar, dado que es solo un int
				int* unidadesDeTrabajo = malloc(sizeof(int));
				unidadesDeTrabajo = paquete->buffer->stream;

				printf("Voy a dormir, reyes, la cantidad de %d unidades de trabajo\n",*unidadesDeTrabajo);
				printf("Voy a dormir: %d\n", (*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO ));
				sleep((*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO)/1000);

				//DEVOLVER AL KERNEL PARA DESPERTAR
				//avisar_despertar_kernel();
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				break;
			case STDIN:
				// DESEMPAQUETAMOS
				int* tamanio_instruccion_in = malloc(sizeof(int));
				memcpy(tamanio_instruccion_in, paquete->buffer->stream, sizeof(int));
				char* instruccion_in = malloc(*tamanio_instruccion_in);
				memcpy(instruccion_in, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_in);

				// Mostremos por pantalla
				printf("La instruccion que ha llegado es: %s\n", instruccion_in);
				char** instruccion_partida_in = string_split(instruccion_in, " ");
				int tamanio_in = atoi(instruccion_partida_in[4]);
				printf("Tamaño de la instrucción: %d\n", tamanio_in);

				// Ingresamos el texto a escribir
				printf("Vamos a llevar a cabo STDIN\n");
				printf("Ingrese un texto por teclado: \n");
				char* leido_in;
				leido_in = readline(">> ");
				printf("Texto leído: %s\n", leido_in);

				// Debemos ir enviando poco a poco las direcciones físicas a memoria para que las vaya escribiendo
				// REFERENCIA // IO_STDIN_READ Int1 BX CX 12 15 16 17 18 19 20 21 22 23 24 25 26
				for (int i = 0; i < tamanio_in; i++) 
				{
					printf("ESTOY ITERANDO\n");
					int* df_to_send_in = malloc(sizeof(int)); //liberar
					*df_to_send_in = atoi(instruccion_partida_in[5 + i]);
					char* c_in = malloc(sizeof(char)); //liberar
					*c_in = leido_in[i];
					new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
					free(c_in);
					//free(df_to_send_in);
				}

				printf("ESTOY ITERANDO\n");
				int* df_to_send_in = malloc(sizeof(int)); //liberar
				*df_to_send_in = -1;
				char* c_in = malloc(sizeof(char)); //liberar
				*c_in = 'L';
				new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
				free(c_in);
				//free(df_to_send_in);
				free(tamanio_instruccion_in);
				free(instruccion_in);
				string_array_destroy(instruccion_partida_in);
				// new_enviar_stdin_to_write_memoria(direccionFisica, caracter);
				//despertamos
				//enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				break;
			case STDOUT:
				
				printf("CHECKPOINT DEL OUT 1\n");
				int* tamanio_instruccion_out = malloc(sizeof(int));
				memcpy(tamanio_instruccion_out, paquete->buffer->stream, sizeof(int));
				char* instruccion_out = malloc(*tamanio_instruccion_out);
				memcpy(instruccion_out, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_out);
				printf("Llegó el siguiente tamaño: %d\n",*tamanio_instruccion_out);
				printf("Llegó la siguiente instrucción: %s\n",instruccion_out);
				char** instrucciones_partidas_out = string_split(instruccion_out," ");

				printf("CHECKPOINT DEL OUT 2\n");
				//creamos una lista de las direcciones que vayamos obteniendo	
				t_list* lista_direcciones = list_create();
				int* tamanio_out = malloc(sizeof(int));
				*tamanio_out = atoi(instrucciones_partidas_out[4]);
				for(int i = 0; i < *tamanio_out; i++)
				{
					int* df_out = malloc(sizeof(int));
					*df_out = atoi(instrucciones_partidas_out[5+i]);
					list_add(lista_direcciones,df_out);
				}
				printf("CHECKPOINT DEL OUT 3\n");

				//una ves tengamos la lista hecha habra que empaquetarlo de alguna forma
				new_enviar_stdout_to_print_memoria(lista_direcciones,tamanio_out);

				printf("CHECKPOINT DEL OUT 3\n");
				//free(tamanio_instruccion_out);
				//free(instruccion_out);
				/*
				//DESEMPAQUETAMOS
				int* direccionFisicaOUT = malloc(sizeof(int));
				direccionFisicaOUT = paquete->buffer->stream;
				int* tamanioOUT = malloc(sizeof(int));
				tamanioOUT = paquete->buffer->stream + sizeof(int);
				printf("La direccion física que ha llegado es: %d\n",*direccionFisicaOUT);
				printf("La direccion física que ha llegado es: %d\n",*tamanioOUT);

				enviar_stdout_to_print_memoria(direccionFisicaOUT, tamanioOUT);
				*/
				break;
			case NUEVOPID:
				int* new_pid = paquete->buffer->stream;
				*pid_actual = *new_pid;
				printf("Esta interfaz está siendo usada actualmente por el proceso cuyo PID es %d\n",*pid_actual);
				break;
			case IO_FS_CREATE:
				// DESEMPAQUETAMOS
				int* tamanio_instruccion_fs = malloc(sizeof(int));
				memcpy(tamanio_instruccion_fs, paquete->buffer->stream, sizeof(int));
				char* instruccion_fs = malloc(*tamanio_instruccion_fs);
				memcpy(instruccion_fs, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs);
				char** instruccion_fs_partida = string_split(instruccion_fs," "); 

				// Mostremos por pantalla
				printf("La instruccion que ha llegado es: %s\n", instruccion_fs);
				crear_archivo(instruccion_fs_partida[2]);

				break;
			case MENSAJE:
				//
				break;
			case PAQUETE:
				//
				break;
			case -1:
				log_error(entradasalida_logger, "Desconexion de kernel.");
				control_key = 0;
			default:
				log_warning(entradasalida_logger,"Operacion desconocida. No quieras meter la pata");
				break;
			}
			free(paquete->buffer->stream);
			free(paquete->buffer);
			free(paquete);
		}	
}

void enviarDatos(int fd_servidor, char** datos, char* tipo_interfaz)
{
	//creamos el struct a enviar
	EntradaSalida* to_send = malloc(sizeof(EntradaSalida));
	to_send->fd_cliente = 0;
	to_send->nombre_length = strlen(datos[0])+1;
	to_send->nombre = malloc(to_send->nombre_length);
	to_send->nombre = datos[0];
	to_send->path_length = strlen(datos[1])+1;
	to_send->path = malloc(to_send->path_length);
	to_send->path = datos[1];

	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	//calculamos su tamaño
	buffer->size = sizeof(int) + sizeof(uint32_t)*2 + (to_send->nombre_length) + (to_send->path_length);
    buffer->offset = 0;
	buffer->stream = malloc(buffer->size);

	//movemos los valores al buffer
	memcpy(buffer->stream + buffer->offset, &to_send->fd_cliente, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);

	memcpy(buffer->stream + buffer->offset, &to_send->nombre_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, to_send->nombre,to_send->nombre_length);
	buffer->offset += to_send->nombre_length;
	
	memcpy(buffer->stream + buffer->offset, &to_send->path_length, sizeof(uint32_t));
	buffer->offset += sizeof(uint32_t);
	memcpy(buffer->stream + buffer->offset, to_send->path,to_send->path_length);//no seria (to_send->path_length+1) ???

	//creamos el paquete
	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));

	
	if( strcmp(tipo_interfaz,"GENERICA") == 0 )
	{

		paquete->codigo_operacion = GENERICA;
	}
	else
	{
		if( strcmp(tipo_interfaz,"STDIN") == 0 )
		{
			paquete->codigo_operacion = STDIN;
		}
		else
		{
			if( strcmp(tipo_interfaz,"STDOUT") == 0 )
			{
				paquete->codigo_operacion = STDOUT;
			}
			else
			{
				if( strcmp(tipo_interfaz,"DIALFS") == 0 )
				{
					paquete->codigo_operacion = DIALFS;
				}
				else
				{
					paquete->codigo_operacion = WRONG;
				}
			}
		}
	}
	
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
    send(fd_servidor, a_enviar, buffer->size + sizeof(op_code) + sizeof(uint32_t), 0);

    // No nos olvidamos de liberar la memoria que ya no usaremos
    //free(to_send);
	free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);	
}

#endif /* ENTRADASALIDA_H_ */