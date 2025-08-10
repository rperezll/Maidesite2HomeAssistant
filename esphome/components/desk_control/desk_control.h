#pragma once

/**
 * Este archivo define la estructura de cómo será el componente DeskControl.
 * Aquí no está la lógica, sino las definiciones del archivo .cpp.
 */

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/components/number/number.h"
#include "esphome/components/sensor/sensor.h"

namespace esphome {
  namespace desk_control {

  class DeskControl : public esphome::Component, public esphome::uart::UARTDevice {
    public:
      explicit DeskControl(esphome::uart::UARTComponent *parent) : esphome::uart::UARTDevice(parent) {}

      void loop() override;

      void set_height_slider(esphome::number::Number *slider) { height_slider = slider; }
      void set_sensor_m1(esphome::sensor::Sensor *sensor) { sensor_m1 = sensor; }
      void set_sensor_m2(esphome::sensor::Sensor *sensor) { sensor_m2 = sensor; }
      void set_sensor_m3(esphome::sensor::Sensor *sensor) { sensor_m3 = sensor; }
      void set_sensor_m4(esphome::sensor::Sensor *sensor) { sensor_m4 = sensor; }

    protected:
      enum State {
        WAIT_FOR_BYTE_1 = 0,
        WAIT_FOR_BYTE_2 = 1,
        RECEIVE_COMMAND = 2,
        RECEIVE_SIZE = 3,
        RECEIVE_DATA = 4,
        VERIFY_CHECKSUM = 5,
        PROCESS_COMMAND = 6
      } state = WAIT_FOR_BYTE_1;

      int value;
      int command;
      int chksum = 0;
      int bufsize = 0;
      int bufread = 0;
      float valueF;
      uint8_t buffer[10];

      esphome::number::Number *height_slider = nullptr;
      esphome::sensor::Sensor *sensor_m1 = nullptr;
      esphome::sensor::Sensor *sensor_m2 = nullptr;
      esphome::sensor::Sensor *sensor_m3 = nullptr;
      esphome::sensor::Sensor *sensor_m4 = nullptr;

      void handleWaitForByte1(uint8_t c);
      void handleWaitForByte2(uint8_t c);
      void handleReceiveCommand(uint8_t c);
      void handleReceiveSize(uint8_t c);
      void handleReceiveData(uint8_t c);
      void handleProcessCommand(uint8_t c);
      void handleVerifyChecksum(uint8_t c);
      void processCommand();
      void publishSensorState(int command);
      };

  }
}