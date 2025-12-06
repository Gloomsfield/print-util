#include <stdio.h>

#include "pu-constants.h"
#include "pu.h"

int main(int argc, char * argv[]) {
	if(pu_init(argc, argv) != PU_SUCCESS) { return 1; }
	if(pu_select() != PU_SUCCESS) { return 1; }

	pu_text_t text = {
		.emphasized = 0,
		.justification = PU_JUSTIFICATION_LEFT,
		.font_index = 0,
		.character_width = 1,
		.character_height = 1,
		.string = "this is a test\nthis is a test TWO...\nAFJKHJKSJJS\nHGDAJKFSHFHJD",
		.cutmode = PU_CUTMODE_FULL,
	};

	if(pu_print_text(&text) != PU_SUCCESS) { return 1; }

	pu_stop();

	return 0;
}

