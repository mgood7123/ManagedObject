/*
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 *
 * Hashmap is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <errno.h>

#include <managed_object_hashmap_base.h>


/* Table sizes must be powers of 2 */
#define MANAGED_OBJECT_HASHMAP_SIZE_MIN                32
#define MANAGED_OBJECT_HASHMAP_SIZE_DEFAULT            128
#define MANAGED_OBJECT_HASHMAP_SIZE_MOD(map, val)      ((val) & ((map)->table_size - 1))

/* Return the next linear probe index */
#define MANAGED_OBJECT_HASHMAP_PROBE_NEXT(map, index)  MANAGED_OBJECT_HASHMAP_SIZE_MOD(map, (index) + 1)


struct managed_object_hashmap_entry {
    void *key;
    void *data;
};


/*
 * Calculate the optimal table size, given the specified max number
 * of elements.
 */
static inline size_t managed_object_hashmap_calc_table_size(const struct managed_object_hashmap_base *hb, size_t size)
{
    size_t table_size;

    /* Enforce a maximum 0.75 load factor */
    table_size = size + (size / 3);

    /* Ensure capacity is not lower than the hashmap initial size */
    if (table_size < hb->table_size_init) {
        table_size = hb->table_size_init;
    } else {
        /* Round table size up to nearest power of 2 */
        table_size = 1 << ((sizeof(unsigned long) << 3) - __builtin_clzl(table_size - 1));
    }

    return table_size;
}

/*
 * Get a valid hash table index from a key.
 */
static inline size_t managed_object_hashmap_calc_index(const struct managed_object_hashmap_base *hb, const void *key)
{
    size_t index = hb->hash(key);

    /*
     * Run a secondary hash on the index. This is a small performance hit, but
     * reduces clustering and provides more consistent performance if a poor
     * hash function is used.
     */
    index = managed_object_hashmap_hash_default(&index, sizeof(index));

    return MANAGED_OBJECT_HASHMAP_SIZE_MOD(hb, index);
}

/*
 * Return the next populated entry, starting with the specified one.
 * Returns NULL if there are no more valid entries.
 */
static struct managed_object_hashmap_entry *managed_object_hashmap_entry_get_populated(const struct managed_object_hashmap_base *hb,
        const struct managed_object_hashmap_entry *entry)
{
    if (hb->size > 0) {
        for (; entry < &hb->table[hb->table_size]; ++entry) {
            if (entry->key) {
                return (struct managed_object_hashmap_entry *)entry;
            }
        }
    }
    return NULL;
}

/*
 * Find the hashmap entry with the specified key, or an empty slot.
 * Returns NULL if the entire table has been searched without finding a match.
 */
static struct managed_object_hashmap_entry *managed_object_hashmap_entry_find(const struct managed_object_hashmap_base *hb,
    const void *key, bool find_empty)
{
    size_t i;
    size_t index;
    struct managed_object_hashmap_entry *entry;

    index = managed_object_hashmap_calc_index(hb, key);

    /* Linear probing */
    for (i = 0; i < hb->table_size; ++i) {
        entry = &hb->table[index];
        if (!entry->key) {
            if (find_empty) {
                return entry;
            }
            return NULL;
        }
        if (hb->compare(key, entry->key) == 0) {
            return entry;
        }
        index = MANAGED_OBJECT_HASHMAP_PROBE_NEXT(hb, index);
    }
    return NULL;
}

/*
 * Removes the specified entry and processes the following entries to
 * keep the chain contiguous. This is a required step for hash maps
 * using linear probing.
 */
static void managed_object_hashmap_entry_remove(struct managed_object_hashmap_base *hb, struct managed_object_hashmap_entry *removed_entry)
{
    size_t i;
    size_t index;
    size_t entry_index;
    size_t removed_index = (removed_entry - hb->table);
    struct managed_object_hashmap_entry *entry;

    /* Free the key */
    if (hb->key_free) {
        hb->key_free(removed_entry->key);
    }
    --hb->size;

    /* Fill the free slot in the chain */
    index = MANAGED_OBJECT_HASHMAP_PROBE_NEXT(hb, removed_index);
    for (i = 0; i < hb->size; ++i) {
        entry = &hb->table[index];
        if (!entry->key) {
            /* Reached end of chain */
            break;
        }
        entry_index = managed_object_hashmap_calc_index(hb, entry->key);
        /* Shift in entries in the chain with an index at or before the removed slot */
        if (MANAGED_OBJECT_HASHMAP_SIZE_MOD(hb, index - entry_index) >
                MANAGED_OBJECT_HASHMAP_SIZE_MOD(hb, removed_index - entry_index)) {
            *removed_entry = *entry;
            removed_index = index;
            removed_entry = entry;
        }
        index = MANAGED_OBJECT_HASHMAP_PROBE_NEXT(hb, index);
    }
    /* Clear the last removed entry */
    memset(removed_entry, 0, sizeof(*removed_entry));
}

#include <stdio.h>

/*
 * Reallocates the hash table to the new size and rehashes all entries.
 * new_size MUST be a power of 2.
 * Returns 0 on success and -errno on allocation or hash function failure.
 */
static int managed_object_hashmap_rehash(struct managed_object_hashmap_base *hb, void * user_data, size_t table_size)
{
    size_t old_size;
    struct managed_object_hashmap_entry *old_table;
    struct managed_object_hashmap_entry *new_table;
    struct managed_object_hashmap_entry *entry;
    struct managed_object_hashmap_entry *new_entry;

    assert((table_size & (table_size - 1)) == 0);
    assert(table_size >= hb->size);

    new_table = (struct managed_object_hashmap_entry *)calloc(table_size, sizeof(struct managed_object_hashmap_entry));
    if (!new_table) {
        return -ENOMEM;
    }
    old_size = hb->table_size;
    old_table = hb->table;
    hb->table_size = table_size;
    hb->table = new_table;

    if (!old_table) {
        if (hb->on_realloc) {
            hb->on_realloc(user_data, new_table, table_size*2);
        }
        return 0;
    }

    /* Rehash */
    printf("rehashing from size %zu to size %zu\n", old_size, table_size);
    size_t o = 0;
    for (entry = old_table; entry < &old_table[old_size]; ++entry) {
        if (!entry->key) {
            continue;
        }
        new_entry = managed_object_hashmap_entry_find(hb, entry->key, true);
        /* Failure indicates an algorithm bug */
        assert(new_entry != NULL);

        /* Shallow copy */
        *new_entry = *entry;
        o++;
    }
    printf("rehashed %zu keys\n", o);
    if (hb->on_realloc) {
        hb->on_realloc(user_data, new_table, table_size*2);
    }
    free(old_table);
    return 0;
}

/*
 * Iterate through all entries and free all keys.
 */
static void managed_object_hashmap_free_keys(struct managed_object_hashmap_base *hb)
{
    struct managed_object_hashmap_entry *entry;

    if (!hb->key_free || hb->size == 0) {
        return;
    }
    for (entry = hb->table; entry < &hb->table[hb->table_size]; ++entry) {
        if (entry->key) {
            hb->key_free(entry->key);
        }
    }
}

/*
 * Initialize an empty hashmap.
 *
 * hash_func should return an even distribution of numbers between 0
 * and SIZE_MAX varying on the key provided.
 *
 * compare_func should return 0 if the keys match, and non-zero otherwise.
 */
void managed_object_hashmap_base_init(struct managed_object_hashmap_base *hb,
        size_t (*hash_func)(const void *), int (*compare_func)(const void *, const void *))
{
    assert(hash_func != NULL);
    assert(compare_func != NULL);

    memset(hb, 0, sizeof(*hb));

    hb->table_size_init = MANAGED_OBJECT_HASHMAP_SIZE_DEFAULT;
    hb->hash = hash_func;
    hb->compare = compare_func;
}

/*
 * Free the hashmap and all associated memory.
 */
void managed_object_hashmap_base_cleanup(struct managed_object_hashmap_base *hb, void * user_data)
{
    if (!hb) {
        return;
    }
    managed_object_hashmap_free_keys(hb);
    if (hb->on_realloc) {
        hb->on_realloc(user_data, NULL, 0);
    }
    free(hb->table);
    memset(hb, 0, sizeof(*hb));
}

/*
 * Hook into reallocation of table contigious memory.
 */
void managed_object_hashmap_base_set_on_realloc_func(struct managed_object_hashmap_base *hb,
    void (*on_realloc_func)(void * user_data, void * new_mem, size_t length_in_bytes)) {
    hb->on_realloc = on_realloc_func;
}

/*
 * Enable internal memory management of hash keys.
 */
void managed_object_hashmap_base_set_key_alloc_funcs(struct managed_object_hashmap_base *hb,
    void *(*key_dup_func)(const void *),
    void (*key_free_func)(void *))
{
    hb->key_dup = key_dup_func;
    hb->key_free = key_free_func;
}

/*
 * Set the hashmap's initial allocation size such that no rehashes are
 * required to fit the specified number of entries.
 * Returns 0 on success, or -errno on failure.
 */
int managed_object_hashmap_base_reserve(struct managed_object_hashmap_base *hb, void * user_data, size_t capacity)
{
    size_t old_size_init;
    int r = 0;

    /* Backup original init size in case of failure */
    old_size_init = hb->table_size_init;

    /* Set the minimal table init size to support the specified capacity */
    hb->table_size_init = MANAGED_OBJECT_HASHMAP_SIZE_MIN;
    hb->table_size_init = managed_object_hashmap_calc_table_size(hb, capacity);

    if (hb->table_size_init > hb->table_size) {
        r = managed_object_hashmap_rehash(hb, user_data, hb->table_size_init);
        if (r < 0) {
            hb->table_size_init = old_size_init;
        }
    }
    return r;
}

/*
 * Add a new entry to the hashmap. If an entry with a matching key
 * already exists -EEXIST is returned.
 * Returns 0 on success, or -errno on failure.
 */
int managed_object_hashmap_base_put(struct managed_object_hashmap_base *hb, void * user_data, const void *key, void *data)
{
    struct managed_object_hashmap_entry *entry;
    size_t table_size;
    int r = 0;

    if (!key || !data) {
        return -EINVAL;
    }

    /* Preemptively rehash with 2x capacity if load factor is approaching 0.75 */
    table_size = managed_object_hashmap_calc_table_size(hb, hb->size);
    if (table_size > hb->table_size) {
        r = managed_object_hashmap_rehash(hb, user_data, table_size);
    }

    /* Get the entry for this key */
    entry = managed_object_hashmap_entry_find(hb, key, true);
    if (!entry) {
        /*
         * Cannot find an empty slot. Either out of memory,
         * or hash or compare functions are malfunctioning.
         */
        if (r < 0) {
            /* Return rehash error, if set */
            return r;
        }
        return -EADDRNOTAVAIL;
    }

    if (entry->key) {
        /* Do not overwrite existing data */
        return -EEXIST;
    }

    if (hb->key_dup) {
        /* Allocate copy of key to simplify memory management */
        entry->key = hb->key_dup(key);
        if (!entry->key) {
            return -ENOMEM;
        }
    } else {
        entry->key = (void *)key;
    }
    entry->data = data;
    ++hb->size;
    return 0;
}

/*
 * Return the data pointer, or NULL if no entry exists.
 */
void *managed_object_hashmap_base_get(const struct managed_object_hashmap_base *hb, const void *key)
{
    struct managed_object_hashmap_entry *entry;

    if (!key) {
        return NULL;
    }

    entry = managed_object_hashmap_entry_find(hb, key, false);
    if (!entry) {
        return NULL;
    }
    return entry->data;
}

/*
 * Remove an entry with the specified key from the map.
 * Returns the data pointer, or NULL, if no entry was found.
 */
void *managed_object_hashmap_base_remove(struct managed_object_hashmap_base *hb, const void *key)
{
    struct managed_object_hashmap_entry *entry;
    void *data;

    if (!key) {
        return NULL;
    }

    entry = managed_object_hashmap_entry_find(hb, key, false);
    if (!entry) {
        return NULL;
    }
    data = entry->data;
    /* Clear the entry and make the chain contiguous */
    managed_object_hashmap_entry_remove(hb, entry);
    return data;
}

/*
 * Remove all entries.
 */
void managed_object_hashmap_base_clear(struct managed_object_hashmap_base *hb)
{
    managed_object_hashmap_free_keys(hb);
    hb->size = 0;
    memset(hb->table, 0, sizeof(struct managed_object_hashmap_entry) * hb->table_size);
}

/*
 * Remove all entries and reset the hash table to its initial size.
 */
int managed_object_hashmap_base_reset(struct managed_object_hashmap_base *hb, void * user_data)
{

    managed_object_hashmap_free_keys(hb);
    hb->size = 0;
    if (hb->table_size != hb->table_size_init) {
        struct managed_object_hashmap_entry *new_table;
        struct managed_object_hashmap_entry *old_table;
        new_table = (struct managed_object_hashmap_entry *)calloc(hb->table_size_init, sizeof(struct managed_object_hashmap_entry));
        if (!new_table) {
            return -ENOMEM;
        }
        old_table = hb->table;
        hb->table = new_table;
        hb->table_size = hb->table_size_init;
        if (hb->on_realloc) {
            hb->on_realloc(user_data, new_table, hb->table_size*2);
        }
        free(old_table);
    } else {
        memset(hb->table, 0, sizeof(struct managed_object_hashmap_entry) * hb->table_size);
    }
    return 0;
}

/*
 * Get a new hashmap iterator. The iterator is an opaque
 * pointer that may be used with managed_object_hashmap_iter_*() functions.
 * Hashmap iterators are INVALID after a put or remove operation is performed.
 * managed_object_hashmap_iter_remove() allows safe removal during iteration.
 */
struct managed_object_hashmap_entry *managed_object_hashmap_base_iter(const struct managed_object_hashmap_base *hb,
        const struct managed_object_hashmap_entry *pos)
{
    if (!pos) {
        pos = hb->table;
    }
    return managed_object_hashmap_entry_get_populated(hb, pos);
}

/*
 * Return true if an iterator is valid and safe to use.
 */
bool managed_object_hashmap_base_iter_valid(const struct managed_object_hashmap_base *hb, const struct managed_object_hashmap_entry *iter)
{
    return hb && iter && iter->key && iter >= hb->table && iter < &hb->table[hb->table_size];
}

/*
 * Advance an iterator to the next hashmap entry.
 * Returns false if there are no more entries.
 */
bool managed_object_hashmap_base_iter_next(const struct managed_object_hashmap_base *hb, struct managed_object_hashmap_entry **iter)
{
    if (!*iter) {
        return false;
    }
    return (*iter = managed_object_hashmap_entry_get_populated(hb, *iter + 1)) != NULL;
}

/*
 * Remove the hashmap entry pointed to by this iterator and advance the
 * iterator to the next entry.
 * Returns true if the iterator is valid after the operation.
 */
bool managed_object_hashmap_base_iter_remove(struct managed_object_hashmap_base *hb, struct managed_object_hashmap_entry **iter)
{
    if (!*iter) {
        return false;
    }
    if ((*iter)->key) {
        /* Remove entry if iterator is valid */
        managed_object_hashmap_entry_remove(hb, *iter);
    }
    return (*iter = managed_object_hashmap_entry_get_populated(hb, *iter)) != NULL;
}

/*
 * Return the key of the entry pointed to by the iterator.
 */
const void *managed_object_hashmap_base_iter_get_key(const struct managed_object_hashmap_entry *iter)
{
    if (!iter) {
        return NULL;
    }
    return (const void *)iter->key;
}

/*
 * Return the data of the entry pointed to by the iterator.
 */
void *managed_object_hashmap_base_iter_get_data(const struct managed_object_hashmap_entry *iter)
{
    if (!iter) {
        return NULL;
    }
    return iter->data;
}

/*
 * Set the data pointer of the entry pointed to by the iterator.
 */
int managed_object_hashmap_base_iter_set_data(struct managed_object_hashmap_entry *iter, void *data)
{
    if (!iter) {
        return -EFAULT;
    }
    if (!data) {
        return -EINVAL;
    }
    iter->data = data;
    return 0;
}

/*
 * Return the load factor.
 */
double managed_object_hashmap_base_load_factor(const struct managed_object_hashmap_base *hb)
{
    if (!hb->table_size) {
        return 0;
    }
    return (double)hb->size / hb->table_size;
}

/*
 * Return the number of collisions for this key.
 * This would always be 0 if a perfect hash function was used, but in ordinary
 * usage, there may be a few collisions, depending on the hash function and
 * load factor.
 */
size_t managed_object_hashmap_base_collisions(const struct managed_object_hashmap_base *hb, const void *key)
{
    size_t i;
    size_t index;
    struct managed_object_hashmap_entry *entry;

    if (!key) {
        return 0;
    }

    index = managed_object_hashmap_calc_index(hb, key);

    /* Linear probing */
    for (i = 0; i < hb->table_size; ++i) {
        entry = &hb->table[index];
        if (!entry->key) {
            /* Key does not exist */
            return 0;
        }
        if (hb->compare(key, entry->key) == 0) {
            break;
        }
        index = MANAGED_OBJECT_HASHMAP_PROBE_NEXT(hb, index);
    }

    return i;
}

/*
 * Return the average number of collisions per entry.
 */
double managed_object_hashmap_base_collisions_mean(const struct managed_object_hashmap_base *hb)
{
    struct managed_object_hashmap_entry *entry;
    size_t total_collisions = 0;

    if (!hb->size) {
        return 0;
    }
    for (entry = hb->table; entry < &hb->table[hb->table_size]; ++entry) {
        if (!entry->key) {
            continue;
        }

        total_collisions += managed_object_hashmap_base_collisions(hb, entry->key);
    }
    return (double)total_collisions / hb->size;
}

/*
 * Return the variance between entry collisions. The higher the variance,
 * the more likely the hash function is poor and is resulting in clustering.
 */
double managed_object_hashmap_base_collisions_variance(const struct managed_object_hashmap_base *hb)
{
    struct managed_object_hashmap_entry *entry;
    double mean_collisions;
    double variance;
    double total_variance = 0;

    if (!hb->size) {
        return 0;
    }
    mean_collisions = managed_object_hashmap_base_collisions_mean(hb);
    for (entry = hb->table; entry < &hb->table[hb->table_size]; ++entry) {
        if (!entry->key) {
            continue;
        }
        variance = (double)managed_object_hashmap_base_collisions(hb, entry->key) - mean_collisions;
        total_variance += variance * variance;
    }
    return total_variance / hb->size;
}

/*
 * Recommended hash function for data keys.
 *
 * This is an implementation of the well-documented Jenkins one-at-a-time
 * hash function. See https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
size_t managed_object_hashmap_hash_default(const void *data, size_t len)
{
    const uint8_t *byte = (const uint8_t *)data;
    size_t hash = 0;

    for (size_t i = 0; i < len; ++i) {
        hash += *byte++;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/*
 * Recommended hash function for string keys.
 *
 * This is an implementation of the well-documented Jenkins one-at-a-time
 * hash function. See https://en.wikipedia.org/wiki/Jenkins_hash_function
 */
size_t managed_object_hashmap_hash_string(const char *key)
{
    size_t hash = 0;

    for (; *key; ++key) {
        hash += *key;
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

/*
 * Case insensitive hash function for string keys.
 */
size_t managed_object_hashmap_hash_string_i(const char *key)
{
    size_t hash = 0;

    for (; *key; ++key) {
        hash += tolower(*key);
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}
