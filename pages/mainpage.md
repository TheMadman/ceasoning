\mainpage Ceasoning: Syntactic sugar for Common C Tasks

\section start Where to Start

This project starts at generalized algorithms, then implements concrete types to use within
those algorithms. Two examples are:

- csalt_store_transfer() - Non-blocking transfer of data between any two points
- csalt_resource_use() - Initialize, test, use and free a system resource

With the implementation of abstract data types alongside the concrete data types,
this library will allow for simple expression of complex data transfers and resource lifecycles.

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
