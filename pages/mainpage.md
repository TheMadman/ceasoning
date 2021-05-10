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

- csalt_resource_list - implements the same store interface as csalt_store_list, as 
  well as functionality for initializing, checking and deinitializing resources
- csalt_resource_fallback - implements csalt_resource_list resource initialization logic
  with csalt_store_fallback logic for the store interface
- csalt_resource_first - attempts to initialize resources in the order they're given,
  returning the first successfuly-initialized resource

Concrete and composite stores may be combined in any way, allowing simple expression of
complex relationships - for example, a csalt_store_fallback may contain a csalt_heap and a
csalt_resource_list of csalt_resource_file%s.

\section example_1 An Example: Bytewise Copy Program

The following example program shows a program which takes, as command line arguments,
the file names of a file to copy from and a file to copy to.

A few points to take away from this:

- We never have to explicitely open, check, and clean up the files - 
this is done for us by the csalt_resource_use() function;
- We have written the function which does bytewise copying to use generic
types - if the stores were, instead, heap memory, or network sockets, we
could pass a csalt_resource_list of those instead, making reusing or refactoring
this code very simple.
- There are still error cases that this program does not concern itself with
checking or correcting: what should happen if the transfer only partially completes,
then throws an error? Should the behaviour be different if the output file is an
existing file vs. a file created by this program? What if the output "file" isn't
a file at all, but a virtual file in e.g. `/dev/tcp`, `/proc` etc.?
Since this issue can easily become very complicated, and the caller has much better
knowledge of the arguments than the program, we just omit those considerations.

\include bytewise_copy.c

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
