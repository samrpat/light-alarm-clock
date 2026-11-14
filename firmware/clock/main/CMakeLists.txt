idf_component_register(SRCS "main.c" "alarm_engine.c" "store.c" "light_fx.c" "audio_out.c" "rtc.c" "console.c" INCLUDE_DIRS "." REQUIRES clockcore nvs_flash esp_timer esp_system driver)
