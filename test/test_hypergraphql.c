/*-
 * Basic test for HypergraphQL functionality
 * Compile: gcc -I../src/include test_hypergraphql.c ../src/lib/hypergraphql.c ../src/lib/atomspace.c -o test_hypergraphql
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <xhyve/hypergraphql.h>
#include <xhyve/atomspace.h>

static void
test_server_init(void)
{
	atomspace_t *as;
	hgql_server_t *server;
	int error;

	printf("Testing hgql_server_init/destroy...");

	error = atomspace_init(&as, false);
	assert(error == 0);

	error = hgql_server_init(&server, as, 8080);
	assert(error == 0);
	assert(server != NULL);
	assert(server->atomspace == as);
	assert(server->port == 8080);
	assert(server->running == false);

	hgql_server_destroy(server);
	atomspace_destroy(as);

	printf(" OK\n");
}

static void
test_get_schema(void)
{
	atomspace_t *as;
	hgql_server_t *server;
	const char *schema;

	printf("Testing hgql_get_schema...");

	atomspace_init(&as, false);
	hgql_server_init(&server, as, 8080);

	schema = hgql_get_schema(server);
	assert(schema != NULL);
	assert(strlen(schema) > 0);
	assert(strstr(schema, "type Query") != NULL);
	assert(strstr(schema, "type Mutation") != NULL);
	assert(strstr(schema, "enum AtomType") != NULL);

	hgql_server_destroy(server);
	atomspace_destroy(as);

	printf(" OK\n");
}

static void
test_resolve_atom(void)
{
	atomspace_t *as;
	atom_handle_t h;
	char *json;

	printf("Testing hgql_resolve_atom...");

	atomspace_init(&as, false);

	h = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "TestAtom");
	assert(h != 0);

	json = hgql_resolve_atom(as, h);
	assert(json != NULL);
	assert(strstr(json, "\"handle\"") != NULL);
	assert(strstr(json, "TestAtom") != NULL);
	assert(strstr(json, "\"truthValue\"") != NULL);

	free(json);
	atomspace_destroy(as);

	printf(" OK\n");
}

static void
test_resolve_node(void)
{
	atomspace_t *as;
	atom_handle_t h;
	char *json;

	printf("Testing hgql_resolve_node...");

	atomspace_init(&as, false);

	h = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "FindMe");
	assert(h != 0);

	json = hgql_resolve_node(as, "FindMe");
	assert(json != NULL);
	assert(strstr(json, "FindMe") != NULL);

	free(json);
	atomspace_destroy(as);

	printf(" OK\n");
}

static void
test_resolve_query_pattern(void)
{
	atomspace_t *as;
	atom_handle_t h1, h2;
	char *json;

	printf("Testing hgql_resolve_query_pattern...");

	atomspace_init(&as, false);

	h1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Pattern1");
	h2 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Pattern2");
	assert(h1 != 0);
	assert(h2 != 0);

	json = hgql_resolve_query_pattern(as, "Pattern");
	assert(json != NULL);
	assert(strlen(json) > 2);  /* More than just "[]" */
	assert(json[0] == '[');
	assert(json[strlen(json) - 1] == ']');

	free(json);
	atomspace_destroy(as);

	printf(" OK\n");
}

static void
test_execute_query(void)
{
	atomspace_t *as;
	hgql_server_t *server;
	hgql_query_t query;
	hgql_response_t *response;

	printf("Testing hgql_execute_query...");

	atomspace_init(&as, false);
	hgql_server_init(&server, as, 8080);

	/* Add some test data */
	atomspace_add_node(as, ATOM_TYPE_CONCEPT, "TestNode");

	/* Execute query */
	memset(&query, 0, sizeof(query));
	query.type = HGQL_QUERY;
	strcpy(query.query, "{ atoms }");

	response = hgql_execute_query(server, &query);
	assert(response != NULL);
	assert(strlen(response->data) > 0);

	hgql_free_response(response);
	hgql_server_destroy(server);
	atomspace_destroy(as);

	printf(" OK\n");
}

int
main(void)
{
	printf("HypergraphQL Test Suite\n");
	printf("========================\n\n");

	test_server_init();
	test_get_schema();
	test_resolve_atom();
	test_resolve_node();
	test_resolve_query_pattern();
	test_execute_query();

	printf("\nAll tests passed!\n");
	return 0;
}
