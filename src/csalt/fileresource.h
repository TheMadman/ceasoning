#ifndef FILERESOURCE_H
#define FILERESOURCE_H

#include "stores.h"
#include "baseresources.h"

#ifdef __cplusplus
extern "C" {
#endif

ssize_t csalt_resource_file_read(const csalt_store *store, void *buffer, size_t size);

ssize_t csalt_resource_file_write(csalt_store *store, const void *buffer, size_t size);



#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILERESOURCE_H
