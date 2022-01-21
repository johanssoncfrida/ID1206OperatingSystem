
void *dalloc(size_t request);
void dfree(void *memory);
void sanity();
void initializeArena();
void checkFreeSpace();
void checkSizeOfBlocks();
int checkLength();
int checkAvgLength();

