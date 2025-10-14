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

#include <xhyve/atomspace.h>
#include <xhyve/hypergraphql.h>

/*
 * HyperCog - Integration layer between HyperKit hypervisor and OpenCog AtomSpace
 * Provides distributed cognitive architecture across virtualized environments
 */

/* Global HyperCog instance */
typedef struct hypercog_ctx {
	atomspace_t *atomspace;
	hgql_server_t *graphql_server;
	bool enabled;
	bool distributed;
	uint16_t graphql_port;
} hypercog_ctx_t;

/* Initialize HyperCog system */
int hypercog_init(hypercog_ctx_t **ctx, bool distributed, uint16_t port);

/* Shutdown HyperCog system */
void hypercog_destroy(hypercog_ctx_t *ctx);

/* Start/stop services */
int hypercog_start(hypercog_ctx_t *ctx);
int hypercog_stop(hypercog_ctx_t *ctx);

/* VM lifecycle hooks - integrate atomspace with VM operations */
int hypercog_vm_created(hypercog_ctx_t *ctx, int vcpu);
int hypercog_vm_destroyed(hypercog_ctx_t *ctx, int vcpu);

/* Knowledge operations accessible from hypervisor */
atom_handle_t hypercog_add_vm_knowledge(hypercog_ctx_t *ctx, int vcpu,
    const char *key, const char *value);
atom_handle_t hypercog_query_vm_knowledge(hypercog_ctx_t *ctx, int vcpu,
    const char *query);

/* Global context accessor */
hypercog_ctx_t* hypercog_get_global_context(void);
int hypercog_set_global_context(hypercog_ctx_t *ctx);
