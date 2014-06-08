#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

struct action {
	const char *cmd;
};

#define MAX_EVID 255

struct action *
build_action_table(const char *config_filename)
{
	struct action *actab = malloc((MAX_EVID + 1) * sizeof(struct action));
	if (actab == NULL) {
		fprintf(stderr, "malloc() failed\n");
		return NULL;
	}

	FILE *fp;
	fp = fopen(config_filename, "r");
	if (fp == NULL) {
		perror("fopen");
		return NULL;
	}

	for (;;) {
		unsigned int code;
		char cmd[1000];
		size_t n = fscanf(fp, "%u %[^\n]", &code, cmd);
		if (n != 2) {
			break;
		}

		if (code > MAX_EVID) {
			fprintf(stderr, "code %u is higher than the highest supported %u\n", code, MAX_EVID);
			continue;
		}

		actab[code].cmd = strdup(cmd);
		if (actab[code].cmd == NULL) {
			fprintf(stderr, "strdup() failed");
			return NULL;
		}

		printf("code: [%u], cmd: [%s]\n", code, cmd);

	}

	return actab;
}

void usage(const char *progname)
{
	printf("Usage:\n%s: EVENT_FILE CONFIG_FILE\n", progname);
}

void print_help(const char *progname)
{
	usage(progname);
	printf("\n");
	printf("EVENT_FILE is an evdev such as /dev/input/event0\n");
	printf("\n");
	printf(
"CONFIG_FILE is a configuration file. Each line has a key code followed by a\n"
"space, then the command to execute when that key is pressed.\n"
"\n"
"Key codes are the ones found in /usr/include/linux/input.h.\n"
"\n"
"Example:\n"
"115 echo \"Request to increase volume\"\n"
);
}

int run(int argc, char **argv)
{
	if (argc == 2) {
		if (!strcmp("-h", argv[1])) {
			print_help(argv[0]);
			return 0;
		} else {
			usage(argv[0]);
			return -1;
		}
	}

	if (argc != 3) {
		fprintf(stderr, "unexpected number of arguments\n");
		usage(argv[0]);
		return -1;
	}

	const char *evdev_filename = argv[1];
	const char *config_filename = argv[2];

	int fd = open(evdev_filename, O_RDONLY);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	struct action *action_table = build_action_table(config_filename);
	if (action_table == NULL) {
		fprintf(stderr, "failed to load config file\n");
		return -1;
	}
	
	for (;;) {
		struct input_event ev;
		int result;

		result = read(fd, &ev, sizeof(ev));
		if (result == -1) {
			perror("read");
			return -1;
		}
		if (result == 0) {
			continue;
		}
		if (result < sizeof(ev)) {
			fprintf(stderr, "got an event of a strange size\n");
			continue;
		}

		//printf("%hu,%hu,%u\n", ev.type, ev.code, ev.value);

		if (ev.type != 1 || ev.value != 1) {
			continue;
		}

		if (action_table[ev.code].cmd) {
			system(action_table[ev.code].cmd);
		}
	}

	return 0;
}
int main(int argc, char **argv)
{
	if (run(argc, argv) == -1) {
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

