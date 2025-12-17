#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

__attribute__((constructor))
void alttest_global_init(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

__attribute__((destructor))
void alttest_global_cleanup(void) {
    curl_global_cleanup();
}

struct alttest_buffer {
    char *data;
    size_t size;
};

static size_t alttest_write_cb(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct alttest_buffer *buf = (struct alttest_buffer *)userp;

    char *ptr = realloc(buf->data, buf->size + real_size + 1);
    if (!ptr) {
        return 0; // Ошибка realloc
    }

    buf->data = ptr;
    memcpy(&(buf->data[buf->size]), contents, real_size);
    buf->size += real_size;
    buf->data[buf->size] = '\0';

    return real_size;
}


int alttest_http_get(const char* url, char** response) {
    if (!url || !response) {
        printf("Error: Null URL or response pointer\n");
        return -1;
    }

    CURL *curl = curl_easy_init(); 
    if (!curl) {
        printf("Error: Failed to init curl\n");
        return -2;
    }

    struct alttest_buffer buf = { .data = NULL, .size = 0 };

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, alttest_write_cb);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "alt-test-client/1.0");

    CURLcode res = curl_easy_perform(curl);
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    if (res != CURLE_OK) {
        printf("Error: curl_easy_perform failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        if (buf.data) free(buf.data);
        return -3;
    }

    curl_easy_cleanup(curl);

    if (http_code < 200 || http_code >= 300) {
        printf("Error: HTTP request failed with code %ld\n", http_code);
        if (buf.data) free(buf.data);
        return (int)http_code; 
    }

    if (!buf.data) {
         printf("Error: No data received\n");
         return -4;
    }

    *response = buf.data; 
    return 0; 
}


int alttest_fetch_branch(const char* branch, const char* arch, char** packages_json) {
    if (!branch || !arch || !packages_json) {
        return -1;
    }

    char url[512];
    int ret = snprintf(url, sizeof(url),
                       "https://rdb.altlinux.org/api/export/branch_binary_packages/%s?arch=%s",
                       branch, arch);
    if (ret < 0 || (size_t)ret >= sizeof(url)) {
        printf("Error: URL too long for branch '%s', arch '%s'\n", branch, arch);
        return -1;
    }

    
    int status = alttest_http_get(url, packages_json);
    if (status != 0) {
        printf("Error: Failed fetching data for branch '%s', arch '%s'. Status: %d\n", branch, arch, status);
    }
    return status;
}

int alttest_compare_branches(const char* branch1,
                             const char* branch2,
                             const char* arch,
                             char** result_json)
{
    if (!branch1 || !branch2 || !arch || !result_json) return -1;

    printf("Info: Fetching %s %s...\n", branch1, arch);
    char *json1 = NULL;
    int rc = alttest_fetch_branch(branch1, arch, &json1);
    if (rc != 0) {
        printf("Error: Failed to fetch %s: %d\n", branch1, rc);
        return rc;
    }

    printf("Info: Fetching %s %s...\n", branch2, arch);
    char *json2 = NULL;
    rc = alttest_fetch_branch(branch2, arch, &json2);
    if (rc != 0) {
        free(json1);
        printf("Error: Failed to fetch %s: %d\n", branch2, rc);
        return rc;
    }

    printf("Info: %s: %zu bytes, %s: %zu bytes\n", 
           branch1, strlen(json1), branch2, strlen(json2));

    free(json1);
    free(json2);

    *result_json = strdup("{\"status\":\"HTTP requests work, JSON received\"}");
    return 0;
}

