\mainpage Syntactic sugar for Common C Tasks

\section start Where to Start

This project starts at generalized algorithms, then implements concrete types to use within
those algorithms. Two examples are:

- csalt_store_transfer() - Non-blocking transfer of data between any two points
- csalt_resource_use() - Initialize, test, use and free a system resource

Concrete stores and resources represent real resources, such as memory and disk space:

- csalt_memory - represents a block of unmanaged memory, such as a stack member
  or a global variable
- csalt_heap - represents a heap memory allocation request, which can be managed automatically
  with csalt_resource_use()
- csalt_resource_file - represents a file, opened by a path

Composite stores provide simple means to operate on multiple stores:

- csalt_store_list - treats multiple stores as if they are a single store
- csalt_store_fallback - uses stores as a priority list, where later stores are only
  checked if earlier stores don't contain the requested data

Concrete and composite stores may be combined in any way, allowing simple expression of
complex relationships - for example, a csalt_store_fallback may contain a csalt_heap and a
csalt_store_list of csalt_resource_file%s.

\section concepts Core Concepts

Interfaces are struct definitions including the types of functions they support.  
Implementations are defined as global variables of the Interface struct types.  
Finally, object types are simply structs whose first member is a pointer-to-interface.  
For example, see the csalt_store_interface and csalt_memory.  

Decorating objects with new data is a simple case of creating a struct, whose first member
is of the type of an existing struct:

```cpp
struct my_struct {
	struct csalt_memory parent;
	int my_data;
}
```

Passing a pointer to a `my_struct` to `csalt_store` functions will work as expected.
