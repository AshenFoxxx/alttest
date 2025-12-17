#include <stdio.h>
#include "../include/test.h"
#include "lib.c"

int main() {
    printf("RPM Unit Tests - Comprehensive Suite\n");
    printf("======================================\n\n");
    
    int passed = 0, total = 0;
    
    // rpmvercmp BASIC TESTS
    printf("1 Basic rpmvercmp Tests\n");
    printf("------------------------\n");
    
    total++; int r1 = alttest_rpmvercmp("2.38", "2.37");          printf("  2.38 > 2.37: %d %s\n", r1, r1 > 0 ? "✅" : "❌");  passed += (r1 > 0);
    total++; int r2 = alttest_rpmvercmp("1.2.3", "1.2.3");        printf("  1.2.3 = 1.2.3: %d %s\n", r2, r2 == 0 ? "✅" : "❌"); passed += (r2 == 0);
    total++; int r3 = alttest_rpmvercmp("1", "1.0");              printf("  1 < 1.0: %d %s\n",   r3, r3 < 0 ? "✅" : "❌");  passed += (r3 < 0);
    total++; int r4 = alttest_rpmvercmp("10", "2");               printf("  10 > 2: %d %s\n",    r4, r4 > 0 ? "✅" : "❌");  passed += (r4 > 0);
    printf("\n");
    
    // NUMERICAL TESTS (critical for RPM!)
    printf("2 Numeric Edge Cases\n");
    printf("----------------------\n");
    
    total++; int n1 = alttest_rpmvercmp("1.10", "1.2");           printf("  1.10 > 1.2: %d %s\n",     n1, n1 > 0 ? "✅" : "❌"); passed += (n1 > 0);
    total++; int n2 = alttest_rpmvercmp("1.09", "1.10");          printf("  1.09 < 1.10: %d %s\n",    n2, n2 < 0 ? "✅" : "❌"); passed += (n2 < 0);
    total++; int n3 = alttest_rpmvercmp("2.1", "2.01");           printf("  2.1 > 2.01: %d %s\n",     n3, n3 > 0 ? "✅" : "❌"); passed += (n3 > 0);
    total++; int n4 = alttest_rpmvercmp("9", "10");               printf("  9 < 10: %d %s\n",         n4, n4 < 0 ? "✅" : "❌"); passed += (n4 < 0);
    total++; int n5 = alttest_rpmvercmp("1.999", "1.1000");       printf("  1.999 < 1.1000: %d %s\n", n5, n5 < 0 ? "✅" : "❌"); passed += (n5 < 0);
    printf("\n");
    
    // SEPARATORS AND DASHES
    printf("3 Separators & Dashes\n");
    printf("-----------------------\n");
    
    total++; int s1 = alttest_rpmvercmp("1.2-3", "1.2.3");        printf("  1.2-3 < 1.2.3: %d %s\n",  s1, s1 < 0 ? "✅" : "❌"); passed += (s1 < 0);
    total++; int s2 = alttest_rpmvercmp("1--2", "1-2");           printf("  1--2 = 1-2: %d %s\n",     s2, s2 == 0 ? "✅" : "❌"); passed += (s2 == 0);
    total++; int s3 = alttest_rpmvercmp("1...2", "1.2");          printf("  1...2 = 1.2: %d %s\n",    s3, s3 == 0 ? "✅" : "❌"); passed += (s3 == 0);
    printf("\n");
    
    // NON-NUMERIC TOKENS (rc, alpha, beta, fc)
    printf("4 Non-numeric Tokens\n");
    printf("-----------------------\n");
    
    total++; int t1 = alttest_rpmvercmp("1.2rc1", "1.2");         printf("  1.2rc1 < 1.2: %d %s\n",   t1, t1 < 0 ? "✅" : "❌"); passed += (t1 < 0);
    total++; int t2 = alttest_rpmvercmp("1.2beta", "1.2alpha");   printf("  beta > alpha: %d %s\n",   t2, t2 > 0 ? "✅" : "❌"); passed += (t2 > 0);
    total++; int t3 = alttest_rpmvercmp("1.2fc39", "1.2fc40");    printf("  fc39 < fc40: %d %s\n",    t3, t3 < 0 ? "✅" : "❌"); passed += (t3 < 0);
    total++; int t4 = alttest_rpmvercmp("1.2~", "1.2");           printf("  1.2~ < 1.2: %d %s\n",     t4, t4 < 0 ? "✅" : "❌"); passed += (t4 < 0);
    printf("\n");
    
    // RPM_cmp (version + release)
    printf("5 rpm_cmp Tests\n");
    printf("------------------\n");
    
    total++; int c1 = alttest_rpm_cmp("2", "38.alt1", "2", "37.alt2"); printf("  2.38.alt1 > 2.37.alt2: %d %s\n", c1, c1 > 0 ? "✅" : "❌"); passed += (c1 > 0);
    total++; int c2 = alttest_rpm_cmp("2.0", "1", "1.9", "999");       printf("  2.0.1 > 1.9.999: %d %s\n",      c2, c2 > 0 ? "✅" : "❌"); passed += (c2 > 0);
    total++; int c3 = alttest_rpm_cmp("1.10", "1", "1.2", "100");     printf("  1.10.1 > 1.2.100: %d %s\n",     c3, c3 > 0 ? "✅" : "❌"); passed += (c3 > 0);
    total++; int c4 = alttest_rpm_cmp("", "", "", "");                 printf("  NULL = NULL: %d %s\n",          c4, c4 == 0 ? "✅" : "❌"); passed += (c4 == 0);
    total++; int c5 = alttest_rpm_cmp("1", NULL, "1", "1");            printf("  1 < 1.1: %d %s\n",              c5, c5 < 0 ? "✅" : "❌"); passed += (c5 < 0);
    printf("\n");
    
    // EDGE CASES (empty strings, NULL, strange characters)
    printf("6 Edge Cases\n");
    printf("--------------\n");
    
    total++; int e1 = alttest_rpmvercmp("", "");                      printf("  '' = '': %d %s\n",              e1, e1 == 0 ? "✅" : "❌"); passed += (e1 == 0);
    total++; int e2 = alttest_rpmvercmp("", "1");                     printf("  '' < 1: %d %s\n",               e2, e2 < 0 ? "✅" : "❌"); passed += (e2 < 0);
    total++; int e3 = alttest_rpmvercmp("abc123", "abc012");          printf("  abc123 > abc012: %d %s\n",      e3, e3 > 0 ? "✅" : "❌"); passed += (e3 > 0);
    printf("\n");
    
    // results
    printf("RESULTS: %d/%d PASSED (%.1f%%)\n", passed, total, (float)passed/total*100);
    printf("%s\n", passed == total ? "ALL TESTS PASS!" : "❌ SOME TESTS FAILED!");
    
    return (passed == total) ? 0 : 1;
}
