#include <stdio.h>
#include <stdlib.h>

#include "types/types.h"

#include "parse/parse.h"
#include "strategy/strategy.h"
#include "send/send.h"

#include "./backend.h"

int main(int argc, char* argv[]) {
	request_data_t request_data;

	encoding_scheme_t encoding_scheme;
	sending_scheme_t sending_scheme;

	encoded_data_t encoded_data;

	if(parse_device(argc, argv, &request_data) < 0) {
		printf("failed to parse device data!");
		exit(1);
	}

	if(parse_text(argc, argv, &request_data) < 0) {
		printf("failed to parse text data!");
		exit(2);
	}

	if(determine_encoding_scheme(request_data, &encoding_scheme) < 0) {
		printf("failed to determine encoding scheme!");
		exit(3);
	}

	if(determine_sending_scheme(request_data, &sending_scheme) < 0) {
		printf("failed to determine sending scheme!");
		exit(4);
	}

	if(encoding_scheme.encode(request_data.string, &encoded_data) < 0) {
		printf("failed to encode request string!");
		exit(5);
	}

	if(sending_scheme.send(encoded_data) < 0) {
		printf("failed to send data!");
		exit(6);
	}

	return 0;
}
