csaltinclude_HEADERS = %reldir%/csalt/util.h\
		       %reldir%/csalt/resources.h\
		       %reldir%/csalt/stores.h\
		       %reldir%/csalt/baseresources.h

lib_LTLIBRARIES = %reldir%/resources/libcsaltresources.la\
		  %reldir%/stores/libcsaltstores.la

%canon_reldir%_resources_libcsaltresources_la_SOURCES = %reldir%/resources/resources.c\
	%reldir%/csalt/resources.h
%canon_reldir%_resources_libcsaltresources_la_LDFLAGS = -version-info 0

%canon_reldir%_stores_libcsaltstores_la_SOURCES = %reldir%/stores/stores.c\
	%reldir%/csalt/stores.h
%canon_reldir%_stores_libcsaltstores_la_LDFLAGS = -version-info 0

