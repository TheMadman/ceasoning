TESTS = %reldir%/csalt_resources_use\
	%reldir%/csalt_memory_resource\
	%reldir%/csalt_store_split\
	%reldir%/csalt_store_transfer\
	%reldir%/csalt_file_resource

check_PROGRAMS = %reldir%/csalt_resources_use\
		 %reldir%/csalt_memory_resource\
		 %reldir%/csalt_store_split\
		 %reldir%/csalt_store_transfer\
		 %reldir%/csalt_file_resource

#%canon_reldir%_csalt_resources_use_SOURCES = %reldir%/csalt_resources_use.c
#
#%canon_reldir%_csalt_memory_resource_SOURCES = %reldir%/csalt_memory_resource.c
#
#%canon_reldir%_csalt_store_split_SOURCES = %reldir%/csalt_store_split.c
#
#%canon_reldir%_csalt_store_transfer_SOURCES = %reldir%/csalt_store_transfer.c
#
LDADD = $(top_builddir)/src/resources/libcsaltresources.la \
	$(top_builddir)/src/stores/libcsaltstores.la \
	$(top_builddir)/src/resources/libcsaltfileresource.la

