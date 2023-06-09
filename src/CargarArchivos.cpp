#ifndef CHM_CPP
#define CHM_CPP

#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

#include "CargarArchivos.hpp"
std::vector<std::vector<std::pair<timespec, timespec>>> tiempoPorThread;
std::vector<std::pair<timespec, timespec>> cargarArchivo2(
    HashMapConcurrente &hashMap,
    std::string filePath) {
    std::fstream file;
    int cant = 0;
    std::string palabraActual;
    std::vector<std::pair<timespec, timespec>> tiempoPorLetra(26);
    // Abro el archivo.
    file.open(filePath, file.in);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo '" << filePath << "'" << std::endl;
        return tiempoPorLetra;
    }
    unsigned int actualLetter;
    timespec letterStart, letterEnd;
    while (file >> palabraActual) {
        if(cant == 0) {
            actualLetter = (unsigned int)palabraActual[0] - 'a';
            clock_gettime(CLOCK_REALTIME, &letterStart);
        }

        if(actualLetter != (unsigned int)palabraActual[0] - 'a'){
            clock_gettime(CLOCK_REALTIME, &letterEnd);
            tiempoPorLetra[actualLetter] = std::make_pair(letterStart, letterEnd);
            actualLetter = (unsigned int)palabraActual[0] - 'a';
            clock_gettime(CLOCK_REALTIME, &letterStart);
        }
        // Completar (Ejercicio 4)
        //file >> palabraActual ya carga en palabraActual un string
        hashMap.incrementar(palabraActual);
        cant++;
    }

    clock_gettime(CLOCK_REALTIME, &letterEnd);
    tiempoPorLetra[actualLetter] = std::make_pair(letterStart, letterEnd);


    // Cierro el archivo.
    if (!file.eof()) {
        std::cerr << "Error al leer el archivo" << std::endl;
        file.close();
        return tiempoPorLetra;
    }
    file.close();

    return tiempoPorLetra;
}

int cargarArchivo(
    HashMapConcurrente &hashMap,
    std::string filePath
) {
    std::fstream file;
    int cant = 0;
    std::string palabraActual;

    // Abro el archivo.
    file.open(filePath, file.in);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo '" << filePath << "'" << std::endl;
        return -1;
    }
    while (file >> palabraActual) {
        // Completar (Ejercicio 4)
        //file >> palabraActual ya carga en palabraActual un string
        hashMap.incrementar(palabraActual);
        cant++;
    }
    // Cierro el archivo.
    if (!file.eof()) {
        std::cerr << "Error al leer el archivo" << std::endl;
        file.close();
        return -1;
    }
    file.close();
    return cant;
}

void cargarArchivoThread(std::atomic<int>& currentFile, std::vector<std::string> &filePaths, int maxFiles, HashMapConcurrente& hashMap){
    //recibimos los parámetros por referencia.
    //current file tiene el int atómico que sirve para que dos threads no se pisen
    //filepaths es el vector de strings, lo pasamos por refercia para ahorrar memoria
    //maxfiles es la cantidad maxima de files que hay en el vector.
    //el hashmap que modificar también recibido por referencia.
    int currIndex = currentFile.fetch_add(1);
    //obtenemos e incrementamos atómicamente el int.
    while(currIndex < maxFiles){ //<- chequiamos que haya files por procesar
        cargarArchivo(hashMap, filePaths[currIndex]); //<- procesamos
        currIndex = currentFile.fetch_add(1); //<- incrementamos y obtenemos el valor anterior atómicamente
    }
}

void cargarArchivoThread2(std::atomic<int>& currentFile, std::vector<std::string> &filePaths, int maxFiles, HashMapConcurrente& hashMap){
    int currIndex = currentFile.fetch_add(1);
    //obtenemos e incrementamos atómicamente el int.
    while(currIndex < maxFiles){ //<- chequiamos que haya files por procesar
        tiempoPorThread[currIndex] = cargarArchivo2(hashMap, filePaths[currIndex]); //<- procesamos
        currIndex = currentFile.fetch_add(1); //<- incrementamos y obtenemos el valor anterior atómicamente
    }
}

std::vector<std::vector<std::pair<timespec, timespec>>> cargarMultiplesArchivos2(HashMapConcurrente &hashMap,unsigned int cantThreads, std::vector<std::string> filePaths) {
    std::vector<std::thread> threads;
    std::atomic<int> currentFile(0);
    int maxFiles = filePaths.size();

    for(int i = 0; i < 10; i++){
        tiempoPorThread.push_back(std::vector<std::pair<timespec, timespec>>());
    }

    if(cantThreads == 0){
        threads.emplace_back(&cargarArchivoThread2, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));

    }else{
        for(unsigned int i = 0; i < cantThreads; i++) {
            //threads[i] = std::thread(&cargarArchivoThread2, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap), std::ref(tiempoPorThread[i]));
            threads.emplace_back(&cargarArchivoThread2, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
        }
    }

        
    for(auto &thread : threads) {
            //joineamos y esperamos a que terminen
        thread.join();
    }

    return tiempoPorThread;
}
/*
*
*
*
**
*
**
*
**
*HAY QUE CONTEMPLAR cantThreads = 0 en cargarMultiplesArchivos!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! los test dados usan ContarPalabras normal!
**
*
**
*
**
*
**
*
**
*
**
*
*/
void cargarMultiplesArchivos(HashMapConcurrente &hashMap,unsigned int cantThreads,std::vector<std::string> filePaths) {
    std::vector<std::thread> threads(cantThreads);
    std::atomic<int> currentFile(0);
    int maxFiles = filePaths.size();

    for(unsigned int i = 0; i < cantThreads; i++) {
        //disparamos los threads y le pasamos las refernecias que necesite.
        threads[i] = std::thread(&cargarArchivoThread, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
    }
    
    for(auto &thread : threads) {
        //joineamos y esperamos a que terminen
        thread.join();
    }


}

#endif


/*

    obtener el tiempo que tarda cada thread -cuando carga un archivo- en cambiar de letra,
    para cada cantidad de threads

    osea:
    si corremos con 0 threads, la idea es ver que secuencialmente el programa llega a cada cambio de letra mas tarde EN GENERAL que 
    cuando corremos con 9 threas que tardaran un poco mas en cambiar de letra AL PRINCIPIO, y luego menos.

    seguro necesitamos que sea sobre archivos con misma cantidad de palabras x letra

    tiempoPorThread->[(vector26),
                      vector26

                        ]


*/