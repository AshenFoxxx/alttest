#ifndef LIBALTTTEST_H
#define LIBALTTTEST_H

#include <curl/curl.h>
#include <jansson.h>

int alttest_http_get(const char* url, char** response);
int alttest_auth_login(const char* login, const char* pass, char** token);
int alttest_get_stats(const char* token, char** stats_json);

#endif
