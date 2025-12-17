#include "lib.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    printf("--- Starting alttest CLI ---\n");

    // Жёстко заданные ветки и архитектура для тестирования
    const char* branch1 = "p10";
    const char* branch2 = "sisyphus";
    const char* arch = "x86_64";

    printf("Comparing branches: %s vs %s, architecture: %s\n", branch1, branch2, arch);

    char *result_json = NULL;
    int status = alttest_compare_branches(branch1, branch2, arch, &result_json);

    if (status == 0 && result_json) {
        printf("Comparison successful.\nResult:\n%s\n", result_json);
        free(result_json); 
    } else {
        printf("Comparison failed. Status: %d\n", status);
        if (result_json) {
            // В случае ошибки, библиотека могла вернуть JSON с сообщением об ошибке
            printf("Error details: %s\n", result_json);
            free(result_json);
        }
    }

    printf("--- Ending alttest CLI ---\n");
    return status != 0 ? 1 : 0;
}