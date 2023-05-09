#ifndef LISTA_ATOMICA_HPP
#define LISTA_ATOMICA_HPP

#include <atomic>

template<typename T>
class ListaAtomica {
 private:
    struct Nodo {
        Nodo(const T &val) : _valor(val), _siguiente(nullptr) {}

        T _valor;
        Nodo *_siguiente;
    };

    std::atomic<Nodo *> _cabeza;

 public:
    ListaAtomica() : _cabeza(nullptr) {}

    ~ListaAtomica() {
        Nodo *n, *t;
        n = _cabeza.load();
        while (n) {
            t = n;
            n = n->_siguiente;
            delete t;
        }
    }

    void insertar(const T &valor) {
        //Creamos nuevo nodo con el valor suministrado
        Nodo *nuevoNodo = new Nodo(valor);
        //el siguiente del nuevo nodo debe ser la anterior cabeza
        nuevoNodo->_siguiente = _cabeza.load();
        //La cabeza ahora debe ser el nuevo nodo.
        //Utilizamos compare_exchange_weak para asegurarnos de que no hubo thread intermedio que insertó.
        //Si algún thread llegó a insertar antes que se ejecute esta operación,
        //el siguiente elemento del que este thread está insertando debe ser la cabeza que dejó el thread anterior.
        //luego, es concurrent-safe.
        while(!_cabeza.compare_exchange_weak(nuevoNodo->_siguiente, nuevoNodo)){}
    }

    T &cabeza() const {
        return _cabeza.load()->_valor;
    }

    T &iesimo(unsigned int i) const {
        Nodo *n = _cabeza.load();
        for (unsigned int j = 0; j < i; j++) {
            n = n->_siguiente;
        }
        return n->_valor;
    }

    unsigned int longitud() const {
        Nodo *n = _cabeza.load();
        unsigned int cant = 0;
        while (n != nullptr) {
            cant++;
            n = n->_siguiente;
        }
        return cant;
    }


    class Iterador {
     private:
        ListaAtomica *_lista;

        typename ListaAtomica::Nodo *_nodo_sig;

        Iterador(ListaAtomica<T> *lista, typename ListaAtomica<T>::Nodo *sig)
            : _lista(lista), _nodo_sig(sig) {}

        friend typename ListaAtomica<T>::Iterador ListaAtomica<T>::crearIt();

     public:
        Iterador() : _lista(nullptr), _nodo_sig(nullptr) {}

        Iterador &operator=(const typename ListaAtomica::Iterador &otro) {
            _lista = otro._lista;
            _nodo_sig = otro._nodo_sig;
            return *this;
        }

        bool haySiguiente() const {
            return _nodo_sig != nullptr;
        }

        T &siguiente() {
            return _nodo_sig->_valor;
        }

        void avanzar() {
            _nodo_sig = _nodo_sig->_siguiente;
        }

        bool operator==(const typename ListaAtomica::Iterador &otro) const {
            return _lista->_cabeza.load() == otro._lista->_cabeza.load()
                && _nodo_sig == otro._nodo_sig;
        }
    };

    Iterador crearIt() {
        return Iterador(this, _cabeza);
    }
};

#endif /* LISTA_ATOMICA_HPP */
