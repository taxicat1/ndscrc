#include <stdio.h>

#include "nds_file.h"


int main(int argc, char* argv[]) {
	if (argc != 2) {
		printf("Usage: %s <romfile.nds>\n", argv[0]);
		return 0;
	}
	
	const char* input_fname = argv[1];
	
	NDSFile nds;
	if (NDSFile_Open(&nds, input_fname) != 0) {
		printf("Failed to open ROM file: %s\n", input_fname);
		return 1;
	}
	
	if (NDSFile_FixCRCs(&nds) != 0) {
		printf("Failed to access ROM file: %s\n", input_fname);
		NDSFile_Close(&nds);
		return 1;
	}
	
	NDSFile_Close(&nds);
	
	return 0;
}
