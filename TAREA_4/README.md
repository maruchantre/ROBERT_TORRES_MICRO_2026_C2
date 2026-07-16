# Control KACATA RC433: demostracion sin hardware

## Objetivo de la tarea

Este proyecto convierte la prueba inicial de la placa KACATA-RC433-V1 en un firmware de control demostrable sin disponer fisicamente del mando del profesor. Genera paquetes de control completos y los informa por el monitor serie.

## Uso de OpenCode y repositorio

La implementacion se realizo con OpenCode sobre el repositorio clonado proporcionado. Se inspecciono el esquematico de KiCad incluido y se conservaron sus asignaciones de GPIO. La carpeta `hardware/` no se ha modificado.

## Hardware identificado en el esquematico

El esquema indica un ESP32-S3, dos joysticks analogicos con pulsador, cuatro botones frontales, cuatro botones laterales, bateria, OLED y MPU-6050 por I2C, y modulos receptor/transmisor RF de 433 MHz.

| Funcion | GPIO | Nota |
|---|---:|---|
| JOY1 MD / MT | 1 / 2 | ADC1 canales 0 / 1 |
| JOY0 MT / MD | 4 / 5 | ADC1 canales 3 / 4 |
| Bateria VBAT | 8 | ADC1 canal 7 |
| JOY0, BTN0, BTN2, BTN1, BTN3 | 3, 9, 10, 11, 12 | Entradas activas en bajo |
| BTNL4, BTNL3, BTNL2, BTNL1, JOY1 | 39, 40, 41, 42, 46 | Entradas activas en bajo |
| I2C SDA / SCL | 6 / 7 | OLED y MPU-6050 |
| RF enable / datos TX / datos RX | 15 / 17 / 18 | RF 433 MHz |
| Interrupcion MPU | 16 | Entrada |
| LED6, LED5, LED4, LED1, LED3, LED2 | 13, 14, 21, 45, 47, 48 | Salidas activas en bajo |

## Funcionamiento

`control_input` obtiene los valores de entrada y `control_packet` construye un paquete con cabecera `0xCA43`, secuencia, dos ejes por joystick, mascara de botones, bateria, comando y checksum XOR. El joystick izquierdo decide `STOP`, `FORWARD`, `BACKWARD`, `LEFT` o `RIGHT`; una zona muerta de 350 cuentas alrededor de 2048 evita movimientos accidentales.

`rf433_send_packet()` conserva el punto de envio. No define una modulacion ni trama radio inventada: no hay un protocolo RF documentado en el repositorio.

## Modo simulacion

En `main/pin_config.h` esta definida la constante:

```c
#define CONTROL_SIMULATION_MODE 1
```

Con valor `1`, no se inicializa GPIO, ADC, I2C, OLED, MPU-6050 ni RF. Se generan posiciones variables para ambos joysticks, pulsaciones simuladas y nivel de bateria; cada paquete se imprime mediante `ESP_LOGI` y nunca se transmite por 433 MHz.

Con valor `0`, se inicializan los botones con pull-up interno y antirrebote de 40 ms, los cinco canales ADC y el bus I2C en GPIO6/GPIO7. La transmision RF sigue deliberadamente desactivada y registrada como pendiente hasta disponer de su protocolo documentado.

## Compilacion

Con el entorno de ESP-IDF 5.5.4 activo, desde la raiz del proyecto:

```sh
idf.py set-target esp32s3
idf.py build
```

Para observar la demostracion tras grabarlo: `idf.py -p <puerto> flash monitor`.

## Limitaciones y pendientes

No se realizo una validacion electrica porque el control fisico pertenece al profesor. Una prueba real debe confirmar orientacion y centros de los ejes, calibrar la conversion de VBAT a voltios/porcentaje, verificar OLED y MPU-6050, y definir el formato, codificacion y temporizacion del protocolo RF de 433 MHz antes de habilitar la transmision.
