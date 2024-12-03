# ⚡Maidesite2HomeAssistant

Guía sencilla para integrar tu mesa **Maidesite con Home Assistant** usando ESPHome. Aquí encontrarás todo lo necesario, desde la configuración del hardware hasta la integración final, para que puedas controlar y automatizar tu mesa de forma fácil e inteligente.

> Este DIY viene inspirado en las geniales aportaciones de [shades66](https://github.com/shades66/Maidesite-standing-desk). Desde aquí todos mis kudos para él!

# 🤔 Prerequisitos

Aunque esta guía cubre todo lo necesario para completar la integración, tener los siguientes conocimientos previos facilitará la comprensión de los temas tratados:

| Requisito                                                                 | Estado       |
|---------------------------------------------------------------------------|--------------|
| Contar con una instalación funcional de Home Assistant.                  | Obligatorio  |
| Tener la integración de ESPHome configurada en Home Assistant y experiencia en su uso. | Obligatorio  |
| Conocer la sintaxis básica de YAML.                                      | Deseado      |
| Conocimientos básicos de soldadura electrónica.                          | Deseado      |

## 🛠️ Materiales

- Placa ESP32-WROOM-32 (o similar), con conector USB Type-C de 5V y conectividad Wi-Fi + Bluetooth.
- Resistencias de 1.5kΩ (x1) y 3.3kΩ (x1).
- Kit de soldadura de estaño.
- Cable RJ12.
- Cable USB Type-C.
- Mesa Maidesite TH2 Pro Plus ✨.

# ✨ Home Assistant

La integración de **Home Assistant con ESPHome** nos proporcionará la siguiente serie de funcionalidades:

- Monitorización de la altura, desplazamiento y uso de las 4 alturas memorizadas.
- Monitorización de si estamos sentados o de pie, con contador de tiempo.
- Bloqueo de la mesa.
- Uso de las memorias 1 y 2 para representar la altura de pie y sentado respectivamente.

![Vista de controles disponibles desde ESPHome](resources/ha-controls.png)
![Vista de sensores disponibles desde ESPHome](resources/ha-sensors.png)

# 🤖 Circuito Madesite-ESP32

En la parte trasera de nuestro controlador Maidesite, es necesario utilizar el puerto RJ12 que no está siendo utilizado por ningún otro componente de la mesa.

![Parte trasera del controlador Madesite](resources/maidesite-back-control.png)

Siguiendo la misma orientación que se muestra en la imagen anterior, podemos contar los **6 pines** de nuestro conector RJ12. La numeración de los pines será del **6 al 1, de izquierda a derecha**. Tras mapear los pines de nuestro controlador, podemos proceder a esbozar el circuito.

![Circuito ESP32 y RJ12](resources/rj12-esp32.png)

La alimentación del ESP32 se proporciona a través de su conector USB Type-C, el cual está conectado al puerto de carga USB del controlador de nuestra mesa.

> En mi caso, he conectado la entrada **TX al pin 26** y la salida **RX al pin 27** de mi ESP32.

# 🕹️ ESPHome

Para realizar el trabajo de firmware en nuestro ESP32, debemos añadir un nuevo dispositivo para crear un nuevo "sketch". En mi caso, lo he llamado [`maidesite-desk.yaml`](/esp32/maidesite-desk.yaml).

En el archivo [`maidesite-desk.yaml`](/esp32/maidesite-desk.yaml), como se podrían haber fijado, se incluye un archivo llamado [`desk-control.h`](/esp32/desk-control.h). Este archivo contiene la clase **DeskControl**, que permite al ESP32 comunicarse con el controlador de la mesa Maidesite. A través de esta clase, el ESP32 puede ajustar la altura de la mesa y acceder a las posiciones predefinidas almacenadas en el controlador. Los valores de altura y las posiciones guardadas se envían como sensores a Home Assistant, lo que permite controlar y monitorear la mesa de manera remota.

```
esp32/
│
├── maidesite-desk.yaml (archivo principal)
├── desk-control.h (comunicación controlador-esp32)
```

# ✨ Futuras exploraciones

- Guardar posiciones en los slots de memoria, actualmente solo es posible desde el controlador de Maidesite.
- Explorar la funcionalidad de alarma, que permite activar la vibración en el controlador de Maidesite.