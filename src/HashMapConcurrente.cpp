#ifndef CHM_CPP
#define CHM_CPP

#include <iostream>
#include <fstream>
#include <pthread.h>

#include "HashMapConcurrente.hpp"

HashMapConcurrente::HashMapConcurrente() {
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++) {
        tabla[i] = new ListaAtomica<hashMapPair>();
    }
}

unsigned int HashMapConcurrente::hashIndex(std::string clave) {
    return (unsigned int)(clave[0] - 'a');
}

void HashMapConcurrente::incrementar(std::string clave) {
    //obtenemos el indice al que debería pertenecer esta clave si existe.
    unsigned int claveIndex = hashIndex(clave);
    //obtenemos la longitud de la lista atómica asociada con esta clave
    unsigned int longitudTabla = tabla[claveIndex]->longitud();
    //la recorremos y queremos ver si está presente o no
    bool clavePresente = false;
    for(unsigned int i = 0; i < longitudTabla; i++){
        auto& elemento = tabla[claveIndex]->iesimo(i);
        if(elemento.first == clave){
            //obtengo atómicamente el valor actual
            unsigned int actual = elemento.second.load();
            //el valor deseado
            unsigned int nuevo = actual + 1;
            //utilizamos compare exchange weak para chequear que el valor actual sea, en efecto, el valor real actual
            //y que no haya ningún thread que lo haya modificado.
            //si eso sucedió, la operación cargará en actual el valor real actualizado luego de que el otro thread lo haya modificado;
            //además también re-incrementamos el valor deseado una vez que hemos obtenido el nuevo valor real.
            //cuando sea seguro, actualizamos la variable.
            while(!elemento.second.compare_exchange_weak(actual, nuevo)){
                nuevo = actual + 1;
            }
            clavePresente = true;
            break;
        }
    }
    //si la clave no estaba presente, la insertamos.
    if(!clavePresente){
        hashMapPair<std::string, unsigned int> newPair;
        newPair.first = clave;
        newPair.second = 1;
        tabla[claveIndex]->insertar(newPair);
    }
}

std::vector<std::string> HashMapConcurrente::claves() {
    // Completar (Ejercicio 2)
    std::vector<std::string> aux;

    for(unsigned int i = 0; i < 26; i++){
        for(auto p : tabla[i]){
            aux.push_back(p.first);
        }
    }

    return aux;
}

unsigned int HashMapConcurrente::valor(std::string clave) {
    // Completar (Ejercicio 2)
}

hashMapPair HashMapConcurrente::maximo() {
    hashMapPair *max = new hashMapPair();
    max->second = 0;

    for (unsigned int index = 0; index < HashMapConcurrente::cantLetras; index++) {
        for (
            auto it = tabla[index]->crearIt();
            it.haySiguiente();
            it.avanzar()
        ) {
            if (it.siguiente().second > max->second) {
                max->first = it.siguiente().first;
                max->second = it.siguiente().second;
            }
        }
    }

    return *max;
}

hashMapPair HashMapConcurrente::maximoParalelo(unsigned int cantThreads) {
    // Completar (Ejercicio 3)
}

#endif
