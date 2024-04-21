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

The two main types are:

- `store`s - Represents locations that can be read, written to, or "split" (more
on that later). Used in csalt_store_transfer().
- `resource`s - Represents a resource that must be requested from the operating
system; may be rejected; and if accepted, must be freed after use. Used in
csalt_resource_use().

The default csalt_store is a dynamic store - meaning, it has a size which
can be queried and changed. A csalt_static_store, however, has a static
size, or no size. csalt_resource%s return csalt_store%s, and csalt_static_resource%s
return csalt_static_store%s. csalt_store can be safely cast down to a csalt_static_store,
but not the other way around.

csalt_store_read() and csalt_store_write() always act on the beginning of a store.
To read or write from other locations, use csalt_store_split() to first split
the store. This creates a new csalt_static_store and passes it to your callback,
allowing you to write to an arbitrary location, and avoid writing more data
than intended.

csalt_store_size() reports the size of a csalt_store. csalt_store_resize() attempts
to change the size of a csalt_store, returning the result. It is not safe to pass
csalt_static_store%s to these functions.

\section example1 Simple bytewise-copy example

This is an example program which takes two files and copies bytes from one to
the other.

\include bytewise_copy.c

A few things to note:
- We never need to call open/close on the file descriptors for us - we don't
  need to write the logic of making sure every file descriptor opens, and
  cleaning up the file descriptors that don't. This is handled by
  csalt_resource_pair when we call csalt_resource_use().
- We never have to close the file descriptors - once the bytewise_copy function
  finishes, csalt_resource_use() will clean up for us.
- The bytewise_copy function is very generic - if we want to re-use it for,
  say, a network socket transferring bytes into heap memory, no modifications
  are necessary. We can just pass a list containing those two stores.
- All the types we create to manage our resources are created on the stack.
  Likewise, the pointer passed to bytewise_copy is into the stack. The only
  system calls the library performs are the calls you request - for example,
  to use a heap resource, or read/write from a file descriptor store.

