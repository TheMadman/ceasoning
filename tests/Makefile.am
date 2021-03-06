TESTS = csalt_resources_use\
	csalt_memory_resource\
	csalt_store_split

check_PROGRAMS = csalt_resources_use\
		 csalt_memory_resource\
		 csalt_store_split

csalt_resources_use_SOURCES = csalt_resources_use.c

csalt_memory_resource_SOURCES = csalt_memory_resource.c

csalt_store_split_SOURCES = csalt_store_split.c

# Set this or explicitely #include "../src/*.h"?
# Doing this for now in case we move headers in
# the future
AM_CFLAGS = -I$(top_srcdir)/src
LDADD = $(top_builddir)/src/resources/libcsaltresources.la \
	$(top_builddir)/src/stores/libcsaltstores.la

