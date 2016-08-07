#include <lignite_music.h>

Settings current_settings;

Settings settings_get_settings(){
    current_settings.battery_saver = false;
    return current_settings;
}

void settings_set_settings(Settings new_settings){
    current_settings = new_settings;

    settings_save();
}

int settings_load(){
    int bytes_read = persist_read_data(SETTINGS_KEY, &current_settings, sizeof(Settings));

    #ifdef DEBUG_LOGS
        NSLog("Read %d bytes from storage into settings.", bytes_read);
    #endif

    return bytes_read;
}

int settings_save(){
    int bytes_written = persist_write_data(SETTINGS_KEY, &current_settings, sizeof(Settings));

    #ifdef DEBUG_LOGS
        NSLog("Wrote %d bytes into storage from settings.", bytes_written);
    #endif

    return bytes_written;
}