# Trabajo Final ED3 - 2023
## Rodriguez, Luciano Ariel - Robles Karen Yesica
## Comisión: Ing. Fernando Gallardo - Ing. Julio Sánchez.

[![N|Solid](https://cldup.com/dTxpPi9lDf.thumb.png)](https://nodesource.com/products/nsolid)



El trabajo final consta de la implementación de un reloj tipo Running que permite observar:
- Distancias.📏
- Pulsaciones promedio. ❤️
- Tiempo. ⏳
- Resumen de actividades. 📈



## Herramientas

- NXP LPC1769 
- PC para scripts de python y comunicación.
- Elementos para implementación física.
- Osciloscopio para observar salidas analógicas.


## Módulos & Tools de LPC1769

Hemos usado:

- SysTick.
- Interrupciones externas mediante pulsadores.
- Timers en modo MATH & modo CAPTURE.
- DAC.
- DMA (en relación a DAC).
- Comunicación UART para recibo y envío.

## Funcionamiento

![Pseudo aproximado](/pseudo.jpg)

## Pines

| Pin | Funcion |
| ------ | ------ |
| GND | GND |
| EXT_VIN |+5V |
| P0.0 & 0.1  | TX & RX |
| P0.15, 0.16, 0.17, 2.7, 2.8 | Multiplexacion de displays |
| P2.0 to 2.6  | Salidas de displays |
| P0.26 | Salida analógica -> Osiloscopio |
| P2.10 | EINT0 |
| P2.11 | EINT0 |
| P0.4 | Capture |

## Interfaces & IMG
![Descripción de la imagen](/if1.jpg)


![Descripción de la imagen](/if2.jpg)


![Descripción de la imagen](/sd.jpg)


![Descripción de la imagen](/sd1.jpg)
