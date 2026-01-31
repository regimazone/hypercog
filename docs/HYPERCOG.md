# HyperCog: OpenCog AtomSpace with HypergraphQL for Distributed Hypervisors

## Overview

HyperCog integrates OpenCog's AtomSpace knowledge representation system with HyperKit's hypervisor capabilities, providing a distributed cognitive architecture across virtualized environments. This enables:

- **Distributed Knowledge Management**: Share and synchronize cognitive knowledge across multiple VM instances
- **GraphQL Query Interface**: Query and manipulate the hypergraph knowledge base via HypergraphQL
- **VM-Aware Reasoning**: Track and reason about VM states, relationships, and metadata
- **Scalable Architecture**: Distribute cognitive processing across multiple hypervisor nodes

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        HyperCog Layer                        │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────────┐         ┌─────────────────────┐      │
│  │   AtomSpace      │◄────────┤  HypergraphQL       │      │
│  │   (Knowledge)    │         │  (Query Interface)  │      │
│  └────────┬─────────┘         └──────────┬──────────┘      │
│           │                               │                  │
│           │                               │                  │
│  ┌────────▼───────────────────────────────▼──────────┐      │
│  │         Distributed Sync Layer                     │      │
│  │         (Cross-Hypervisor Communication)           │      │
│  └───────────────────────┬────────────────────────────┘      │
│                          │                                    │
├──────────────────────────┼────────────────────────────────────┤
│      HyperKit Hypervisor │                                    │
├──────────────────────────┼────────────────────────────────────┤
│  ┌───────────────┐  ┌────▼──────────┐  ┌───────────────┐    │
│  │   VM 0        │  │   VM 1        │  │   VM N        │    │
│  │   (vcpu 0)    │  │   (vcpu 1)    │  │   (vcpu N)    │    │
│  └───────────────┘  └───────────────┘  └───────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

## Components

### 1. AtomSpace (`src/lib/atomspace.c`)

The AtomSpace is the core knowledge representation system based on OpenCog's design:

- **Atoms**: Basic units of knowledge (nodes and links)
- **Truth Values**: Probabilistic strength and confidence for each atom
- **Attention Values**: ECAN-based importance metrics (STI, LTI, VLTI)
- **Pattern Matching**: Query atoms based on patterns

#### Key Data Structures:

```c
typedef struct atom {
    atom_handle_t handle;          // Unique identifier
    atom_type_t type;               // Node or Link type
    char name[256];                 // Atom name
    truth_value_t tv;               // Probabilistic reasoning
    attention_value_t av;           // Attention allocation
    atom_handle_t *outgoing;        // For links: target atoms
    uint32_t outgoing_count;
    uint64_t timestamp;
    bool distributed;               // Sync status
} atom_t;
```

#### API Functions:

- `atomspace_init()`: Initialize atomspace
- `atomspace_add_node()`: Add a knowledge node
- `atomspace_add_link()`: Create relationships between atoms
- `atomspace_query()`: Pattern-based search
- `atomspace_broadcast_atom()`: Distribute to other nodes

### 2. HypergraphQL (`src/lib/hypergraphql.c`)

Provides a GraphQL interface for querying and manipulating the hypergraph:

#### GraphQL Schema:

```graphql
type Query {
  atom(handle: ID!): Atom
  node(name: String!): Node
  queryPattern(pattern: String!): [Atom]
  atoms: [Atom]
}

type Mutation {
  addNode(type: AtomType!, name: String!): Node
  addLink(type: AtomType!, outgoing: [ID!]!): Link
  setTruthValue(handle: ID!, strength: Float!, confidence: Float!): Atom
}

enum AtomType {
  NODE
  LINK
  CONCEPT
  PREDICATE
  VARIABLE
  EVALUATION
  INHERITANCE
  SIMILARITY
}
```

#### API Functions:

- `hgql_server_init()`: Initialize GraphQL server
- `hgql_server_start()`: Start listening on configured port
- `hgql_execute_query()`: Execute GraphQL query
- `hgql_resolve_*()`: Resolver functions for GraphQL fields

### 3. HyperCog Integration (`src/lib/hypercog.c`)

Bridges the hypervisor and cognitive architecture:

- **VM Lifecycle Hooks**: Track VM creation/destruction
- **Knowledge Operations**: Add/query VM-specific knowledge
- **Global Context**: Thread-safe access to HyperCog services

#### API Functions:

- `hypercog_init()`: Initialize HyperCog system
- `hypercog_start()`: Start all services
- `hypercog_vm_created()`: Hook for VM creation
- `hypercog_add_vm_knowledge()`: Store VM metadata
- `hypercog_query_vm_knowledge()`: Retrieve VM knowledge

## Usage

### Basic Initialization

```c
#include <xhyve/hypercog.h>

hypercog_ctx_t *ctx;
int error;

// Initialize with distributed mode enabled, GraphQL on port 8080
error = hypercog_init(&ctx, true, 8080);
if (error == 0) {
    hypercog_set_global_context(ctx);
    hypercog_start(ctx);
}
```

### Adding VM Knowledge

```c
// When a VM is created
hypercog_vm_created(ctx, vcpu_id);

// Add custom metadata
hypercog_add_vm_knowledge(ctx, vcpu_id, "os", "Linux");
hypercog_add_vm_knowledge(ctx, vcpu_id, "memory", "4096");
```

### Querying via GraphQL

Send HTTP POST to `http://localhost:8080/graphql`:

```graphql
query {
  queryPattern(pattern: "VM-0") {
    handle
    name
    type
    truthValue {
      strength
      confidence
    }
  }
}
```

### Direct AtomSpace Operations

```c
atomspace_t *as = ctx->atomspace;

// Add nodes
atom_handle_t concept1 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Cat");
atom_handle_t concept2 = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Animal");

// Create inheritance link
atom_handle_t outgoing[2] = {concept1, concept2};
atom_handle_t link = atomspace_add_link(as, ATOM_TYPE_INHERITANCE, 
                                        outgoing, 2);

// Set truth value
truth_value_t tv = {0.95f, 0.90f};  // 95% strength, 90% confidence
atomspace_set_tv(as, link, tv);

// Query
query_result_t *results = atomspace_query(as, "Cat");
for (uint32_t i = 0; i < results->count; i++) {
    atom_t *atom = atomspace_get_atom(as, results->handles[i]);
    printf("Found: %s\n", atom->name);
}
atomspace_free_query_result(results);
```

## Distributed Mode

When distributed mode is enabled, atoms are automatically synchronized across multiple hypervisor nodes:

1. **Node Discovery**: Hypervisor nodes discover each other via network
2. **Atom Broadcasting**: New/modified atoms are broadcast to peers
3. **Conflict Resolution**: Timestamps determine the latest version
4. **Eventually Consistent**: All nodes converge to the same knowledge state

### Configuration

```c
// Enable distributed mode
hypercog_init(&ctx, true, 8080);  // distributed=true

// Start synchronization
atomspace_sync_start(ctx->atomspace);
```

## Integration with HyperKit

HyperCog is designed to work seamlessly with HyperKit's VM lifecycle:

1. **VM Creation**: `hypercog_vm_created()` is called
   - Creates VM concept node in AtomSpace
   - Records creation timestamp
   - Broadcasts to distributed nodes

2. **VM Operation**: Knowledge can be added dynamically
   - Store runtime metrics
   - Track resource usage
   - Record inter-VM relationships

3. **VM Destruction**: `hypercog_vm_destroyed()` is called
   - Updates truth value to mark as inactive
   - Preserves historical knowledge

## Performance Considerations

- **Memory**: Each atom uses ~400 bytes (configurable limits)
- **Scalability**: Tested with 1000+ atoms and 2000+ links
- **Query Speed**: O(n) pattern matching (optimization possible)
- **Network**: Async broadcasting for distributed sync

## Future Enhancements

1. **Advanced Pattern Matching**: Implement full OpenCog pattern matcher
2. **PLN Integration**: Probabilistic Logic Networks for reasoning
3. **ECAN**: Economic Attention Network for attention allocation
4. **Scheme Interface**: Expose Scheme/Guile scripting API
5. **Web Dashboard**: Visual interface for exploring the hypergraph
6. **Persistent Storage**: Save/load atomspace to disk

## API Reference

See header files:
- `src/include/xhyve/atomspace.h` - AtomSpace API
- `src/include/xhyve/hypergraphql.h` - HypergraphQL API
- `src/include/xhyve/hypercog.h` - Integration API

## License

BSD License (same as HyperKit)
