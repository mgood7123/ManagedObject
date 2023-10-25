/*
 * Copyright (c) 2016-2020 David Leeds <davidesleeds@gmail.com>
 *
 * Hashmap is free software; you can redistribute it and/or modify
 * it under the terms of the MIT license. See LICENSE for details.
 */

#ifndef MANAGED_OBJECT_HASHMAP_BASE_H
#define MANAGED_OBJECT_HASHMAP_BASE_H

#include <stdbool.h>
#include <errno.h>

struct managed_object_hashmap_entry;

struct managed_object_hashmap_base {
    size_t table_size_init;
    size_t table_size;
    size_t size;
    struct managed_object_hashmap_entry *table;
    size_t (*hash)(const void *);
    int (*compare)(const void *, const void *);
    void *(*key_dup)(const void *);
    void (*key_free)(void *);
    void (*on_realloc)(void * user_data, void * new_mem, size_t length_in_bytes);
};

void managed_object_hashmap_base_init(struct managed_object_hashmap_base *hb,
        size_t (*hash_func)(const void *), int (*compare_func)(const void *, const void *));
void managed_object_hashmap_base_cleanup(struct managed_object_hashmap_base *hb, void * user_data);

void managed_object_hashmap_base_set_on_realloc_func(struct managed_object_hashmap_base *hb,
    void (*on_realloc_func)(void * user_data, void * new_mem, size_t length_in_bytes));

void managed_object_hashmap_base_set_key_alloc_funcs(struct managed_object_hashmap_base *hb,
    void *(*key_dup_func)(const void *), void (*key_free_func)(void *));

int managed_object_hashmap_base_reserve(struct managed_object_hashmap_base *hb, void * user_data, size_t capacity);

int managed_object_hashmap_base_put(struct managed_object_hashmap_base *hb, void * user_data, const void *key, void *data);
void *managed_object_hashmap_base_get(const struct managed_object_hashmap_base *hb, const void *key);
void *managed_object_hashmap_base_remove(struct managed_object_hashmap_base *hb, const void *key);

void managed_object_hashmap_base_clear(struct managed_object_hashmap_base *hb);
int managed_object_hashmap_base_reset(struct managed_object_hashmap_base *hb, void * user_data);

struct managed_object_hashmap_entry *managed_object_hashmap_base_iter(const struct managed_object_hashmap_base *hb,
        const struct managed_object_hashmap_entry *pos);
bool managed_object_hashmap_base_iter_valid(const struct managed_object_hashmap_base *hb, const struct managed_object_hashmap_entry *iter);
bool managed_object_hashmap_base_iter_next(const struct managed_object_hashmap_base *hb, struct managed_object_hashmap_entry **iter);
bool managed_object_hashmap_base_iter_remove(struct managed_object_hashmap_base *hb, struct managed_object_hashmap_entry **iter);
const void *managed_object_hashmap_base_iter_get_key(const struct managed_object_hashmap_entry *iter);
void *managed_object_hashmap_base_iter_get_data(const struct managed_object_hashmap_entry *iter);
int managed_object_hashmap_base_iter_set_data(struct managed_object_hashmap_entry *iter, void *data);

double managed_object_hashmap_base_load_factor(const struct managed_object_hashmap_base *hb);
size_t managed_object_hashmap_base_collisions(const struct managed_object_hashmap_base *hb, const void *key);
double managed_object_hashmap_base_collisions_mean(const struct managed_object_hashmap_base *hb);
double managed_object_hashmap_base_collisions_variance(const struct managed_object_hashmap_base *hb);

size_t managed_object_hashmap_hash_default(const void *data, size_t len);
size_t managed_object_hashmap_hash_string(const char *key);
size_t managed_object_hashmap_hash_string_i(const char *key);

#endif