#include "../include/util.h"

bool g_isRunning = true;
char *g_root_dir = NULL;


Uint32 Util_FletcherChecksum(Uint8 *data, size_t len) {
	Uint32 sum_1 = 0x55555555, sum_2 = 0xAAAAAAAA;

	size_t remaining_len = len;
	Uint8 *curr_data = data;
	while (remaining_len > 0) {
		Uint16 num;
		if (remaining_len > 1) {
			num = (*(curr_data) << 8) | *(curr_data+1);
			curr_data += 2;
			remaining_len -= 2;
		} else {
			num = *(curr_data);
			curr_data += 1;
			remaining_len -= 1;
		}
		sum_1 = (sum_1 + num) & 0xFFFF;
		sum_2 = (sum_2 + sum_1) & 0xFFFF;
	}

	return (sum_2 << 16) | sum_1;
}

char *Util_FS_Basename(char *fullpath) {
	size_t oldlen = SDL_strlen(fullpath);
	char *endslash = SDL_strrchr(fullpath, '\\');
	size_t newlen = oldlen - (endslash - fullpath);
	char *base = SDL_malloc(sizeof(char) * newlen);
	SDL_memcpy(base, endslash+1, newlen);
	return base;
}

char *Util_FS_Dirname(char *fullpath) {
	char *endslash = SDL_strrchr(fullpath, '\\');
	size_t newlen = endslash - fullpath + 2;
	char *dir = SDL_malloc(sizeof(char) * newlen);
	SDL_memcpy(dir, fullpath, newlen);
	dir[newlen-1] = '\0';
	return dir;
}

char *Util_FS_GetValidPath(char *path) {
	if (g_root_dir == NULL) g_root_dir = SDL_GetBasePath();
	
	// Create new path variable
	size_t concat_len = SDL_strlen(g_root_dir) + SDL_strlen(path) + 1;
	char *newpath = SDL_malloc(sizeof(char) * concat_len);
	newpath[0] = '\0';
	SDL_strlcat(newpath, path, concat_len);
	
	// Check if path alone works
	FILE *file = fopen(newpath, "rw");
	if (file != NULL) return newpath;

	// Try adding base path
	newpath[0] = '\0';
	SDL_strlcat(newpath, g_root_dir, concat_len);
	SDL_strlcat(newpath, path, concat_len);
	file = fopen(newpath, "rw");
	return newpath;
}

FILE *Util_FS_Open(char *filename, char *mode) {
	if (g_root_dir == NULL) g_root_dir = SDL_GetBasePath();
	
	// Check if path alone works
	FILE *file = fopen(filename, mode);
	if (file != NULL) return file;

	// Try adding base path
	size_t concat_len = SDL_strlen(g_root_dir) + SDL_strlen(filename) + 1;
	char fullpath[concat_len];
	fullpath[0] = '\0';
	SDL_strlcat(fullpath, g_root_dir, concat_len);
	SDL_strlcat(fullpath, filename, concat_len);
	file = fopen(fullpath, mode);

	return file;
}