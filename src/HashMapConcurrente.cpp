#ifndef CHM_CPP
#define CHM_CPP

#include <iostream>
#include <fstream>
#include <thread>
#include <algorithm>

#include "HashMapConcurrente.hpp"

std::vector<std::mutex> readers(26);
std::vector<std::mutex> writers(26);
std::vector<int> cantLectores(26);


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
    // obtenemos la longitud de la lista atómica asociada con esta clave
    // debería encerrarse todo esto en un while(true)?
    ListaAtomica<hashMapPair> *entrada = (this->tabla)[claveIndex];
    writers[claveIndex].lock();
    unsigned int longitudTabla = (*entrada).longitud();
    // la recorremos y queremos ver si está presente o no
    bool clavePresente = false;
    for (unsigned int i = 0; i < longitudTabla; i++)
    {
        auto &elemento = tabla[claveIndex]->iesimo(i);
        if (elemento.first == clave)
        {   
            elemento.second++;
            writers[claveIndex].unlock();
            clavePresente = true;
            break;
        }
    }
    // si la clave no estaba presente, la insertamos.
    if (!clavePresente)
    {
        entrada->insertar(make_pair(clave, 1));
        writers[claveIndex].unlock();
    }
}


std::vector<std::string> HashMapConcurrente::claves()
{
    std::vector<std::string> aux;
    for (unsigned int i = 0; i < HashMapConcurrente::cantLetras; i++)
    {
        readers[i].lock();
        cantLectores[i]++;
        if(cantLectores[i] == 1) writers[i].lock();
        readers[i].unlock();

        unsigned int longitud = tabla[i]->longitud();
        for(unsigned int j = 0; j < longitud; j++){
            auto &elemento = tabla[i]->iesimo(j);
            aux.push_back(elemento.first);
        }
        
        readers[i].lock();
        cantLectores[i]--;
        if(cantLectores[i] == 0) writers[i].unlock();
        readers[i].unlock();
    }
    return aux;
}

unsigned int HashMapConcurrente::valor(std::string clave)
{
    unsigned int res = 0;
    unsigned int index = HashMapConcurrente::hashIndex(clave);

    readers[index].lock();
    cantLectores[index]++;
    if(cantLectores[index] == 1) writers[index].lock();
    readers[index].unlock();
    
    ListaAtomica<hashMapPair> *entrada = (this->tabla)[index];
    unsigned int longitud = entrada->longitud();
    for(unsigned int i = 0; i < longitud; i++){
        auto &elemento = entrada->iesimo(i);
        if(elemento.first == clave){
            res = elemento.second;
            break;
        }
    }

    readers[index].lock();
    cantLectores[index]--;
    if(cantLectores[index] == 0) writers[index].unlock();
    readers[index].unlock();

    return res;
}

hashMapPair HashMapConcurrente::maximo()
{
    hashMapPair *max = new hashMapPair();
    max->second = 0;

    for (unsigned int index = 0; index < HashMapConcurrente::cantLetras; index++)
    {  
        writers[index].lock();
        for (auto it = tabla[index]->crearIt(); it.haySiguiente(); it.avanzar())
        {
            if (it.siguiente().second > max->second)
            {
                max->first = it.siguiente().first;
                max->second = it.siguiente().second;
            }
        }
        writers[index].unlock();
    }

    return *max;
}

void HashMapConcurrente::threadFila(){
    unsigned int myIndex = indexParalelo.fetch_add(1, std::memory_order_relaxed);
    while(myIndex < 26){
        hashMapPair *maximoParcial = new hashMapPair();
        maximoParcial->second = 0;
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
        maximosParciales[myIndex] = (*maximoParcial);
        myIndex = indexParalelo.fetch_add(1, std::memory_order_relaxed);
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

    return *std::max_element(maximosParciales.begin(), maximosParciales.end(), [](const hashMapPair &a, const hashMapPair &b){return a.second < b.second;}); 
}

#endif
