TESTS = csalt_resources_use\
	csalt_memory_resource

check_PROGRAMS = csalt_resources_use\
		 csalt_memory_resource

csalt_resources_use_SOURCES = csalt_resources_use.c

csalt_memory_resource_SOURCES = csalt_memory_resource.c

# Set this or explicitely #include "../src/*.h"?
# Doing this for now in case we move headers in
# the future
AM_CFLAGS = -I$(top_srcdir)/src
AM_LDFLAGS = $(top_srcdir)/src/csaltresources.o

