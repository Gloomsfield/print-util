#include "pu-constants.h"
#include "pu.h"

int main(int argc, char * argv[]) {
	pu_context p_ctx;

	pu_init(&p_ctx);
	pu_run(&p_ctx);
}

