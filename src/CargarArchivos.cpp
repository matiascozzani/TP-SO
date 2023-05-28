#ifndef CHM_CPP
#define CHM_CPP

#include <vector>
#include <iostream>
#include <fstream>
#include <thread>
#include <atomic>

#include "CargarArchivos.hpp"

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

void cargarMultiplesArchivos(HashMapConcurrente &hashMap,unsigned int cantThreads,std::vector<std::string> filePaths) {
    std::vector<std::thread> threads(cantThreads);
    std::atomic<int> currentFile(0);
    int maxFiles = filePaths.size();

    for(int i = 0; i < cantThreads; i++) {
        //disparamos los threads y le pasamos las refernecias que necesite.
        threads[i] = std::thread(&cargarArchivoThread, std::ref(currentFile), std::ref(filePaths), maxFiles, std::ref(hashMap));
    }
    
    for(auto &thread : threads) {
        //joineamos y esperamos a que terminen
        thread.join();
    }


}

#endif
