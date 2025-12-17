#include <stdio.h>
#include "lib.h"

int main() {
    char *response = NULL;
    int rc = alttest_http_get("https://httpbin.org/get", &response);
    
    if (rc == 0) {
        printf("HTTP OK: %s\n", response);
        free(response);
    } else {
        printf("HTTP error: %d\n", rc);
    }
    
    return 0;
}