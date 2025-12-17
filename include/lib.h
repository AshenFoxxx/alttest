#ifndef LIBALTTTEST_H
#define LIBALTTTEST_H

#include <curl/curl.h>
#include <jansson.h>

// A structure for presenting information about a package
typedef struct {
    char *name;
    char *version;
    char *release;
    char *arch;
} package_info_t;

// The structure for the result of a comparison on the same architecture
typedef struct {
    json_t *only_in_branch1;  // json_array
    json_t *only_in_branch2;  // json_array
    json_t *newer_in_branch1; // json_array
} comparison_result_t;

// Functions for working with HTTP
int alttest_http_get(const char* url, char** response);

// Functions for working with the API
int alttest_fetch_branch(const char* branch, const char* arch, char** packages_json);

int alttest_compare_branches(const char* branch1,
                             const char* branch2,
                             const char* arch,
                             char** result_json);

#endif
