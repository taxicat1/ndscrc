#ifndef NDS_FILE_H
#define NDS_FILE_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
	FILE* file_handle;
} NDSFile;

int NDSFile_Open(NDSFile* nds, const char* path);
void NDSFile_Close(NDSFile* nds);
int NDSFile_FixCRCs(NDSFile* nds);

#endif /* NDS_FILE_H */
