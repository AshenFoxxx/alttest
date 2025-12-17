#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lib.h"

struct alttest_buffer {
    char *data;
    size_t size;
};

static size_t alttest_write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct alttest_buffer *buf = (struct alttest_buffer *)userp;

    char *ptr = realloc(buf->data, buf->size + real_size + 1);
    if (!ptr) {
        return 0;
    }

    buf->data = ptr;
    memcpy(&(buf->data[buf->size]), contents, real_size);
    buf->size += real_size;
    buf->data[buf->size] = '\0';

    return real_size;
}

int alttest_http_get(const char* url, char** response) {
    if (!url || !response) {
        return -1;
    }

    CURL *curl = curl_easy_init();
    if (!curl) {
        return -2;
    }

    struct alttest_buffer buf = { .data = NULL, .size = 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, alttest_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        curl_easy_cleanup(curl);
        free(buf.data);
        return -3;
    }

    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    curl_easy_cleanup(curl);

    if (http_code < 200 || http_code >= 300) {
        free(buf.data);
        return (int)http_code;
    }

    *response = buf.data;
    return 0;
}

int alttest_auth_login(const char* login, const char* pass, char** token) {
    // TODO: POST /auth/login
    return -1;
}

int alttest_get_stats(const char* token, char** stats_json) {
    // TODO: GET /stats с Bearer token
    return -1;
}