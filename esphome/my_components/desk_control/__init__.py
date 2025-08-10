# Este archivo es el "puente" que conecta el YAML con el código C++

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import uart
from esphome.components import sensor, uart, number

# Organizamos el código de nuestro cpp con un namespace
desk_control_ns = cg.esphome_ns.namespace('desk_control')

# Definimos la clase DeskControl que usa UART y funciona como componente ESPHome
DeskControl = desk_control_ns.class_('DeskControl', cg.Component, uart.UARTDevice)

# Definimos alias para las opciones de configuración
CONF_UART_ID = "uart_id"
CONF_HEIGHT_SLIDER = "height_slider"
CONF_SENSOR_M1 = "sensor_m1"
CONF_SENSOR_M2 = "sensor_m2"
CONF_SENSOR_M3 = "sensor_m3"
CONF_SENSOR_M4 = "sensor_m4"

# Esquema de configuración que valida la estructura que escribiremos en YAML
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(DeskControl),
    cv.Required(CONF_UART_ID): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_HEIGHT_SLIDER): cv.use_id(number.Number),
    cv.Required(CONF_SENSOR_M1): cv.use_id(sensor.Sensor),
    cv.Required(CONF_SENSOR_M2): cv.use_id(sensor.Sensor),
    cv.Required(CONF_SENSOR_M3): cv.use_id(sensor.Sensor),
    cv.Required(CONF_SENSOR_M4): cv.use_id(sensor.Sensor)
}).extend(cv.COMPONENT_SCHEMA)

# Esta función se trata de un Hook de ESPHome que se llama automaticamente cuando compilamos nuestro firmware
async def to_code(config):
    uart_var = await cg.get_variable(config[CONF_UART_ID])

    # Creamos una nueva instancia de DeskControl pasándole el UART
    var = cg.new_Pvariable(config[CONF_ID], uart_var)

    # Recuperamos las variables asociadas a cada sensor y control numérico
    height_slider = await cg.get_variable(config[CONF_HEIGHT_SLIDER])
    sensor_m1 = await cg.get_variable(config[CONF_SENSOR_M1])
    sensor_m2 = await cg.get_variable(config[CONF_SENSOR_M2])
    sensor_m3 = await cg.get_variable(config[CONF_SENSOR_M3])
    sensor_m4 = await cg.get_variable(config[CONF_SENSOR_M4])

    # Vinculamos los sensores y control numérico con la instancia DeskControl
    cg.add(var.set_height_slider(height_slider))
    cg.add(var.set_sensor_m1(sensor_m1))
    cg.add(var.set_sensor_m2(sensor_m2))
    cg.add(var.set_sensor_m3(sensor_m3))
    cg.add(var.set_sensor_m4(sensor_m4))

    # Registramos la instancia DeskControl para que ESPHome la maneje
    await cg.register_component(var, config)