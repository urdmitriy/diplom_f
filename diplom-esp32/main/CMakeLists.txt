idf_component_register(
        SRCS "diplom-esp32.c"
        "../../diplom-common/leds.c"
        "../../diplom-common/wifi_esp.c"
        "../../diplom-common/mqtt_esp.c"
        "../../diplom-common/dht11.c"
        "../../diplom-common/di.c"
        "../../diplom-common/photosensor.c"
        "../../diplom-common/uart_esp.c"
        "../../diplom-common/crc8.c"
        INCLUDE_DIRS
        "."
        "../../diplom-common/"
)
