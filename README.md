# Trabajo práctico 1 - Threading

## Estructura
* En la carpeta `src` encontrarán el código que completado.
* En la carpeta `Experimentos/data` encontarán los archivos asociados con los
  experimentos corridos.
* En la carpeta `scripts` encontrarán dos scripts utilizados para generar
  y ordenar los archivos de experimentación.

## Compilación
* El `Makefile` debería permitirles compilar el código con el comando `make`.
  El ejecutable generado estará en la carpeta `build`.
* El ejecutable se puede correr de la siguiente forma:
  ```
  ./ContarPalabras <threads_lectura> <threads_maximo> <archivo1> [<archivo2>...]
  ```
  donde:
  * `threads_lectura` es la cantidad de threads que se usará para leer archivos,
  * `threads_maximo` es la cantidad de threads que se usará para computar
    la palabra con más apariciones, y
  * `archivo1`, `archivo2`, etc. son los nombres de los archivos a procesar.
* También es posible generar el binario de experimentación a través de la utilización
  de `make exp`. Este ejecutable se corre de la siguiente forma:
   ```
  ./ContarPalabrasExperimentar <threads_lectura> <threads_maximo> <modo> <archivo1> [<archivo2>...]
  ```
  donde:
  * `modo` establece si se desea utilizar el binario para el experimento 1.3 o no.
  Si `modo == 1` se asume que se correrán misma cantidad de threads que archivos.
## Tests
* Ejecutando `make test` podrán compilar y ejecutar una pequeña suite de tests
  unitarios. Estos tests solo pretenden facilitarles el proceso de desarrollo
  verificando algunos aspectos básicos del funcionamiento del código.
  **No evalúan aspectos de concurrencia** y por lo tanto no brindan ninguna
  garantía de que el código escrito sea correcto.
