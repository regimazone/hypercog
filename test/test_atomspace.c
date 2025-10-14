/*-
 * Basic test for AtomSpace functionality
 * Compile: gcc -I../src/include test_atomspace.c ../src/lib/atomspace.c -o test_atomspace
 */

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <xhyve/atomspace.h>

static void
test_init_destroy(void)
{
	atomspace_t *as;
	int error;

	printf("Testing atomspace_init/destroy...");

	error = atomspace_init(&as, false);
	assert(error == 0);
	assert(as != NULL);
	assert(as->atom_count == 0);
	assert(as->distributed_mode == false);

	atomspace_destroy(as);
	printf(" OK\n");
}

static void
test_add_node(void)
{
	atomspace_t *as;
	atom_handle_t h1, h2;
	atom_t *atom;

	printf("Testing atomspace_add_node...");

	atomspace_init(&as, false);

	h1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "TestConcept");
	assert(h1 != 0);
	assert(as->atom_count == 1);

	h2 = atomspace_add_node(as, ATOM_TYPE_PREDICATE, "TestPredicate");
	assert(h2 != 0);
	assert(h2 != h1);
	assert(as->atom_count == 2);

	atom = atomspace_get_atom(as, h1);
	assert(atom != NULL);
	assert(strcmp(atom->name, "TestConcept") == 0);
	assert(atom->type == ATOM_TYPE_CONCEPT);

	atomspace_destroy(as);
	printf(" OK\n");
}

static void
test_add_link(void)
{
	atomspace_t *as;
	atom_handle_t h1, h2, link;
	atom_handle_t outgoing[2];
	atom_t *link_atom;

	printf("Testing atomspace_add_link...");

	atomspace_init(&as, false);

	h1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Cat");
	h2 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Animal");

	outgoing[0] = h1;
	outgoing[1] = h2;
	link = atomspace_add_link(as, ATOM_TYPE_INHERITANCE, outgoing, 2);

	assert(link != 0);
	assert(as->atom_count == 3);

	link_atom = atomspace_get_atom(as, link);
	assert(link_atom != NULL);
	assert(link_atom->type == ATOM_TYPE_INHERITANCE);
	assert(link_atom->outgoing_count == 2);
	assert(link_atom->outgoing[0] == h1);
	assert(link_atom->outgoing[1] == h2);

	atomspace_destroy(as);
	printf(" OK\n");
}

static void
test_truth_value(void)
{
	atomspace_t *as;
	atom_handle_t h;
	atom_t *atom;
	truth_value_t tv;
	int error;

	printf("Testing truth value operations...");

	atomspace_init(&as, false);

	h = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Test");
	atom = atomspace_get_atom(as, h);

	/* Check default values */
	assert(atom->tv.strength == 1.0f);
	assert(atom->tv.confidence == 1.0f);

	/* Set new values */
	tv.strength = 0.75f;
	tv.confidence = 0.80f;
	error = atomspace_set_tv(as, h, tv);
	assert(error == 0);

	atom = atomspace_get_atom(as, h);
	assert(atom->tv.strength == 0.75f);
	assert(atom->tv.confidence == 0.80f);

	atomspace_destroy(as);
	printf(" OK\n");
}

static void
test_query(void)
{
	atomspace_t *as;
	atom_handle_t h1, h2, h3;
	query_result_t *result;

	printf("Testing atomspace_query...");

	atomspace_init(&as, false);

	h1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Dog");
	h2 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Cat");
	h3 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Dogfish");

	/* Query for "Dog" */
	result = atomspace_query(as, "Dog");
	assert(result != NULL);
	assert(result->count == 2);  /* Should match "Dog" and "Dogfish" */

	atomspace_free_query_result(result);

	/* Query for "Cat" */
	result = atomspace_query(as, "Cat");
	assert(result != NULL);
	assert(result->count == 1);
	assert(result->handles[0] == h2);

	atomspace_free_query_result(result);

	atomspace_destroy(as);
	printf(" OK\n");
}

static void
test_distributed_mode(void)
{
	atomspace_t *as;
	atom_handle_t h;

	printf("Testing distributed mode...");

	atomspace_init(&as, true);
	assert(as->distributed_mode == true);
	assert(strlen(as->node_id) > 0);

	h = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "DistributedConcept");
	assert(h != 0);

	/* Should be marked for distribution */
	atom_t *atom = atomspace_get_atom(as, h);
	assert(atom->distributed == true);

	atomspace_destroy(as);
	printf(" OK\n");
}

int
main(void)
{
	printf("AtomSpace Test Suite\n");
	printf("====================\n\n");

	test_init_destroy();
	test_add_node();
	test_add_link();
	test_truth_value();
	test_query();
	test_distributed_mode();

	printf("\nAll tests passed!\n");
	return 0;
}
