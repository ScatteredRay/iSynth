
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <linux/input.h>
#include <dirent.h>
#include <fcntl.h>

#define INPUT_EVENT_DIR "/dev/input"
static int g_argc;
static char **g_argv;
static int input_files[16];
static int input_file_count;
static int max_input_fd;

void initInput(int argc, char **argv) {
	// Open every file in /dev/input/*
	DIR *directory;
	struct dirent *ent;

	g_argc = argc;
	g_argv = argv;

	directory = opendir(INPUT_EVENT_DIR);
	if(!directory) {
		perror("Unable to open input event directory");
		return;
	}

	fprintf(stderr, "Reading directory %s...\n", INPUT_EVENT_DIR);
	while((ent = readdir(directory))) {
		if(strstr(ent->d_name, "event")) {
			char full_path[4096];
			snprintf(full_path, sizeof(full_path)-1, "%s/%s",
					INPUT_EVENT_DIR, ent->d_name);
			

			// Attempt to open the input device.
			if((input_files[input_file_count] = open(full_path, O_RDONLY)) == -1) {
				char err[128];
				snprintf(err, sizeof(err), "Unable to open input event %s",
						full_path);
				perror(err);
				continue;
			}
			fprintf(stderr, "Opening event device %s: %d\n", full_path, input_files[input_file_count]);

			// See if the newly-added input file has a greater fd than
			// we've ever seen before.  This is handy for passing to
			// select().
			if(input_files[input_file_count] > max_input_fd)
				max_input_fd = input_files[input_file_count];
			input_file_count++;
		}
	}
	closedir(directory);
}

void deinitInput() {
	// Close all files we know about.
	int input_number;
	for(input_number=0; input_number<input_file_count; input_number++)
		close(input_files[input_number]);
}

int argCount() {
	return g_argc;
}

char *getArg(int n) {
	return g_argv[n];
}


static int update_input_queue(int fd, int *x, int *y, int *touch) {
	struct input_event event;
	int bytes;
	bool should_loop = 0;

	do {
		bytes = read(fd, &event, sizeof(event));
		if(bytes != sizeof(event)) {
			fprintf(stderr, "Detected a short file read!  "
							"Wanted %d bytes, got %d\n",
							sizeof(event), bytes);
			return 1;
		}

		if(event.type == EV_SYN) {
			return 0;
		}

		else if(event.type == EV_KEY) {
			*touch = event.value;
		}

		else if(event.type == EV_ABS) {
			should_loop = true;
			if(event.code == ABS_X)
				*x = event.value;
			else if(event.code == ABS_Y)
				*y = event.value;
			else
				fprintf(stderr, "Unrecognized axis: %d\n", event.code);
		}

		else {
			fprintf(stderr, "Unrecognized event type: %d\n", event.type);
			return 1;
		}
	} while(should_loop);
	return 0;
}


void readInputAxis(int axis, float *buffer, int size) {
	struct timeval timeout;
	fd_set         read_fds;
	int            input_number;
	static int     x=0, y=0, touch=0;
	float          value;

//	fprintf(stderr, "Reading from axis %d into buffer %p of size %d\n",
//			axis, buffer, size);
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	FD_ZERO(&read_fds);

	for(input_number=0; input_number<input_file_count; input_number++)
		FD_SET(input_files[input_number], &read_fds);

	if(select(max_input_fd+1, &read_fds, NULL, NULL, &timeout) > 0)
		for(input_number=0; input_number<input_file_count; input_number++)
			if(FD_ISSET(input_files[input_number], &read_fds))
				update_input_queue(input_files[input_number], &x, &y, &touch);

	if(axis==0)
		value = (x-160)/320.;
	if(axis==1)
		value = (y-120)/240.;
	if(axis==2)
		value = touch;

	if(value < -1)
		value = -1;
	if(value >  1)
		value = 1;

	for(int i=0; i<size; i++)
		buffer[i] = value;
}

char getKey() {
	return 0;
}

std::string getPatchLocation(const char* patchname)
{
    return string("patches/") + patchname + string(".pat");
}
