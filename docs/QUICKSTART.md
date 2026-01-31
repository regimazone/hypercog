# HyperCog Quick Start Guide

Get up and running with HyperCog's OpenCog AtomSpace and HypergraphQL integration in minutes.

## Overview

HyperCog adds cognitive intelligence to the HyperKit hypervisor through:
- **AtomSpace**: Knowledge graphs for representing VM relationships and metadata
- **HypergraphQL**: Query interface for accessing and manipulating knowledge
- **Distributed Sync**: Share knowledge across multiple hypervisor nodes

## 5-Minute Tutorial

### 1. Include HyperCog Headers

```c
#include <xhyve/hypercog.h>
```

### 2. Initialize HyperCog

```c
hypercog_ctx_t *ctx;
int error;

// Initialize with distributed mode, GraphQL on port 8080
error = hypercog_init(&ctx, true, 8080);
if (error != 0) {
    fprintf(stderr, "Failed to init HyperCog: %d\n", error);
    return error;
}

// Set as global context
hypercog_set_global_context(ctx);

// Start services
hypercog_start(ctx);
```

### 3. Track VM Lifecycle

```c
// When creating a VM
int vcpu = 0;
hypercog_vm_created(ctx, vcpu);

// Add VM metadata
hypercog_add_vm_knowledge(ctx, vcpu, "os", "Ubuntu");
hypercog_add_vm_knowledge(ctx, vcpu, "memory", "4096");
hypercog_add_vm_knowledge(ctx, vcpu, "cpus", "2");
```

### 4. Query VM Knowledge

```c
// Query VM information
atom_handle_t result = hypercog_query_vm_knowledge(ctx, vcpu, "os");
if (result != 0) {
    printf("Found VM knowledge: handle=%lu\n", result);
}
```

### 5. Query via GraphQL

Once started, the GraphQL endpoint is available at `http://localhost:8080/graphql`.

Example query:
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

### 6. Cleanup

```c
// When VM is destroyed
hypercog_vm_destroyed(ctx, vcpu);

// Shutdown HyperCog
hypercog_stop(ctx);
hypercog_destroy(ctx);
```

## Common Use Cases

### Use Case 1: Track VM Relationships

```c
// Create parent-child VM relationship
atom_handle_t parent = hypercog_add_vm_knowledge(ctx, 0, "type", "parent");
atom_handle_t child = hypercog_add_vm_knowledge(ctx, 1, "type", "child");

// Create link between them
atom_handle_t outgoing[2] = {child, parent};
atom_handle_t link = atomspace_add_link(
    ctx->atomspace,
    ATOM_TYPE_INHERITANCE,
    outgoing,
    2
);
```

### Use Case 2: Store Performance Metrics

```c
// Store CPU usage
char cpu_usage[32];
snprintf(cpu_usage, sizeof(cpu_usage), "%.2f", get_cpu_usage());
hypercog_add_vm_knowledge(ctx, vcpu, "cpu_usage", cpu_usage);

// Store memory usage
char mem_usage[32];
snprintf(mem_usage, sizeof(mem_usage), "%lu", get_memory_usage());
hypercog_add_vm_knowledge(ctx, vcpu, "memory_usage", mem_usage);
```

### Use Case 3: Pattern Matching

```c
// Find all VMs with specific characteristics
query_result_t *results = atomspace_query(ctx->atomspace, "Ubuntu");

printf("Found %u VMs running Ubuntu:\n", results->count);
for (uint32_t i = 0; i < results->count; i++) {
    atom_t *atom = atomspace_get_atom(ctx->atomspace, results->handles[i]);
    printf("  - %s\n", atom->name);
}

atomspace_free_query_result(results);
```

### Use Case 4: Probabilistic Reasoning

```c
// Set truth value for uncertain knowledge
truth_value_t tv;
tv.strength = 0.85f;    // 85% certain
tv.confidence = 0.90f;  // 90% confidence in that certainty

atom_handle_t knowledge = hypercog_add_vm_knowledge(ctx, vcpu, 
    "suspected_malware", "true");
atomspace_set_tv(ctx->atomspace, knowledge, tv);
```

## Direct AtomSpace Access

For advanced use cases, you can directly access the AtomSpace:

```c
atomspace_t *as = ctx->atomspace;

// Add concept nodes
atom_handle_t dog = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Dog");
atom_handle_t animal = atomspace_add_node(as, ATOM_TYPE_CONCEPT, "Animal");

// Create inheritance link
atom_handle_t outgoing[2] = {dog, animal};
atom_handle_t link = atomspace_add_link(as, ATOM_TYPE_INHERITANCE, 
                                        outgoing, 2);

// Set truth value
truth_value_t tv = {0.95f, 0.90f};
atomspace_set_tv(as, link, tv);
```

## Distributed Mode

To enable knowledge sharing across multiple hypervisor nodes:

```c
// Initialize with distributed=true
hypercog_init(&ctx, true, 8080);

// Start synchronization
atomspace_sync_start(ctx->atomspace);

// All atoms added will now be broadcast to peer nodes
hypercog_add_vm_knowledge(ctx, vcpu, "shared_property", "value");

// Stop synchronization when done
atomspace_sync_stop(ctx->atomspace);
```

## GraphQL Examples

### Query All Atoms
```graphql
query {
  atoms {
    count
  }
}
```

### Query by Pattern
```graphql
query {
  queryPattern(pattern: "VM-0") {
    handle
    name
    truthValue {
      strength
      confidence
    }
  }
}
```

### Add Node (Mutation)
```graphql
mutation {
  addNode(type: CONCEPT, name: "NewConcept") {
    handle
    name
  }
}
```

## Performance Tips

1. **Batch Operations**: Add multiple atoms before broadcasting in distributed mode
2. **Query Optimization**: Use specific patterns rather than broad searches
3. **Memory Management**: Regularly clean up unused atoms (future feature)
4. **Truth Values**: Use appropriate confidence levels to avoid false positives

## Debugging

Enable verbose logging:
```c
// Set log level (future feature)
hypercog_set_log_level(HYPERCOG_LOG_DEBUG);
```

Check atomspace statistics:
```c
printf("Total atoms: %u\n", ctx->atomspace->atom_count);
printf("Distributed mode: %s\n", ctx->distributed ? "yes" : "no");
printf("Node ID: %s\n", ctx->atomspace->node_id);
```

## Next Steps

- Read [HYPERCOG.md](HYPERCOG.md) for detailed API documentation
- Check [examples/hypercog_demo.c](../examples/hypercog_demo.c) for complete examples
- Run tests in [test/](../test/) directory
- Explore the GraphQL schema via introspection

## Troubleshooting

### Port Already in Use
```c
// Try a different port
hypercog_init(&ctx, true, 8081);  // Use 8081 instead
```

### Memory Issues
```c
// Check atom capacity limits in atomspace.h
#define ATOMSPACE_MAX_NODES 1024
#define ATOMSPACE_MAX_LINKS 2048
```

### Compilation Errors
Ensure you're linking all required libraries:
```bash
gcc -I src/include your_app.c \
    src/lib/hypercog.c \
    src/lib/atomspace.c \
    src/lib/hypergraphql.c \
    -o your_app
```

## Support

- Documentation: [docs/HYPERCOG.md](HYPERCOG.md)
- Examples: [examples/](../examples/)
- Tests: [test/](../test/)
- Issues: GitHub Issues

Happy coding with HyperCog! 🚀🧠
