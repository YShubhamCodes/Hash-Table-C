#define _POSIX_C_SOURCE 200809L  /* exposes strdup() */
#include "hash_table.h"

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* ─────────────────────────────────────────────
 *  FNV-1a 64-bit constants
 *  https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
 * ───────────────────────────────────────────── */
#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME  1099511628211UL

#define INITIAL_CAPACITY 16

static uint64_t hash_key(const char *key)
{
    uint64_t hash = FNV_OFFSET;
    for (const char *p = key; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}

static const char *ht_set_entry(ht_entry *buckets, size_t capacity,
                                 const char *key, void *value,
                                 size_t *psize)
{
    uint64_t hash  = hash_key(key);
    size_t   index = (size_t)(hash & (uint64_t)(capacity - 1));

    while (buckets[index].key != NULL) {
        if (strcmp(key, buckets[index].key) == 0) {
            /* Key already exists — update value in place. */
            buckets[index].value = value;
            return buckets[index].key;
        }
        /* Linear probe: advance, wrap at end. */
        if (++index >= capacity) index = 0;
    }

    /* Empty slot found — new insertion. */
    if (psize != NULL) {
        /* Live insert: take ownership of the key string. */
        key = strdup(key);
        if (key == NULL) return NULL;   /* OOM */
        (*psize)++;
    }

    buckets[index].key   = key;
    buckets[index].value = value;
    return key;
}

/*
 * ht_expand — double the bucket array and rehash all live entries.
 * Returns false on allocation failure
 */
static bool ht_expand(ht *table)
{
    size_t new_capacity = table->capacity * 2;
    if (new_capacity < table->capacity) return false; /* overflow guard */

    ht_entry *new_buckets = calloc(new_capacity, sizeof(ht_entry));
    if (new_buckets == NULL) return false;

    /* Rehash: psize=NULL so we reuse already-allocated key strings. */
    for (size_t i = 0; i < table->capacity; i++) {
        ht_entry e = table->buckets[i];
        if (e.key != NULL)
            ht_set_entry(new_buckets, new_capacity, e.key, e.value, NULL);
    }

    free(table->buckets);
    table->buckets  = new_buckets;
    table->capacity = new_capacity;
    return true;
}

/* ─────────────────────────────────────────────
 *  Public API
 * ───────────────────────────────────────────── */

ht *ht_create(void)
{
    ht *table = malloc(sizeof(ht));
    if (table == NULL) return NULL;

    table->capacity = INITIAL_CAPACITY;
    table->size     = 0;
    table->buckets  = calloc(table->capacity, sizeof(ht_entry));

    if (table->buckets == NULL) {
        free(table);
        return NULL;
    }
    return table;
}

void ht_destroy(ht *table)
{
    for (size_t i = 0; i < table->capacity; i++)
        free((void *)table->buckets[i].key);

    free(table->buckets);
    free(table);
}

void *ht_get(ht *table, const char *key)
{
    uint64_t hash  = hash_key(key);
    size_t   index = (size_t)(hash & (uint64_t)(table->capacity - 1));

    while (table->buckets[index].key != NULL) {
        if (strcmp(key, table->buckets[index].key) == 0)
            return table->buckets[index].value;

        if (++index >= table->capacity) index = 0;
    }
    return NULL;
}

const char *ht_set(ht *table, const char *key, void *value)
{
    if (value == NULL) return NULL;

    /* Expand before load factor hits 0.5. */
    if (table->size >= table->capacity / 2) {
        if (!ht_expand(table)) return NULL;
    }

    return ht_set_entry(table->buckets, table->capacity, key, value, &table->size);
}

size_t ht_length(ht *table)
{
    return table->size;
}

hti ht_iterator(ht *table)
{
    return (hti){ ._table = table, ._index = 0 };
}

bool ht_next(hti *it)
{
    ht *table = it->_table;
    while (it->_index < table->capacity) {
        size_t i = it->_index++;
        if (table->buckets[i].key != NULL) {
            it->key   = table->buckets[i].key;
            it->value = table->buckets[i].value;
            return true;
        }
    }
    return false;
}
