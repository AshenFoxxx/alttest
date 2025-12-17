#ifndef LIBALTTTEST_H
#define LIBALTTTEST_H

#include <curl/curl.h>
#include <jansson.h>

// Структура для представления информации о пакете
typedef struct {
    char *name;
    char *version;
    char *release;
    char *arch;
} package_info_t;

// Структура для результата сравнения по одной архитектуре
typedef struct {
    json_t *only_in_branch1;  // json_array
    json_t *only_in_branch2;  // json_array
    json_t *newer_in_branch1; // json_array
} comparison_result_t;

// Функции для работы с HTTP 
int alttest_http_get(const char* url, char** response);

// Функции для работы с API 
int alttest_fetch_branch(const char* branch, const char* arch, char** packages_json);

int alttest_compare_branches(const char* branch1,
                             const char* branch2,
                             const char* arch,
                             char** result_json);

#endif
