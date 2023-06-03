import random

# Ruta del archivo de entrada y salida
archivo_entrada = "./data/corpus3"
archivo_salida = "./data/corpus3D"

# Leer las líneas del archivo de entrada
with open(archivo_entrada, 'r') as f:
    lineas = f.readlines()

# Desordenar las líneas
random.shuffle(lineas)

# Escribir las líneas desordenadas en el archivo de salida
with open(archivo_salida, 'w') as f:
    f.writelines(lineas)
    
print("Las líneas se han desordenado exitosamente y se han guardado en", archivo_salida)
