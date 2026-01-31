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
#include <time.h>
#include <errno.h>
#include <xhyve/atomspace.h>

/*
 * OpenCog AtomSpace implementation for distributed hypervisor
 */

int
atomspace_init(atomspace_t **as, bool distributed)
{
	atomspace_t *space;

	if (as == NULL) {
		return (EINVAL);
	}

	space = calloc(1, sizeof(atomspace_t));
	if (space == NULL) {
		return (ENOMEM);
	}

	space->atom_capacity = ATOMSPACE_MAX_NODES + ATOMSPACE_MAX_LINKS;
	space->atoms = calloc(space->atom_capacity, sizeof(atom_t));
	if (space->atoms == NULL) {
		free(space);
		return (ENOMEM);
	}

	space->atom_count = 0;
	space->next_handle = 1;
	space->distributed_mode = distributed;

	/* Generate unique node ID for distributed mode */
	if (distributed) {
		snprintf(space->node_id, sizeof(space->node_id),
		    "hypervisor-%lu", (unsigned long)time(NULL));
	}

	*as = space;
	return (0);
}

void
atomspace_destroy(atomspace_t *as)
{
	if (as == NULL) {
		return;
	}

	if (as->atoms != NULL) {
		/* Free outgoing arrays for links */
		for (uint32_t i = 0; i < as->atom_count; i++) {
			if (as->atoms[i].outgoing != NULL) {
				free(as->atoms[i].outgoing);
			}
		}
		free(as->atoms);
	}

	free(as);
}

atom_handle_t
atomspace_add_node(atomspace_t *as, atom_type_t type, const char *name)
{
	atom_t *atom;

	if (as == NULL || name == NULL) {
		return (0);
	}

	if (as->atom_count >= as->atom_capacity) {
		return (0);
	}

	atom = &as->atoms[as->atom_count];
	atom->handle = as->next_handle++;
	atom->type = type;
	strncpy(atom->name, name, ATOMSPACE_NAME_MAX - 1);
	atom->name[ATOMSPACE_NAME_MAX - 1] = '\0';

	/* Initialize default values */
	atom->tv.strength = 1.0f;
	atom->tv.confidence = 1.0f;
	atom->av.sti = 0;
	atom->av.lti = 0;
	atom->av.vlti = 0;
	atom->outgoing = NULL;
	atom->outgoing_count = 0;
	atom->timestamp = (uint64_t)time(NULL);
	atom->distributed = as->distributed_mode;

	as->atom_count++;

	/* Broadcast if in distributed mode */
	if (as->distributed_mode) {
		atomspace_broadcast_atom(as, atom->handle);
	}

	return (atom->handle);
}

atom_handle_t
atomspace_add_link(atomspace_t *as, atom_type_t type,
    atom_handle_t *outgoing, uint32_t count)
{
	atom_t *atom;

	if (as == NULL || outgoing == NULL || count == 0) {
		return (0);
	}

	if (as->atom_count >= as->atom_capacity) {
		return (0);
	}

	atom = &as->atoms[as->atom_count];
	atom->handle = as->next_handle++;
	atom->type = type;
	snprintf(atom->name, ATOMSPACE_NAME_MAX, "Link-%lu",
	    (unsigned long)atom->handle);

	/* Copy outgoing set */
	atom->outgoing = calloc(count, sizeof(atom_handle_t));
	if (atom->outgoing == NULL) {
		return (0);
	}
	memcpy(atom->outgoing, outgoing, count * sizeof(atom_handle_t));
	atom->outgoing_count = count;

	/* Initialize default values */
	atom->tv.strength = 1.0f;
	atom->tv.confidence = 1.0f;
	atom->av.sti = 0;
	atom->av.lti = 0;
	atom->av.vlti = 0;
	atom->timestamp = (uint64_t)time(NULL);
	atom->distributed = as->distributed_mode;

	as->atom_count++;

	/* Broadcast if in distributed mode */
	if (as->distributed_mode) {
		atomspace_broadcast_atom(as, atom->handle);
	}

	return (atom->handle);
}

atom_t*
atomspace_get_atom(atomspace_t *as, atom_handle_t handle)
{
	if (as == NULL || handle == 0) {
		return (NULL);
	}

	for (uint32_t i = 0; i < as->atom_count; i++) {
		if (as->atoms[i].handle == handle) {
			return (&as->atoms[i]);
		}
	}

	return (NULL);
}

int
atomspace_set_tv(atomspace_t *as, atom_handle_t handle, truth_value_t tv)
{
	atom_t *atom;

	atom = atomspace_get_atom(as, handle);
	if (atom == NULL) {
		return (ENOENT);
	}

	atom->tv = tv;

	/* Sync if distributed */
	if (as->distributed_mode) {
		atomspace_broadcast_atom(as, handle);
	}

	return (0);
}

int
atomspace_set_av(atomspace_t *as, atom_handle_t handle, attention_value_t av)
{
	atom_t *atom;

	atom = atomspace_get_atom(as, handle);
	if (atom == NULL) {
		return (ENOENT);
	}

	atom->av = av;

	return (0);
}

int
atomspace_sync_start(atomspace_t *as)
{
	if (as == NULL) {
		return (EINVAL);
	}

	if (!as->distributed_mode) {
		return (ENOTSUP);
	}

	/* TODO: Initialize network connections to other hypervisor nodes */
	return (0);
}

int
atomspace_sync_stop(atomspace_t *as)
{
	if (as == NULL) {
		return (EINVAL);
	}

	/* TODO: Close network connections */
	return (0);
}

int
atomspace_broadcast_atom(atomspace_t *as, atom_handle_t handle)
{
	atom_t *atom;

	if (as == NULL || !as->distributed_mode) {
		return (EINVAL);
	}

	atom = atomspace_get_atom(as, handle);
	if (atom == NULL) {
		return (ENOENT);
	}

	/* TODO: Serialize and send atom to other nodes */
	/* For now, just mark as synchronized */
	atom->distributed = true;

	return (0);
}

int
atomspace_receive_atom(atomspace_t *as, const atom_t *atom)
{
	if (as == NULL || atom == NULL) {
		return (EINVAL);
	}

	/* TODO: Deserialize and merge atom into local atomspace */
	/* Handle conflict resolution based on timestamps */

	return (0);
}

query_result_t*
atomspace_query(atomspace_t *as, const char *pattern)
{
	query_result_t *result;

	if (as == NULL || pattern == NULL) {
		return (NULL);
	}

	result = calloc(1, sizeof(query_result_t));
	if (result == NULL) {
		return (NULL);
	}

	/* Simple pattern matching: match by name prefix */
	uint32_t match_count = 0;
	atom_handle_t *temp = calloc(as->atom_count, sizeof(atom_handle_t));
	if (temp == NULL) {
		free(result);
		return (NULL);
	}

	for (uint32_t i = 0; i < as->atom_count; i++) {
		if (strstr(as->atoms[i].name, pattern) != NULL) {
			temp[match_count++] = as->atoms[i].handle;
		}
	}

	if (match_count > 0) {
		result->handles = calloc(match_count, sizeof(atom_handle_t));
		if (result->handles != NULL) {
			memcpy(result->handles, temp, 
			    match_count * sizeof(atom_handle_t));
			result->count = match_count;
		}
	}

	free(temp);
	return (result);
}

void
atomspace_free_query_result(query_result_t *result)
{
	if (result == NULL) {
		return;
	}

	if (result->handles != NULL) {
		free(result->handles);
	}
	free(result);
}
