#include <iostream>
#include <time.h>
#include "HashMapConcurrente.hpp"
#include "CargarArchivos.hpp"
#define BILLION  1000000000L;

int main(int argc, char **argv) {
    struct timespec cargaStart, cargaEnd;
    struct timespec maxStart, maxEnd;
    if (argc < 4) {
        std::cout << "Error: faltan argumentos." << std::endl;
        std::cout << std::endl;
        std::cout << "Modo de uso: " << argv[0] << " <threads_lectura> <threads_maximo>" << std::endl;
        std::cout << "    " << "<archivo1> [<archivo2>...]" << std::endl;
        std::cout << std::endl;
        std::cout << "    threads_lectura: "
            << "Cantidad de threads a usar para leer archivos." << std::endl;
        std::cout << "    threads_maximo: "
            << "Cantidad de threads a usar para computar máximo." << std::endl;
        std::cout << "    archivo1, archivo2...: "
            << "Archivos a procesar." << std::endl;
        return 1;
    }
    int cantThreadsLectura = std::stoi(argv[1]);
    int cantThreadsMaximo = std::stoi(argv[2]);

    std::vector<std::string> filePaths = {};
    for (int i = 3; i < argc; i++) {
        filePaths.push_back(argv[i]);
    }

    HashMapConcurrente hashMap = HashMapConcurrente();
    
    if(cantThreadsLectura != 0) {
        clock_gettime(CLOCK_REALTIME, &cargaStart);
        cargarMultiplesArchivos(hashMap, cantThreadsLectura, filePaths);
        clock_gettime(CLOCK_REALTIME, &cargaEnd);
    }
    else{
        clock_gettime(CLOCK_REALTIME, &cargaStart);
        for(int i = 0; i < filePaths.size(); i++){
            cargarArchivo(hashMap, filePaths[i]);
        }
        clock_gettime(CLOCK_REALTIME, &cargaEnd);

    }
    double startTime = (double)(cargaEnd.tv_sec - cargaStart.tv_sec)
                  + (double)(cargaEnd.tv_nsec - cargaStart.tv_nsec) 
                  / (double)1e9;
    hashMapPair maximo;
    if(cantThreadsMaximo == 0){
        clock_gettime(CLOCK_REALTIME, &maxStart);
        maximo = hashMap.maximo();
        clock_gettime(CLOCK_REALTIME, &maxEnd);
    }
    else{
        clock_gettime(CLOCK_REALTIME, &maxStart);
        maximo = hashMap.maximoParalelo(cantThreadsMaximo);
        clock_gettime(CLOCK_REALTIME, &maxEnd);
    }
    double maxTime = ((maxEnd.tv_sec - maxStart.tv_sec) + (double)(maxEnd.tv_nsec - maxStart.tv_nsec)) / BILLION;

    std::cout << startTime << " " << maxTime << " " << maximo.first << " " << maximo.second << std::endl;

    return 0;
}