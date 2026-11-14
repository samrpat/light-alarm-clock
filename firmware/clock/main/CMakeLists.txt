file(GLOB FONTS "fonts/*.c")
idf_component_register(SRCS "main.c" "alarm_engine.c" "store.c" "light_fx.c" "audio_out.c" "rtc.c" "console.c" "ui.c" "ui_draw.c" "gfx.c" "sounds.c" ${FONTS} INCLUDE_DIRS "." REQUIRES clockcore nvs_flash esp_timer esp_system driver)
