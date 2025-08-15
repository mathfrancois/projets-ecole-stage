#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_FILE "controller.cfg"

void load_config();

int get_controller_port();
int get_display_timeout_value();
double get_fish_update_interval();

#endif // CONFIG_H
