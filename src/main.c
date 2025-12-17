#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>

void print_banner() {
    printf("ALT Linux Branch Comparator v1.0\n");
    printf("=====================================\n\n");
}

void print_error(const char* msg) {
    printf("ERROR: %s\n\n", msg);
}

void print_stats(const char* branch1, const char* branch2, char* result_json) {
    FILE *f = fopen("comparison.json", "w");
    if (f) {
        fputs(result_json, f);
        fclose(f);
        printf("Saved: comparison.json (%zu bytes)\n", strlen(result_json));
    }
    
    json_error_t err;
    json_t *root = json_loads(result_json, 0, &err);
    if (root && json_is_object(root)) {
        printf("STATS:\n");
        printf("  %-12s: %6d pkgs\n", branch1, json_integer_value(json_object_get(root, "total_branch1")));
        printf("  %-12s: %6d pkgs\n", branch2, json_integer_value(json_object_get(root, "total_branch2")));
        printf("  %-12s: %6zu pkgs\n", "only_in_b1", json_array_size(json_object_get(root, "only_in_branch1")));
        printf("  %-12s: %6zu pkgs\n", "only_in_b2", json_array_size(json_object_get(root, "only_in_branch2")));
        printf("  %-12s: %6zu pkgs\n", "newer_in_b1", json_array_size(json_object_get(root, "newer_in_branch1")));
        json_decref(root);
    } else {
        printf("Could not parse stats\n");
    }
}

int interactive_mode() {
    char arch[32], branch1[32], branch2[32];
    
    printf("Supported architectures (Sisyphus): i586, x86_64, armh, aarch64, e2k*, riscv64*, mipsel*, loongarch*\n");
    printf("(* — may not be available in all branches)\n\n");
    printf("Enter architecture: ");
    
    if (scanf("%30s", arch) != 1) {
        print_error("Failed to read input");
        return 1;
    }
    
    printf("Enter branch 1 (p10, p9, p11, sisyphus): ");
    if (scanf("%30s", branch1) != 1) {
        print_error("Failed to read branch 1");
        return 1;
    }
    
    printf("Enter branch 2 (p10, p9, p11, sisyphus): ");
    if (scanf("%30s", branch2) != 1) {
        print_error("Failed to read branch 2");
        return 1;
    }
    
    printf("\nComparing %s vs %s (%s)...\n\n", branch1, branch2, arch);
    
    char *result_json = NULL;
    int status = alttest_compare_branches(branch1, branch2, arch, &result_json);
    
    if (status != 0) {
        print_error("Failed to fetch data — branch or architecture not available");
        if (result_json) free(result_json);
        return 1;
    }
    
    print_stats(branch1, branch2, result_json);
    return 0;
}


int main(int argc, char *argv[]) {
    print_banner();
    
    if (argc == 4) {
        // CLI mode: ./alttest x86_64 p10 p9
        const char* arch = argv[1];
        const char* branch1 = argv[2];
        const char* branch2 = argv[3];
        
        printf("CLI mode: %s vs %s (%s)\n\n", branch1, branch2, arch);
        
        char *result_json = NULL;
        int status = alttest_compare_branches(branch1, branch2, arch, &result_json);
        
        if (status != 0) {
            print_error("Failed to fetch data — branch or architecture not available");
            if (result_json) free(result_json);
            return 1;
        }
        
        print_stats(branch1, branch2, result_json);
        
    } else {
        printf("No arguments. Starting interactive mode...\n\n");
        return interactive_mode();
    }
    
    printf("\n--- Done ---\n");
    return 0;
}
