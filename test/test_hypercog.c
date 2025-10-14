/*-
 * Basic test for HyperCog integration
 * Compile: gcc -I../src/include test_hypercog.c ../src/lib/hypercog.c ../src/lib/hypergraphql.c ../src/lib/atomspace.c -o test_hypercog
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <xhyve/hypercog.h>
#include <xhyve/atomspace.h>
#include <xhyve/hypergraphql.h>

static void
test_hypercog_init(void)
{
	hypercog_ctx_t *ctx;
	int error;

	printf("Testing hypercog_init/destroy...");

	error = hypercog_init(&ctx, false, 8080);
	assert(error == 0);
	assert(ctx != NULL);
	assert(ctx->atomspace != NULL);
	assert(ctx->graphql_server != NULL);
	assert(ctx->enabled == true);
	assert(ctx->distributed == false);
	assert(ctx->graphql_port == 8080);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_hypercog_distributed(void)
{
	hypercog_ctx_t *ctx;
	int error;

	printf("Testing hypercog distributed mode...");

	error = hypercog_init(&ctx, true, 8081);
	assert(error == 0);
	assert(ctx->distributed == true);
	assert(ctx->atomspace->distributed_mode == true);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_vm_lifecycle(void)
{
	hypercog_ctx_t *ctx;
	int error;

	printf("Testing VM lifecycle hooks...");

	hypercog_init(&ctx, false, 8080);

	/* Test VM creation */
	error = hypercog_vm_created(ctx, 0);
	assert(error == 0);
	assert(ctx->atomspace->atom_count > 0);

	/* Test VM destruction */
	error = hypercog_vm_destroyed(ctx, 0);
	assert(error == 0);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_vm_knowledge(void)
{
	hypercog_ctx_t *ctx;
	atom_handle_t h1, h2, query_result;

	printf("Testing VM knowledge operations...");

	hypercog_init(&ctx, false, 8080);

	/* Add VM knowledge */
	h1 = hypercog_add_vm_knowledge(ctx, 0, "os", "Linux");
	assert(h1 != 0);

	h2 = hypercog_add_vm_knowledge(ctx, 0, "memory", "4096");
	assert(h2 != 0);
	assert(h2 != h1);

	/* Verify atoms were created */
	assert(ctx->atomspace->atom_count >= 4);  /* 2 keys + 2 values */

	/* Query VM knowledge */
	query_result = hypercog_query_vm_knowledge(ctx, 0, "os");
	assert(query_result != 0);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_global_context(void)
{
	hypercog_ctx_t *ctx;
	hypercog_ctx_t *retrieved;
	int error;

	printf("Testing global context...");

	error = hypercog_init(&ctx, false, 8080);
	assert(error == 0);

	error = hypercog_set_global_context(ctx);
	assert(error == 0);

	retrieved = hypercog_get_global_context();
	assert(retrieved == ctx);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_start_stop(void)
{
	hypercog_ctx_t *ctx;
	int error;

	printf("Testing start/stop services...");

	hypercog_init(&ctx, false, 8080);

	/* Test start */
	error = hypercog_start(ctx);
	assert(error == 0);
	assert(ctx->graphql_server->running == true);

	/* Test stop */
	error = hypercog_stop(ctx);
	assert(error == 0);
	assert(ctx->graphql_server->running == false);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

static void
test_multiple_vms(void)
{
	hypercog_ctx_t *ctx;
	atom_handle_t h1, h2, h3;

	printf("Testing multiple VMs...");

	hypercog_init(&ctx, false, 8080);

	/* Create multiple VMs */
	hypercog_vm_created(ctx, 0);
	hypercog_vm_created(ctx, 1);
	hypercog_vm_created(ctx, 2);

	/* Add knowledge to each VM */
	h1 = hypercog_add_vm_knowledge(ctx, 0, "name", "vm0");
	h2 = hypercog_add_vm_knowledge(ctx, 1, "name", "vm1");
	h3 = hypercog_add_vm_knowledge(ctx, 2, "name", "vm2");

	assert(h1 != 0);
	assert(h2 != 0);
	assert(h3 != 0);

	/* Verify they're all different */
	assert(h1 != h2);
	assert(h2 != h3);
	assert(h1 != h3);

	/* Query each VM */
	assert(hypercog_query_vm_knowledge(ctx, 0, "name") != 0);
	assert(hypercog_query_vm_knowledge(ctx, 1, "name") != 0);
	assert(hypercog_query_vm_knowledge(ctx, 2, "name") != 0);

	hypercog_destroy(ctx);

	printf(" OK\n");
}

int
main(void)
{
	printf("HyperCog Test Suite\n");
	printf("===================\n\n");

	test_hypercog_init();
	test_hypercog_distributed();
	test_vm_lifecycle();
	test_vm_knowledge();
	test_global_context();
	test_start_stop();
	test_multiple_vms();

	printf("\nAll tests passed!\n");
	return 0;
}
