#pragma once

#include <inttypes.h>

#include "../status.h"

#include "../types.h"

typedef base_handle_t server_handle_t;

static server_handle_t SERVER_HANDLE;

typedef struct server_config_t {
	uint16_t listen_port;
} server_config_t;

// starts a new server instance using `config` and populates
// `*server_handle` with the new server's handle.
//
// TODO - we probably need a `server_config_t` struct or
// something through which we can pass stuff like the port
// number and printer ID.
status_t server_start(server_config_t config, server_handle_t* server_handle);

// stops the server associated with `server_handle` and
// sets `*server_handle` to `NULL_HANDLE`.
status_t server_stop(server_handle_t* server_handle);

