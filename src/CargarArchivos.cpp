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
    //Inicializamos vector de tiempos para devolver.
    std::vector<std::pair<timespec, timespec>> tiempoPorLetra(26);
    // Abro el archivo.
    file.open(filePath, file.in);
    if (!file.is_open()) {
        std::cerr << "Error al abrir el archivo '" << filePath << "'" << std::endl;
        return tiempoPorLetra;
    }
    //Inicializamos estructuras de tiempo, y el actualLetter.
    unsigned int actualLetter;
    timespec letterStart, letterEnd;
    while (file >> palabraActual) {
        if(cant == 0) {
            //Asignamos actualLetter para que sea la letra actual, y comenzamos a tomar su tiempo.
            actualLetter = (unsigned int)palabraActual[0] - 'a';
            clock_gettime(CLOCK_REALTIME, &letterStart);
        }

        if(actualLetter != (unsigned int)palabraActual[0] - 'a'){
            //Si en algún momento ya no estamos en la misma letra que teníamos guardada, paramos el tiempo.
            clock_gettime(CLOCK_REALTIME, &letterEnd);
            //Lo asignamos correspondientemente.
            tiempoPorLetra[actualLetter] = std::make_pair(letterStart, letterEnd);
            //Actualizamos la letra y comenzamos a tomar el tiempo nuevamnete.
            actualLetter = (unsigned int)palabraActual[0] - 'a';
            clock_gettime(CLOCK_REALTIME, &letterStart);
        }
        hashMap.incrementar(palabraActual);
        cant++;
    }

    //Como el experimento para el que esta función fue diseñada solo se corre sobre archivos que tienen siempre iniciales de la a a la z,
    //y que tienen exactamente la misma cantidad de palabras por letra, si llegamos a este punto es debido a que ya procesamos la última de las palabras con z.
    clock_gettime(CLOCK_REALTIME, &letterEnd);
    tiempoPorLetra[actualLetter] = std::make_pair(letterStart, letterEnd);
    //Luego, actualizamos el tiempo correspondiente.


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
    while(currIndex < maxFiles){
        //Como en el experimento cada thread carga un y solo un archivo, entonces nos basta con utilizar currIndex.
        //En el caso de 0 threads, nos aclanza con uno cualquiera de los tiempos tardados, de todos modos guardamos todos.
        tiempoPorThread[currIndex] = cargarArchivo2(hashMap, filePaths[currIndex]);
        currIndex = currentFile.fetch_add(1);
    }
}

std::vector<std::vector<std::pair<timespec, timespec>>> cargarMultiplesArchivos2(HashMapConcurrente &hashMap,unsigned int cantThreads, std::vector<std::string> filePaths) {
    std::vector<std::thread> threads;
    std::atomic<int> currentFile(0);
    int maxFiles = filePaths.size();

    for(int i = 0; i < 10; i++){
        //Inicializamos vectores de tiempos dentro del vector que los contiene.
        tiempoPorThread.push_back(std::vector<std::pair<timespec, timespec>>());
    }

    if(cantThreads == 0){
        threads.emplace_back(&cargarArchivoThread2, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));

    }else{
        for(unsigned int i = 0; i < cantThreads; i++) {
            threads.emplace_back(&cargarArchivoThread2, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
        }
    }

    for(auto &thread : threads) {
        thread.join();
    }

    return tiempoPorThread;
}
void cargarMultiplesArchivos(HashMapConcurrente &hashMap,unsigned int cantThreads,std::vector<std::string> filePaths) {
    std::vector<std::thread> threads;
    std::atomic<int> currentFile(0);
    int maxFiles = filePaths.size();

    if(cantThreads == 0){
        threads.emplace_back(&cargarArchivoThread, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
    }
    else{
        for(unsigned int i = 0; i < cantThreads; i++) {
            threads.emplace_back(&cargarArchivoThread, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
        }
    }

    for(auto &thread : threads) {
        //joineamos y esperamos a que terminen
        thread.join();
    }


}

#endif