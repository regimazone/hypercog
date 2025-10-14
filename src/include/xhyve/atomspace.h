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

/*
 * OpenCog AtomSpace integration for distributed hypervisor
 * Provides hypergraph knowledge representation across VM instances
 */

#define ATOMSPACE_MAX_NODES 1024
#define ATOMSPACE_MAX_LINKS 2048
#define ATOMSPACE_NAME_MAX 256

/* Atom types following OpenCog taxonomy */
typedef enum {
	ATOM_TYPE_NODE = 0,
	ATOM_TYPE_LINK,
	ATOM_TYPE_CONCEPT,
	ATOM_TYPE_PREDICATE,
	ATOM_TYPE_VARIABLE,
	ATOM_TYPE_EVALUATION,
	ATOM_TYPE_INHERITANCE,
	ATOM_TYPE_SIMILARITY,
	ATOM_TYPE_MAX
} atom_type_t;

/* Atom handle - unique identifier for atoms in the space */
typedef uint64_t atom_handle_t;

/* Truth value for probabilistic reasoning */
typedef struct {
	float strength;     /* confidence level [0,1] */
	float confidence;   /* weight of evidence [0,1] */
} truth_value_t;

/* Attention value for ECAN (Economic Attention Networks) */
typedef struct {
	int16_t sti;        /* Short-term importance */
	int16_t lti;        /* Long-term importance */
	int16_t vlti;       /* Very long-term importance */
} attention_value_t;

/* Atom structure - basic unit of knowledge */
typedef struct atom {
	atom_handle_t handle;
	atom_type_t type;
	char name[ATOMSPACE_NAME_MAX];
	truth_value_t tv;
	attention_value_t av;
	atom_handle_t *outgoing;  /* For links: array of target atoms */
	uint32_t outgoing_count;
	uint64_t timestamp;
	bool distributed;         /* Whether this atom is synchronized */
} atom_t;

/* AtomSpace - main knowledge repository */
typedef struct atomspace {
	atom_t *atoms;
	uint32_t atom_count;
	uint32_t atom_capacity;
	atom_handle_t next_handle;
	bool distributed_mode;
	char node_id[64];         /* Unique ID for this hypervisor node */
} atomspace_t;

/* AtomSpace operations */
int atomspace_init(atomspace_t **as, bool distributed);
void atomspace_destroy(atomspace_t *as);

atom_handle_t atomspace_add_node(atomspace_t *as, atom_type_t type, 
                                  const char *name);
atom_handle_t atomspace_add_link(atomspace_t *as, atom_type_t type,
                                  atom_handle_t *outgoing, uint32_t count);

atom_t* atomspace_get_atom(atomspace_t *as, atom_handle_t handle);
int atomspace_set_tv(atomspace_t *as, atom_handle_t handle, truth_value_t tv);
int atomspace_set_av(atomspace_t *as, atom_handle_t handle, attention_value_t av);

/* Distributed operations */
int atomspace_sync_start(atomspace_t *as);
int atomspace_sync_stop(atomspace_t *as);
int atomspace_broadcast_atom(atomspace_t *as, atom_handle_t handle);
int atomspace_receive_atom(atomspace_t *as, const atom_t *atom);

/* Query operations for pattern matching */
typedef struct query_result {
	atom_handle_t *handles;
	uint32_t count;
} query_result_t;

query_result_t* atomspace_query(atomspace_t *as, const char *pattern);
void atomspace_free_query_result(query_result_t *result);
