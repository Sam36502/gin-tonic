#include "../include/arena.h"


GTAllocScratch *GTAlloc_Scratch_Create(size_t cap) {
	if (cap == 0) cap = ARENA_DEFAULT_SIZE;
	GTAllocScratch *alo = SDL_malloc(sizeof(GTAllocScratch) + cap);

	alo->cap = cap;
	alo->curr = alo + sizeof(GTAllocScratch);

	return alo;
}

void GTAlloc_Scratch_Destroy(GTAllocScratch *alo) {
	if (alo == NULL) return;
	SDL_free(alo);
}

GTAllocScratch *GTAlloc_Scratch_Resize(GTAllocScratch *alo, size_t size) {
	if (alo == NULL || size <= alo->cap) return alo;
	
	while (alo->cap < size) alo->cap += ARENA_BLOCK_SIZE;
	alo = SDL_realloc(alo, sizeof(GTAllocScratch) + alo->cap);
	return alo;
}

size_t GTAlloc_Scratch_Used(GTAllocScratch *alo) {
	if (alo == NULL) return 0;
	return alo->curr - (void *)(alo + sizeof(GTAllocScratch));
}

void GTAlloc_Scratch_PrintStats(GTAllocScratch *alo) {
	printf("Allocator Stats:\n");
	if (alo == NULL) {
		printf("  NULL\n");
		return;
	}

	size_t in_use = GTAlloc_Scratch_Used(alo);
	size_t avail = alo->cap - in_use;
	double used_pct = (double)(in_use) / (double)(alo->cap);
	double avail_pct = 1.0f - used_pct;

	printf("  %16s: %s\n", "Capacity", Util_Format_SizeT(alo->cap, 2, 2, 8));
	printf("  %16s: %s (%2.2f%%)\n", "Allocated", Util_Format_SizeT(in_use, 2, 2, 8), used_pct * 100.0f);
	printf("  %16s: %s (%2.2f%%)\n", "Free/Available", Util_Format_SizeT(avail, 2, 2, 8), avail_pct * 100.0f);
	printf("%02.2f%% [", used_pct * 100.0f);
	for (int i=0; i<32; i++) {
		char c = '#';
		double pct = (double)(i)/32.0f;
		if (pct > used_pct || used_pct < 0.001f) c = ' ';
		putchar(c);
	}
	printf("] %02.2f%%\n", avail_pct * 100.0f);
	fflush(stdout);
}

void *GTAlloc_Scratch_Alloc(GTAllocScratch *alo, size_t size) {
	if (alo == NULL) {
		Log_Message(LOG_ERROR, "Tried to allocate scratch memory from NULL allocator");
		return NULL;
	}

	if (size == 0) {
		Log_Message(LOG_ERROR, "Tried to allocate 0 Bytes of scratch memory");
		return NULL;
	}

	size_t used = GTAlloc_Scratch_Used(alo);
	if (size > alo->cap - used) {
		char msg[256];
		SDL_snprintf(msg, 256, "Allocator pool is too small (%s)", Util_Format_SizeT(alo->cap, -1, 2, -1));
		SDL_snprintf(msg, 256, "%s for requested size (%s)", msg, Util_Format_SizeT(size, -1, -1, -1));
		Log_Message(LOG_ERROR, msg);
		return NULL;
	}

	void *ptr = alo->curr;
	alo->curr += size;

	return ptr;
}

void GTAlloc_Scratch_FreeAll(GTAllocScratch *alo) {
	if (alo == NULL) return;
	alo->curr = alo + sizeof(GTAllocScratch);
}