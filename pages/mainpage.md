\mainpage Syntactic sugar for Common C Tasks

Help I wrote an object-oriented declarative library in an imperative language
and now I'm having an identity crisis

\section start Where to Start

To use ceasoning, you should link against the `csalt` library (e.g. `-lcsalt`
for GCC/Clang) and include definitions from the `csalt` sub-directory. The
`csalt/stores.h` file includes the interface definition for `store`s and all
built-in `store` types, while `csalt/resources.h` includes the interface
definition for `resource`s and the built-in `resource` types.

\section overview Overarching Concepts

This project starts at generalized algorithms, then implements concrete types
to use within those algorithms. Two examples are:

- csalt_store_transfer() - Non-blocking transfer of data between any two points
- csalt_resource_use() - Initialize, test, use and free a system resource

Concrete stores and resources represent real resources, such as memory and disk
space:

- csalt_memory - represents a block of unmanaged memory, such as a stack member
  or a global variable
- csalt_heap - represents a heap memory allocation request, which can be managed
  automatically with csalt_resource_use()
- csalt_resource_vector - represents heap-allocated memory which can expand
  to accommodate large csalt_store_write and csalt_store_split calls
- csalt_resource_file - represents a file, opened by a path

Composite stores provide simple means to operate on multiple stores:

- csalt_store_pair - treats multiple stores as if they are a single store
- csalt_store_fallback - uses stores as a priority list, where later stores are
  only checked if earlier stores don't contain the requested data
- csalt_resource_first - attempts to initialize resources in the order they're
  given, returning the first successfuly-initialized resource

Concrete and composite stores may be combined in any way, allowing simple
expression of complex relationships - for example, a csalt_store_fallback may
contain a csalt_heap and a csalt_resource_pair list of csalt_resource_file%s.

\section example1 Simple bytewise-copy example

This is an example program which takes two files and copies bytes from one to
the other.

\include bytewise_copy.c

A few things to note:
- We never need to call open/close on the file descriptors for us - we don't
  need to write the logic of making sure every file descriptor opens, and
  cleaning up the file descriptors that don't. This is handled by
  csalt_resource_pair when we call csalt_resource_use.
- We never have to close the file descriptors - once the bytewise_copy function
  finishes, csalt_resource_use will clean up for us.
- The bytewise_copy function is very generic - if we want to re-use it for,
  say, a network socket transferring bytes into heap memory, no modifications
  are necessary. We can just pass a list containing those two stores.
- All the types we create to manage our resources are created on the stack.
  Likewise, the pointer passed to bytewise_copy is into the stack. The only
  system calls the library performs are the calls you request - for example,
  to use a heap resource, or read/write from a file descriptor store.

