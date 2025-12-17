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

    // Разбираем корневые объекты
    json_error_t err1, err2;
    json_t *root1 = json_loads(json1, 0, &err1);
    json_t *root2 = json_loads(json2, 0, &err2);

    free(json1);
    free(json2);

    if (!root1 || !json_is_object(root1)) {
        fprintf(stderr, "JSON parse error for %s: %s\n", branch1, err1.text);
        if (root1) json_decref(root1);
        if (root2) json_decref(root2);
        return -2;
    }
    if (!root2 || !json_is_object(root2)) {
        fprintf(stderr, "JSON parse error for %s: %s\n", branch2, err2.text);
        json_decref(root1);
        if (root2) json_decref(root2);
        return -2;
    }

    // Достаём массивы packages
    json_t *arr1 = json_object_get(root1, "packages");
    json_t *arr2 = json_object_get(root2, "packages");

    if (!arr1 || !json_is_array(arr1) || !arr2 || !json_is_array(arr2)) {
        fprintf(stderr, "Error: 'packages' field missing or not array\n");
        json_decref(root1);
        json_decref(root2);
        return -2;
    }

    size_t count1 = json_array_size(arr1);
    size_t count2 = json_array_size(arr2);

    printf("Info: %s: %zu packages, %s: %zu packages\n",
           branch1, count1, branch2, count2);

    // Строим карты name -> {version, release}
    json_t *map1 = json_object();
    json_t *map2 = json_object();

    // Парсим branch1
    for (size_t i = 0; i < count1; i++) {
        json_t *pkg = json_array_get(arr1, i);
        if (!json_is_object(pkg)) continue;

        const char *name    = json_string_value(json_object_get(pkg, "name"));
        const char *version = json_string_value(json_object_get(pkg, "version"));
        const char *release = json_string_value(json_object_get(pkg, "release"));

        if (name && version && release) {
            json_t *info = json_pack("{s:s, s:s}", "version", version, "release", release);
            json_object_set_new(map1, name, info);
        }
    }

    // Парсим branch2
    for (size_t i = 0; i < count2; i++) {
        json_t *pkg = json_array_get(arr2, i);
        if (!json_is_object(pkg)) continue;

        const char *name    = json_string_value(json_object_get(pkg, "name"));
        const char *version = json_string_value(json_object_get(pkg, "version"));
        const char *release = json_string_value(json_object_get(pkg, "release"));

        if (name && version && release) {
            json_t *info = json_pack("{s:s, s:s}", "version", version, "release", release);
            json_object_set_new(map2, name, info);
        }
    }

    printf("Info: parsed %zu/%zu unique packages\n", 
           json_object_size(map1), json_object_size(map2));

    // *** СРАВНЕНИЕ ПАКЕТОВ ***
    json_t *only1  = json_array();
    json_t *only2  = json_array(); 
    json_t *newer1 = json_array();

    const char *key;
    json_t *val;

    // 1. Только в branch1 + новее в branch1
    json_object_foreach(map1, key, val) {
        json_t *info2 = json_object_get(map2, key);
        if (!info2) {
            json_array_append_new(only1, json_string(key));
            continue;
        }

        const char *v1 = json_string_value(json_object_get(val, "version"));
        const char *r1 = json_string_value(json_object_get(val, "release"));
        const char *v2 = json_string_value(json_object_get(info2, "version"));
        const char *r2 = json_string_value(json_object_get(info2, "release"));

        // version-release сравнение
        if (strcmp(v1 ? v1 : "", v2 ? v2 : "") > 0 || 
            (strcmp(v1 ? v1 : "", v2 ? v2 : "") == 0 && 
             strcmp(r1 ? r1 : "", r2 ? r2 : "") > 0)) {
            char vr1[256], vr2[256];
            snprintf(vr1, sizeof(vr1), "%s-%s", v1 ? v1 : "", r1 ? r1 : "");
            snprintf(vr2, sizeof(vr2), "%s-%s", v2 ? v2 : "", r2 ? r2 : "");
            json_array_append_new(newer1, json_pack("{s:s,s:s,s:s}",
                "name", key, "branch1", vr1, "branch2", vr2));
        }
    }

    // 2. Только в branch2
    json_object_foreach(map2, key, val) {
        if (!json_object_get(map1, key)) {
            json_array_append_new(only2, json_string(key));
        }
    }

    printf("Info: only1=%zu, only2=%zu, newer1=%zu\n",
           json_array_size(only1), json_array_size(only2), json_array_size(newer1));

    // Освобождаем промежуточные объекты
    json_decref(root1); json_decref(root2);
    json_decref(map1); json_decref(map2);

    // ФИНАЛЬНЫЙ JSON (ТВОЯ ТРЕБОВАЯ СТРУКТУРА!)
    json_t *out = json_pack(
        "{s:s,s:s,s:s, s:o,s:o,s:o}",
        "branch1", branch1,
        "branch2", branch2,
        "arch", arch,
        "only_in_branch1", only1,
        "only_in_branch2", only2,
        "newer_in_branch1", newer1
    );

    *result_json = json_dumps(out, JSON_INDENT(2));
    json_decref(out);

    if (!*result_json) return -3;
    return 0;
}






