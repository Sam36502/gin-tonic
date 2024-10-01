//	
//	Parses the command line arguments
//	

#include "global.h"

void print_help_text();

bool parse_args(int argc, char *argv[]) {
	if (argc < 2) {
		puts("An argument is required!\n");
		print_help_text();
		return false;
	}
	
	for (int i=1; i<argc; i++) {
		char *arg = argv[i];
		bool is_option = false;

		// Help Option
		if (strcmp("-h", arg) == 0
			|| strcmp("--help", arg) == 0
		) {
			print_help_text();
			is_option = true;
			return false;
		}

		// info option
		if (strcmp("-i", arg) == 0
			|| strcmp("--info", arg) == 0
		) {
			g_prog_function = SHOW_INFO;
			is_option = true;
		}

		// loop option
		if (strcmp("-l", arg) == 0
			|| strcmp("--loop", arg) == 0
		) {
			g_loop_song = true;
			is_option = true;
		}

		// loop option
		if (strcmp("-n", arg) == 0
			|| strcmp("--new", arg) == 0
		) {
			g_new_file = true;
			is_option = true;
		}

		// Song Selection option
		if (strcmp("-s", arg) == 0
			|| strcmp("--song", arg) == 0
		) {
			i++;
			if (i >= argc) {
				puts("--song option requires an argument\n");
				print_help_text();
				return false;
			}
			int int_arg = strtol(argv[i], NULL, 0);
			if (int_arg < 0) {
				printf("%i is not a valid song nr.\n\n", int_arg);
				print_help_text();
				return false;
			}
			printf("Loading Song nr. %i....\n", int_arg);
			g_curr_song = int_arg;
			fflush(stdout);
			is_option = true;
		}

		// MIDI import option
		if (strcmp("-m", arg) == 0
			|| strcmp("--midi", arg) == 0
		) {
			i++;
			if (i >= argc) {
				puts("--midi option requires an argument\n");
				print_help_text();
				return false;
			}
			g_midi_file_name = argv[i];
			g_prog_function = IMPORT_MIDI;
			fflush(stdout);
			is_option = true;
		}

		// Assembly import option
		if (strcmp("-c", arg) == 0
			|| strcmp("--compile", arg) == 0
		) {
			i++;
			if (i >= argc) {
				puts("--compile option requires an argument\n");
				print_help_text();
				return false;
			}
			g_asm_file_name = argv[i];
			g_prog_function = IMPORT_ASM;
			fflush(stdout);
			is_option = true;
		}

		// Parse Filename Argument
		if (i == argc-1) {
			if (!is_option) {
				g_music_file_name = arg;
			} else {
				puts("Music-Filename argument is required\n");
				print_help_text();
				return false;
			}
		}
	}

	return true;
}

void print_help_text() {
	puts("Usage:");
	puts("  Bemused.exe [OPTIONS] <music-file>");
	puts("");
	puts("Options:");
	puts("  --help, -h                Display this help text and exit");
	puts("  --info, -i                Display basic useful info about the music file and exit");
	puts("  --loop, -l                Sets the loop flag (not retained) so the song loops when playing");
	puts("  --new, -n                 Create and edit a new file");
	puts("  --song, -s <song>         Select a specific song from the music file to edit");
	puts("  --midi, -m <midi-file>    Imports the provided MIDI file and adds it as a new song to");
	puts("                            the selected music file.");
	puts("  --compile, -c <asm-file>  Compiles a basic song Assembly-style text file and imports it");
	puts("                            the same as with midi files.");
}