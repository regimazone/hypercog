/*-
 * HyperCog Demo - Example usage of OpenCog AtomSpace with HypergraphQL
 * 
 * This demonstrates the basic functionality of the HyperCog system:
 * - Initializing the AtomSpace and HypergraphQL server
 * - Adding nodes and links to create knowledge
 * - Querying the knowledge base
 * - Managing VM-specific knowledge
 * 
 * Compile with:
 *   cc -I../src/include -o hypercog_demo hypercog_demo.c \
 *      -L../build -lhypercog -latomspace -lhypergraphql
 */

#include <stdio.h>
#include <stdlib.h>
#include <xhyve/hypercog.h>
#include <xhyve/atomspace.h>
#include <xhyve/hypergraphql.h>

static void
demo_basic_atomspace(void)
{
	atomspace_t *as;
	atom_handle_t h1, h2, link;
	atom_handle_t outgoing[2];
	truth_value_t tv;
	atom_t *atom;
	int error;

	printf("\n=== Basic AtomSpace Demo ===\n");

	/* Initialize atomspace */
	error = atomspace_init(&as, false);
	if (error != 0) {
		fprintf(stderr, "Failed to initialize atomspace: %d\n", error);
		return;
	}

	/* Add concept nodes */
	h1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Dog");
	h2 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Mammal");

	printf("Added nodes:\n");
	printf("  - Dog (handle: %lu)\n", (unsigned long)h1);
	printf("  - Mammal (handle: %lu)\n", (unsigned long)h2);

	/* Create inheritance link */
	outgoing[0] = h1;
	outgoing[1] = h2;
	link = atomspace_add_link(as, ATOM_TYPE_INHERITANCE, outgoing, 2);
	printf("Created inheritance link (handle: %lu)\n", (unsigned long)link);

	/* Set truth value */
	tv.strength = 0.95f;
	tv.confidence = 0.90f;
	atomspace_set_tv(as, link, tv);
	printf("Set truth value: strength=%.2f, confidence=%.2f\n",
	    tv.strength, tv.confidence);

	/* Query and display */
	atom = atomspace_get_atom(as, link);
	if (atom != NULL) {
		printf("\nRetrieved link:\n");
		printf("  Name: %s\n", atom->name);
		printf("  Type: %d\n", atom->type);
		printf("  TV: strength=%.2f, confidence=%.2f\n",
		    atom->tv.strength, atom->tv.confidence);
	}

	/* Pattern query */
	query_result_t *results = atomspace_query(as, "Dog");
	printf("\nQuery for 'Dog': found %u results\n", results->count);
	atomspace_free_query_result(results);

	atomspace_destroy(as);
}

static void
demo_hypercog_integration(void)
{
	hypercog_ctx_t *ctx;
	atom_handle_t knowledge;
	int error;

	printf("\n=== HyperCog Integration Demo ===\n");

	/* Initialize HyperCog with distributed mode */
	error = hypercog_init(&ctx, true, 8080);
	if (error != 0) {
		fprintf(stderr, "Failed to initialize HyperCog: %d\n", error);
		return;
	}

	printf("Initialized HyperCog (distributed mode, port 8080)\n");

	/* Set global context */
	hypercog_set_global_context(ctx);

	/* Simulate VM creation */
	error = hypercog_vm_created(ctx, 0);
	printf("VM 0 created: %s\n", error == 0 ? "success" : "failed");

	/* Add VM knowledge */
	knowledge = hypercog_add_vm_knowledge(ctx, 0, "os", "Linux");
	printf("Added knowledge (handle: %lu): VM-0:os = Linux\n",
	    (unsigned long)knowledge);

	knowledge = hypercog_add_vm_knowledge(ctx, 0, "memory", "4096MB");
	printf("Added knowledge (handle: %lu): VM-0:memory = 4096MB\n",
	    (unsigned long)knowledge);

	/* Query VM knowledge */
	knowledge = hypercog_query_vm_knowledge(ctx, 0, "os");
	printf("Query result for VM-0:os: handle=%lu\n",
	    (unsigned long)knowledge);

	/* Start services */
	error = hypercog_start(ctx);
	printf("Started HyperCog services: %s\n",
	    error == 0 ? "success" : "failed");

	printf("\nGraphQL endpoint available at http://localhost:8080/graphql\n");
	printf("Example query:\n");
	printf("  query { queryPattern(pattern: \"VM-0\") { handle name } }\n");

	/* Cleanup */
	hypercog_vm_destroyed(ctx, 0);
	printf("\nVM 0 destroyed\n");

	hypercog_stop(ctx);
	hypercog_destroy(ctx);
	printf("HyperCog shutdown complete\n");
}

static void
demo_distributed_atomspace(void)
{
	atomspace_t *as1, *as2;
	atom_handle_t h1, h2;
	int error;

	printf("\n=== Distributed AtomSpace Demo ===\n");

	/* Create two atomspaces (simulating different hypervisor nodes) */
	error = atomspace_init(&as1, true);
	if (error != 0) {
		fprintf(stderr, "Failed to init atomspace 1: %d\n", error);
		return;
	}

	error = atomspace_init(&as2, true);
	if (error != 0) {
		fprintf(stderr, "Failed to init atomspace 2: %d\n", error);
		atomspace_destroy(as1);
		return;
	}

	printf("Created two atomspaces with distributed mode enabled\n");
	printf("  Node 1 ID: %s\n", as1->node_id);
	printf("  Node 2 ID: %s\n", as2->node_id);

	/* Add atom to first atomspace */
	h1 = atomspace_add_node(as1, ATOM_TYPE_CONCEPT, "SharedConcept");
	printf("Added 'SharedConcept' to node 1 (handle: %lu)\n",
	    (unsigned long)h1);

	/* In production, this would sync automatically */
	printf("In distributed mode, this atom would be broadcast to node 2\n");

	/* Add atom to second atomspace */
	h2 = atomspace_add_node(as2, ATOM_TYPE_CONCEPT, "LocalConcept");
	printf("Added 'LocalConcept' to node 2 (handle: %lu)\n",
	    (unsigned long)h2);

	printf("\nAtomSpace 1 has %u atoms\n", as1->atom_count);
	printf("AtomSpace 2 has %u atoms\n", as2->atom_count);

	atomspace_destroy(as1);
	atomspace_destroy(as2);
}

static void
demo_graphql_schema(void)
{
	hgql_server_t *server;
	atomspace_t *as;
	const char *schema;
	int error;

	printf("\n=== HypergraphQL Schema Demo ===\n");

	/* Initialize */
	error = atomspace_init(&as, false);
	if (error != 0) {
		fprintf(stderr, "Failed to initialize atomspace: %d\n", error);
		return;
	}

	error = hgql_server_init(&server, as, 8081);
	if (error != 0) {
		fprintf(stderr, "Failed to initialize GraphQL server: %d\n",
		    error);
		atomspace_destroy(as);
		return;
	}

	/* Get schema */
	schema = hgql_get_schema(server);
	printf("GraphQL Schema:\n");
	printf("---\n%s---\n", schema);

	/* Cleanup */
	hgql_server_destroy(server);
	atomspace_destroy(as);
}

int
main(int argc, char *argv[])
{
	(void)argc;
	(void)argv;

	printf("HyperCog Demo - OpenCog AtomSpace with HypergraphQL\n");
	printf("====================================================\n");

	demo_basic_atomspace();
	demo_hypercog_integration();
	demo_distributed_atomspace();
	demo_graphql_schema();

	printf("\n=== Demo Complete ===\n");
	printf("For more information, see docs/HYPERCOG.md\n");

	return (0);
}
