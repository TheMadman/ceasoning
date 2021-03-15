The Ceasoning (pronounced, "seasoning") project provides syntactic sugar for
common C tasks. It uses the "csalt" prefix (pronounced "seasalt") and uses
the csalt directory for include files.

This project currently only provides interfaces around data stores - which
do not necessarily require initialization or clean-up, such as application-
global memory or stack memory - and resources, which require initialization,
checks for validity, and deinitliazation.

For concrete types, which are possible to initialize on the stack, the
struct keyword is preserved, e.g. `struct csalt_memory` and `struct
csalt_heap`. Typedefs are used for types which are not expected to be directly
initialized - such as pointers-to-interfaces like `csalt_resource` and
`csalt_store`.

Code is written for humans, not computers. Code which is difficult to
understand is considered a bug - please report it!
