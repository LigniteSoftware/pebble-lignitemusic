#include <lignite_music.h>

Settings current_settings;

SettingsCallback current_callback;

void settings_service_subscribe(SettingsCallback settings_callback){
    current_callback = settings_callback;
}

Settings settings_get_settings(){
    return current_settings;
}

void settings_set_settings(Settings new_settings){
    current_settings = new_settings;

    settings_save();

    if(current_callback){
        current_callback(current_settings);
    }
}

void settings_prepare_defaults(){
    current_settings.battery_saver = false;
    current_settings.pebble_controls = false;
    current_settings.artist_label = true;
}

int settings_load(){
    int bytes_read = persist_read_data(SETTINGS_KEY, &current_settings, sizeof(Settings));

    #ifdef DEBUG_LOGS
        NSLog("Read %d bytes from storage into settings.", bytes_read);
    #endif

    if(bytes_read < 0){
        settings_prepare_defaults();
    }

    return bytes_read;
}

int settings_save(){
    int bytes_written = persist_write_data(SETTINGS_KEY, &current_settings, sizeof(Settings));

    #ifdef DEBUG_LOGS
        NSLog("Wrote %d bytes into storage from settings.", bytes_written);
    #endif

    return bytes_written;
}