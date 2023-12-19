idf_component_register(
        SRCS "diplom-esp32-mqtt-adapter.c"
        "../../diplom-common/leds.c"
        "../../diplom-common/wifi_esp.c"
        "../../diplom-common/mqtt_esp.c"
        "../../diplom-common/uart_esp.c"
        "../../diplom-common/crc8.c"
        INCLUDE_DIRS
        "."
        "../../diplom-common/"
)