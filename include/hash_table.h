#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <stddef.h>
#include <stdbool.h>

/* ─────────────────────────────────────────────
 *  Internal bucket layout
 *  Each slot holds a heap-copied key, a void*
 *  value. NOTE: Tombstone (future deletion) is yet
 *  added.
 * ───────────────────────────────────────────── */
typedef struct {
    const char *key;    
    void       *value;
    bool        is_tombstone;
} ht_entry;

/* ─────────────────────────────────────────────
 *  Hash table handle
 *  Open-addressing with linear probing.
 *  Expands when size >= capacity / 2  (load factor 0.5).
 * ───────────────────────────────────────────── */
typedef struct {
    ht_entry *buckets;
    size_t    capacity;
    size_t    size;
} ht;

/* ─────────────────────────────────────────────
 *  Iterator  —  use like:
 *
 *      hti it = ht_iterator(table);
 *      while (ht_next(&it)) {
 *          printf("%s\n", it.key);
 *      }
 * ───────────────────────────────────────────── */
typedef struct {
    ht         *_table;   /* private — do not touch */
    size_t      _index;   /* private — do not touch */
    const char *key;      /* read-only after ht_next() */
    void       *value;    /* read-only after ht_next() */
} hti;

/* ─── Lifecycle ─────────────────────────────── */
ht         *ht_create(void);
void        ht_destroy(ht *table);

/* ─── Core operations ───────────────────────── */

/*
 * ht_get: returns the value for key, or NULL if not found.
 */
void       *ht_get(ht *table, const char *key);

/*
 * ht_set: inserts or updates key→value.
 *   value must not be NULL (NULL signals "not found" in ht_get).
 *   Returns the interned key string on success, NULL on OOM.
 */
const char *ht_set(ht *table, const char *key, void *value);

/*
 * ht_length: returns the number of live entries.
 */
size_t      ht_length(ht *table);

/* ─── Iterator ──────────────────────────────── */
hti         ht_iterator(ht *table);
bool        ht_next(hti *it);

#endif /* HASH_TABLE_H */
