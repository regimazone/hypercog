/*-
 * Copyright (c) 2025 HyperCog Project
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <xhyve/atomspace.h>

/*
 * HypergraphQL - GraphQL query interface for OpenCog AtomSpace
 * Enables querying and manipulating the hypergraph knowledge base
 */

#define HGQL_MAX_QUERY_LEN 8192
#define HGQL_MAX_RESPONSE_LEN 65536

/* Query types */
typedef enum {
	HGQL_QUERY,
	HGQL_MUTATION,
	HGQL_SUBSCRIPTION
} hgql_query_type_t;

/* GraphQL query context */
typedef struct hgql_query {
	hgql_query_type_t type;
	char query[HGQL_MAX_QUERY_LEN];
	char variables[HGQL_MAX_QUERY_LEN];
} hgql_query_t;

/* GraphQL response */
typedef struct hgql_response {
	char data[HGQL_MAX_RESPONSE_LEN];
	char errors[4096];
	bool has_errors;
} hgql_response_t;

/* HypergraphQL server context */
typedef struct hgql_server {
	atomspace_t *atomspace;
	uint16_t port;
	bool running;
	void *server_thread;
} hgql_server_t;

/* Server operations */
int hgql_server_init(hgql_server_t **server, atomspace_t *as, uint16_t port);
void hgql_server_destroy(hgql_server_t *server);
int hgql_server_start(hgql_server_t *server);
int hgql_server_stop(hgql_server_t *server);

/* Query execution */
hgql_response_t* hgql_execute_query(hgql_server_t *server, 
                                     const hgql_query_t *query);
void hgql_free_response(hgql_response_t *response);

/* Schema introspection */
const char* hgql_get_schema(hgql_server_t *server);

/* Built-in resolvers */
char* hgql_resolve_atom(atomspace_t *as, atom_handle_t handle);
char* hgql_resolve_node(atomspace_t *as, const char *name);
char* hgql_resolve_link(atomspace_t *as, atom_type_t type, 
                        const char *outgoing_json);
char* hgql_resolve_query_pattern(atomspace_t *as, const char *pattern);
