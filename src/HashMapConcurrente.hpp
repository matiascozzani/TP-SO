#ifndef HMC_HPP
#define HMC_HPP

#include <atomic>
#include <string>
#include <vector>
#include <semaphore.h>
#include <mutex>

#include "ListaAtomica.hpp"

typedef std::pair<std::string, unsigned int> hashMapPair;

class HashMapConcurrente
{
public:
   static const unsigned int cantLetras = 26;

   HashMapConcurrente();

   void incrementar(std::string clave);
   std::vector<std::string> claves();
   unsigned int valor(std::string clave);

   hashMapPair maximo();
   hashMapPair maximoParalelo(unsigned int cantThreads);


private:
   ListaAtomica<hashMapPair> *tabla[HashMapConcurrente::cantLetras];
   void threadFila();
   static unsigned int hashIndex(std::string clave);
   std::atomic<int> indexParalelo;
   std::vector<hashMapPair> maximosParciales;
   std::mutex mutexes[HashMapConcurrente::cantLetras];

};

#endif /* HMC_HPP */
