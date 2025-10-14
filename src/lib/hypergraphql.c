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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <xhyve/hypergraphql.h>
#include <xhyve/atomspace.h>

/*
 * HypergraphQL implementation - GraphQL interface for AtomSpace
 */

static const char *HGQL_SCHEMA = 
"type Query {\n"
"  atom(handle: ID!): Atom\n"
"  node(name: String!): Node\n"
"  queryPattern(pattern: String!): [Atom]\n"
"  atoms: [Atom]\n"
"}\n"
"\n"
"type Mutation {\n"
"  addNode(type: AtomType!, name: String!): Node\n"
"  addLink(type: AtomType!, outgoing: [ID!]!): Link\n"
"  setTruthValue(handle: ID!, strength: Float!, confidence: Float!): Atom\n"
"}\n"
"\n"
"enum AtomType {\n"
"  NODE\n"
"  LINK\n"
"  CONCEPT\n"
"  PREDICATE\n"
"  VARIABLE\n"
"  EVALUATION\n"
"  INHERITANCE\n"
"  SIMILARITY\n"
"}\n"
"\n"
"type TruthValue {\n"
"  strength: Float!\n"
"  confidence: Float!\n"
"}\n"
"\n"
"type AttentionValue {\n"
"  sti: Int!\n"
"  lti: Int!\n"
"  vlti: Int!\n"
"}\n"
"\n"
"interface Atom {\n"
"  handle: ID!\n"
"  type: AtomType!\n"
"  name: String!\n"
"  truthValue: TruthValue!\n"
"  attentionValue: AttentionValue!\n"
"  timestamp: String!\n"
"}\n"
"\n"
"type Node implements Atom {\n"
"  handle: ID!\n"
"  type: AtomType!\n"
"  name: String!\n"
"  truthValue: TruthValue!\n"
"  attentionValue: AttentionValue!\n"
"  timestamp: String!\n"
"}\n"
"\n"
"type Link implements Atom {\n"
"  handle: ID!\n"
"  type: AtomType!\n"
"  name: String!\n"
"  truthValue: TruthValue!\n"
"  attentionValue: AttentionValue!\n"
"  timestamp: String!\n"
"  outgoing: [Atom!]!\n"
"}\n";

int
hgql_server_init(hgql_server_t **server, atomspace_t *as, uint16_t port)
{
	hgql_server_t *srv;

	if (server == NULL || as == NULL) {
		return (EINVAL);
	}

	srv = calloc(1, sizeof(hgql_server_t));
	if (srv == NULL) {
		return (ENOMEM);
	}

	srv->atomspace = as;
	srv->port = port;
	srv->running = false;
	srv->server_thread = NULL;

	*server = srv;
	return (0);
}

void
hgql_server_destroy(hgql_server_t *server)
{
	if (server == NULL) {
		return;
	}

	if (server->running) {
		hgql_server_stop(server);
	}

	free(server);
}

int
hgql_server_start(hgql_server_t *server)
{
	if (server == NULL) {
		return (EINVAL);
	}

	if (server->running) {
		return (EALREADY);
	}

	/* TODO: Start HTTP/GraphQL server in thread */
	server->running = true;

	return (0);
}

int
hgql_server_stop(hgql_server_t *server)
{
	if (server == NULL) {
		return (EINVAL);
	}

	if (!server->running) {
		return (0);
	}

	/* TODO: Stop server thread */
	server->running = false;

	return (0);
}

hgql_response_t*
hgql_execute_query(hgql_server_t *server, const hgql_query_t *query)
{
	hgql_response_t *response;

	if (server == NULL || query == NULL) {
		return (NULL);
	}

	response = calloc(1, sizeof(hgql_response_t));
	if (response == NULL) {
		return (NULL);
	}

	/* Simple query parser - in production, use a proper GraphQL parser */
	if (strstr(query->query, "atoms") != NULL) {
		/* Return all atoms as JSON */
		snprintf(response->data, HGQL_MAX_RESPONSE_LEN,
		    "{\"data\": {\"atoms\": {\"count\": %u}}}",
		    server->atomspace->atom_count);
	} else if (strstr(query->query, "queryPattern") != NULL) {
		/* Pattern query */
		snprintf(response->data, HGQL_MAX_RESPONSE_LEN,
		    "{\"data\": {\"queryPattern\": []}}");
	} else {
		snprintf(response->errors, sizeof(response->errors),
		    "{\"errors\": [{\"message\": \"Unknown query\"}]}");
		response->has_errors = true;
	}

	return (response);
}

void
hgql_free_response(hgql_response_t *response)
{
	if (response != NULL) {
		free(response);
	}
}

const char*
hgql_get_schema(hgql_server_t *server)
{
	(void)server;
	return (HGQL_SCHEMA);
}

char*
hgql_resolve_atom(atomspace_t *as, atom_handle_t handle)
{
	atom_t *atom;
	char *json;

	atom = atomspace_get_atom(as, handle);
	if (atom == NULL) {
		return (NULL);
	}

	json = malloc(2048);
	if (json == NULL) {
		return (NULL);
	}

	snprintf(json, 2048,
	    "{"
	    "\"handle\": %lu,"
	    "\"type\": %d,"
	    "\"name\": \"%s\","
	    "\"truthValue\": {\"strength\": %.2f, \"confidence\": %.2f},"
	    "\"attentionValue\": {\"sti\": %d, \"lti\": %d, \"vlti\": %d},"
	    "\"timestamp\": %lu"
	    "}",
	    (unsigned long)atom->handle,
	    atom->type,
	    atom->name,
	    atom->tv.strength,
	    atom->tv.confidence,
	    atom->av.sti,
	    atom->av.lti,
	    atom->av.vlti,
	    (unsigned long)atom->timestamp);

	return (json);
}

char*
hgql_resolve_node(atomspace_t *as, const char *name)
{
	query_result_t *result;
	char *json;

	result = atomspace_query(as, name);
	if (result == NULL || result->count == 0) {
		atomspace_free_query_result(result);
		return (NULL);
	}

	json = hgql_resolve_atom(as, result->handles[0]);
	atomspace_free_query_result(result);

	return (json);
}

char*
hgql_resolve_link(atomspace_t *as, atom_type_t type, const char *outgoing_json)
{
	(void)as;
	(void)type;
	(void)outgoing_json;
	/* TODO: Parse outgoing JSON and create link */
	return (NULL);
}

char*
hgql_resolve_query_pattern(atomspace_t *as, const char *pattern)
{
	query_result_t *result;
	char *json;
	size_t json_size = 4096;
	size_t offset = 0;

	result = atomspace_query(as, pattern);
	if (result == NULL) {
		return (NULL);
	}

	json = malloc(json_size);
	if (json == NULL) {
		atomspace_free_query_result(result);
		return (NULL);
	}

	offset += (size_t)snprintf(json + offset, json_size - offset, "[");

	for (uint32_t i = 0; i < result->count && offset < json_size - 100; i++) {
		char *atom_json = hgql_resolve_atom(as, result->handles[i]);
		if (atom_json != NULL) {
			if (i > 0) {
				offset += (size_t)snprintf(json + offset, 
				    json_size - offset, ",");
			}
			offset += (size_t)snprintf(json + offset, 
			    json_size - offset, "%s", atom_json);
			free(atom_json);
		}
	}

	snprintf(json + offset, json_size - offset, "]");

	atomspace_free_query_result(result);
	return (json);
}
