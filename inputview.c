#include <linux/input.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void usage(const char *progname)
{
	printf("Usage:\n%s: EVENT_FILE\n", progname);
	printf("\n");
	printf("EVENT_FILE would typically one of the /dev/input/event* files\n");
}

int run(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "unexpected number of arguments\n");
		usage(argv[0]);
		return -1;
	}

	const char *evdev_filename = argv[1];

	int fd = open(evdev_filename, O_RDONLY);
	if (fd == -1) {
		perror("open");
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

		if (ev.type != 1) {
			continue;
		}

		if (ev.value == 0) {
			printf("key released: %u\n", ev.code);
		} else if (ev.value == 1) {
			printf("key pressed: %u\n", ev.code);
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
