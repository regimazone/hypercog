# HyperCog Architecture

## System Overview

```
┌─────────────────────────────────────────────────────────────────────┐
│                         User Applications                            │
│  (GraphQL Clients, CLI Tools, Monitoring Dashboards)                │
└───────────────────────────────┬─────────────────────────────────────┘
                                │ HTTP/GraphQL
                                │ Port 8080
┌───────────────────────────────▼─────────────────────────────────────┐
│                      HypergraphQL Server                             │
│  ┌────────────────────────────────────────────────────────────┐    │
│  │  GraphQL Schema (Type System, Query/Mutation/Subscription)│    │
│  └────────────────┬───────────────────────────────────────────┘    │
│                   │                                                  │
│  ┌────────────────▼───────────────────────────────────────────┐    │
│  │  Resolvers (atom, node, queryPattern, addNode, addLink)   │    │
│  └────────────────┬───────────────────────────────────────────┘    │
└───────────────────┼──────────────────────────────────────────────────┘
                    │ C API
┌───────────────────▼──────────────────────────────────────────────────┐
│                      HyperCog Integration Layer                       │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Global Context Manager (Singleton, Thread-Safe)             │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  VM Lifecycle Hooks (vm_created, vm_destroyed)               │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Knowledge Operations (add_knowledge, query_knowledge)       │   │
│  └─────────────────────┬────────────────────────────────────────┘   │
└────────────────────────┼─────────────────────────────────────────────┘
                         │
┌────────────────────────▼─────────────────────────────────────────────┐
│                      OpenCog AtomSpace                                │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Knowledge Graph (Atoms, Nodes, Links)                       │   │
│  │  • Atoms: Basic units (handles, types, names)                │   │
│  │  • Nodes: Concepts, Predicates, Variables                    │   │
│  │  • Links: Inheritance, Evaluation, Similarity                │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Truth Values (Probabilistic Reasoning)                      │   │
│  │  • Strength: [0.0 - 1.0]                                     │   │
│  │  • Confidence: [0.0 - 1.0]                                   │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Attention Values (ECAN)                                     │   │
│  │  • STI: Short-term importance                                │   │
│  │  • LTI: Long-term importance                                 │   │
│  │  • VLTI: Very long-term importance                           │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Pattern Matcher (Query Engine)                              │   │
│  │  • Substring matching (current)                              │   │
│  │  • Full pattern matching (future)                            │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Distributed Synchronization Layer                           │   │
│  │  • Node discovery (stub)                                     │   │
│  │  • Atom broadcasting (stub)                                  │   │
│  │  • Conflict resolution                                       │   │
│  └─────────────────────┬────────────────────────────────────────┘   │
└────────────────────────┼─────────────────────────────────────────────┘
                         │ Network (TCP/IP - future)
┌────────────────────────▼─────────────────────────────────────────────┐
│                    HyperKit Hypervisor (VMM)                          │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  VM Management (create, run, suspend, destroy)               │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  vCPU Scheduling (BSP, multi-core support)                   │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  Memory Management (GPA mapping, EPT)                        │   │
│  └──────────────────────────────────────────────────────────────┘   │
│  ┌──────────────────────────────────────────────────────────────┐   │
│  │  I/O Emulation (PCI, UART, Block, Network)                   │   │
│  └──────────────────────────────────────────────────────────────┘   │
└───────────────────────────────────────────────────────────────────────┘
```

## Data Flow

### 1. VM Creation Flow

```
Hypervisor
    │
    ├─> xh_vcpu_create(vcpu)
    │
    ├─> hypercog_vm_created(ctx, vcpu)
    │       │
    │       ├─> atomspace_add_node(as, CONCEPT, "VM-VCPU-0")
    │       │       │
    │       │       └─> atoms[count++] = {handle, type, name, tv, av}
    │       │
    │       ├─> atomspace_add_node(as, NODE, timestamp)
    │       │
    │       └─> atomspace_add_link(as, EVALUATION, [vm_node, ts_node])
    │               │
    │               └─> if (distributed) -> atomspace_broadcast_atom()
    │
    └─> VM Running
```

### 2. Knowledge Addition Flow

```
Application
    │
    ├─> hypercog_add_vm_knowledge(ctx, vcpu, "os", "Linux")
    │       │
    │       ├─> atomspace_add_node(as, PREDICATE, "VM-0:os")
    │       │
    │       ├─> atomspace_add_node(as, CONCEPT, "Linux")
    │       │
    │       └─> atomspace_add_link(as, EVALUATION, [predicate, concept])
    │               │
    │               └─> if (distributed) -> atomspace_broadcast_atom()
    │
    └─> Knowledge Stored
```

### 3. GraphQL Query Flow

```
HTTP Client
    │
    ├─> POST /graphql
    │   Body: { query: "{ queryPattern(pattern: \"VM-0\") { ... } }" }
    │
    ├─> hgql_execute_query(server, query)
    │       │
    │       ├─> Parse GraphQL query
    │       │
    │       ├─> hgql_resolve_query_pattern(as, "VM-0")
    │       │       │
    │       │       ├─> atomspace_query(as, "VM-0")
    │       │       │       │
    │       │       │       └─> Linear scan: if (strstr(atom.name, "VM-0"))
    │       │       │
    │       │       └─> For each result -> hgql_resolve_atom()
    │       │               │
    │       │               └─> Serialize to JSON
    │       │
    │       └─> Build GraphQL response
    │
    └─> Response: { data: { queryPattern: [...] } }
```

### 4. Distributed Sync Flow (Conceptual)

```
Node A                          Node B
    │                               │
    ├─> atomspace_add_node()        │
    │       │                       │
    │       ├─> atom created        │
    │       │                       │
    │       └─> atomspace_broadcast_atom()
    │               │               │
    │               └─────────────> ├─> atomspace_receive_atom()
    │                               │       │
    │                               │       ├─> Check timestamp
    │                               │       │
    │                               │       └─> Merge or reject
    │                               │
    └─<──────────────────────────── ├─> ACK
                                    │
```

## Component Interactions

### AtomSpace Core

```c
typedef struct atom {
    atom_handle_t handle;       // Unique ID (64-bit)
    atom_type_t type;          // Enum: NODE, LINK, CONCEPT, etc.
    char name[256];            // Human-readable name
    truth_value_t tv;          // {strength, confidence}
    attention_value_t av;      // {sti, lti, vlti}
    atom_handle_t *outgoing;   // Array of target handles (for links)
    uint32_t outgoing_count;   // Size of outgoing array
    uint64_t timestamp;        // Creation/modification time
    bool distributed;          // Sync flag
} atom_t;
```

### HypergraphQL Schema

```graphql
type Atom {
  handle: ID!
  type: AtomType!
  name: String!
  truthValue: TruthValue!
  attentionValue: AttentionValue!
  timestamp: String!
}

type TruthValue {
  strength: Float!   # [0.0, 1.0]
  confidence: Float! # [0.0, 1.0]
}

type AttentionValue {
  sti: Int!   # Short-term importance
  lti: Int!   # Long-term importance
  vlti: Int!  # Very long-term importance
}
```

## Memory Layout

### AtomSpace Memory Structure

```
atomspace_t (48 bytes)
    │
    ├─> atoms* ───────┐
    │                 │
    │                 ▼
    │             ┌────────────────┬────────────────┬────────────────┐
    │             │ atom_t[0]      │ atom_t[1]      │ atom_t[N]      │
    │             │ (~400 bytes)   │ (~400 bytes)   │ (~400 bytes)   │
    │             └────────────────┴────────────────┴────────────────┘
    │                     │
    │                     ├─> outgoing* ───┐
    │                     │                 │
    │                     │                 ▼
    │                     │             ┌──────┬──────┬──────┐
    │                     │             │ h[0] │ h[1] │ h[N] │
    │                     │             └──────┴──────┴──────┘
    │
    ├─> atom_count: uint32_t
    ├─> atom_capacity: uint32_t
    ├─> next_handle: atom_handle_t
    ├─> distributed_mode: bool
    └─> node_id[64]: char
```

### Memory Usage

- Empty atomspace: 48 bytes
- Per node: ~400 bytes (includes atom_t + padding)
- Per link: ~400 bytes + (8 * outgoing_count)
- 1000 atoms: ~400 KB
- 10000 atoms: ~4 MB

## Threading Model

```
Main Thread
    │
    ├─> hypercog_init()
    │       │
    │       └─> atomspace_init()
    │               │
    │               └─> Allocate shared memory
    │
    ├─> hypercog_start()
    │       │
    │       └─> hgql_server_start()
    │               │
    │               └─> spawn server_thread ──┐
    │                                         │
    │                                         ▼
    ├─> vCPU threads                   GraphQL Server Thread
    │   (one per VM)                        │
    │       │                               ├─> Listen on port 8080
    │       ├─> VM execution                │
    │       │                               ├─> Accept connections
    │       ├─> hypercog_add_vm_knowledge() │
    │       │       │                       ├─> Parse queries
    │       │       └─> [mutex lock]        │
    │       │           atomspace ops       ├─> Execute resolvers
    │       │           [mutex unlock]      │       │
    │       │                               │       └─> [mutex lock]
    │       │                               │           atomspace ops
    │       │                               │           [mutex unlock]
    │       │                               │
    └───────┴───────────────────────────────┴─> Shared AtomSpace
```

## Performance Characteristics

### Time Complexity

| Operation | Current | Optimized (Future) |
|-----------|---------|-------------------|
| Add node | O(1) | O(1) |
| Add link | O(1) | O(1) |
| Get atom by handle | O(n) | O(1) with hash table |
| Query pattern | O(n) | O(log n) with index |
| Set truth value | O(n) | O(1) with hash table |
| Broadcast atom | O(1) stub | O(k) where k=nodes |

### Space Complexity

| Structure | Size | Notes |
|-----------|------|-------|
| atomspace_t | 48 bytes | Fixed overhead |
| atom_t | ~400 bytes | Per atom |
| outgoing array | 8 * n bytes | Per link, n=targets |
| Query result | 16 + 8*m bytes | m=matches |

## Extension Points

### 1. Custom Atom Types

```c
// In atomspace.h
typedef enum {
    ATOM_TYPE_CUSTOM_START = 1000,
    ATOM_TYPE_VM_STATE,
    ATOM_TYPE_NETWORK_TOPOLOGY,
    ATOM_TYPE_RESOURCE_POOL,
    // ...
} custom_atom_types_t;
```

### 2. Custom Resolvers

```c
// In hypergraphql.c
char* hgql_resolve_custom(atomspace_t *as, const char *custom_query) {
    // Custom resolution logic
    return json_result;
}
```

### 3. Network Backend

```c
// In atomspace.c
int atomspace_broadcast_atom(atomspace_t *as, atom_handle_t handle) {
    // Serialize atom
    uint8_t *buffer = serialize_atom(atom);
    
    // Send to all peers
    for (each peer) {
        tcp_send(peer, buffer, size);
    }
    
    free(buffer);
    return 0;
}
```

## Future Enhancements

1. **Hash-based indexing** for O(1) lookups
2. **Network layer** for true distributed sync
3. **Persistent storage** with RocksDB or LMDB
4. **Pattern matcher** with variable binding
5. **PLN reasoning** for inference
6. **ECAN dynamics** for attention allocation
7. **HTTP server** for production GraphQL
8. **WebSocket subscriptions** for real-time updates

---

For implementation details, see:
- [HYPERCOG.md](HYPERCOG.md) - Complete API reference
- [QUICKSTART.md](QUICKSTART.md) - Getting started guide
- [IMPLEMENTATION_SUMMARY.md](IMPLEMENTATION_SUMMARY.md) - Technical summary
