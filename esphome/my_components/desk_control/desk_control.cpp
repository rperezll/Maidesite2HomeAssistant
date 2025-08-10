#include "desk_control.h"
#include "esphome/core/log.h"

namespace esphome {
  namespace desk_control {

    static const char *TAG = "desk_control";

    /**
    * Con este bucle manejamos la recepción de mensajes que llegan por UART desde nuestra mesa.
    * La utilidad available() viene de esphome::uart::UARTDevice.
    */
    void DeskControl::loop() {
      // Ejecuta la iteración del While cuando available() detecta que "hay algo que leer".
      while (this->available()) {
        // Leemos un byte del mensaje recibido por UART.
        uint8_t c = this->read();

        ESP_LOGD(TAG, "🔥 Byte recibido UART: 0x%02X", c);

        switch (state) {
          // Esperamos el primer byte "mágico" que indica el inicio de un nuevo mensaje válido.
          case WAIT_FOR_BYTE_1:
            handleWaitForByte1(c);
            break;
          // Aquí esperamos el segundo byte "mágico" para evitar falsas señales.
          case WAIT_FOR_BYTE_2:
            handleWaitForByte2(c);
            break;
          // Recibimos el comando (o datos) entrante.
          case RECEIVE_COMMAND:
            handleReceiveCommand(c);
            break;
          // Recibimos el tamaño del bloque de datos que viene.
          case RECEIVE_SIZE:
            handleReceiveSize(c);
            break;
          // Recibimos los datos según el tamaño indicado y los almacenamos.
          case RECEIVE_DATA:
            handleReceiveData(c);
            break;
          // Verificamos que el checksum recibido coincida con el calculado en estados anteriores.
          case VERIFY_CHECKSUM:
            handleVerifyChecksum(c);
            break;
          // Ejecutamos el comando recibido
          case PROCESS_COMMAND:
            handleProcessCommand(c);
            break;
          default:
            ESP_LOGD(TAG, "🔴 Estado desconocido: %i", state);
            state = WAIT_FOR_BYTE_1;
            break;
        }
      }
    }

    /**
    * Esta función espera recibir el primer byte especial (0xF2) que indica el inicio de un mensaje.
    * Si recibe ese byte, cambia el estado para esperar el segundo byte mágico.
    */
    void DeskControl::handleWaitForByte1(uint8_t c) {
      if (c == 0xF2) {
        state = WAIT_FOR_BYTE_2;
      }
    }

    /**
    * Esta función espera recibir el segundo byte que sirve de confirmación de que estamos ante un mensaje válido.
    * Si es correcto pasamos a la fase de poder recibir un comando
    */
    void DeskControl::handleWaitForByte2(uint8_t c) {
      if (c == 0xF2) {
        state = RECEIVE_COMMAND;
        chksum = 0;
      } else {
        state = WAIT_FOR_BYTE_1;
      }
    }

    /**
    * Recibimos el código del comando que indica qué acción o dato se está enviando.
    * Guardamos el comando para después y nos preparamos para recibir el tamaño de los datos que vamos a recibir.
    */
    void DeskControl::handleReceiveCommand(uint8_t c) {
      command = c;
      state = RECEIVE_SIZE;
      chksum += command;
    }

    /**
    * Guarda cuántos bytes de datos se esperan.
    * Nos preparamos, ahora que conocemos su tamaño, a recibir los datos
    */
    void DeskControl::handleReceiveSize(uint8_t c) {
      bufsize = c;
      bufread = 0;
      chksum += bufsize;
      state = RECEIVE_DATA;
    }

    /**
    * Si lo recibido no excede el tamaño esperado, leemos su contenido y lo almacenamos.
    * Solo cuando leemos todo el tamaño indicado, pasamos a la fase de verificación del checksum (para asegurarnos de que estamos leyendo el mensaje correcto)
    */
    void DeskControl::handleReceiveData(uint8_t c) {
      if (bufread < bufsize) {
          buffer[bufread] = c;
          chksum += c;
          bufread++;
      }
      if (bufread == bufsize) {
          state = VERIFY_CHECKSUM;
      }
    }

    /**
    * Verifica que el checksum coincida (lo hemos ido construyendo en los métodos anteriores).
    * Si es correcta, avanzamos a procesar el comando.
    * Si falla, reiniciamos el estado para buscar un nuevo mensaje válido (WAIT_FOR_BYTE_1).
    */
    void DeskControl::handleVerifyChecksum(uint8_t c) {
      // Nos quedamos solo con lo que cabe en un byte.
      chksum = (chksum & 0xff);
      if (chksum == c) {
          state = PROCESS_COMMAND;
      }
      else {
          // Algo ha fallado y no coincide, volvemos al estado inicial
          state = WAIT_FOR_BYTE_1;
      }
    }

    /**
    * Comprobamos que se trata del último byte recibido (0x7e).
    * Si es así, llamamos a processCommand(), encargado de la ejecución del comando
    */
    void DeskControl::handleProcessCommand(uint8_t c)
    {
        if (c == 0x7e) {
          ESP_LOGD(TAG, "👀 Comando completo recibido: 0x%02X, Datos:", command);
          for (int i = 0; i < bufsize; i++) {
              ESP_LOGD(TAG, "  0x%02X", buffer[i]);
          }
          processCommand();
          ESP_LOGD(TAG, "🔵 Procesando comando: %i", command);
        } else {
          ESP_LOGD(TAG, "🔴 Byte inesperado: %02x", c);
        }
        // De cualquier modo, volvemos al estado inicial.
        state = WAIT_FOR_BYTE_1;
    }

    /**
    * Interpreta y maneja los comandos recibidos, actualizando estados y publicando valores según el comando procesado.
    */
    void DeskControl::processCommand() {
      switch (command) {
        // Se recibe y actualiza la altura actual de la mesa.
        case 1:
          valueF = (buffer[0] * 256) + buffer[1];
          if ((int)round(id(height_slider).state) != valueF) {
              id(height_slider).publish_state(valueF);
          }
          break;

        // Cada comando representa un sensor distinto (sensor_m1, sensor_m2, sensor_m3, sensor_m4).
        // Se recibe su estado y se publica su valor.
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
          valueF = (buffer[0] * 256) + buffer[1];
          publishSensorState(command);
          break;

        case 27:  // TO-DO: Comando 27 que no estaba contemplado...
          ESP_LOGD(TAG, "👀 Comando 27 recibido - Datos: %02x %02x %02x ...", buffer[0], buffer[1], buffer[2]);
          break;

        default:
          ESP_LOGD(TAG, "🔴 Comando desconocido: %i", command);
          break;
      }
    }

    void DeskControl::publishSensorState(int command)
    {
        switch (command)
        {
          // sensor_m1
          case 0x25: 
              id(sensor_m1).publish_state(valueF);
              break;

          // sensor_m2
          case 0x26:
              id(sensor_m2).publish_state(valueF);
              break;

          // sensor_m3
          case 0x27:
              id(sensor_m3).publish_state(valueF);
              break;

          // sensor_m4
          case 0x28:
              id(sensor_m4).publish_state(valueF);
              break;

          default:
              ESP_LOGD(TAG, "🔴 Comando desconocido: %i", command);
              break;
        }
    }

  }
}
