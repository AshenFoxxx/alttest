#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>

int main(int argc, char *argv[]) {
    printf("--- alttest branch comparator ---\n");

    const char* branch1 = "p10";
    const char* branch2 = "p9";
    const char* arch = "x86_64";

    printf("Comparing %s vs %s (%s)\n", branch1, branch2, arch);

    char *result_json = NULL;
    int status = alttest_compare_branches(branch1, branch2, arch, &result_json);

    if (status == 0 && result_json) {
        // СОХРАНЯЕМ В ФАЙЛ
        FILE *f = fopen("comparison.json", "w");
        if (f) {
            fputs(result_json, f);
            fclose(f);
            printf("✅ Full result saved: comparison.json (%zu bytes)\n", strlen(result_json));
        }

        // КОМПАКТНАЯ СТАТИСТИКА
        json_error_t err;
        json_t *root = json_loads(result_json, 0, &err);
        if (root && json_is_object(root)) {
            printf("📊 STATS:\n");
            printf("  %-15s: %6d pkgs\n", "p10 total", json_integer_value(json_object_get(root, "total_branch1")));
            printf("  %-15s: %6d pkgs\n", "p9 total", json_integer_value(json_object_get(root, "total_branch2")));
            printf("  %-15s: %6zu pkgs\n", "only_in_p10", json_array_size(json_object_get(root, "only_in_branch1")));
            printf("  %-15s: %6zu pkgs\n", "only_in_p9", json_array_size(json_object_get(root, "only_in_branch2")));
            printf("  %-15s: %6zu pkgs\n", "newer_in_p10", json_array_size(json_object_get(root, "newer_in_branch1")));
            printf("\n✅ Ready! Check comparison.json\n");
        }
        if (root) json_decref(root);

        free(result_json);
        printf("--- Done ---\n");
        return 0;
    } else {
        printf("❌ Failed: %d\n", status);
        if (result_json) free(result_json);
        return 1;
    }
}
