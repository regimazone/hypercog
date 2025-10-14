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
#include <xhyve/hypercog.h>
#include <xhyve/atomspace.h>
#include <xhyve/hypergraphql.h>

/*
 * HyperCog implementation - Bridge between hypervisor and cognitive architecture
 */

static hypercog_ctx_t *global_hypercog_ctx = NULL;

int
hypercog_init(hypercog_ctx_t **ctx, bool distributed, uint16_t port)
{
	hypercog_ctx_t *hc;
	int error;

	if (ctx == NULL) {
		return (EINVAL);
	}

	hc = calloc(1, sizeof(hypercog_ctx_t));
	if (hc == NULL) {
		return (ENOMEM);
	}

	/* Initialize atomspace */
	error = atomspace_init(&hc->atomspace, distributed);
	if (error != 0) {
		free(hc);
		return (error);
	}

	/* Initialize GraphQL server */
	error = hgql_server_init(&hc->graphql_server, hc->atomspace, port);
	if (error != 0) {
		atomspace_destroy(hc->atomspace);
		free(hc);
		return (error);
	}

	hc->enabled = true;
	hc->distributed = distributed;
	hc->graphql_port = port;

	*ctx = hc;
	return (0);
}

void
hypercog_destroy(hypercog_ctx_t *ctx)
{
	if (ctx == NULL) {
		return;
	}

	if (ctx->graphql_server != NULL) {
		hgql_server_destroy(ctx->graphql_server);
	}

	if (ctx->atomspace != NULL) {
		atomspace_destroy(ctx->atomspace);
	}

	free(ctx);
}

int
hypercog_start(hypercog_ctx_t *ctx)
{
	int error;

	if (ctx == NULL || !ctx->enabled) {
		return (EINVAL);
	}

	/* Start distributed sync if enabled */
	if (ctx->distributed) {
		error = atomspace_sync_start(ctx->atomspace);
		if (error != 0) {
			return (error);
		}
	}

	/* Start GraphQL server */
	error = hgql_server_start(ctx->graphql_server);
	if (error != 0) {
		if (ctx->distributed) {
			atomspace_sync_stop(ctx->atomspace);
		}
		return (error);
	}

	return (0);
}

int
hypercog_stop(hypercog_ctx_t *ctx)
{
	if (ctx == NULL) {
		return (EINVAL);
	}

	if (ctx->graphql_server != NULL) {
		hgql_server_stop(ctx->graphql_server);
	}

	if (ctx->distributed && ctx->atomspace != NULL) {
		atomspace_sync_stop(ctx->atomspace);
	}

	return (0);
}

int
hypercog_vm_created(hypercog_ctx_t *ctx, int vcpu)
{
	atom_handle_t vm_node, timestamp_node, link;
	atom_handle_t outgoing[2];
	char vm_name[256];
	char timestamp[64];
	time_t now;

	if (ctx == NULL || !ctx->enabled) {
		return (0);
	}

	/* Create VM node in atomspace */
	snprintf(vm_name, sizeof(vm_name), "VM-VCPU-%d", vcpu);
	vm_node = atomspace_add_node(ctx->atomspace, ATOM_TYPE_CONCEPT, vm_name);

	/* Add timestamp */
	now = time(NULL);
	snprintf(timestamp, sizeof(timestamp), "%ld", (long)now);
	timestamp_node = atomspace_add_node(ctx->atomspace, ATOM_TYPE_NODE,
	    timestamp);

	/* Create link between VM and timestamp */
	outgoing[0] = vm_node;
	outgoing[1] = timestamp_node;
	link = atomspace_add_link(ctx->atomspace, ATOM_TYPE_EVALUATION,
	    outgoing, 2);

	return (link != 0 ? 0 : ENOMEM);
}

int
hypercog_vm_destroyed(hypercog_ctx_t *ctx, int vcpu)
{
	char vm_name[256];
	query_result_t *result;

	if (ctx == NULL || !ctx->enabled) {
		return (0);
	}

	/* Query and mark VM as destroyed */
	snprintf(vm_name, sizeof(vm_name), "VM-VCPU-%d", vcpu);
	result = atomspace_query(ctx->atomspace, vm_name);

	if (result != NULL && result->count > 0) {
		/* Update truth value to indicate VM is no longer active */
		truth_value_t tv = {0.0f, 1.0f};
		atomspace_set_tv(ctx->atomspace, result->handles[0], tv);
	}

	atomspace_free_query_result(result);
	return (0);
}

atom_handle_t
hypercog_add_vm_knowledge(hypercog_ctx_t *ctx, int vcpu,
    const char *key, const char *value)
{
	atom_handle_t key_node, value_node, link;
	atom_handle_t outgoing[2];
	char vm_key[512];

	if (ctx == NULL || !ctx->enabled || key == NULL || value == NULL) {
		return (0);
	}

	/* Create namespaced key for VM */
	snprintf(vm_key, sizeof(vm_key), "VM-%d:%s", vcpu, key);

	key_node = atomspace_add_node(ctx->atomspace, ATOM_TYPE_PREDICATE,
	    vm_key);
	value_node = atomspace_add_node(ctx->atomspace, ATOM_TYPE_CONCEPT,
	    value);

	/* Create evaluation link */
	outgoing[0] = key_node;
	outgoing[1] = value_node;
	link = atomspace_add_link(ctx->atomspace, ATOM_TYPE_EVALUATION,
	    outgoing, 2);

	return (link);
}

atom_handle_t
hypercog_query_vm_knowledge(hypercog_ctx_t *ctx, int vcpu, const char *query)
{
	query_result_t *result;
	atom_handle_t handle = 0;
	char vm_query[512];

	if (ctx == NULL || !ctx->enabled || query == NULL) {
		return (0);
	}

	/* Create namespaced query for VM */
	snprintf(vm_query, sizeof(vm_query), "VM-%d:%s", vcpu, query);

	result = atomspace_query(ctx->atomspace, vm_query);
	if (result != NULL && result->count > 0) {
		handle = result->handles[0];
	}

	atomspace_free_query_result(result);
	return (handle);
}

hypercog_ctx_t*
hypercog_get_global_context(void)
{
	return (global_hypercog_ctx);
}

int
hypercog_set_global_context(hypercog_ctx_t *ctx)
{
	global_hypercog_ctx = ctx;
	return (0);
}
