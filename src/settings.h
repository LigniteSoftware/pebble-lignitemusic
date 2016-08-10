#pragma once

//The persistent storage key to write settings into.
#define SETTINGS_KEY 100

/**
 * The settings struct contains user specific settings about the configuration
 * of the watchapp.
 *
 * @member battery_saver   Whether or not battery saving mode is enabled. If so,
 * text scrolling is disabled and screen update intervals are greatly reduced.
 * @member artist_label    Whether or not the artist should show on the now
 * playing screen.
 * @member pebble_controls If enabled, controls will mimic the default Pebble
 * music app controls.
 */
typedef struct {
    bool battery_saver;
    bool artist_label;
    bool pebble_controls;
} Settings;

/**
 * For getting the latest and greatest settings.
 */
typedef void (*SettingsCallback)(Settings new_settings);

/**
 * Registers a settings callback for new settings to come to.
 * @param settings_callback The settings callback to register.
 */
void settings_service_subscribe(SettingsCallback settings_callback);

/**
 * Gets the current settings as set within settings.c.
 * @return The current settings.
 */
Settings settings_get_settings();

/**
 * Sets the current settings to whatever is provided.
 * @param new_settings The new settings to set.
 */
void settings_set_settings(Settings new_settings);

/**
 * Loads settings from persistent storage.
 * @return The bytes read from storage. See status_t for error codes.
 */
int settings_load();

/**
 * Saves the current settings into persistent storage.
 * @return The bytes read from storage. See status_t for error codes.
 */
int settings_save();