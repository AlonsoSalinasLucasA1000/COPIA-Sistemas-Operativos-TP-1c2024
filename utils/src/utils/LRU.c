
/*
// Estructura de una entrada de TLB
typedef struct {
    int pid;            // Identificador del proceso
    int pagina;         // Número de página
    int marco;          // Número de marco de página física
    int contadorLRU;    // Contador para LRU (agregar a la estructura)
} TLB;

// Lista o arreglo para simular la TLB
TLB listaTLB[MAX_TLB_ENTRIES]; // Suponiendo un tamaño máximo para la TLB. ESTO ES UNA LISTA t_list* listaTLB;

int LRU_counter = 0; // Contador global para LRU (agregar)

void algoritmoSustitucion(int pid, int numero_Pagina, int marco) {
    if (strcmp(ALGORITMO_TLB, "FIFO") == 0) {      //------------------------------------------
       
        // Implementación básica de FIFO para la TLB
        TLB entrada_mas_antigua = listaTLB[0]; // Obtener la entrada más antigua, usa un array
        // Actualizar la entrada más antigua con los nuevos valores
        entrada_mas_antigua.pid = pid;
        entrada_mas_antigua.pagina = numero_Pagina;
        entrada_mas_antigua.marco = marco;         
                        
        //EL mas_tlb_entries ES CANTIDAD_ENTRADAS_TLB QUE VIENE DE CONFIG 

        // Mover la entrada más antigua al final de la lista (simulando FIFO)
        for (int i = 0; i < MAX_TLB_ENTRIES - 1; i++) {
            listaTLB[i] = listaTLB[i + 1];
        }
        listaTLB[MAX_TLB_ENTRIES - 1] = entrada_mas_antigua; // Añadir al final

    
    } else if (strcmp(ALGORITMO_TLB, "LRU") == 0) {     //-------------------------------------------
        // Implementación de LRU para la TLB
        int indiceLRU;    
        TLB* entradaLRU = buscar_en_TLB(numero_Pagina, pid, &indiceLRU);
//AHI VOLVI. HAY QUE CONVERITR EL ARRAY EN LISTA, NO TENIAMOS QUE BUSCAR EL TOPE?
VA A QUEDAR DISTINTO DEL FIFO, NO IMPORTA?
NO CREO QUE HAGA FALTA
JAJA ENTENDI DE LISTA A ARRAY PERDON
SISI
AH JSJSJ
        if (entradaLRU != NULL) {
            // Actualizar la entrada LRU encontrada
            entradaLRU->pid = pid;
            entradaLRU->pagina = numero_Pagina;
            entradaLRU->marco = marco;
            entradaLRU->contadorLRU = LRU_counter++;
        } else {
            // No se encontró una entrada, se debe reemplazar la menos recientemente usada
            indiceLRU = encontrar_indice_menos_recientemente_usado();
            listaTLB[indiceLRU].pid = pid;
            listaTLB[indiceLRU].pagina = numero_Pagina;
            listaTLB[indiceLRU].marco = marco;
            listaTLB[indiceLRU].contadorLRU = LRU_counter++;
        }

    } else {
        printf("Algoritmo de TLB no reconocido\n");
    }
}


// Función para buscar una entrada en la TLB según el número de página y el PID del proceso
TLB* buscar_en_TLB(int numero_Pagina, int pid, int *indiceLRU) {
    TLB* entradaLRU = NULL;
    int min_lru_value = INT_MAX; // Inicializar con el valor máximo de int

    // Recorrer la lista de TLB para encontrar la entrada con el menor valor de LRU
    for (int i = 0; i < MAX_TLB_ENTRIES; i++) {
        TLB* entrada = lista_get(listaTLB,i);  &listaTLB[i]; // Obtener la entrada actual

        // Buscar la entrada que coincida con el PID y número de página
        if (entrada->pid == pid && entrada->pagina == numero_Pagina) {
            *indiceLRU = i;
            return entrada; // Entrada encontrada, retornarla
        }

        // Buscar la entrada con el menor valor de LRU
        if (entrada->contadorLRU < min_lru_value) {
            min_lru_value = entrada->contadorLRU;
            *indiceLRU = i;
            entradaLRU = entrada; // Actualizar la entrada menos recientemente usada
        }
    }

    // Retornar la entrada menos recientemente usada si no se encontró una coincidencia exacta
    return entradaLRU;
}
// Función para encontrar el índice de la entrada menos recientemente usada en la TLB
int encontrar_indice_menos_recientemente_usado() {
    int indiceLRU = 0;
    int min_lru_value = listaTLB[0].contadorLRU;

    // Recorrer la lista de TLB para encontrar la entrada con el menor valor de LRU
    for (int i = 1; i < MAX_TLB_ENTRIES; i++) {
        if (listaTLB[i].contadorLRU < min_lru_value) {
            min_lru_value = listaTLB[i].contadorLRU;
            indiceLRU = i;
        }
    }

    return indiceLRU;
}

*/