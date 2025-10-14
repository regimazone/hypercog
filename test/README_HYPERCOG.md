# HyperCog Test Suite

This directory contains tests for the HyperCog OpenCog AtomSpace and HypergraphQL integration.

## Test Files

### Core Component Tests

1. **test_atomspace.c** - Tests for the AtomSpace knowledge representation system
   - Initialization and cleanup
   - Adding nodes and links
   - Truth value operations
   - Pattern-based queries
   - Distributed mode

2. **test_hypergraphql.c** - Tests for the HypergraphQL interface
   - Server initialization
   - GraphQL schema generation
   - Atom resolution to JSON
   - Query pattern resolution
   - Query execution

3. **test_hypercog.c** - Tests for the HyperCog integration layer
   - System initialization
   - VM lifecycle hooks
   - VM knowledge management
   - Global context management
   - Multiple VM support

## Building and Running Tests

### Individual Tests

```bash
# AtomSpace tests
gcc -I../src/include test_atomspace.c ../src/lib/atomspace.c -o test_atomspace
./test_atomspace

# HypergraphQL tests
gcc -I../src/include test_hypergraphql.c ../src/lib/hypergraphql.c ../src/lib/atomspace.c -o test_hypergraphql
./test_hypergraphql

# HyperCog integration tests
gcc -I../src/include test_hypercog.c ../src/lib/hypercog.c ../src/lib/hypergraphql.c ../src/lib/atomspace.c -o test_hypercog
./test_hypercog
```

### All Tests

```bash
# Run all HyperCog tests
for test in test_atomspace test_hypergraphql test_hypercog; do
    echo "Running $test..."
    gcc -I../src/include $test.c ../src/lib/*.c -o $test 2>/dev/null
    ./$test
    rm $test
done
```

## Test Coverage

### AtomSpace (test_atomspace.c)
- ✓ Initialization and destruction
- ✓ Node creation and retrieval
- ✓ Link creation with outgoing sets
- ✓ Truth value get/set operations
- ✓ Pattern-based query operations
- ✓ Distributed mode configuration

### HypergraphQL (test_hypergraphql.c)
- ✓ Server initialization and lifecycle
- ✓ GraphQL schema introspection
- ✓ Atom to JSON resolution
- ✓ Node lookup by name
- ✓ Query pattern matching
- ✓ Query execution and response handling

### HyperCog Integration (test_hypercog.c)
- ✓ System initialization with options
- ✓ Distributed vs local mode
- ✓ VM creation lifecycle hook
- ✓ VM destruction lifecycle hook
- ✓ VM-specific knowledge storage
- ✓ VM-specific knowledge queries
- ✓ Global context management
- ✓ Service start/stop operations
- ✓ Multiple concurrent VM support

## Test Output

All tests provide clear output indicating:
- Test name
- Pass/fail status
- Assertion failures (if any)

Example output:
```
AtomSpace Test Suite
====================

Testing atomspace_init/destroy... OK
Testing atomspace_add_node... OK
Testing atomspace_add_link... OK
Testing truth value operations... OK
Testing atomspace_query... OK
Testing distributed mode... OK

All tests passed!
```

## Integration with CI/CD

These tests can be integrated into continuous integration pipelines:

```yaml
# Example GitHub Actions
- name: Run HyperCog Tests
  run: |
    cd test
    ./run_all_tests.sh
```

## Future Test Additions

Planned test coverage expansion:
- Network synchronization for distributed mode
- GraphQL server HTTP endpoint testing
- Performance benchmarks
- Stress testing with large atomspaces
- Concurrent access patterns
- Memory leak detection
- Thread safety verification

## Troubleshooting

### Compilation Errors

If you encounter compilation errors, ensure:
1. You're in the correct directory (`test/`)
2. The header path is correct (`-I../src/include`)
3. All required source files are included

### Test Failures

If tests fail:
1. Check that no previous instances are using resources
2. Verify file permissions
3. Ensure adequate memory is available
4. Check for conflicting port numbers (GraphQL tests use ports 8080-8081)

## Contributing

When adding new functionality to HyperCog, please:
1. Add corresponding tests
2. Ensure all existing tests pass
3. Document new test cases in this README
4. Follow the existing test structure and naming conventions
