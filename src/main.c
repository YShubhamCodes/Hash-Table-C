#include "hash_table.h"
 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
 
#define FNV_OFFSET_DEMO 14695981039346656037UL
#define FNV_PRIME_DEMO  1099511628211UL
 
static uint64_t demo_hash(const char *key)
{
    uint64_t h = FNV_OFFSET_DEMO;
    for (const char *p = key; *p; p++) {
        h ^= (uint64_t)(unsigned char)(*p);
        h *= FNV_PRIME_DEMO;
    }
    return h;
}
 
static void exit_nomem(void)
{
    fprintf(stderr, "out of memory\n");
    exit(1);
}
 

static void print_layout(ht *table)
{
    printf("\n--- TABLE LAYOUT (%zu / %zu slots used) ---\n",
           table->size, table->capacity);
 
    for (size_t i = 0; i < table->capacity; i++) {
        ht_entry entry = table->buckets[i];
        if (entry.key != NULL) {
            printf("[%02zu] OCCUPIED  key=%-18s  hash=%-20llu  value=%d\n",
                   i,
                   entry.key,
                   (unsigned long long)demo_hash(entry.key),
                   *(int *)entry.value);
        } else {
            printf("[%02zu] EMPTY\n", i);
        }
    }
    printf("--------------------------------------------\n\n");
}
 

static void demo_hash_table(void)
{
    printf("========== HASH TABLE DEMO ==========\n\n");
 
    ht *table = ht_create();
    if (table == NULL) {
        fprintf(stderr, "Failed to create hash table\n");
        return;
    }
 
    /* Key-value dataset: Christopher Nolan films → release year */
    const char *movies[] = {
        "Dark_Knight", "Inception",   "Interstellar",
        "Memento",     "Tenet",       "Dunkirk",
        "Prestige",    "Batman_Begins","Oppenheimer",
        "Insomnia"
    };
    int years[] = { 2008, 2010, 2014, 2000, 2020, 2017, 2006, 2005, 2023, 2002 };
    size_t n    = sizeof(movies) / sizeof(movies[0]);
 
    printf("Initial capacity: %zu\n\n", table->capacity);
 
    /* ── Phase 1: Insert all entries ── */
    printf("--- Phase 1: Insertions ---\n");
    for (size_t i = 0; i < n; i++) {
        int *year = malloc(sizeof(int));
        if (year == NULL) exit_nomem();
        *year = years[i];
 
        printf("  INSERT %-18s => %d", movies[i], *year);
        ht_set(table, movies[i], year);
        printf("  (size=%zu, cap=%zu)\n", table->size, table->capacity);
    }
 
    print_layout(table);
 
    /* ── Phase 2: Lookup ── */
    printf("--- Phase 2: Lookups ---\n");
    const char *lookups[] = { "Inception", "Tenet", "Ghost" };
    for (size_t i = 0; i < 3; i++) {
        int *v = ht_get(table, lookups[i]);
        if (v)
            printf("  GET %-18s => %d\n", lookups[i], *v);
        else
            printf("  GET %-18s => (not found)\n", lookups[i]);
    }
    printf("\n");
 
    /* ── Phase 3: Update existing keys ── */
    printf("--- Phase 3: Updates (same keys, same values) ---\n");
    int *y1 = malloc(sizeof(int)); *y1 = 2008;
    int *y2 = malloc(sizeof(int)); *y2 = 2010;
    ht_set(table, "Dark_Knight", y1);
    ht_set(table, "Inception",   y2);
    printf("  Updated Dark_Knight and Inception (size should stay %zu)\n\n",
           table->size);
 
    print_layout(table);
 
    /* ── Cleanup: iterate to free all values, then destroy table ── */
    hti it = ht_iterator(table);
    while (ht_next(&it))
        free(it.value);
 
    ht_destroy(table);
    printf("====== Demo Complete =======\n");
}
 
int main(void)
{
    demo_hash_table();
    return 0;
}

