idf_component_register(
  SRCS
    "drivers/vent_antenna.c"
    "drivers/vent_button_control.c"
    "drivers/vent_gpio_driver.c"
    "drivers/vent_uart_driver.c"
    "panel/vent_panel_protocol.c"
    "panel/vent_panel_reader.c"
    "panel/vent_panel_types.c"
  INCLUDE_DIRS "."
  REQUIRES esp_driver_uart esp_driver_gpio esp_timer freertos log esp_common hal
)
