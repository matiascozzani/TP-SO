#include <iostream>
#include <time.h>
#include "HashMapConcurrente.hpp"
#include "CargarArchivos.hpp"
#define BILLION  1000000000L;

int main(int argc, char **argv) {
    struct timespec cargaStart, cargaEnd;
    struct timespec maxStart, maxEnd;
    if (argc < 4) {
        std::cout << "Error: faltan argumentos. Solo se proporcionaron: " << argc << std::endl;
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
    int modo = std::stoi(argv[3]);

    std::vector<std::string> filePaths = {};
    for (int i = 4; i < argc; i++) {
        filePaths.push_back(argv[i]);
    }

    HashMapConcurrente hashMap = HashMapConcurrente();
    std::vector<std::vector<std::pair<timespec, timespec>>> tiempoPorThread;
    
    if(modo == 0){
        if(cantThreadsLectura != 0){
            clock_gettime(CLOCK_REALTIME, &cargaStart);
            cargarMultiplesArchivos(hashMap, cantThreadsLectura, filePaths);
            clock_gettime(CLOCK_REALTIME, &cargaEnd);
        }else{
            clock_gettime(CLOCK_REALTIME, &cargaStart);
            for(unsigned int i = 0; i < filePaths.size(); i++){
                cargarArchivo(hashMap, filePaths[i]);
            }
            clock_gettime(CLOCK_REALTIME, &cargaEnd);
        }
    }else{
        tiempoPorThread = cargarMultiplesArchivos2(hashMap, cantThreadsLectura, filePaths);
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

    if(modo==0) std::cout << startTime << " " << maxTime << " " << maximo.first << " " << maximo.second << std::endl;
    else{
        std::vector<std::vector<double>> tiempos;
        for(size_t i = 0; i < tiempoPorThread.size(); i++){
            std::vector<double> tiemposAux;
            for(size_t j = 0; j < tiempoPorThread[i].size(); j++){
                std::pair<timespec, timespec> currTime = tiempoPorThread[i][j];
                double time = ((currTime.second.tv_sec - currTime.first.tv_sec) + (double)(currTime.second.tv_nsec - currTime.first.tv_nsec)) / BILLION;
                tiemposAux.push_back(time);
            };
            tiempos.push_back(tiemposAux);
        }

        for(size_t i = 0; i < tiempoPorThread.size(); i++){
            for(size_t j = 0; j < tiempoPorThread[i].size(); j++){
                std::cout << tiempos[i][j]*1000 << " ";
            }
            std::cout << std::endl;
        }
    }

    return 0;
}