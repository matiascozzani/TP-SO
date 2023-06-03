import random
import string

# Número de strings a generar
num_strings = 25000

# Longitud máxima del string
longitud_maxima = 10

# Nombre del archivo de salida
archivo_salida = "generado6.txt"

# Generar y guardar los strings en el archivo
with open(archivo_salida, 'w') as f:
    for _ in range(num_strings):
        longitud = random.randint(1, longitud_maxima)
        string_generado = random.choice(string.ascii_lowercase) + ''.join(random.choices(string.ascii_lowercase + string.digits, k=longitud-1))
        f.write(string_generado + '\n')

print("Strings generados exitosamente y guardados en", archivo_salida)
