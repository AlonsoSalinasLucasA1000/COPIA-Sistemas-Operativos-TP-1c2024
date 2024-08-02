#ifndef ENTRADASALIDA_H_
#define ENTRADASALIDA_H_

#include<stdio.h>
#include<stdlib.h>
#include<utils/utils.h>
#include <utils/utils.c>

//semaforo de activacion
sem_t sem_activacion1;
sem_t sem_activacion2;

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

char* dialfs_to_write;
char* path_bloques;

Archivo* elemento_especial; //sirve para que, a la hora de ordenar, dicho archivo quede al final (por la compactación)

void inicializar_path_bloques(const char* PATH_BASE_DIALFS)
{
	path_bloques = malloc(strlen(PATH_BASE_DIALFS) + strlen("/bloques.dat") + 1);
    if (path_bloques == NULL) {
        perror("Error al asignar memoria para path_bloques");
        exit(EXIT_FAILURE);
    }
    strcpy(path_bloques, PATH_BASE_DIALFS);
    strcat(path_bloques, "/bloques.dat");
}

void new_enviar_stdin_to_write_memoria(int* direccionFisica, char* caracter)
{
	
	// Verificamos si los punteros son válidos
    if (direccionFisica == NULL || caracter == NULL) {
        fprintf(stderr, "Error: punteros de entrada (new_enviar_stdin_to_write_memoria) son NULL.\n");
        return;
    }
	
	t_newBuffer* buffer = malloc(sizeof(t_newBuffer));
	if (buffer == NULL) {
        perror("malloc");
        return;
    }

	buffer->offset = 0;
	buffer->size = sizeof(int) + sizeof(char) + 1;
	buffer->stream = malloc(buffer->size);
	 if (buffer->stream == NULL) {
        perror("malloc");
        free(buffer);
        return;
    }
	//copiamos los datos al buffer
	memcpy(buffer->stream + buffer->offset,direccionFisica, sizeof(int));
    buffer->offset += sizeof(int);
	memcpy(buffer->stream + buffer->offset,caracter, sizeof(char));

	t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
	if (paquete == NULL) {
        perror("malloc");
        free(buffer->stream);
        free(buffer);
        return;
    }

    //Podemos usar una constante por operación
    paquete->codigo_operacion = STDIN_TOWRITE;
	paquete->buffer = buffer;

	//Empaquetamos el Buffer
    void* a_enviar = malloc(buffer->size + sizeof(op_code) + sizeof(uint32_t));
	if (a_enviar == NULL) {
        perror("malloc");
        free(paquete->buffer->stream);
        free(paquete->buffer);
        free(paquete);
        return;
    }

	//printf("Enviaremos la siguiente direccion fisica: %d y el siguiente caracter: %c\n",*direccionFisica,*caracter);

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

void new_enviar_stdout_to_print_memoria(t_list* direcciones_fisicas, int* tamanio, op_code codigo_operacion)
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
		//printf("Mandaremos la siguiente direccion física: %d\n",*df);
	}

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

    // No nos olvidamos de liberar la memoria que ya no usaremos
    free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);
}

/*
void recuperarArchivos(const char* path, t_list* lista_archivos) {
    DIR *dir;
    struct dirent *ent;

    // Verifica si la lista es nula
    if (lista_archivos == NULL) {
        fprintf(stderr, "La lista proporcionada es nula.\n");
        return;
    }

    // Abre el directorio
    if ((dir = opendir(path)) != NULL) {
        // Lee cada entrada en el directorio
        while ((ent = readdir(dir)) != NULL) {
            // Omite "." y ".."
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                // Omite "bloques.dat" y "bitmap.dat"
                if (strcmp(ent->d_name, "bloques.dat") != 0 && strcmp(ent->d_name, "bitmap.dat") != 0) {
                    // Construye el path completo
                    size_t path_len = strlen(path);
                    size_t name_len = strlen(ent->d_name);
                    char *path_completo = malloc(path_len + name_len + 2); // +2 para '/' y '\0'
                    
                    if (path_completo == NULL) {
                        perror("Error al asignar memoria");
                        closedir(dir);
                        return; // Salir sin liberar la lista ya que puede que haya elementos válidos
                    }

                    // Construye el path completo
                    snprintf(path_completo, path_len + name_len + 2, "%s/%s", path, ent->d_name);

                    // Crea una nueva estructura Archivo
                    Archivo *archivo = malloc(sizeof(Archivo));
                    if (archivo == NULL) {
                        perror("Error al asignar memoria para Archivo");
                        free(path_completo); // Libera el path si ocurre un error
                        closedir(dir);
                        return; // Salir sin liberar la lista ya que puede que haya elementos válidos
                    }

                    archivo->path_length = name_len;
                    archivo->path = path_completo;
                    archivo->fd_archivo = -1; // No se abre el descriptor de archivo aquí
					//abrimos el config y guardamos el bloque inicial

					t_config* config = config_create(archivo->path);
					int bloque_inicial = config_get_int_value(config,"BLOQUE_INICIAL");

					archivo->bloque_inicial = bloque_inicial;

					config_destroy(config);

                    // Añade la estructura Archivo a la lista
                    list_add(lista_archivos, archivo);
                }
            }
        }
        closedir(dir);
    } else {
        // No se pudo abrir el directorio
        perror("Error al abrir el directorio");
    }
}
*/

void recuperarArchivos(const char* path, t_list* lista_archivos) {
    DIR *dir;
    struct dirent *ent;

    // Verifica si la lista es nula
    if (lista_archivos == NULL) {
        fprintf(stderr, "La lista proporcionada es nula.\n");
        return;
    }

    // Abre el directorio
    if ((dir = opendir(path)) != NULL) {
        // Lee cada entrada en el directorio
        while ((ent = readdir(dir)) != NULL) {
            // Omite "." y ".."
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
                // Omite "bloques.dat" y "bitmap.dat"
                if (strcmp(ent->d_name, "bloques.dat") != 0 && strcmp(ent->d_name, "bitmap.dat") != 0) {
                    // Construye el path completo
                    size_t path_len = strlen(path);
                    size_t name_len = strlen(ent->d_name);
                    char *path_completo = malloc(path_len + name_len + 2); // +2 para '/' y '\0'

                    if (path_completo == NULL) {
                        perror("Error al asignar memoria");
                        closedir(dir);
                        return; // Salir sin liberar la lista ya que puede que haya elementos válidos
                    }

                    // Construye el path completo
                    snprintf(path_completo, path_len + name_len + 2, "%s/%s", path, ent->d_name);

                    // Crea una nueva estructura Archivo
                    Archivo *archivo = malloc(sizeof(Archivo));
                    if (archivo == NULL) {
                        perror("Error al asignar memoria para Archivo");
                        free(path_completo); // Libera el path si ocurre un error
                        closedir(dir);
                        return; // Salir sin liberar la lista ya que puede que haya elementos válidos
                    }

                    archivo->path_length = name_len;
                    archivo->path = path_completo;
                    archivo->fd_archivo = -1; // No se abre el descriptor de archivo aquí

                    // Abre el config y guarda el bloque inicial
                    t_config* config = config_create(archivo->path);
                    if (config == NULL) {
                        perror("Error al crear el archivo de configuración");
                        free(path_completo); // Libera el path si ocurre un error
                        free(archivo);       // Libera el archivo si ocurre un error
                        closedir(dir);
                        return; // Salir sin liberar la lista ya que puede que haya elementos válidos
                    }

                    int bloque_inicial = config_get_int_value(config, "BLOQUE_INICIAL");
                    archivo->bloque_inicial = bloque_inicial;

                    config_destroy(config);

                    // Añade la estructura Archivo a la lista
                    list_add(lista_archivos, archivo);
                }
            }
        }
        closedir(dir);
    } else {
        // No se pudo abrir el directorio
        perror("Error al abrir el directorio");
    }
}


void levantarArchivoDeBloques() {

	char* path_copia = malloc(strlen(PATH_BASE_DIALFS) + strlen("/bloques.dat") + 1);
	strcpy(path_copia,PATH_BASE_DIALFS);
    strcat(path_copia, "/bloques.dat");

    printf("%s\n", path_copia);
   // printf("Hola, entré acá\n");

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
	
	close(fd_bloque);
	munmap(bloques,BLOCK_COUNT * BLOCK_SIZE);
	free(path_copia);
    printf("Archivo mapeado correctamente\n");
}

void levantarArchivoBitMap() {
    // Asignamos memoria suficiente para path_copia
    char* path_copia = malloc(strlen(PATH_BASE_DIALFS) + strlen("/bitmap.dat") + 1);
    if (path_copia == NULL) {
        perror("Error al asignar memoria");
        exit(EXIT_FAILURE);
    }
    strcpy(path_copia, PATH_BASE_DIALFS);
    strcat(path_copia, "/bitmap.dat");

    printf("%s\n", path_copia);
    //printf("Hola, entré acá\n");

    // Abre el archivo en modo lectura/escritura
    fd_bitmap = open(path_copia, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bitmap == -1) {
        perror("Error al abrir el archivo");
        free(path_copia);
        exit(EXIT_FAILURE);
    }

    // Aseguramos que el archivo tenga el tamaño adecuado
    int tamanio_bit_map = BLOCK_COUNT / 8;
    if (ftruncate(fd_bitmap, tamanio_bit_map) != 0) {
        perror("Error ajustando el tamaño del archivo");
        close(fd_bitmap);
        free(path_copia);
        exit(EXIT_FAILURE);
    }

    // Mapeamos el archivo en memoria
    espacio_bit_map = mmap(NULL, tamanio_bit_map, PROT_WRITE, MAP_SHARED, fd_bitmap, 0);
    if (espacio_bit_map == MAP_FAILED) {
        perror("Error en mmap");
        close(fd_bitmap);
        free(path_copia);
        exit(EXIT_FAILURE);
    }

    // Creamos el bitarray
    bit_map = bitarray_create_with_mode(espacio_bit_map, tamanio_bit_map, LSB_FIRST);

	//de cumplirse esta condición, significa que el FS es nuevo
	if( list_size(lista_archivos) == 0 )
	{
		for(int i = 0; i < tamanio_bit_map*8; i++)
		{
			bitarray_clean_bit(bit_map,i);
		}
		if (bit_map == NULL) {
			perror("Error creando el bitarray");
			munmap(espacio_bit_map, tamanio_bit_map);
			close(fd_bitmap);
			free(path_copia);
			exit(EXIT_FAILURE);
		}
	}

    printf("Archivo mapeado correctamente\n");

    // Liberamos la memoria asignada para path_copia
    free(path_copia);
}

Archivo* encontrar_archivo(t_list* lista_archivos, char* nombre)
{
	Archivo* to_ret;
	for(int i = 0; i < list_size(lista_archivos); i++)
	{
		Archivo* a = list_get(lista_archivos,i);
		char** path_partido = string_split(a->path,"/");
		if( strcmp(path_partido[4], nombre) == 0 )
		{
			to_ret = a;
			string_array_destroy(path_partido);
			return to_ret;
		}
		string_array_destroy(path_partido);
	}
	to_ret = NULL;
	return to_ret;
}

void crear_archivo(char* nombre)
{
	Archivo* a = encontrar_archivo(lista_archivos, nombre);
	//De no existir el archivo, lo creamos, de lo contrario no hacemos nada
	if( a == NULL)
	{
		char* path_copia = malloc(strlen(PATH_BASE_DIALFS) + 1 + strlen(nombre) + 1);
		strcpy(path_copia, PATH_BASE_DIALFS);
		strcat(path_copia, "/");
		strcat(path_copia, nombre);

		int fd_random = open(path_copia, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
		
		//Creamos el archivo y lo añadimos a la lista
		Archivo* archivo = malloc(sizeof(Archivo));
		archivo->path_length = strlen(path_copia) + 1;
		archivo->path = malloc(archivo->path_length);
		strcpy(archivo->path, path_copia);
		archivo->fd_archivo = fd_random;
		

		//le asignamos un bloque:
		int i = 0;
		bool finished = false;
		while( i < BLOCK_COUNT && finished != true)
		{
			int value = bitarray_test_bit(bit_map,i); //obtenemos el valor actual de dicho bit array en la posición i
			//printf("El valor actual es: %d\n",value);
			if( value == 0 )
			{
				//si es cero, está libre, así que lo ocupamos y actualizamos el archivo de bitmap
				bitarray_set_bit(bit_map, i);
				/*
				memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
				*/
				msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
				finished = true;
				i--;
			}
			//printf("valor de i es: %d\n",i);
			i++;
		}

		archivo->bloque_inicial = i;
		list_add(lista_archivos, archivo);
		msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
		//escribimos en el archivo los datos obtenidos
		char first_block[12]; // 12 es suficiente para almacenar cualquier entero de 32 bits
		sprintf(first_block, "%d", i);
		char to_write[256]; // Ajusta el tamaño según sea necesario
		snprintf(to_write, sizeof(to_write), "BLOQUE_INICIAL=%s\nTAMANIO_ARCHIVO=0\n", first_block);
		ftruncate(fd_random, strlen(to_write)+1);
		void* mapeo = mmap(NULL,strlen(to_write)+1, PROT_WRITE, MAP_SHARED, fd_random, 0);
		memcpy(mapeo,to_write,strlen(to_write)+1);
		msync(mapeo,strlen(mapeo)+1,MS_SYNC);
		close(fd_random); //QUIZÁ NOS CAUSE PROBLEMAS - MIRAR LAS PRUEBAS MÁS TARDE 30/7 19:50
		munmap(mapeo,strlen(to_write)+1);
		free(path_copia);
	}
}

bool esPosibleTruncar(int base, int cantidad)
{
    //printf("La base encontrada es: %d\n", base);
    //printf("La cantidad a aumentar es: %d\n", cantidad);
    //printf("Mostremos los valores que tenemos en el bitmap\n");
   // printf("El tamanio que posee es: %d\n", bit_map->size);
	int cant_bloques = (int)ceil((double)cantidad / BLOCK_SIZE);

    for (int i = base + 1; i < cant_bloques + base; i++)
    {
        // Verificar si 'i' excede el tamaño del bitmap
        if (i >= bit_map->size * 8)
        {
            //printf("El valor de i es: %d\n", i);
            //printf("i ha excedido el tamaño del bitmap\n");
            return false;
        }

        int value = bitarray_test_bit(bit_map, i);
        //printf("El valor de i es: %d\n", i);
        //printf("El valor del bit es %d\n", value);

        if (value != 0)
        {
            return false;
        }
    }

    return true;
}

int encontrar_bloques_continuos(t_bitarray* bit_array, size_t desired_length) {
    size_t max_bit = bitarray_get_max_bit(bit_array);
    size_t consecutive_free_blocks = 0;

    for (size_t i = 0; i < max_bit; ++i) {
        if (!bitarray_test_bit(bit_array, i)) {
            // El bit actual es 0 (bloque libre)
            ++consecutive_free_blocks;
            if (consecutive_free_blocks == desired_length) {
                // Encontramos una secuencia de bloques libres
                return i - desired_length + 1;
            }
        } else {
            // Reiniciamos la cuenta si encontramos un bloque ocupado
            consecutive_free_blocks = 0;
        }
    }

    // No se encontró ninguna secuencia adecuada
    return -1;
}

bool hay_bloques_libres(t_bitarray* bitarray, int cantidad_bloques) {
    int contador_libres = 0;
    int tamanio = bitarray_get_max_bit(bitarray);

    for (int i = 0; i < tamanio; i++) {
        if (!bitarray_test_bit(bitarray, i)) {
            contador_libres++;
        }
        if (contador_libres >= cantidad_bloques) {
            return true;
        }
    }
    return false;
}

// Función de comparación
bool comparar_bloque_inicial(void* a, void* b) {
    Archivo* archivo_a = (Archivo*)a;
    Archivo* archivo_b = (Archivo*)b;

    // Si uno de los elementos es el elemento especial, el especial siempre va al final
    if (archivo_a == elemento_especial) return false;
    if (archivo_b == elemento_especial) return true;

    return archivo_a->bloque_inicial < archivo_b->bloque_inicial;
}


void compactacion_a(Archivo* archivo, t_config* archivo_config, int nuevo_tamanio)
{
	printf("Se compactará el FS para truncar al archivo %s\n",archivo->path);
	int bloque_base = config_get_int_value(archivo_config,"BLOQUE_INICIAL");
	int tamanio_archivo = config_get_int_value(archivo_config,"TAMANIO_ARCHIVO");
	int cant_bloques = (int)ceil((double)tamanio_archivo / BLOCK_SIZE);
	elemento_especial = archivo;

	//mapeamos bloques.dat
	fd_bloque = open(path_bloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

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

	//nos guardamos de forma auxiliar lo que estaba en ese archivo
	void* aux = malloc(cant_bloques*BLOCK_SIZE);
	memmove(aux,bloques + bloque_base*BLOCK_SIZE,tamanio_archivo);

	//liberamos los bits
	for(int i = bloque_base; i < cant_bloques; i++)
	{
		int value = bitarray_test_bit(bit_map,i);
		printf("El valor del bit es %d\n",value);
		if( value == 1 )
		{
			bitarray_clean_bit(bit_map,i);
		}
		msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
	}

	//ya tenemos guardado el archivo de forma auxiliar, debemos ordenar los archivos de nuestro FS según donde se encuentre su bloque inicial
	list_sort(lista_archivos,comparar_bloque_inicial);
	
	//Teniendo la lista ordenada es momento de COMPACTAR
	int pointer = 0; //puntero dentro del archivo
	for(int i = 0; i < list_size(lista_archivos); i++)
	{
		printf("DialFS - Inicio Compactación: PID %d",pid_actual);
		Archivo* got = list_get(lista_archivos,i);
		t_config* config_got = config_create(got->path);
		int bloque_base_got = config_get_int_value(config_got,"BLOQUE_INICIAL");
		int tamanio_got = config_get_int_value(config_got,"TAMANIO_ARCHIVO");
		int cant_bloques_got = (int)ceil((double)tamanio_got / BLOCK_SIZE); 

		memmove(bloques + pointer, bloque_base_got + tamanio_got*BLOCK_SIZE, cant_bloques_got*BLOCK_SIZE); //copiamos los bytes de la cantidad de bloques, llevandonos la fragmentación interna
		//actualizamos el bloque_base del config
		int new_base = pointer/BLOCK_SIZE;
		char base_string_got[32];
		sprintf(base_string_got,"%d",new_base);
		config_set_value(config_got,"BLOQUE_INICIAL",base_string_got);
		config_save_in_file(config_got,archivo->path);
		archivo->bloque_inicial = new_base;
		

		//liberamos los bits anteriores
		for(int i = bloque_base_got; i < cant_bloques_got; i++)
		{
			int value = bitarray_test_bit(bit_map,i);
			printf("El valor del bit es %d\n",value);
			if( value == 1 )
			{
				bitarray_clean_bit(bit_map,i);
			}
			msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
		}

		//ocupamos los nuevos
		for(int i = new_base; i < cant_bloques_got; i++)
		{
			int value = bitarray_test_bit(bit_map,i);
			printf("El valor del bit es %d\n",value);
			bitarray_set_bit(bit_map,i);
			msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
		}

		pointer = pointer + cant_bloques_got*BLOCK_SIZE;

		int nueva_base_compactado = encontrar_bloques_continuos(bit_map,cant_bloques);
		if( nueva_base_compactado != (-1))
		{
			//encontramos una nueva secuencia de bloques libres, actualizamos bloque base y asignamos tamaño
			printf("Hemos encontrado una secuencia de bloques libres, podemos asignar\n");

			//antes liberamos el bloque que poseía anteriormente
			//bitarray_clean_bit(bit_map,base);

			//llevamos a cabo la asignación
			for(int i = nueva_base_compactado; i < cant_bloques+nueva_base_compactado; i++)
			{
				int value = bitarray_test_bit(bit_map,i);
				printf("El valor del bit es %d\n",value);
				if( value == 0 )
				{
					bitarray_set_bit(bit_map,i);
				}
				//guardamos
				/*
				memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
				*/
				msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
				
				//escribimos en el config el nuevo valor
			}

			//actualizamos la base
			char base_string[32];
			sprintf(base_string,"%d",nueva_base_compactado);
			config_set_value(archivo_config,"BLOQUE_INICIAL",base_string);
			config_save_in_file(archivo_config,archivo->path);
			archivo->bloque_inicial = nueva_base_compactado;

			//actualizamos el tamanio
			char cantidad_string[32];
			sprintf(cantidad_string,"%d",nuevo_tamanio);
			config_set_value(archivo_config,"TAMANIO_ARCHIVO",cantidad_string);
			config_save_in_file(archivo_config,archivo->path);
			printf("DialFS - Fin Compactación: PID %d",pid_actual);
			
			//cerramos todo
			memmove(bloques + pointer, aux, cant_bloques*BLOCK_SIZE);	
			msync(bloques,BLOCK_COUNT * BLOCK_SIZE,MS_SYNC);
			close(fd_bloque);
			munmap(bloques,BLOCK_COUNT * BLOCK_SIZE);
			config_destroy(config_got);
			free(aux);
			return;
		}
		config_destroy(config_got);
	}
}

void compactacion(Archivo* archivo, t_config* archivo_config, int nuevo_tamanio)
{
    //printf("Se compactará el FS para truncar al archivo %s\n", archivo->path);
    int bloque_base = config_get_int_value(archivo_config, "BLOQUE_INICIAL");
    int tamanio_archivo = config_get_int_value(archivo_config, "TAMANIO_ARCHIVO");
    int cant_bloques = (int)ceil((double)tamanio_archivo / BLOCK_SIZE);
    elemento_especial = archivo;

    // Mapeamos bloques.dat
    int fd_bloque = open(path_bloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd_bloque, BLOCK_COUNT * BLOCK_SIZE) != 0) {
        perror("Error ajustando el tamaño del archivo");
        close(fd_bloque);
        exit(EXIT_FAILURE);
    }

    void* bloques = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd_bloque, 0);
    if (bloques == MAP_FAILED) {
        perror("Error en mmap");
        close(fd_bloque);
        exit(EXIT_FAILURE);
    }

    // Guardamos de forma auxiliar lo que estaba en ese archivo
    void* aux = malloc(cant_bloques*BLOCK_SIZE);
    memmove(aux, bloques + bloque_base * BLOCK_SIZE, tamanio_archivo);

    // Liberamos los bits
    for (int i = bloque_base; i < bloque_base + cant_bloques; i++) {
        if (bitarray_test_bit(bit_map, i)) {
            bitarray_clean_bit(bit_map, i);
        }
    }
    msync(espacio_bit_map, BLOCK_COUNT / 8, MS_SYNC);

    // Ordenamos la lista de archivos
    list_sort(lista_archivos, comparar_bloque_inicial);

    // Compactamos
	log_info(entradasalida_logger,"PID: <%d> - Inicio Compactación.",*pid_actual);
    int pointer = 0;
    for (int i = 0; i < list_size(lista_archivos)-1; i++) 
	{
        Archivo* got = list_get(lista_archivos, i);
        t_config* config_got = config_create(got->path);
        int bloque_base_got = config_get_int_value(config_got, "BLOQUE_INICIAL");
        int tamanio_got = config_get_int_value(config_got, "TAMANIO_ARCHIVO");
        int cant_bloques_got = (int)ceil((double)tamanio_got / BLOCK_SIZE);

        memmove(bloques + pointer, bloques + bloque_base_got * BLOCK_SIZE, cant_bloques_got * BLOCK_SIZE);

        // Actualizamos el bloque_base del config
        int new_base = pointer / BLOCK_SIZE;
		printf("La nueba base es %d\n",new_base);
        char base_string_got[32];
        sprintf(base_string_got, "%d", new_base);
        config_set_value(config_got, "BLOQUE_INICIAL", base_string_got);
        config_save_in_file(config_got, got->path);
        got->bloque_inicial = new_base;

        // Liberamos los bits anteriores
        for (int j = bloque_base_got; j < bloque_base_got + cant_bloques_got; j++) {
            bitarray_clean_bit(bit_map, j);
			msync(espacio_bit_map, BLOCK_COUNT / 8, MS_SYNC);
        }

        // Ocupamos los nuevos
        for (int j = new_base; j < new_base + cant_bloques_got; j++) {
            bitarray_set_bit(bit_map, j);
			msync(espacio_bit_map, BLOCK_COUNT / 8, MS_SYNC);
        }

        pointer += cant_bloques_got * BLOCK_SIZE;

        config_destroy(config_got);
    }

	usleep(RETRASO_COMPACTACION*1000);
	log_info(entradasalida_logger,"PID: <%d> - Fin Compactación.",*pid_actual);
	

    int nueva_base_compactado = encontrar_bloques_continuos(bit_map, cant_bloques);
	//printf("La nueva base encontrada es: %d\n",nueva_base_compactado);
    if (nueva_base_compactado != -1) 
	{
        //printf("Hemos encontrado una secuencia de bloques libres, podemos asignar\n");

        // Asignamos los nuevos bloques
        for (int i = nueva_base_compactado; i < nueva_base_compactado + cant_bloques; i++) {
            if (!bitarray_test_bit(bit_map, i)) {
                bitarray_set_bit(bit_map, i);
            }
        }
        msync(espacio_bit_map, BLOCK_COUNT / 8, MS_SYNC);

        // Actualizamos la base y tamaño en el archivo de configuración
        char base_string[32];
        sprintf(base_string, "%d", nueva_base_compactado);
        config_set_value(archivo_config, "BLOQUE_INICIAL", base_string);
        config_save_in_file(archivo_config, archivo->path);
        archivo->bloque_inicial = nueva_base_compactado;

        char cantidad_string[32];
        sprintf(cantidad_string, "%d", nuevo_tamanio);
        config_set_value(archivo_config, "TAMANIO_ARCHIVO", cantidad_string);
        config_save_in_file(archivo_config, archivo->path);
        //printf("DialFS - Fin Compactación: PID %d\n", *pid_actual);

        // Movemos los datos auxiliares al nuevo espacio
        memmove(bloques + nueva_base_compactado * BLOCK_SIZE, aux, tamanio_archivo);
        msync(bloques, BLOCK_COUNT * BLOCK_SIZE, MS_SYNC);
    }

    // Liberamos memoria y cerramos todo
    free(aux);
    close(fd_bloque);
    munmap(bloques, BLOCK_COUNT * BLOCK_SIZE);
}



void truncarArchivo(char* nombre, int cantidad)
{
	//Primero consideremos un "truncado" que aumente el tamanio del archivo
	//debemos obtener el archivo y su bloque base
	Archivo* archivo = encontrar_archivo(lista_archivos, nombre);
	t_config* metadata_archivo = config_create(archivo->path);
	int base = config_get_int_value(metadata_archivo,"BLOQUE_INICIAL");
	int tamanio = config_get_int_value(metadata_archivo,"TAMANIO_ARCHIVO");
	int cant_bloques = (int)ceil((double)cantidad / BLOCK_SIZE);
	//printf("Para el archivo: %s, necesitamos la cantidad de bloques de %d\n",nombre,cant_bloques);

	if( cantidad > tamanio )
	{
		//se busca aumentar
		if( esPosibleTruncar(base,cantidad) )
		{
			//realizamos la asignación
			//printf("Hay espacio suficiente, PODEMOS TRUNCAR\n");
			//printf("La base es la siguiente: %d\n",base);
			//printf("La cantidad de bloque es %d\n",cant_bloques);
			//se lleva a cabo la asignación
			for(int i = base+1; i < cant_bloques+base; i++)
			{
				int value = bitarray_test_bit(bit_map,i);
				//printf("El valor del bit es %d\n",value);
				if( value == 0 )
				{
					bitarray_set_bit(bit_map,i);
				}
				//guardamos
				/*
				memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
				*/
				msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
				
				//escribimos en el config el nuevo valor
			}
			char cantidad_string[32];
			sprintf(cantidad_string,"%d",cantidad);
			config_set_value(metadata_archivo,"TAMANIO_ARCHIVO",cantidad_string);
			config_save_in_file(metadata_archivo,archivo->path);
		}
		else
		{
			//introducir lógica de compactación
			//printf("Desafortunadamente no podemos truncar de forma continua\n");
			//Debemos introducir otro if para determinar si hay espacio suficiente
			//printf("Verificaremos si hay, en alguna parte del espacio de memoria, alguna secuencia de bloques libres de forma continua\n");
			int new_base = encontrar_bloques_continuos(bit_map,cant_bloques);
			if( new_base != (-1))
			{
				//encontramos una nueva secuencia de bloques libres, actualizamos bloque base y asignamos tamaño
				//printf("Hemos encontrado una secuencia de bloques libres, podemos asignar\n");

				//antes liberamos el bloque que poseía anteriormente
				bitarray_clean_bit(bit_map,base);

				//llevamos a cabo la asignación
				for(int i = new_base; i < cant_bloques+new_base; i++)
				{
					int value = bitarray_test_bit(bit_map,i);
					//printf("El valor del bit es %d\n",value);
					if( value == 0 )
					{
						bitarray_set_bit(bit_map,i);
					}
					//guardamos
					/*
					memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
					*/
					msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
					
					//escribimos en el config el nuevo valor
				}

				//actualizamos la base
				char base_string[32];
				sprintf(base_string,"%d",new_base);
				config_set_value(metadata_archivo,"BLOQUE_INICIAL",base_string);
				config_save_in_file(metadata_archivo,archivo->path);
				archivo->bloque_inicial = new_base;

				//actualizamos el tamanio
				char cantidad_string[32];
				sprintf(cantidad_string,"%d",cantidad);
				config_set_value(metadata_archivo,"TAMANIO_ARCHIVO",cantidad_string);
				config_save_in_file(metadata_archivo,archivo->path);
			}
			else
			{
				//llevar a cabo compactación
				//antes de llevar a cabo la compactación debemos ver si siquiera existe la cantidad de bloques libres 
				if( hay_bloques_libres(bit_map,cant_bloques) )
				{
					//printf("Existe la cantidad de bloques libres, pero no se encuentran de forma continua. Debemos llevar a cabo la compactación\n");
					compactacion(archivo,metadata_archivo,tamanio);
				}
				else
				{
					printf("No hay espacio suficiente, lo siento\n");
				}
				
			}
		}
	}
	else
	{
		//se busca reducir el tamanio
		//printf("Se busca reducir el archivo %s\n", nombre);
		for(int i = base+1; i < cant_bloques+base; i++)
		{
			int value = bitarray_test_bit(bit_map,i);
			//printf("El valor del bit es %d\n",value);
			if( value == 1 )
			{
				bitarray_clean_bit(bit_map,i);
			}
			//guardamos
			/*
			memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
			*/
			msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
			
			//escribimos en el config el nuevo valor
		}
		char cantidad_string[32];
		sprintf(cantidad_string,"%d",cantidad+base);
		config_set_value(metadata_archivo,"TAMANIO_ARCHIVO",cantidad_string);
		config_save_in_file(metadata_archivo,archivo->path);
	}
	config_destroy(metadata_archivo);
}

void escribirArchivo(char** instruccion)
{
	//interpretamos lo datos
	//IO_FS_WRITE FS Goku.config AX ECX EDX 0 14 11 12 13 14 15 16 17 18 19 20 21 22 23 24
	Archivo* archivo = encontrar_archivo(lista_archivos,instruccion[2]);
	t_config* config_archivo = config_create(archivo->path);

//puntero del archivo
	int pointer = atoi(instruccion[6]);
//cantidad de direcciones físicas
	int cant_df = atoi(instruccion[7]);
	
	t_list* lista_direcciones = list_create();
	for(int i = 0; i < cant_df; i++)
	{
		int* df_out = malloc(sizeof(int));
		*df_out = atoi(instruccion[8+i]);
		list_add(lista_direcciones,df_out);
	}

	int* tamanio_fs = malloc(sizeof(int));
	*tamanio_fs = cant_df;
	new_enviar_stdout_to_print_memoria(lista_direcciones,tamanio_fs,IO_FS_WRITE);
	free(tamanio_fs);
	list_clean_and_destroy_elements(lista_direcciones,free);
	list_destroy(lista_direcciones);
	//printf("ESTOY ITERANDO\n");

	sem_wait(&sem_activacion1);
	char* a_escribir_en_archivo = malloc(strlen(dialfs_to_write)+1);
	strcpy(a_escribir_en_archivo,dialfs_to_write);
	free(dialfs_to_write);
	sem_post(&sem_activacion2);
	//llegamos acá y tenemos
	printf("%s\n",a_escribir_en_archivo);
	//printf("No se qué más hacer jajajaj\n");


	fd_bloque = open(path_bloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque == -1) {
        perror("Error al abrir el archivo");
        exit(EXIT_FAILURE);
    }
	
	int bloque_base = config_get_int_value(config_archivo,"BLOQUE_INICIAL");
	int tamanio = config_get_int_value(config_archivo,"TAMANIO_ARCHIVO");
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

	memmove(bloques + bloque_base*BLOCK_SIZE + pointer, a_escribir_en_archivo, cant_df);	
	msync(bloques,BLOCK_COUNT * BLOCK_SIZE,MS_SYNC);
	close(fd_bloque);
	munmap(bloques,BLOCK_COUNT * BLOCK_SIZE);
	config_destroy(config_archivo);
	free(a_escribir_en_archivo);
}

/*
void liberarElementoLista(t_list* lista_archivos, char* name)
{
	for(int i = 0; i < list_size(lista_archivos); i++)
	{
		Archivo* a = list_get(lista_archivos,i);
		char** path_partido = string_split(a->path,"/");
		if( strcmp(path_partido[4], name) == 0 )
		{
			list_remove_and_destroy_element(lista_archivos,i,free);
		}
		string_array_destroy(path_partido);
	}
}
*/

void liberarElementoLista(t_list* lista_archivos, char* name)
{
    for(int i = 0; i < list_size(lista_archivos); )
    {
        Archivo* a = list_get(lista_archivos, i);
        char** path_partido = string_split(a->path, "/");

        // Compara el nombre para encontrar el archivo a eliminar
        if (strcmp(path_partido[4], name) == 0)
        {
            // Elimina el elemento y libera la memoria asociada
            free(a->path); // Libera el path
            free(a); // Libera la estructura Archivo

            // Elimina el elemento de la lista y libera la memoria asociada
            list_remove(lista_archivos,i);
        }
        else
        {
            // Solo incrementa el índice si no se eliminó el elemento
            i++;
        }

        // Libera el array de strings
        string_array_destroy(path_partido);
    }
}

void eliminarArchivo(char** instruccion_fs_partida_delete)
{
	//Ejemplo de cómo luce
	//IO_FS_DELETE Int4 notas.txt
	Archivo* archivo = encontrar_archivo(lista_archivos,instruccion_fs_partida_delete[2]);
	t_config* config_archivo = config_create(archivo->path);

	int base = config_get_int_value(config_archivo,"BLOQUE_INICIAL");
	int tamanio = config_get_int_value(config_archivo,"TAMANIO_ARCHIVO");
	int cant_bloques = (int)ceil((double)tamanio / BLOCK_SIZE);

	//liberamos aquellos bits que utilizaba
	for(int i = base; i < cant_bloques; i++)
	{
		int value = bitarray_test_bit(bit_map,i);
		//printf("El valor del bit es %d\n",value);
		if( value == 1 )
		{
			bitarray_clean_bit(bit_map,i);
		}
		//guardamos
		/*
		memcpy(espacio_bit_map,bit_map->bitarray,BLOCK_COUNT/8);
		*/
		msync(espacio_bit_map,BLOCK_COUNT/8,MS_SYNC);
		
		//escribimos en el config el nuevo valor
	}

	//finalmente borramos el archivo del directorio
	if (unlink(archivo->path) == 0) {
        printf("El archivo %s fue eliminado exitosamente.\n", archivo->path);
    } else {
        perror("Error al eliminar el archivo");
    }

	liberarElementoLista(lista_archivos,instruccion_fs_partida_delete[2]);
	config_destroy(config_archivo);
}

void entradasalida_escuchar_memoria (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_memoria);
			//printf("Codigo de operación de memoria recibido\n");

			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			if (paquete == NULL) {
			perror("malloc");
			continue;
       		}
			paquete->buffer = malloc(sizeof(t_newBuffer));
			if (paquete->buffer == NULL) {
            perror("malloc");
            continue;
        	}
			// Recibimos el tamaño del buffer
			ssize_t received_size = recv(fd_memoria, &(paquete->buffer->size), sizeof(uint32_t), 0);
			if (received_size <= 0) {
				log_error(entradasalida_logger, "Error al recibir tamaño del buffer o desconexión.");
				control_key = false;
				free(paquete->buffer);
				free(paquete);
				continue;
			}
			
			//recv(fd_memoria,&(paquete->buffer->size),sizeof(uint32_t),0);	
			//asignamos memoria para el byffer
			paquete->buffer->stream = malloc(paquete->buffer->size);
			if (paquete->buffer->stream == NULL) {
            perror("malloc");
            free(paquete->buffer);
            free(paquete);
            continue;
        	}

			//recibimos offset si es encesario
			if( cod_op == 14 || cod_op == 33) //ojo con este hardcodeo, de añadir mas codigos de operacion pueden modificarse
			{
				received_size = recv(fd_memoria, &(paquete->buffer->offset), sizeof(uint32_t), 0);
				if (received_size <= 0) {
					log_error(entradasalida_logger, "Error al recibir offset o desconexión.");
					free(paquete->buffer->stream);
					free(paquete->buffer);
					free(paquete);
					continue;
				}
				//recv(fd_memoria,&(paquete->buffer->offset), sizeof(uint32_t),0);
			}

			//recibimos stream
			//recv(fd_memoria,paquete->buffer->stream, paquete->buffer->size,0);
			received_size = recv(fd_memoria, paquete->buffer->stream, paquete->buffer->size, 0);
			if (received_size <= 0) {
				log_error(entradasalida_logger, "Error al recibir datos del buffer o desconexión.");
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);
				continue;
			}

			switch (cod_op) {
			case STDOUT_TOPRINT:
				//printf("Entramos acá adentro reyes\n");
				char* text_to_print = malloc(paquete->buffer->size + 1);  // Reservar un byte extra para '\0'
				if (text_to_print != NULL) {
                    memcpy(text_to_print, paquete->buffer->stream, paquete->buffer->size);
                    text_to_print[paquete->buffer->size] = '\0'; // para terminar la cadena
                    printf("%s\n", text_to_print);
                    free(text_to_print);
                }
				/*
				text_to_print = paquete->buffer->stream;
				printf("%s\n",text_to_print);
				*/
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				break;
			case DESPERTAR:
			//
				//printf("Hola, entré a despertar\n");
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
			break;
			case IO_FS_WRITE:
				//
				//printf("Entramos acá adentro reyes\n");
				dialfs_to_write = malloc(paquete->buffer->size + 1);
                if (dialfs_to_write != NULL) {
                    memcpy(dialfs_to_write, paquete->buffer->stream, paquete->buffer->size);
                    dialfs_to_write[paquete->buffer->size] = '\0'; // Asegúrate de terminar la cadena
                    printf("%s\n", dialfs_to_write);
					sem_post(&sem_activacion1);
					sem_wait(&sem_activacion2);				
                }
				/*
				dialfs_to_write = malloc(paquete->buffer->size);
				dialfs_to_write = paquete->buffer->stream;
				printf("%s\n",dialfs_to_write);
				*/
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


void leerArchivo(char** instruccion_fs_partida_read)
{
    // Interpretamos los datos
    // IO_FS_READ FS Goku.config AX ECX EDX //base//0 //tamanio//14 11 12 13 14 15 16 17 18 19 20 21 22 23 24
    Archivo* archivo = encontrar_archivo(lista_archivos, instruccion_fs_partida_read[2]);
    t_config* config_archivo = config_create(archivo->path);

    // Puntero del archivo
    int pointer = atoi(instruccion_fs_partida_read[6]);
    // Cantidad de direcciones físicas
    int cant_df = atoi(instruccion_fs_partida_read[7]);

    // Asignamos memoria y la inicializamos
    char* to_read = malloc(cant_df + 1);
    if (to_read == NULL) {
        perror("Error al asignar memoria para to_read");
        exit(EXIT_FAILURE);
    }
    memset(to_read, 0, cant_df + 1); // Inicializamos la memoria a 0

    fd_bloque = open(path_bloques, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd_bloque == -1) {
        perror("Error al abrir el archivo");
        free(to_read);
        exit(EXIT_FAILURE);
    }

    int bloque_base = config_get_int_value(config_archivo, "BLOQUE_INICIAL");
    int tamanio = config_get_int_value(config_archivo, "TAMANIO_ARCHIVO");
    // Aseguramos que el archivo tenga el tamaño adecuado
    if (ftruncate(fd_bloque, BLOCK_COUNT * BLOCK_SIZE) != 0) {
        perror("Error ajustando el tamaño del archivo");
        close(fd_bloque);
        free(to_read);
        exit(EXIT_FAILURE);
    }

    // Mapeamos el archivo en memoria
    bloques = mmap(NULL, BLOCK_COUNT * BLOCK_SIZE, PROT_WRITE, MAP_SHARED, fd_bloque, 0);
    if (bloques == MAP_FAILED) {
        perror("Error en mmap");
        close(fd_bloque);
        free(to_read);
        exit(EXIT_FAILURE);
    }

    // Copiamos lo leído
    strncpy(to_read, bloques + BLOCK_SIZE * bloque_base + pointer, cant_df);
    to_read[cant_df] = '\0'; // Aseguramos que la cadena esté terminada en nulo
    //printf("Lo que hemos leído fue: %s\n", to_read);

    // Cerramos todo
    close(fd_bloque);
    munmap(bloques, BLOCK_COUNT * BLOCK_SIZE);

    // Escribimos todo en memoria
    for (int i = 0; i < cant_df; i++) {
        //printf("ESTOY ITERANDO\n");
        int* df_to_send_in = malloc(sizeof(int));
        if (df_to_send_in == NULL) {
            perror("Error al asignar memoria para df_to_send_in");
            free(to_read);
            exit(EXIT_FAILURE);
        }
        *df_to_send_in = atoi(instruccion_fs_partida_read[8 + i]);
        char* c_in = malloc(sizeof(char));
        if (c_in == NULL) {
            perror("Error al asignar memoria para c_in");
            free(df_to_send_in);
            free(to_read);
            exit(EXIT_FAILURE);
        }
        *c_in = to_read[i];
        new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
        free(c_in);
        free(df_to_send_in);
    }

    //printf("ESTOY ITERANDO\n");
    int* df_to_send_in = malloc(sizeof(int));
    if (df_to_send_in == NULL) {
        perror("Error al asignar memoria para df_to_send_in");
        free(to_read);
        exit(EXIT_FAILURE);
    }
    *df_to_send_in = -1;
    char* c_in = malloc(sizeof(char));
    if (c_in == NULL) {
        perror("Error al asignar memoria para c_in");
        free(df_to_send_in);
        free(to_read);
        exit(EXIT_FAILURE);
    }
    *c_in = 'L';
    new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
    free(c_in);
    free(df_to_send_in);

    // Liberamos la memoria asignada para to_read
    free(to_read);
	config_destroy(config_archivo);
}


void entradasalida_escuchar_kernel (){
	bool control_key = 1;
	while (control_key) {
			int cod_op = recibir_operacion(fd_kernel);

			//printf("Checkpoin1 \n");
			t_newPaquete* paquete = malloc(sizeof(t_newPaquete));
			if (paquete == NULL) {
            perror("malloc");
            continue;
        	}
			
			paquete->buffer = malloc(sizeof(t_newBuffer));
			if (paquete->buffer == NULL) {
            perror("malloc");
            free(paquete);
            continue;
	        }

			// Recibimos el tamaño del buffer
			ssize_t received_size = recv(fd_kernel, &(paquete->buffer->size), sizeof(uint32_t), 0);
			if (received_size <= 0) {
				log_error(entradasalida_logger, "Error al recibir tamaño del buffer o desconexión.");
				free(paquete->buffer);
				free(paquete);
				control_key = false;
				continue;
			}
			//recv(fd_kernel,&(paquete->buffer->size),sizeof(uint32_t),0);

			paquete->buffer->stream = malloc(paquete->buffer->size);
			if (paquete->buffer->stream == NULL) {
            perror("malloc");
            free(paquete->buffer);
            free(paquete);
            continue;
        	}
			//recv(fd_kernel,paquete->buffer->stream, paquete->buffer->size,0);

			received_size = recv(fd_kernel, paquete->buffer->stream, paquete->buffer->size, 0);
			if (received_size <= 0) {
				log_error(entradasalida_logger, "Error al recibir datos del buffer o desconexión.");
				free(paquete->buffer->stream);
				free(paquete->buffer);
				free(paquete);
				continue;
			}


			switch (cod_op) {
			case GENERICA:

				//printf("Checkpoin2 \n");
				/*
				
				//hay que deserializar, dado que es solo un int
				int* unidadesDeTrabajo = malloc(sizeof(int));
				unidadesDeTrabajo = paquete->buffer->stream;

				printf("Voy a dormir, reyes, la cantidad de %d unidades de trabajo\n",*unidadesDeTrabajo);
				printf("Voy a dormir: %d\n", (*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO ));
				sleep((*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO)/1000);
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				*/
				log_info (entradasalida_logger, "PID: <%d> - Operación: <INTERFAZ_GENÉRICA_SLEEP>",*pid_actual);
				
				int* unidadesDeTrabajo = (int*)paquete->buffer->stream; // Evitamos duplicar memoria

				log_info (entradasalida_logger, "PID: <%d> - Operación: <INTERFAZ_GENÉRICA_SLEEP> - DUERME: <%d>",*pid_actual,*unidadesDeTrabajo);

				//printf("Voy a dormir, reyes, la cantidad de %d unidades de trabajo\n", *unidadesDeTrabajo);
				//printf("Voy a dormir: %d\n", (*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO));
				//sleep((*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO) / 1000);
				usleep((*unidadesDeTrabajo * TIEMPO_UNIDAD_TRABAJO)*1000);
				// DEVOLVER AL KERNEL PARA DESPERTAR
				enviarEntero(pid_actual, fd_kernel, DESPERTAR);
				break;
			case STDIN:
				// DESEMPAQUETAMOS
				/*int* tamanio_instruccion_in = malloc(sizeof(int));
				memcpy(tamanio_instruccion_in, paquete->buffer->stream, sizeof(int));
				char* instruccion_in = malloc(*tamanio_instruccion_in);
				memcpy(instruccion_in, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_in);
				*/
				log_info (entradasalida_logger, "PID: <%d> - Operación: <INTERFAZ_STDIN>",*pid_actual);

				int* tamanio_instruccion_in = (int*)paquete->buffer->stream;
                char* instruccion_in = malloc(*tamanio_instruccion_in);
                if (instruccion_in == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
	                continue;
                }
                memcpy(instruccion_in, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_in);


				// Mostremos por pantalla
				//printf("La instruccion que ha llegado es: %s\n", instruccion_in);
				char** instruccion_partida_in = string_split(instruccion_in, " ");
				int tamanio_in = atoi(instruccion_partida_in[4]);
				//printf("Tamaño de la instrucción: %d\n", tamanio_in);

				// Ingresamos el texto a escribir
				//printf("Vamos a llevar a cabo STDIN\n");
				//printf("Ingrese un texto por teclado: \n");
				char* leido_in;
				leido_in = readline(">> ");
				//printf("Texto leído: %s\n", leido_in);

				// Debemos ir enviando poco a poco las direcciones físicas a memoria para que las vaya escribiendo
				// REFERENCIA // IO_STDIN_READ Int1 BX CX 12 15 16 17 18 19 20 21 22 23 24 25 26
				for (int i = 0; i < tamanio_in; i++) 
				{
					//printf("ESTOY ITERANDO\n");
					int* df_to_send_in = malloc(sizeof(int)); //liberar
					if (df_to_send_in == NULL) {
                        perror("malloc");
                        break;
                    }

					*df_to_send_in = atoi(instruccion_partida_in[5 + i]);
					char* c_in = malloc(sizeof(char)); //liberar
					if (c_in == NULL) {
                        perror("malloc");
                        free(df_to_send_in);
                        break;
                    }
					
					*c_in = leido_in[i];
					new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
					free(c_in);
					free(df_to_send_in);
				}

				//printf("ESTOY ITERANDO\n");
				int* df_to_send_in = malloc(sizeof(int)); //liberar
				 if (df_to_send_in == NULL) {
                    perror("malloc");
                    free(leido_in);
                    break;
                }
				
				*df_to_send_in = -1;
				char* c_in = malloc(sizeof(char)); //liberar
				if (c_in == NULL) {
                    perror("malloc");
                    free(df_to_send_in);
                    free(leido_in);
                    break;
                }
				*c_in = 'L';
				new_enviar_stdin_to_write_memoria(df_to_send_in, c_in);
				free(c_in);
				free(df_to_send_in);
				free(leido_in);
				//free(tamanio_instruccion_in);
				free(instruccion_in);
				string_array_destroy(instruccion_partida_in);
				// new_enviar_stdin_to_write_memoria(direccionFisica, caracter);
				//despertamos
				//enviarEntero(pid_actual,fd_kernel,DESPERTAR);
				break;
			case STDOUT:
				
				log_info (entradasalida_logger, "PID: <%d> - Operación: <INTERFAZ_STDOUT>",*pid_actual);

				//printf("CHECKPOINT DEL OUT 1\n");
				int* tamanio_instruccion_out = (int*)paquete->buffer->stream;
				char* instruccion_out = malloc(*tamanio_instruccion_out);
				if (instruccion_out == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				memcpy(instruccion_out, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_out);
				//printf("Llegó el siguiente tamaño: %d\n",*tamanio_instruccion_out);
				//printf("Llegó la siguiente instrucción: %s\n",instruccion_out);
				
				char** instrucciones_partidas_out = string_split(instruccion_out," ");
				if (instrucciones_partidas_out == NULL) {
					perror("string_split");
					free(instruccion_out);
					free(paquete->buffer->stream);
					free(paquete->buffer);
					free(paquete);
					continue;
				}
				//printf("CHECKPOINT DEL OUT 2\n");

				//creamos una lista de las direcciones que vayamos obteniendo	
				t_list* lista_direcciones = list_create();
				if (lista_direcciones == NULL) {
					perror("list_create");
					free(instrucciones_partidas_out);
					free(instruccion_out);
					free(paquete->buffer->stream);
					free(paquete->buffer);
					free(paquete);
					continue;
				}
				int* tamanio_out = malloc(sizeof(int));
				if (tamanio_out == NULL) {
                    perror("malloc");
					free(instrucciones_partidas_out);
                    free(instruccion_out);
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				*tamanio_out = atoi(instrucciones_partidas_out[4]);
				for(int i = 0; i < *tamanio_out; i++)
				{
					int* df_out = malloc(sizeof(int));
					if (df_out == NULL) {
                        perror("malloc");
                        list_destroy_and_destroy_elements(lista_direcciones, free);
                        free(tamanio_out);
                        free(instrucciones_partidas_out);
                        free(instruccion_out);
                        free(paquete->buffer->stream);
                        free(paquete->buffer);
                        free(paquete);
                        continue;
                    }
					*df_out = atoi(instrucciones_partidas_out[5+i]);
					list_add(lista_direcciones,df_out);
				}
				//printf("CHECKPOINT DEL OUT 3\n");

				//una ves tengamos la lista hecha habra que empaquetarlo de alguna forma
				new_enviar_stdout_to_print_memoria(lista_direcciones,tamanio_out,STDOUT_TOPRINT);

				//printf("CHECKPOINT DEL OUT 3\n");
				list_destroy_and_destroy_elements(lista_direcciones, free);
                free(tamanio_out);
                free(instruccion_out);
                string_array_destroy(instrucciones_partidas_out);

				break;

			case NUEVOPID:
				int* new_pid = (int*)paquete->buffer->stream;
				*pid_actual = *new_pid;
				printf("Esta interfaz está siendo usada actualmente por el proceso cuyo PID es %d\n",*pid_actual);
				break;
			case IO_FS_CREATE:
				// DESEMPAQUETAMOS
				int* tamanio_instruccion_fs_create = (int*)paquete->buffer->stream;
				char* instruccion_fs_create = malloc(*tamanio_instruccion_fs_create);
				if (instruccion_fs_create == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				memcpy(instruccion_fs_create, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs_create);
				char** instruccion_fs_partida_create = string_split(instruccion_fs_create, " ");

				log_info (entradasalida_logger, "PID: <%d> -Crear Archivo: <%s>",*pid_actual,instruccion_fs_partida_create[2]);

				// Mostremos por pantalla
				//printf("La instrucción que ha llegado es: %s\n", instruccion_fs_create);
				crear_archivo(instruccion_fs_partida_create[2]);
				free(instruccion_fs_create);
				string_array_destroy(instruccion_fs_partida_create);
				usleep(TIEMPO_UNIDAD_TRABAJO*1000);
				enviarEntero(pid_actual, fd_kernel, DESPERTAR);
				break;
			case IO_FS_TRUNCATE:
				int* tamanio_instruccion_fs_truncate = (int*)paquete->buffer->stream;
				char* instruccion_fs_truncate = malloc(*tamanio_instruccion_fs_truncate);
                    if (instruccion_fs_truncate == NULL) {
                        perror("malloc");
                        free(paquete->buffer->stream);
                        free(paquete->buffer);
                        free(paquete);
                        continue;
                    }
				
				memcpy(instruccion_fs_truncate, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs_truncate);
				char** instruccion_fs_partida_truncate = string_split(instruccion_fs_truncate, " ");

				log_info (entradasalida_logger, "PID: <%d> - Truncar Archivo: <%s> - Tamaño: <%s>",*pid_actual,instruccion_fs_partida_truncate[2],instruccion_fs_partida_truncate[4]);

				// Mostremos por pantalla
				//printf("La instrucción que ha llegado es: %s\n", instruccion_fs_truncate);
				// Realiza la operación de truncamiento aquí (por ejemplo, llamando a una función)
				truncarArchivo(instruccion_fs_partida_truncate[2], atoi(instruccion_fs_partida_truncate[4]));
				free(instruccion_fs_truncate);
				string_array_destroy(instruccion_fs_partida_truncate);
				usleep(TIEMPO_UNIDAD_TRABAJO*1000);
				enviarEntero(pid_actual, fd_kernel, DESPERTAR);
				break;
			break;
			case IO_FS_WRITE:

				int* tamanio_instruccion_fs_write = (int*)paquete->buffer->stream;
                char* instruccion_fs_write = malloc(*tamanio_instruccion_fs_write);
                if (instruccion_fs_write == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				memcpy(instruccion_fs_write, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs_write);
				char** instruccion_fs_partida_write = string_split(instruccion_fs_write, " ");

				log_info (entradasalida_logger, "PID: <%d> - Escribir Archivo: <%s> - Tamaño a Leer: <%s> - Puntero Archivo: <%s>",*pid_actual,instruccion_fs_partida_write[2],instruccion_fs_partida_write[7],instruccion_fs_partida_write[6]);				

				// Mostremos por pantalla
				//printf("La instrucción que ha llegado es: %s\n", instruccion_fs_write);
				// Realiza la operación de escritura aquí (por ejemplo, llamando a una función)
				escribirArchivo(instruccion_fs_partida_write);
				free(instruccion_fs_write);
				string_array_destroy(instruccion_fs_partida_write);
				usleep(TIEMPO_UNIDAD_TRABAJO*1000);
				enviarEntero(pid_actual, fd_kernel, DESPERTAR);
				break;
			case IO_FS_READ:
				//
				 int* tamanio_instruccion_fs_read = (int*)paquete->buffer->stream;
                char* instruccion_fs_read = malloc(*tamanio_instruccion_fs_read);
                if (instruccion_fs_read == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				memcpy(instruccion_fs_read, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs_read);
				char** instruccion_fs_partida_read = string_split(instruccion_fs_read, " ");

				//IO_FS_READ FS Goku.config AX ECX EDX //base//0 //tamanio//14 11 12 13 14 15 16 17 18 19 20 21 22 23 24
				log_info (entradasalida_logger, "PID: <%d> - Leer Archivo: <%s> - Tamaño a Leer: <%s> - Puntero Archivo: <%s>",*pid_actual,instruccion_fs_partida_read[2],instruccion_fs_partida_read[7],instruccion_fs_partida_read[6]);

				// Mostramos por pantalla
				//printf("La instrucción que ha llegado es: %s\n", instruccion_fs_read);
				// Realiza la operación de lectura aquí (por ejemplo, llamando a una función)
				usleep(TIEMPO_UNIDAD_TRABAJO*1000);
				leerArchivo(instruccion_fs_partida_read);
				free(instruccion_fs_read);
                string_array_destroy(instruccion_fs_partida_read);
				break;

			case IO_FS_DELETE:
			//
				int* tamanio_instruccion_fs_delete = (int*)paquete->buffer->stream;
                char* instruccion_fs_delete = malloc(*tamanio_instruccion_fs_delete);
                if (instruccion_fs_delete == NULL) {
                    perror("malloc");
                    free(paquete->buffer->stream);
                    free(paquete->buffer);
                    free(paquete);
                    continue;
                }
				memcpy(instruccion_fs_delete, paquete->buffer->stream + sizeof(int), *tamanio_instruccion_fs_delete);
				char** instruccion_fs_partida_delete = string_split(instruccion_fs_delete, " ");

				log_info (entradasalida_logger, "PID: <%d> - Eliminar Archivo: <%s>",*pid_actual,instruccion_fs_partida_delete[2]);
				
				// Mostramos por pantalla
				//printf("La instrucción que ha llegado es: %s\n", instruccion_fs_delete);
				// Realiza la operación de eliminación aquí (por ejemplo, llamando a una función)
				eliminarArchivo(instruccion_fs_partida_delete);
				free(instruccion_fs_delete);
                string_array_destroy(instruccion_fs_partida_delete);
				usleep(TIEMPO_UNIDAD_TRABAJO*1000);
				enviarEntero(pid_actual,fd_kernel,DESPERTAR);
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
	memcpy(to_send->nombre, datos[0], to_send->nombre_length);
	//to_send->nombre = datos[0];
	to_send->path_length = strlen(datos[1])+1;
	to_send->path = malloc(to_send->path_length);
	memcpy(to_send->path, datos[1], to_send->path_length);
	//to_send->path = datos[1];


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
    free(to_send->nombre);
    free(to_send->path);
	free(to_send);
	free(a_enviar);
    free(paquete->buffer->stream);
    free(paquete->buffer);
    free(paquete);	
}

#endif /* ENTRADASALIDA_H_ */