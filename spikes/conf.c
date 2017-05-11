

#include <stdio.h>

#include <libconfig.h>

int main(int argc, char** argv) 
{
	int i = 0;
	const char *p_str = "";
	config_t config;
	config_init(&config);
	if(config_read_file(&config, "example.conf") != CONFIG_TRUE) {
		printf("Failed to read conf file: %d %s\n",
			config_error_line(&config),
			config_error_text(&config));
		return -1;
	}
	config_lookup_string(&config, "server", &p_str);

	printf("Hello World '%s'\n", p_str);

	config_destroy(&config);
	return 0;	
}

