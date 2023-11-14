import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk  
import serial, time, sys,re
import math
import numpy as np
from matplotlib import pyplot as plt
import struct
distancia =0
rt = []
pulsaciones_promedio = 123
tiempo =3 
kcal = 2
wform = []
rebote = 0;
def continuar_conexion():
    conex = serial.Serial('COM6',9600,  timeout=None)
    salida = 1;
    print("Conexion establecida\n")
    while (salida==1):
        data = conex.readline().strip()
        cleaned_data = data.replace(b'\x00', b'')
        if cleaned_data.startswith(b'*'):       #Significa que termino la medicion
            conex.close();
            #conex.flush();

            cleaned_data_str = cleaned_data.decode('utf-8')
            matches = re.search(r'\*(\d+)\*(\d+)\*(\d+)\*', cleaned_data_str)
            if matches:
                # Obtener los tres grupos y convertirlos a enteros
                cifra1 = int(matches.group(1))
                cifra2 = int(matches.group(2))
                cifra3 = int(matches.group(3))
                print(f"Cifras obtenidas: {cifra1}, {cifra2}, {cifra3}")
                imprimir_sesion(cifra1,cifra2,cifra3)
                wform.append(cifra1)
            
            #wform.append(distancia)
            salida = 0;
            sys.stdout.flush()
        else:
            cleaned_data_str = cleaned_data.decode('utf-8')
            print(cleaned_data_str)
    conex.close();
    #imprimir_sesion()
    

def imprimir_sesion(d,p,t):
        # Crear la ventana secundaria
    k = 7 * 70* (t/3600)
    k = round(k)
    ventana_running = tk.Toplevel(ventana)
    ventana_running.title("Running App")  # Título de la ventana

    # Etiqueta para el título
    etiqueta_titulo = ttk.Label(ventana_running, text="Estadísticas de Running", font=("Arial", 16))
    etiqueta_titulo.grid(row=0, column=0, columnspan=3, pady=10)  # Ajustar el diseño según sea necesario

    # Etiqueta y valor para la distancia
    etiqueta_distancia = ttk.Label(ventana_running, text="Distancia (m):")
    etiqueta_distancia.grid(row=1, column=0, padx=5, pady=5, sticky="e")

    valor_distancia = ttk.Label(ventana_running, text=f"{d}")
    valor_distancia.grid(row=1, column=1, padx=5, pady=5, sticky="w")

    # Etiqueta y valor para las pulsaciones promedio
    etiqueta_pulsaciones = ttk.Label(ventana_running, text="Pulsaciones Promedio:")
    etiqueta_pulsaciones.grid(row=2, column=0, padx=5, pady=5, sticky="e")

    valor_pulsaciones = ttk.Label(ventana_running, text=f"{p}")
    valor_pulsaciones.grid(row=2, column=1, padx=5, pady=5, sticky="w")

    # Etiqueta y valor para el tiempo
    etiqueta_tiempo = ttk.Label(ventana_running, text="Tiempo (s):")
    etiqueta_tiempo.grid(row=3, column=0, padx=5, pady=5, sticky="e")

    valor_tiempo = ttk.Label(ventana_running, text=f"{t}")
    valor_tiempo.grid(row=3, column=1, padx=5, pady=5, sticky="w")

    # Etiqueta y valor para las kcal
    etiqueta_kcal = ttk.Label(ventana_running, text="Kcal:")
    etiqueta_kcal.grid(row=4, column=0, padx=5, pady=5, sticky="e")

    valor_kcal = ttk.Label(ventana_running, text=f"{k}")
    valor_kcal.grid(row=4, column=1, padx=5, pady=5, sticky="w")


def mostrar_grafo_funcion():
        x = np.arange(len(wform))
        # Crear el gráfico de líneas
        plt.plot(x, wform, marker='o', linestyle='-')

        # Configurar etiquetas y título
        plt.xlabel('Índice')
        plt.ylabel('Valor')
        plt.title('Gráfico de líneas para valores enteros')

        # Mostrar el gráfico
        plt.show()

def enviar_datos_placa():
    conex = serial.Serial('COM6',9600,  timeout=None)
    salida = 1;
    print("Conexion para envio establecida\n")
    for i in range(len(wform)):
        datos_a_enviar = struct.pack('!B', wform[i])
        conex.write(datos_a_enviar) 
    conex.close()
# Crear la ventana principal de Tkinter
ventana = tk.Tk()
ventana.title("Ventana de Conexión")  # Establecer el título de la ventana

# Cambiar el fondo a blanco
ventana.configure(bg="white")

# Etiqueta para el título
etiqueta_titulo = ttk.Label(ventana, text="Trabajo Final - ED3 - 2023", font=("Arial", 16), background="white")
etiqueta_titulo.pack(pady=10)  # pady agrega un poco de espacio vertical

# Cargar una imagen
imagen = Image.open("img.jpg") 
imagen = ImageTk.PhotoImage(imagen)

# Mostrar la imagen
etiqueta_imagen = ttk.Label(ventana, image=imagen, background="white")
etiqueta_imagen.image = imagen  # Evita que la imagen se elimine debido a la recolección de basura
etiqueta_imagen.pack()

# Botón para continuar la conexión
boton_continuar = ttk.Button(ventana, text="Establecer comunicación con el reloj", command=continuar_conexion)
boton_continuar.pack(pady=10)

mostrar_grafo = ttk.Button(ventana, text="Mostrar resumen de actividades", command=mostrar_grafo_funcion)
mostrar_grafo.pack(pady=10)

enviar_datos = ttk.Button(ventana, text="Enviar datos", command=enviar_datos_placa)
enviar_datos.pack(pady=10)

# Iniciar el bucle principal de Tkinter
ventana.mainloop()
