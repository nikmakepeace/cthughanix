// Filesystem/catalog loader for CoreOption entries.

#ifndef __CORE_OPTION_ASSET_LOADER_H
#define __CORE_OPTION_ASSET_LOADER_H

#include "CoreOption.h"

#include <stdio.h>

typedef CoreOptionEntry* (*CoreOptionLoader)(FILE*, const char*, const char*, const char*);
typedef CoreOptionEntry* (*CoreOptionContextLoader)(
    FILE*, const char*, const char*, const char*, void*);

/**
 * Loads matching files from search paths and appends successful entries.
 *
 * @param option CoreOption catalog to receive loaded entries.
 * @param searchPath Null-terminated list of directories.
 * @param extraPath Subdirectory appended to the command-line extra library path.
 * @param extension Extension to match, such as ".map" or ".png".
 * @param loader File parser returning a new CoreOptionEntry, or NULL to skip.
 * @return Zero after scanning configured paths.
 */
int loadCoreOptionEntries(CoreOption& option, const char* searchPath[],
    const char* extraPath, const char* extension, CoreOptionLoader loader);

/**
 * Context-aware variant of loadCoreOptionEntries().
 *
 * @param context Borrowed parser context passed through to loader.
 */
int loadCoreOptionEntries(CoreOption& option, const char* searchPath[],
    const char* extraPath, const char* extension, CoreOptionContextLoader loader,
    void* context);

#endif
