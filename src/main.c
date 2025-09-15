#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pu-constants.h"
#include "pu.h"

#define PU_STATUSES(CHOOSE_FUNC) \
	CHOOSE_FUNC(PU_SUCCESS, "print-util success") \
	CHOOSE_FUNC(PU_FAILURE, "print-util failure")

#define PU_CHOOSE_ENUM(ENUM, STR) ENUM,
#define PU_CHOOSE_STRING(ENUM, STR) STR,

FILE * printer_f;

PU_STATUS_T pu_open_printer_f(const char * printer_path) {
	printer_f = fopen(printer_path, "r+b");

	if(printer_f == NULL) {
		return PU_FAILURE;
	}

	return PU_SUCCESS;
}

PU_STATUS_T pu_print_task(const char * task) {
	fprintf(printer_f, "%c%c%c", 0x1d, 0x21, 0x10 | 0x01);

	fprintf(printer_f, "%s", task);
	fprintf(printer_f, "%c", 0xa);

	return PU_SUCCESS;
}

PU_STATUS_T pu_print_task_path(const char * path) {
	fprintf(printer_f, "%c%c%c", 0x1d, 0x21, 0x00 | 0x00);

	fprintf(printer_f, "%s", path);
	fprintf(printer_f, "%c", 0xa);

	return PU_SUCCESS;
}

PU_STATUS_T pu_print_image() {
	fprintf(printer_f, "%c%c", PU_ESC, 0x2a);
}

PU_STATUS_T pu_print() {
	pu_print_task_path("5:30p /friends/events/");

	pu_print_task("group meowing");

	fprintf(printer_f, "%c%c%c%c", 0x1d, 'V', 66, 0);

	return PU_SUCCESS;
}

int main(int argc, char * argv[]) {
	// pu_init("/dev/usb/lp0");

	// pu_print();
	
	pu_context p_ctx;

	pu_init(&p_ctx);
	pu_run(&p_ctx);
}

