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

// Prepares the current settings struct with default settings in case settings
// don't exist.
void settings_prepare_defaults(){
    current_settings.battery_saver = false;
    current_settings.pebble_controls = true;
    current_settings.artist_label = true;
    current_settings.show_time = false;
}

void settings_load(){
    int bytes_read = persist_read_data(SETTINGS_KEY, &current_settings, sizeof(Settings));

    if(bytes_read < 0){
        settings_prepare_defaults();
    }
}

void settings_save(){
    persist_write_data(SETTINGS_KEY, &current_settings, sizeof(Settings));
}