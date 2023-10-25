/*
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 *
 * Hashmap is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef MANAGED_OBJECT_HASHMAP_H
#define MANAGED_OBJECT_HASHMAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <managed_object_hashmap_base.h>

/*
 * INTERNAL USE ONLY: Updates an iterator structure after the current element was removed.
 */
#define MANAGED_OBJECT_PRIVATE_HASHMAP_ITER_RESET(iter) ({                                   \
    ((iter)->iter_pos = managed_object_hashmap_base_iter((iter)->iter_map, (iter)->iter_pos)) != NULL; \
})

/*
 * INTERNAL USE ONLY: foreach macro internals.
 */
#define MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT_2(x, y)        x ## y
#define MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT(x, y)          MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT_2(x, y)
#define MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(prefix)   MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT(MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT(prefix, __COUNTER__), _)
#define MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(unique, name)  MANAGED_OBJECT_PRIVATE_HASHMAP_CONCAT(unique, name)
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH(x, key, data, h)                              \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        ((key) = managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))) &&     \
            ((data) = managed_object_hashmap_iter_get_data(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))); \
        managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_SAFE(x, key, data, h, temp_ptr)               \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        ((temp_ptr) = (void *)((key) = managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))) && \
            ((data) = managed_object_hashmap_iter_get_data(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))); \
        ((temp_ptr) == (void *)managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))) ? \
            managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)) : MANAGED_OBJECT_PRIVATE_HASHMAP_ITER_RESET(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_KEY(x, key, h)                                \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        (key = managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)));         \
        managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_KEY_SAFE(x, key, h, temp_ptr)                 \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        ((temp_ptr) = (void *)((key) = managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))); \
        ((temp_ptr) == (void *)managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))) ? \
            managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)) : MANAGED_OBJECT_PRIVATE_HASHMAP_ITER_RESET(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_DATA(x, data, h)                              \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        (data = managed_object_hashmap_iter_get_data(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)));       \
        managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))
#define MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_DATA_SAFE(x, data, h, temp_ptr)               \
    for (MANAGED_OBJECT_HASHMAP_ITER(*(h)) MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it) = managed_object_hashmap_iter(h);  \
        ((temp_ptr) = (void *)managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))) && \
            ((data) = managed_object_hashmap_iter_get_data(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))); \
        ((temp_ptr) == (void *)managed_object_hashmap_iter_get_key(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it))) ? \
            managed_object_hashmap_iter_next(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)) : MANAGED_OBJECT_PRIVATE_HASHMAP_ITER_RESET(&MANAGED_OBJECT_PRIVATE_HASHMAP_UNIQUE(x, it)))


/*
 * Template macro to define a type-specific hashmap.
 *
 * Example declarations:
 *   MANAGED_OBJECT_HASHMAP(int, struct foo) map1;
 *   // key_type:       const int *
 *   // data_type:      struct foo *
 *
 *   MANAGED_OBJECT_HASHMAP(char, char) map2;
 *   // key_type:       const char *
 *   // data_type:      char *
 */
#define MANAGED_OBJECT_HASHMAP(key_type, data_type)                                    \
    struct {                                                            \
        struct managed_object_hashmap_base map_base;                                   \
        struct {                                                        \
            const key_type *t_key;                                      \
            data_type *t_data;                                          \
            size_t (*t_hash_func)(const key_type *);                    \
            int (*t_compare_func)(const key_type *, const key_type *);  \
            key_type *(*t_key_dup_func)(const key_type *);              \
            void (*t_key_free_func)(key_type *);                        \
            int (*t_foreach_func)(const key_type *, data_type *, void *); \
            struct {                                                    \
                struct managed_object_hashmap_base *iter_map;                          \
                struct managed_object_hashmap_entry *iter_pos;                         \
                struct {                                                \
                    const key_type *t_key;                              \
                    data_type *t_data;                                  \
                } iter_types[0];                                        \
            } t_iterator;                                               \
        } map_types[0];                                                 \
    }

/*
 * Template macro to define a hashmap iterator.
 *
 * Example declarations:
 *   MANAGED_OBJECT_HASHMAP_ITER(my_hashmap) iter;
 */
#define MANAGED_OBJECT_HASHMAP_ITER(managed_object_hashmap_type)                                      \
    typeof((managed_object_hashmap_type).map_types->t_iterator)


/*
 * Initialize an empty hashmap.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   size_t (*hash_func)(const <key_type> *) - hash function that should return an
 *              even distribution of numbers between 0 and SIZE_MAX varying on the key provided.
 *   int (*compare_func)(const <key_type> *, const <key_type> *) - key comparison function that
 *              should return 0 if the keys match, and non-zero otherwise.
 *
 * This library provides some basic hash functions:
 *   size_t managed_object_hashmap_hash_default(const void *data, size_t len) - Jenkins one-at-a-time hash for
 *           keys of any data type. Create a type-specific wrapper function to pass to managed_object_hashmap_init().
 *   size_t managed_object_hashmap_hash_string(const char *key) - case sensitive string hash function.
 *           Pass this directly to managed_object_hashmap_init().
 *   size_t managed_object_hashmap_hash_string_i(const char *key) - non-case sensitive string hash function.
 *           Pass this directly to managed_object_hashmap_init().
 */
#define managed_object_hashmap_init(h, hash_func, compare_func) do {                   \
    typeof((h)->map_types->t_hash_func) __map_hash = (hash_func);       \
    typeof((h)->map_types->t_compare_func) __map_compare = (compare_func); \
    managed_object_hashmap_base_init(&(h)->map_base, (size_t (*)(const void *))__map_hash, (int (*)(const void *, const void *))__map_compare); \
} while (0)

/*
 * Free the hashmap and all associated memory.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_cleanup(h, user_data)                                              \
    managed_object_hashmap_base_cleanup(&(h)->map_base, user_data)

/*
 * Hook into reallocation of table contigious memory.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   void (*on_realloc_func)(void *, void *, size_t) - inspect the reallocated memory before the old memory is freed.
 */
#define managed_object_hashmap_set_on_realloc_func(h, on_realloc_func) do { \
    managed_object_hashmap_base_set_on_realloc_func(&(h)->map_base, on_realloc_func); \
} while (0)

/*
 * Enable internal memory allocation and management for hash keys.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   <key_type> *(*key_dup_func)(const <key_type> *) - allocate a copy of the key to be
 *              managed internally by the hashmap.
 *   void (*key_free_func)(<key_type> *) - free resources associated with a key
 */
#define managed_object_hashmap_set_key_alloc_funcs(h, key_dup_func, key_free_func) do { \
    typeof((h)->map_types->t_key_dup_func) __map_key_dup = (key_dup_func); \
    typeof((h)->map_types->t_key_free_func) __map_key_free = (key_free_func); \
    managed_object_hashmap_base_set_key_alloc_funcs(&(h)->map_base, (void *(*)(const void *))__map_key_dup, (void(*)(void *))__map_key_free); \
} while (0)

/*
 * Return the number of entries in the hash map.
 *
 * Parameters:
 *   const MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_size(h)                                                 \
    ((typeof((h)->map_base.size))(h)->map_base.size)

/*
 * Set the hashmap's initial allocation size such that no rehashes are
 * required to fit the specified number of entries.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   size_t capacity - number of entries.
 *
 * Returns 0 on success, or -errno on failure.
 */
#define managed_object_hashmap_reserve(h, user_data, capacity)                                    \
    managed_object_hashmap_base_reserve(&(h)->map_base, user_data, capacity)

/*
 * Add a new entry to the hashmap. If an entry with a matching key
 * already exists -EEXIST is returned.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   <key_type> *key - pointer to the entry's key
 *   <data_type> *data - pointer to the entry's data
 *
 * Returns 0 on success, or -errno on failure.
 */
#define managed_object_hashmap_put(h, user_data, key, data) ({                                    \
    typeof((h)->map_types->t_key) __map_key = (key);                    \
    typeof((h)->map_types->t_data) __map_data = (data);                 \
    managed_object_hashmap_base_put(&(h)->map_base, user_data, (const void *)__map_key, (void *)__map_data); \
})

/*
 * Do a constant-time lookup of a hashmap entry.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   <key_type> *key - pointer to the key to lookup
 *
 * Return the data pointer, or NULL if no entry exists.
 */
#define managed_object_hashmap_get(h, key) ({                                          \
    typeof((h)->map_types->t_key) __map_key = (key);                    \
    (typeof((h)->map_types->t_data))managed_object_hashmap_base_get(&(h)->map_base, (const void *)__map_key); \
})

/*
 * Remove an entry with the specified key from the map.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   <key_type> *key - pointer to the key to remove
 *
 * Returns the data pointer, or NULL, if no entry was found.
 *
 * Note: it is not safe to call this function while iterating, unless
 * the "safe" variant of the foreach macro is used, and only the current
 * key is removed.
 */
#define managed_object_hashmap_remove(h, key) ({                                       \
    typeof((h)->map_types->t_key) __map_key = (key);                    \
    (typeof((h)->map_types->t_data))managed_object_hashmap_base_remove(&(h)->map_base, (const void *)__map_key); \
})

/*
 * Remove all entries.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_clear(h)                                                \
    managed_object_hashmap_base_clear(&(h)->map_base)

/*
 * Remove all entries and reset the hash table to its initial size.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *
 * Returns 0 on success, or -errno on failure.
 */
#define managed_object_hashmap_reset(h, user_data)                                                \
    managed_object_hashmap_base_reset(&(h)->map_base, user_data)

/*
 * Return an iterator for this hashmap. The iterator is a type-specific
 * structure that may be declared using the MANAGED_OBJECT_HASHMAP_ITER() macro.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_iter(h)                                                 \
    ((MANAGED_OBJECT_HASHMAP_ITER(*(h))){ &(h)->map_base, managed_object_hashmap_base_iter(&(h)->map_base, NULL) })

/*
 * Return true if an iterator is valid and safe to use.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 */
#define managed_object_hashmap_iter_valid(iter)                                        \
    managed_object_hashmap_base_iter_valid((iter)->iter_map, (iter)->iter_pos)

/*
 * Advance an iterator to the next hashmap entry.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 *
 * Returns true if the iterator is valid after the operation.
 */
#define managed_object_hashmap_iter_next(iter)                                         \
    managed_object_hashmap_base_iter_next((iter)->iter_map, &(iter)->iter_pos)

/*
 * Remove the hashmap entry pointed to by this iterator and advance the
 * iterator to the next entry.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 *
 * Returns true if the iterator is valid after the operation.
 */
#define managed_object_hashmap_iter_remove(iter)                                       \
    managed_object_hashmap_base_iter_remove((iter)->iter_map, &(iter)->iter_pos)

/*
 * Return the key of the entry pointed to by the iterator.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 */
#define managed_object_hashmap_iter_get_key(iter)                                      \
    ((typeof((iter)->iter_types->t_key))managed_object_hashmap_base_iter_get_key((iter)->iter_pos))

/*
 * Return the data of the entry pointed to by the iterator.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 */
#define managed_object_hashmap_iter_get_data(iter)                                     \
    ((typeof((iter)->iter_types->t_data))managed_object_hashmap_base_iter_get_data((iter)->iter_pos))

/*
 * Set the data pointer of the entry pointed to by the iterator.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP_ITER(<managed_object_hashmap_type>) *iter - iterator pointer
 *   <data_type> *data - new data pointer
 */
#define managed_object_hashmap_iter_set_data(iter, data) ({                            \
    (typeof((iter)->iter_types->t_data)) __map_data = (data);           \
    managed_object_hashmap_base_iter_set_data((iter)->iter_pos), (void *)__map_data); \
})

/*
 * Convenience macro to iterate through the contents of a hashmap.
 * key and data are assigned pointers to the current hashmap entry.
 * It is NOT safe to modify the hashmap while iterating.
 *
 * Parameters:
 *   const <key_type> *key - key pointer assigned on each iteration
 *   <data_type> *data - data pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_foreach(key, data, h)                                   \
    MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (key), (data), (h))

/*
 * Convenience macro to iterate through the contents of a hashmap.
 * key and data are assigned pointers to the current hashmap entry.
 * Unlike managed_object_hashmap_foreach(), it is safe to call managed_object_hashmap_remove() on the
 * current entry.
 *
 * Parameters:
 *   const <key_type> *key - key pointer assigned on each iteration
 *   <data_type> *data - data pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   void *temp_ptr - opaque pointer assigned on each iteration
 */
#define managed_object_hashmap_foreach_safe(key, data, h, temp_ptr)                    \
    MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_SAFE(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (key), (data), (h), (temp_ptr))

/*
 * Convenience macro to iterate through the keys of a hashmap.
 * key is assigned a pointer to the current hashmap entry.
 * It is NOT safe to modify the hashmap while iterating.
 *
 * Parameters:
 *   const <key_type> *key - key pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_foreach_key(key, h)                                     \
    MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_KEY(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (key), (h))

/*
 * Convenience macro to iterate through the keys of a hashmap.
 * key is assigned a pointer to the current hashmap entry.
 * Unlike managed_object_hashmap_foreach_key(), it is safe to call managed_object_hashmap_remove() on the
 * current entry.
 *
 * Parameters:
 *   const <key_type> *key - key pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   void *temp_ptr - opaque pointer assigned on each iteration
 */
#define managed_object_hashmap_foreach_key_safe(key, h, temp_ptr)                      \
        MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_KEY_SAFE(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (key), (h), (temp_ptr))

/*
 * Convenience macro to iterate through the data of a hashmap.
 * data is assigned a pointer to the current hashmap entry.
 * It is NOT safe to modify the hashmap while iterating.
 *
 * Parameters:
 *   <data_type> *data - data pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_foreach_data(data, h)                                   \
    MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_DATA(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (data), (h))

/*
 * Convenience macro to iterate through the data of a hashmap.
 * data is assigned a pointer to the current hashmap entry.
 * Unlike managed_object_hashmap_foreach_data(), it is safe to call managed_object_hashmap_remove() on the
 * current entry.
 *
 * Parameters:
 *   <data_type> *data - data pointer assigned on each iteration
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   void *temp_ptr - opaque pointer assigned on each iteration
 */
#define managed_object_hashmap_foreach_data_safe(data, h, temp_ptr)                    \
    MANAGED_OBJECT_PRIVATE_HASHMAP_FOREACH_DATA_SAFE(MANAGED_OBJECT_PRIVATE_HASHMAP_MAKE_UNIQUE(__map), (data), (h), (temp_ptr))

/*
 * Return the load factor.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_load_factor(h)                                          \
    managed_object_hashmap_base_load_factor(&(h)->map_base)

/*
 * Return the number of collisions for this key.
 * This would always be 0 if a perfect hash function was used, but in ordinary
 * usage, there may be a few collisions, depending on the hash function and
 * load factor.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 *   <key_type> *key - pointer to the entry's key
 */
#define managed_object_hashmap_collisions(h, key) ({                                   \
    typeof((h)->map_types->t_key) __map_key = (key);                    \
    managed_object_hashmap_base_collisions(&(h)->map_base, (const void *)__map_key);   \
})

/*
 * Return the average number of collisions per entry.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_collisions_mean(h)                                      \
    managed_object_hashmap_base_collisions_mean(&(h)->map_base)

/*
 * Return the variance between entry collisions. The higher the variance,
 * the more likely the hash function is poor and is resulting in clustering.
 *
 * Parameters:
 *   MANAGED_OBJECT_HASHMAP(<key_type>, <data_type>) *h - hashmap pointer
 */
#define managed_object_hashmap_collisions_variance(h)                                  \
    managed_object_hashmap_base_collisions_variance(&(h)->map_base)

#ifdef __cplusplus
}
#endif

#endif