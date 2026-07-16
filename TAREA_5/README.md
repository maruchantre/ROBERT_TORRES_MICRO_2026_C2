# TAREA 5: control KACATA RC433

Firmware en C para ESP-IDF 5.5.4 y ESP32-S3. Lee los joysticks, botones y MPU-6050 de la placa KACATA-RC433-V1, normaliza los controles a `-100..100`, conserva calibracion en NVS y publica el estado por MQTT. La carpeta `hardware/` no es modificada.

## Hardware confirmado

| Funcion | GPIO | Nota |
|---|---:|---|
| JOY0 X / Y | 5 / 4 | ADC1 canales 4 / 3 |
| JOY1 X / Y | 1 / 2 | ADC1 canales 0 / 1 |
| Bateria | 8 | ADC1 canal 7; conversion electrica pendiente |
| Botones frontales | 9, 11, 10, 12 | BTN_0, BTN_1, BTN_2, BTN_3; activos en bajo |
| Botones laterales | 39, 40, 41, 42 | BTNL4, BTNL3, BTNL2, BTNL1; activos en bajo |
| I2C SDA / SCL | 6 / 7 | MPU-6050 y OLED |
| MPU INT | 16 | Disponible, no requerido para el sondeo actual |
| RF enable / TX / RX | 15 / 17 / 18 | Interfaz conservada, RF deshabilitado |
| LED de calibracion | 45 | LED1, activo en bajo por U5 |

El MPU-6050 tiene AD0 a GND, por lo que su direccion es `0x68`. Se configura a `+/-250 dps` y `+/-2 g`.

La OLED figura como `HS96L03W2C03`, pero el repositorio no contiene su controlador, resolucion ni direccion I2C. Por seguridad no se asume SSD1306: `display.c` presenta las cuatro paginas por consola hasta obtener esos datos.

## Joysticks y botones

Cada lectura ADC usa ocho muestras, un filtro IIR y centros independientes. La salida final se limita a `-100..100`; la zona muerta configurable de `app_config.h` produce exactamente `0` cerca del centro calibrado.

Los botones incluyen pull-up, activo bajo, antirrebote de 40 ms, flancos de pulsacion/liberacion y pulsacion larga de 800 ms. `front_buttons` y `trigger_buttons` usan mascaras separadas de cuatro bits.

Los archivos de diseno no muestran cuales laterales son los gatillos superiores. `PIN_CALIBRATION_TRIGGER_A` y `PIN_CALIBRATION_TRIGGER_B` en `pin_config.h` son configurables y por defecto seleccionan GPIO39 y GPIO40. Deben validarse fisicamente antes de usar el control real.

## Calibracion

Mantener ambos gatillos configurados durante tres segundos. Se muestra el progreso, parpadea LED1 y se cancela al soltar cualquiera. Luego se promedian 100 muestras de los cuatro ADC y del gyro X/Y/Z, y se guardan centros y biases en NVS. Una cancelacion no escribe NVS ni reemplaza la calibracion previa.

## Pantallas

La pagina cambia con el boton frontal 0, que no forma parte de la combinacion de calibracion.

1. Principal: joysticks, Wi-Fi, MQTT, RF, bateria y calibracion.
2. Botones: mascaras, flancos y pulsacion larga.
3. IMU: gyro normalizado/fisico y acelerometro.
4. Sistema: IP, MQTT, paquetes, bateria y calibracion.

## Wi-Fi y MQTT

Copiar `main/secrets.example.h` a `main/secrets.h` y completar SSID y clave. `main/secrets.h` esta ignorado por Git. Sin ese archivo el firmware real no inicia Wi-Fi ni MQTT, sin incluir credenciales en el repositorio.

Broker: `mqtt://broker.emqx.io:1883`.

Prefijo predeterminado: `itla/kakata433/maruchantre_7f29`. El cliente MQTT se inicia al recibir una IP de Wi-Fi.

Se publican cada 200 ms, y tambien al cambiar un boton:

```text
/state
/joystick/left/x
/joystick/left/y
/joystick/right/x
/joystick/right/y
/gyro/x
/gyro/y
/gyro/z
/accel/x
/accel/y
/accel/z
/buttons/front
/buttons/triggers
/battery
/status
```

`/state` es JSON e incluye secuencia, joysticks, gyro normalizado y en dps, acelerometro en g, temperatura, botones, bateria y calibracion. `/status` usa `online` y el LWT `offline`.

Para verlo desde el celular, conectar el telefono a la misma red que el ESP32-S3, instalar un cliente MQTT, configurar el mismo broker y suscribirse a `itla/kakata433/maruchantre_7f29/#`.

## Simulacion

`CONTROL_SIMULATION_MODE` vale `1` por defecto en `main/pin_config.h`. Genera joysticks, ocho botones, IMU, una secuencia de calibracion, JSON MQTT y paginas por consola; no inicializa ADC, GPIO, I2C, Wi-Fi ni RF reales. Permite revisar la logica sin el control fisico.

## Compilacion

Desde un **ESP-IDF Command Prompt o PowerShell** con ESP-IDF 5.5.4 inicializado:

```powershell
idf.py set-target esp32s3
idf.py build
idf.py -p <puerto> flash monitor
```

El entorno MSYS/MINGW de este espacio de trabajo rechaza `export.sh` antes de ejecutar la compilacion. Use una consola Windows soportada por Espressif para el build real.

## Limitaciones

- Falta confirmar controlador, resolucion y direccion de la OLED.
- Falta validar fisicamente los dos gatillos de calibracion y orientacion/signo de joysticks e IMU.
- El divisor de bateria necesita validacion electrica antes de interpretar `battery_mv` como tension real.
- Los modulos RF 433 MHz no tienen protocolo documentado y aparecen DNP en el esquematico. No se transmite RF.
