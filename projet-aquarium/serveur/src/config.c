#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

typedef struct {
    int controller_port;
    int display_timeout_value;
    int fish_update_interval;
} Config;

Config config;

void load_config() {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (!file) {
        log_error("Erreur ouverture du fichier de configuration");
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "controller-port", 15) == 0)
            sscanf(line, "controller-port = %d", &config.controller_port);
        else if (strncmp(line, "display-timeout-value", 21) == 0)
            sscanf(line, "display-timeout-value = %d", &config.display_timeout_value);
        else if (strncmp(line, "fish-update-interval", 20) == 0)
            sscanf(line, "fish-update-interval = %d", &config.fish_update_interval);
    }

    fclose(file);
}

int get_controller_port() {
    return config.controller_port;
}

int get_display_timeout_value() {
    return config.display_timeout_value;
}

double get_fish_update_interval() {
    return config.fish_update_interval;
}
