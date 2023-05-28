#ifndef CHM_CPP
#define CHM_CPP

#include <iostream>
#include <fstream>
#include <thread>

#include "HashMapConcurrente.hpp"

HashMapConcurrente::HashMapConcurrente()
{
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++)
    {
        tabla[i] = new ListaAtomica<hashMapPair>();
    }
}

unsigned int HashMapConcurrente::hashIndex(std::string clave)
{
    return (unsigned int)(clave[0] - 'a');
}

void HashMapConcurrente::incrementar(std::string clave)
{
    // obtenemos el indice al que debería pertenecer esta clave si existe.
    unsigned int claveIndex = hashIndex(clave);
    mutexes[claveIndex].lock();
    // obtenemos la longitud de la lista atómica asociada con esta clave
    // debería encerrarse todo esto en un while(true)?
    ListaAtomica<hashMapPair> *entrada = (this->tabla)[claveIndex];
    unsigned int longitudTabla = (*entrada).longitud();
    // la recorremos y queremos ver si está presente o no
    bool clavePresente = false;
    for (unsigned int i = 0; i < longitudTabla; i++)
    {
        auto &elemento = tabla[claveIndex]->iesimo(i);
        if (elemento.first == clave)
        {   
            elemento.second++;
            mutexes[claveIndex].unlock();
            clavePresente = true;
            break;
        }
    }
    // si la clave no estaba presente, la insertamos.
    if (!clavePresente)
    {
        entrada->insertar(make_pair(clave, 1));
        mutexes[claveIndex].unlock();
    }
}

std::vector<std::string> HashMapConcurrente::claves()
{
    std::vector<std::string> aux;
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++)
    {
        mutexes[i].lock();
        unsigned int longitud = tabla[i]->longitud();
        for(unsigned int j = 0; j < longitud; j++){
            auto &elemento = tabla[i]->iesimo(j);
            aux.push_back(elemento.first);
        }
        mutexes[i].unlock();
    }
    return aux;
}

//TODO: implementar estructura incremental para claves -> podría ser una cola?

unsigned int HashMapConcurrente::valor(std::string clave)
{
    unsigned int res = 0;
    unsigned int index = HashMapConcurrente::hashIndex(clave);
    mutexes[index].lock();
    ListaAtomica<hashMapPair> *entrada = (this->tabla)[index];
    unsigned int longitud = entrada->longitud();
    for(unsigned int i = 0; i < longitud; i++){
        auto &elemento = entrada->iesimo(i);
        if(elemento.first == clave){
            res = elemento.second;
            break;
        }
    }
    mutexes[index].unlock();
    return res;
}

hashMapPair HashMapConcurrente::maximo()
{
    hashMapPair *max = new hashMapPair();
    max->second = 0;

    for (unsigned int index = 0; index < HashMapConcurrente::cantLetras; index++)
    {  
        mutexes[index].lock();
        for (auto it = tabla[index]->crearIt(); it.haySiguiente(); it.avanzar())
        {
            if (it.siguiente().second > max->second)
            {
                max->first = it.siguiente().first;
                max->second = it.siguiente().second;
            }
        }
        mutexes[index].unlock();
    }

    return *max;
}

void HashMapConcurrente::threadFila(){
    unsigned int myIndex = indexParalelo.fetch_add(1);
    while(myIndex < 26){
        hashMapPair *maximoParcial = new hashMapPair();
        maximoParcial->second = 0;
        HashMapConcurrente::mutexes[myIndex].lock();
        ListaAtomica<hashMapPair> *entrada = (this->tabla)[myIndex];
        for(auto it = entrada->crearIt();
            it.haySiguiente();
            it.avanzar()){
                if(it.siguiente().second > maximoParcial->second)
                {
                    maximoParcial->first = it.siguiente().first;
                    maximoParcial->second = it.siguiente().second;
                }
            }
        mutexes[myIndex].unlock();
        maximosParciales.push_back(*maximoParcial);
        myIndex = indexParalelo.fetch_add(1);
    }
}

hashMapPair HashMapConcurrente::maximoParalelo(unsigned int cantThreads)
{
    std::vector<std::thread> threads(cantThreads);
    indexParalelo = 0;
    maximosParciales = std::vector<hashMapPair>(26);
    for(unsigned int i = 0; i < cantThreads; i++){
        threads[i] = std::thread(&HashMapConcurrente::threadFila, this);
    }

    for(auto &id : threads)
        id.join();

    hashMapPair *maximo = new hashMapPair();
    maximo->second = 0;
    for(auto p : maximosParciales){
        if(p.second > maximo->second){
            maximo->first = p.first;
            maximo->second = p.second;
        }
    }

    return *maximo; 
}

#endif
