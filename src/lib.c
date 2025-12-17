#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <curl/curl.h>
#include <ctype.h>

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
        return 0;
    }

    buf->data = ptr;
    memcpy(&(buf->data[buf->size]), contents, real_size);
    buf->size += real_size;
    buf->data[buf->size] = '\0';

    return real_size;
}

// RPM-compatible version comparison for ALT Linux
static int rpmvercmp(const char *a, const char *b) {
	const char *p1 = a, *p2 = b;
    
    while (*p1 || *p2) {
        // Skipping IDENTICAL characters
        while (*p1 == *p2 && *p1) { p1++; p2++; }
        
		if(!*p1 && !*p2) return 0;
        if (!*p1) return -1;
        if (!*p2) return 1;
        
        // Skip separators (only one at a time)
        if (*p1 == '.' || *p1 == '-') { p1++; continue; }
        if (*p2 == '.' || *p2 == '-') { p2++; continue; }
        
        if (isdigit((unsigned char)*p1) && isdigit((unsigned char)*p2)) {
            unsigned long n1 = 0, n2 = 0;
            while (isdigit((unsigned char)*p1)) n1 = n1 * 10 + (*p1++ - '0');
            while (isdigit((unsigned char)*p2)) n2 = n2 * 10 + (*p2++ - '0');
            if (n1 != n2) return (n1 > n2) ? 1 : -1;
        } else {
            unsigned char c1 = (unsigned char)*p1++;
            unsigned char c2 = (unsigned char)*p2++;
            if (c1 != c2) return (c1 > c2) ? 1 : -1;
        }
    }
    return 0;
}

static int rpm_cmp(const char* v1, const char* r1, const char* v2, const char* r2) {
    // 1. COMPARING VERSION FIRST!
	
    int ver_cmp = rpmvercmp(v1 ? v1 : "", v2 ? v2 : "");
    if (ver_cmp != 0) return ver_cmp;
    
    // 2. If VERSION is equal, compare RELEASE
    return rpmvercmp(r1 ? r1 : "", r2 ? r2 : "");
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

    char *json1 = NULL;
    int rc = alttest_fetch_branch(branch1, arch, &json1);
    if (rc != 0) return rc;

    char *json2 = NULL;
    rc = alttest_fetch_branch(branch2, arch, &json2);
    if (rc != 0) {
        free(json1);
        return rc;
    }

    json_error_t err1, err2;
    json_t *root1 = json_loads(json1, 0, &err1);
    json_t *root2 = json_loads(json2, 0, &err2);
    free(json1); free(json2);

    if (!root1 || !json_is_object(root1) || !root2 || !json_is_object(root2)) {
        if (root1) json_decref(root1);
        if (root2) json_decref(root2);
        return -2;
    }

    json_t *arr1 = json_object_get(root1, "packages");
    json_t *arr2 = json_object_get(root2, "packages");

    if (!arr1 || !json_is_array(arr1) || !arr2 || !json_is_array(arr2)) {
        json_decref(root1); json_decref(root2);
        return -2;
    }

    size_t count1 = json_array_size(arr1);
    size_t count2 = json_array_size(arr2);

    json_t *map1 = json_object(), *map2 = json_object();

    for (size_t i = 0; i < count1; i++) {
        json_t *pkg = json_array_get(arr1, i);
        if (!json_is_object(pkg)) continue;
        const char *name = json_string_value(json_object_get(pkg, "name"));
        const char *version = json_string_value(json_object_get(pkg, "version"));
        const char *release = json_string_value(json_object_get(pkg, "release"));
        if (name && version && release) {
            json_t *info = json_pack("{s:s, s:s}", "version", version, "release", release);
            json_object_set_new(map1, name, info);
        }
    }

    for (size_t i = 0; i < count2; i++) {
        json_t *pkg = json_array_get(arr2, i);
        if (!json_is_object(pkg)) continue;
        const char *name = json_string_value(json_object_get(pkg, "name"));
        const char *version = json_string_value(json_object_get(pkg, "version"));
        const char *release = json_string_value(json_object_get(pkg, "release"));
        if (name && version && release) {
            json_t *info = json_pack("{s:s, s:s}", "version", version, "release", release);
            json_object_set_new(map2, name, info);
        }
    }

    // COMPARISON with the RPM algorithm!
    json_t *only1 = json_array(), *only2 = json_array(); 
    json_t *newer1 = json_array();

    const char *key; json_t *val;
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
        
        int cmp = rpm_cmp(v1, r1, v2, r2);
        
        if (cmp > 0) {
        // Only branch1 is newer
        char vr1[512], vr2[512];
        snprintf(vr1, sizeof(vr1), "%s-%s", v1 ? v1 : "", r1 ? r1 : "");
        snprintf(vr2, sizeof(vr2), "%s-%s", v2 ? v2 : "", r2 ? r2 : "");
        json_array_append_new(newer1, json_pack("{s:s,s:s,s:s}", 
            "name", key, "branch1", vr1, "branch2", vr2));
    }
    }

    json_object_foreach(map2, key, val) {
        if (!json_object_get(map1, key)) {
            json_array_append_new(only2, json_string(key));
        }
    }

    json_decref(root1); json_decref(root2);
    json_decref(map1); json_decref(map2);

    json_t *out = json_pack("{s:s,s:s,s:s,s:i,s:i,s:o,s:o,s:o}",
        "branch1", branch1,
        "branch2", branch2,
        "arch", arch,
        "total_branch1", (int)count1,
        "total_branch2", (int)count2,
        "only_in_branch1", only1,
        "only_in_branch2", only2,
        "newer_in_branch1", newer1
    );

    *result_json = json_dumps(out, JSON_INDENT(2));
    json_decref(out);

    return (*result_json) ? 0 : -3;
}
// Public API for tests
int alttest_rpmvercmp(const char *a, const char *b) {
    return rpmvercmp(a, b);
}

int alttest_rpm_cmp(const char* v1, const char* r1, const char* v2, const char* r2) {
    return rpm_cmp(v1, r1, v2, r2);
}

