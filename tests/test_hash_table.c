#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/* ─────────────────────────────────────────────
 *  Minimal test file
 *  Each test_ function calls assert() on failure.
 *  A passing run prints "All tests passed." Thats it!
 * ───────────────────────────────────────────── */

static int passed = 0;
static int total  = 0;

#define RUN(fn) do {                          \
    total++;                                  \
    fn();                                     \
    passed++;                                 \
    printf("  PASS  %s\n", #fn);             \
} while(0)

/* ─── Individual test cases ─────────────────── */

static void test_create_and_destroy(void)
{
    ht *t = ht_create();
    assert(t != NULL);
    assert(ht_length(t) == 0);
    ht_destroy(t);
}

static void test_set_and_get(void)
{
    ht *t = ht_create();
    int v = 42;
    ht_set(t, "answer", &v);

    int *got = ht_get(t, "answer");
    assert(got != NULL);
    assert(*got == 42);

    ht_destroy(t);
}

static void test_get_missing_key(void)
{
    ht *t = ht_create();
    assert(ht_get(t, "ghost") == NULL);
    ht_destroy(t);
}

static void test_update_existing_key(void)
{
    ht *t  = ht_create();
    int v1 = 1, v2 = 2;

    ht_set(t, "x", &v1);
    assert(ht_length(t) == 1);

    ht_set(t, "x", &v2);
    assert(ht_length(t) == 1);          /* size must not grow on update */

    int *got = ht_get(t, "x");
    assert(got != NULL && *got == 2);   /* value must be the new one */

    ht_destroy(t);
}

static void test_multiple_keys(void)
{
    ht *t = ht_create();
    int a = 1, b = 2, c = 3;

    ht_set(t, "alpha", &a);
    ht_set(t, "beta",  &b);
    ht_set(t, "gamma", &c);

    assert(ht_length(t) == 3);
    assert(*(int *)ht_get(t, "alpha") == 1);
    assert(*(int *)ht_get(t, "beta")  == 2);
    assert(*(int *)ht_get(t, "gamma") == 3);

    ht_destroy(t);
}

static void test_expand_and_rehash(void)
{
    /*
     * Insert enough entries to test the expansion feature
     * (initial capacity = 16, expansion threshold = capacity / 2 = 8).
     * After inserting 20 entries everything must still be retrievable.
     */
    ht  *t = ht_create();
    int  vals[20];
    char key[16];

    for (int i = 0; i < 20; i++) {
        vals[i] = i * 10;
        snprintf(key, sizeof(key), "key_%d", i);
        ht_set(t, key, &vals[i]);
    }

    assert(ht_length(t) == 20);

    for (int i = 0; i < 20; i++) {
        snprintf(key, sizeof(key), "key_%d", i);
        int *got = ht_get(t, key);
        assert(got != NULL && *got == i * 10);
    }

    ht_destroy(t);
}

static void test_iterator(void)
{
    ht *t = ht_create();
    int a = 1, b = 2, c = 3;

    ht_set(t, "one",   &a);
    ht_set(t, "two",   &b);
    ht_set(t, "three", &c);

    int count = 0;
    hti it = ht_iterator(t);
    while (ht_next(&it)) {
        assert(it.key   != NULL);
        assert(it.value != NULL);
        count++;
    }
    assert(count == 3);

    ht_destroy(t);
}

static void test_null_value_rejected(void)
{
    /*
     * ht_set with value=NULL must return NULL... DUH!
     * and must NOT increase the table size.
     */
    ht *t = ht_create();
    const char *result = ht_set(t, "key", NULL);
    assert(result == NULL);
    assert(ht_length(t) == 0);
    ht_destroy(t);
}

/* ─── Runner ─────────────────────────────────── */

int main(void)
{
    printf("\n========== HASH TABLE TESTS ==========\n\n");

    RUN(test_create_and_destroy);
    RUN(test_set_and_get);
    RUN(test_get_missing_key);
    RUN(test_update_existing_key);
    RUN(test_multiple_keys);
    RUN(test_expand_and_rehash);
    RUN(test_iterator);
    RUN(test_null_value_rejected);

    printf("\n%d / %d tests passed.\n", passed, total);
    if (passed == total)
        printf("All tests passed.\n\n");
    else
        printf("SOME TESTS FAILED.\n\n");

    return (passed == total) ? 0 : 1;
}
