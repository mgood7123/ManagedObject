#ifndef MANAGED_OBJECT____OBJ_METADATA_H
#define MANAGED_OBJECT____OBJ_METADATA_H

#include <mpscawl.h>

// metadata is required for dynamic pointers

/*

C++ objects are currently created via

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_scanned_with_finalizer(
        &state, managed_object_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_object_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, memory[1])->a = memory[0];

additionally they cannot be modified at runtime (eg, add fields, remove fields, look up methods, look up fields, ect)

the same applies to C, both structs and classes cannot be created at runtime, only instantiated

this attempts to remedy this

*/

// subsystem - metadata/reflection
//
//   we define metadata as both object metadata and reflection apis
//     both go hand in hand one, and one cannot be used without the other
//
// the GC itself has limited knowledge of metadata
//

// we use a moving GC to manage the metadata
//

typedef struct managed_obj_global_metadata_ht_keys_s {
    void * deleted;
    void * unused;
} managed_obj_global_metadata_ht_keys_s;

typedef struct managed_obj_metadata_buckets_s {
  managed_obj_type_t type;                           /* MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET */
  mps_bool_t pinned;
  struct managed_obj_metadata_buckets_s *dependent;  /* the dependent object */
  size_t length;                                     /* number of buckets (tagged) */
  size_t used;                                       /* number of buckets in use (tagged) */
  size_t deleted;                                    /* number of deleted buckets (tagged) */
  managed_obj_t bucket[1];                           /* hash buckets */
} managed_obj_metadata_buckets_s, *managed_obj_metadata_buckets_t;

typedef unsigned long (*managed_obj_metadata_hash_t)(ManagedObjState * state, managed_obj_t obj, mps_ld_t ld);
typedef int (*managed_obj_metadata_cmp_t)(managed_obj_t obj1, managed_obj_t obj2);

/* %%MPS: The hash table is address-based, and so depends on the
 * location of its keys: when the garbage collector moves the keys,
 * the table needs to be re-hashed. The 'ld' structure is used to
 * detect this. See topic/location. */
typedef struct managed_obj_metadata_hashtable_s {
  managed_obj_type_t type;                          /* MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE */
  mps_bool_t pinned;
  managed_obj_metadata_hash_t hash;                 /* hash function */
  managed_obj_metadata_cmp_t cmp;                   /* comparison function */
  mps_ld_s ld;                                      /* location dependency */
  mps_ap_t key_ap, value_ap;                        /* allocation points for keys and values */
  managed_obj_metadata_buckets_t keys, values;      /* hash buckets for keys and values */
} managed_obj_metadata_hashtable_s;

typedef struct managed_obj_metadata_string_s {
  managed_obj_type_t type;      /* MANAGED_OBJECT_TYPE_METADATA_STRING */
  mps_bool_t pinned;
  size_t length;                /* number of chars in string */
  char string[1];               /* string, NUL terminated */
} managed_obj_metadata_string_s;


// metadata can be created and destroyed as part of the reflection system

// what do we need?

// a class object has fields
// a class object has methods
// a class object has virtual methods

enum {
    MANAGED_OBJECT_METADATA_PRIMATIVE_INTEGER_8,
    MANAGED_OBJECT_METADATA_PRIMATIVE_INTEGER_16,
    MANAGED_OBJECT_METADATA_PRIMATIVE_INTEGER_32,
    MANAGED_OBJECT_METADATA_PRIMATIVE_INTEGER_64,
    MANAGED_OBJECT_METADATA_PRIMATIVE_FLOAT_32,
    MANAGED_OBJECT_METADATA_PRIMATIVE_FLOAT_64,
    MANAGED_OBJECT_METADATA_PRIMATIVE_POINTER
};

// one metadata object per class definition

// fields and methods may be added/removed, we need a way of storing and these up
//
// we use a hash table object

typedef struct managed_obj_metadata_s {
  managed_obj_type_t type;                                    /* MANAGED_OBJECT_TYPE_METADATA */
  mps_bool_t pinned;
  void * table;
} managed_obj_metadata_s;

// metadata GC info, per metadata object

typedef struct managed_obj_metadata_field_s {
  managed_obj_type_t type;                                    /* MANAGED_OBJECT_TYPE_METADATA_FIELD_POINTER */
  mps_bool_t pinned;
  managed_obj_type_t field_return_type;                       /* field return type */
} managed_obj_metadata_field_s;

typedef struct managed_obj_metadata_method_s {
  managed_obj_type_t type;                                    /* MANAGED_OBJECT_TYPE_METADATA_METHOD_POINTER */
  mps_bool_t pinned;
  managed_obj_type_t method_return_type;                      /* method return type */
  size_t parameter_count;                                     /* parameter count */
  managed_obj_type_t parameters[1];                           /* method parameters */
} managed_obj_metadata_method_s;

#define MANAGED_OBJECT_METADATA_ENUMS               \
    MANAGED_OBJECT_TYPE_METADATA,                   \
    MANAGED_OBJECT_TYPE_METADATA_FIELD_POINTER,     \
    MANAGED_OBJECT_TYPE_METADATA_METHOD_POINTER,    \
    MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE,        \
    MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET, \
    MANAGED_OBJECT_TYPE_METADATA_STRING

#define MANAGED_OBJECT_METADATA_UNIONS \
    managed_obj_metadata_s metadata; \
    managed_obj_metadata_field_s metadata_field; \
    managed_obj_metadata_method_s metadata_method; \
    managed_obj_metadata_buckets_s metadata_bucket; \
    managed_obj_metadata_hashtable_s metadata_hashtable; \
    managed_obj_metadata_string_s metadata_string; \

/* %%MPS: Objects in AWL pools must be formatted so that aligned
 * pointers (with bottom bit(s) zero) can be distinguished from other
 * data types (with bottom bit(s) non-zero). Here we use a bottom
 * bit of 1 for integers. See pool/awl. */
#define MANAGED_OBJECT_METADATA_TAG_COUNT(i) (((i) << 1) + 1)
#define MANAGED_OBJECT_METADATA_UNTAG_COUNT(i) ((i) >> 1)

#define MANAGED_OBJECT_METADATA_SOURCE_PREINIT \
    if (state == &managed_obj_global_metadata) { \
        if (managed_obj_global_metadata_init == 2) { \
            return; \
        } \
        if (managed_obj_global_metadata_init == 1) { \
            managed_obj_global_metadata_init = 2; \
        } \
    } \

#define MANAGED_OBJECT_METADATA_SOURCE_INIT \
  LockClaimGlobalRecursive(); \
    if (managed_obj_global_metadata_init == 0) { \
        managed_obj_global_metadata_init = 1; \
        managed_obj_init_with_user_memory(&managed_obj_global_metadata, 4096*2, &managed_obj_global_metadata_ids, sizeof(managed_obj_global_metadata_ht_keys_s)); \
        managed_obj_global_metadata_ids.unused = managed_obj_make_empty(&managed_obj_global_metadata); \
        managed_obj_global_metadata_ids.deleted = managed_obj_make_empty(&managed_obj_global_metadata); \
    } \
  LockReleaseGlobalRecursive(); \
\
  /* Create an Automatic Weak Linked (AWL) pool to manage the hash table \
     buckets. */ \
  MPS_ARGS_BEGIN(args) { \
    MPS_ARGS_ADD(args, MPS_KEY_FORMAT, state->fmt); \
    MPS_ARGS_ADD(args, MPS_KEY_AWL_FIND_DEPENDENT, managed_obj_metadata_buckets_find_dependent); \
    res = mps_pool_create_k(&state->buckets_pool, state->arena, mps_class_awl(), args); \
  } MPS_ARGS_END(args); \
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create buckets pool"); \
\
  /* Create allocation points for weak and strong buckets. */ \
  MPS_ARGS_BEGIN(args) { \
    MPS_ARGS_ADD(args, MPS_KEY_RANK, mps_rank_exact()); \
    res = mps_ap_create_k(&state->strong_buckets_ap, state->buckets_pool, args); \
  } MPS_ARGS_END(args); \
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create strong buckets allocation point"); \
  MPS_ARGS_BEGIN(args) { \
    MPS_ARGS_ADD(args, MPS_KEY_RANK, mps_rank_weak()); \
    res = mps_ap_create_k(&state->weak_buckets_ap, state->buckets_pool, args); \
  } MPS_ARGS_END(args); \
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create weak buckets allocation point"); \

#define MANAGED_OBJECT_METADATA_SOURCE_DEINIT \
  LockClaimGlobalRecursive(); \
    if (managed_obj_global_metadata_init == 2) { \
        managed_obj_global_metadata_init = 0; \
        managed_obj_global_metadata_ids.unused = NULL; \
        managed_obj_global_metadata_ids.deleted = NULL; \
        managed_obj_deinit(&managed_obj_global_metadata); \
    } \
  LockReleaseGlobalRecursive(); \
\
  mps_ap_destroy(state->weak_buckets_ap);         /* destroy ap before pool */ \
  mps_ap_destroy(state->strong_buckets_ap);       /* destroy ap before pool */ \
  mps_pool_destroy(state->buckets_pool);          /* destroy pool */ \

#define MANAGED_OBJECT_METADATA_SCAN \
      case MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET: { \
        size_t i, length = MANAGED_OBJECT_METADATA_UNTAG_COUNT(obj->metadata_bucket.length); \
        if(obj->metadata_bucket.dependent != NULL) { \
            MANAGED_OBJECT_FIX(obj->metadata_bucket.dependent); \
            AVER(obj->metadata_bucket.dependent->length == obj->metadata_bucket.length); \
        } \
        for (i = 0; i < length; ++i) { \
            mps_addr_t p = obj->metadata_bucket.bucket[i]; \
            if (MPS_FIX1(ss, p)) { \
              mps_res_t res = MPS_FIX2(ss, &p); \
              if (res != MPS_RES_OK) return res; \
              if (p == NULL) { \
                /* key/value was splatted: splat value/key too */ \
                p = managed_obj_global_metadata_ids.deleted; \
                obj->metadata_bucket.deleted = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(obj->metadata_bucket.deleted) + 1); \
                if (obj->metadata_bucket.dependent != NULL) { \
                    obj->metadata_bucket.dependent->bucket[i] = p; \
                    obj->metadata_bucket.dependent->deleted \
                        = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(obj->metadata_bucket.dependent->deleted) + 1); \
                } \
              } \
              if (obj->metadata_bucket.bucket[i] != p) { \
                obj->metadata_bucket.bucket[i] = p; \
              } \
            } \
        } \
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_buckets_s, bucket) + \
                                        length * sizeof(obj->metadata_bucket.bucket[0])); \
        break; \
      } \
      case MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE: { \
        size_t i, length; \
        MANAGED_OBJECT_FIX(obj->metadata_hashtable.keys); \
        MANAGED_OBJECT_FIX(obj->metadata_hashtable.values); \
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_metadata_hashtable_s)); \
        break; } \
      case MANAGED_OBJECT_TYPE_METADATA_STRING: \
        base = (char *)base + \
          MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_string_s, string) + obj->metadata_string.length + 1); \
        break; \

#define MANAGED_OBJECT_METADATA_SKIP \
      case MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET: \
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_buckets_s, bucket) \
            + MANAGED_OBJECT_METADATA_UNTAG_COUNT(obj->metadata_bucket.length) \
            * sizeof(obj->metadata_bucket.bucket[0])); \
        break; \
      case MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE: \
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_metadata_hashtable_s)); \
        break; \
      case MANAGED_OBJECT_TYPE_METADATA_STRING: \
        base = (char *)base \
            + MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_string_s, string) + obj->metadata_string.length + 1); \
        break; \

#define MANAGED_OBJECT_METADATA_FINALIZE \

#define MANAGED_OBJECT_METADATA_STATE \
    mps_pool_t buckets_pool; /* pool for hash table buckets */ \
    mps_ap_t strong_buckets_ap; /* allocation point for strong buckets */ \
    mps_ap_t weak_buckets_ap; /* allocation point for weak buckets */ \

#define MANAGED_OBJECT_METADATA_HEADER_FUNCTIONS \
extern int managed_obj_global_metadata_init; \
extern ManagedObjState managed_obj_global_metadata; \
extern managed_obj_global_metadata_ht_keys_s managed_obj_global_metadata_ids; /* hashtable identity keys */ \
extern mps_pool_t managed_obj_global_metadata_buckets_pool; /* pool for hash table buckets */ \
extern mps_ap_t managed_obj_global_metadata_strong_buckets_ap; /* allocation point for strong buckets */ \
extern mps_ap_t managed_obj_global_metadata_weak_buckets_ap; /* allocation point for weak buckets */ \
\
managed_obj_t managed_obj_metadata_make_string(ManagedObjState * state, size_t length, const char *string); \
\
/* buckets_find_dependent -- find dependent object for buckets       %%MPS \
 * \
 * Each object in an AWL pool can have a "dependent object". The MPS \
 * ensures that when an object is being scanned, its dependent object \
 * is unprotected. This allows prompt deletion of values in a weak-key \
 * hash table, and keys in a weak-value hash table. \
 */ \
\
mps_addr_t managed_obj_metadata_buckets_find_dependent(mps_addr_t addr); \
managed_obj_t managed_obj_metadata_make_string(ManagedObjState * state, size_t length, const char *string); \
managed_obj_t managed_obj_metadata_make_buckets(ManagedObjState * state, size_t length, mps_ap_t ap); \
managed_obj_t managed_obj_metadata_make_table(ManagedObjState * state, size_t length, managed_obj_metadata_hash_t hashf, managed_obj_metadata_cmp_t cmpf, int weak_key, int weak_value); \
\
/* hash -- hash a string to an unsigned long \
 * \
 * This hash function was derived (with permission) from \
 * Paul Haahr's hash in the most excellent rc 1.4. \
 */ \
\
unsigned long managed_obj_metadata_hash(const char *s, size_t length); \
\
/* %%MPS: When taking the hash of an address, we record the dependency \
 * on its location by calling mps_ld_add. See topic/location. \
 */ \
unsigned long managed_obj_metadata_eq_hash(ManagedObjState * state, managed_obj_t obj, mps_ld_t ld); \
int managed_obj_metadata_eqp(managed_obj_t obj1, managed_obj_t obj2); \
unsigned long managed_obj_metadata_string_hash(ManagedObjState * state, managed_obj_t obj, mps_ld_t ld); \
int managed_obj_metadata_string_equalp(managed_obj_t obj1, managed_obj_t obj2); \
int managed_obj_metadata_buckets_find(ManagedObjState * state, managed_obj_t tbl, managed_obj_metadata_buckets_t buckets, managed_obj_t key, int add, size_t *b); \
size_t managed_obj_metadata_table_size(managed_obj_t tbl); \
\
/* Rehash 'tbl' so that it has 'new_length' buckets. If 'key' is found \
 * during this process, update 'key_bucket' to be the index of the \
 * bucket containing 'key' and return true, otherwise return false. \
 * \
 * %%MPS: When re-hashing the table we reset the associated location \
 * dependency and re-add a dependency on each object in the table. \
 * This is because the table gets re-hashed when the locations of \
 * objects have changed. See topic/location. \
 */ \
int managed_obj_metadata_table_rehash(ManagedObjState * state, managed_obj_t tbl, size_t new_length, managed_obj_t key, size_t *key_bucket); \
\
/* %%MPS: If we fail to find 'key' in the table, and if mps_ld_isstale \
 * returns true, then some of the keys in the table might have been \
 * moved by the garbage collector: in this case we need to re-hash the \
 * table. See topic/location. \
 */ \
int managed_obj_metadata_table_find(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, int add, size_t *b); \
managed_obj_t managed_obj_metadata_table_ref(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key); \
int managed_obj_metadata_table_try_set(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, managed_obj_t value); \
int managed_obj_metadata_table_full(managed_obj_t tbl); \
void managed_obj_metadata_table_set(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, managed_obj_t value); \
void managed_obj_metadata_table_delete(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key); \
managed_obj_t managed_obj_metadata_intern_string(ManagedObjState * state, managed_obj_t tbl, managed_obj_t name); \
managed_obj_t managed_obj_metadata_intern(ManagedObjState * state, managed_obj_t tbl, const char *string); \

#define MANAGED_OBJECT_METADATA_SOURCE_FUNCTIONS \
int managed_obj_global_metadata_init = 0; \
ManagedObjState managed_obj_global_metadata; \
managed_obj_global_metadata_ht_keys_s managed_obj_global_metadata_ids; /* hashtable identity keys */ \
mps_pool_t managed_obj_global_metadata_buckets_pool = NULL; /* pool for hash table buckets */ \
mps_ap_t managed_obj_global_metadata_strong_buckets_ap = NULL; /* allocation point for strong buckets */ \
mps_ap_t managed_obj_global_metadata_weak_buckets_ap = NULL; /* allocation point for weak buckets */ \
\
mps_addr_t managed_obj_metadata_buckets_find_dependent(mps_addr_t addr) \
{ \
  return MANAGED_OBJECT_TYPE(((managed_obj_t)addr)) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET ? ((managed_obj_t)addr)->metadata_bucket.dependent : NULL; \
} \
\
managed_obj_t managed_obj_metadata_make_string(ManagedObjState * state, size_t length, const char *string) \
{ \
  managed_obj_t obj; \
  size_t size = MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_string_s, string) + length+1); \
  while(1) { \
    mps_res_t res = mps_reserve((mps_addr_t*)&obj, state->ap, size); \
    if (res != MPS_RES_OK) managed_obj_error("out of memory in make_string"); \
    obj->metadata_string.type = MANAGED_OBJECT_TYPE_METADATA_STRING; \
    obj->metadata_string.pinned = FALSE; \
    obj->metadata_string.length = length; \
    if (string) memcpy(obj->metadata_string.string, string, length+1); \
    else managed_obj_volatile_memset(obj->metadata_string.string, 0, length+1); \
    if (mps_commit(state->ap, obj, size)) { \
      break; \
    } \
  } \
  state->allocated_obj_bytes += sizeof(managed_obj_metadata_string_s); \
  state->allocated_aligned_obj_bytes += size; \
\
  mps_finalize(state->arena, (mps_addr_t*)&obj); \
\
  return obj; \
} \
\
managed_obj_t managed_obj_metadata_make_buckets(ManagedObjState * state, size_t length, mps_ap_t ap) \
{ \
  managed_obj_t obj; \
  size_t size = MANAGED_OBJECT_ALIGN_OBJ(offsetof(managed_obj_metadata_buckets_s, bucket) + (length * sizeof(obj->metadata_bucket.bucket[0]))); \
  while(1) { \
    mps_res_t res = mps_reserve((mps_addr_t*)&obj, ap, size); \
    size_t i; \
    if (res != MPS_RES_OK) managed_obj_error("out of memory in make_buckets"); \
    obj->metadata_bucket.type = MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE_BUCKET; \
    obj->metadata_bucket.pinned = FALSE; \
    obj->metadata_bucket.dependent = NULL; \
    obj->metadata_bucket.length = MANAGED_OBJECT_METADATA_TAG_COUNT(length); \
    obj->metadata_bucket.used = MANAGED_OBJECT_METADATA_TAG_COUNT(0); \
    obj->metadata_bucket.deleted = MANAGED_OBJECT_METADATA_TAG_COUNT(0); \
    for(i = 0; i < length; ++i) { \
      obj->metadata_bucket.bucket[i] = managed_obj_global_metadata_ids.unused; \
    } \
    if (mps_commit(ap, obj, size)) { \
      break; \
    } \
  } \
  state->allocated_obj_bytes += sizeof(managed_obj_metadata_buckets_s); \
  state->allocated_aligned_obj_bytes += size; \
\
  mps_finalize(state->arena, (mps_addr_t*)&obj); \
\
  return obj; \
} \
\
managed_obj_t managed_obj_metadata_make_table(ManagedObjState * state, size_t length, managed_obj_metadata_hash_t hashf, managed_obj_metadata_cmp_t cmpf, int weak_key, int weak_value) \
{ \
  managed_obj_t obj; \
  size_t l, size = MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_metadata_hashtable_s)); \
  while(1) { \
    mps_res_t res = mps_reserve((mps_addr_t*)&obj, state->ap, size); \
    if (res != MPS_RES_OK) managed_obj_error("out of memory in make_table"); \
    obj->metadata_hashtable.type = MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE; \
    obj->metadata_hashtable.pinned = FALSE; \
    obj->metadata_hashtable.keys = obj->metadata_hashtable.values = NULL; \
    if (mps_commit(state->ap, obj, size)) { \
      obj->metadata_hashtable.hash = hashf; \
      obj->metadata_hashtable.cmp = cmpf; \
      /* round up to next power of 2 */ \
      for(l = 1; l < length; l *= 2); \
      obj->metadata_hashtable.key_ap = weak_key ? state->weak_buckets_ap : state->strong_buckets_ap; \
      obj->metadata_hashtable.value_ap = weak_value ? state->weak_buckets_ap : state->strong_buckets_ap; \
      obj->metadata_hashtable.keys = (managed_obj_metadata_buckets_t)managed_obj_metadata_make_buckets(state, l, obj->metadata_hashtable.key_ap); \
      obj->metadata_hashtable.values = (managed_obj_metadata_buckets_t)managed_obj_metadata_make_buckets(state, l, obj->metadata_hashtable.value_ap); \
      obj->metadata_hashtable.keys->dependent = obj->metadata_hashtable.values; \
      obj->metadata_hashtable.values->dependent = obj->metadata_hashtable.keys; \
      mps_ld_reset(&obj->metadata_hashtable.ld, state->arena); \
      break; \
    } \
  } \
  state->allocated_obj_bytes += sizeof(managed_obj_metadata_hashtable_s); \
  state->allocated_aligned_obj_bytes += size; \
\
  mps_finalize(state->arena, (mps_addr_t*)&obj); \
\
  return obj; \
} \
\
/* hash -- hash a string to an unsigned long \
 * \
 * This hash function was derived (with permission) from \
 * Paul Haahr's hash in the most excellent rc 1.4. \
 */ \
\
unsigned long managed_obj_metadata_hash(const char *s, size_t length) { \
  unsigned long c, h=0; \
  size_t i = 0; \
  switch(length % 4) { \
    do { \
      c=(unsigned long)s[i++]; h+=(c<<17)^(c<<11)^(c<<5)^(c>>1); \
    case 3: \
      c=(unsigned long)s[i++]; h^=(c<<14)+(c<<7)+(c<<4)+c; \
    case 2: \
      c=(unsigned long)s[i++]; h^=(~c<<11)|((c<<3)^(c>>1)); \
    case 1: \
      c=(unsigned long)s[i++]; h-=(c<<16)|(c<<9)|(c<<2)|(c&3); \
    case 0: \
      ; \
    } while(i < length); \
  } \
  return h; \
} \
\
/* Hash table implementation */ \
\
/* %%MPS: When taking the hash of an address, we record the dependency \
 * on its location by calling mps_ld_add. See topic/location. \
 */ \
unsigned long managed_obj_metadata_eq_hash(ManagedObjState * state, managed_obj_t obj, mps_ld_t ld) \
{ \
  union {char s[sizeof(managed_obj_t)]; managed_obj_t addr;} u; \
  if (ld) mps_ld_add(ld, state->arena, obj); \
  u.addr = obj; \
  return managed_obj_metadata_hash(u.s, sizeof(managed_obj_t)); \
} \
\
int managed_obj_metadata_eqp(managed_obj_t obj1, managed_obj_t obj2) \
{ \
  return obj1 == obj2; \
} \
\
unsigned long managed_obj_metadata_string_hash(ManagedObjState * state, managed_obj_t obj, mps_ld_t ld) \
{ \
  UNUSED(state); \
  UNUSED(ld); \
    printf("hashing: hashing object %p\n", obj); \
  if(MANAGED_OBJECT_TYPE(obj) != MANAGED_OBJECT_TYPE_METADATA_STRING) \
    managed_obj_error("string-hash: argument must be a string"); \
  return managed_obj_metadata_hash(obj->metadata_string.string, obj->metadata_string.length); \
} \
\
int managed_obj_metadata_string_equalp(managed_obj_t obj1, managed_obj_t obj2) \
{ \
  return obj1 == obj2 || \
         (MANAGED_OBJECT_TYPE(obj1) == MANAGED_OBJECT_TYPE_METADATA_STRING && \
          MANAGED_OBJECT_TYPE(obj2) == MANAGED_OBJECT_TYPE_METADATA_STRING && \
          obj1->metadata_string.length == obj2->metadata_string.length && \
          0 == strcmp(obj1->metadata_string.string, obj2->metadata_string.string)); \
} \
\
int managed_obj_metadata_buckets_find(ManagedObjState * state, managed_obj_t tbl, managed_obj_metadata_buckets_t buckets, managed_obj_t key, int add, size_t *b) \
{ \
  unsigned long i, h, probe; \
  unsigned long l = MANAGED_OBJECT_METADATA_UNTAG_COUNT(buckets->length) - 1; \
  int result = 0; \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  h = tbl->metadata_hashtable.hash(state, key, add ? &tbl->metadata_hashtable.ld : NULL); \
  probe = (h >> 8) | 1; \
  h &= l; \
  i = h; \
  do { \
    managed_obj_t k = buckets->bucket[i]; \
    printf("find bucket: comparing key %p against wanted key %p\n", k, key); \
    if(k == managed_obj_global_metadata_ids.unused || tbl->metadata_hashtable.cmp(k, key)) { \
      *b = i; \
      return 1; \
    } \
    if(result == 0 && k == managed_obj_global_metadata_ids.deleted) { \
      *b = i; \
      result = 1; \
    } \
    i = (i+probe) & l; \
  } while(i != h); \
  return result; \
} \
\
size_t managed_obj_metadata_table_size(managed_obj_t tbl) \
{ \
  size_t used, deleted; \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  used = MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->used); \
  deleted = MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->deleted); \
  AVER(used >= deleted); \
  return used - deleted; \
} \
\
/* Rehash 'tbl' so that it has 'new_length' buckets. If 'key' is found \
 * during this process, update 'key_bucket' to be the index of the \
 * bucket containing 'key' and return true, otherwise return false. \
 * \
 * %%MPS: When re-hashing the table we reset the associated location \
 * dependency and re-add a dependency on each object in the table. \
 * This is because the table gets re-hashed when the locations of \
 * objects have changed. See topic/location. \
 */ \
int managed_obj_metadata_table_rehash(ManagedObjState * state, managed_obj_t tbl, size_t new_length, managed_obj_t key, size_t *key_bucket) \
{ \
    printf("rehash keys\n"); \
  size_t i, length; \
  managed_obj_metadata_buckets_t new_keys, new_values; \
  int result = 0; \
\
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  length = MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->length); \
  new_keys = (managed_obj_metadata_buckets_t)managed_obj_metadata_make_buckets(state, new_length, tbl->metadata_hashtable.key_ap); \
  new_values = (managed_obj_metadata_buckets_t)managed_obj_metadata_make_buckets(state, new_length, tbl->metadata_hashtable.value_ap); \
  new_keys->dependent = new_values; \
  new_values->dependent = new_keys; \
  mps_ld_reset(&tbl->metadata_hashtable.ld, state->arena); \
\
  for (i = 0; i < length; ++i) { \
    managed_obj_t old_key = tbl->metadata_hashtable.keys->bucket[i]; \
    if (old_key != managed_obj_global_metadata_ids.unused && old_key != managed_obj_global_metadata_ids.deleted) { \
      int found; \
      size_t b; \
      found = managed_obj_metadata_buckets_find(state, tbl, new_keys, old_key, 1, &b); \
      AVER(found);            /* new table shouldn't be full */ \
      AVER(new_keys->bucket[b] == managed_obj_global_metadata_ids.unused); /* shouldn't be in new table */ \
      new_keys->bucket[b] = old_key; \
      new_values->bucket[b] = tbl->metadata_hashtable.values->bucket[i]; \
      if (key != NULL && tbl->metadata_hashtable.cmp(old_key, key)) { \
        *key_bucket = b; \
        result = 1; \
      } \
      new_keys->used = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(new_keys->used) + 1); \
    } \
  } \
\
  AVER(MANAGED_OBJECT_METADATA_UNTAG_COUNT(new_keys->used) == managed_obj_metadata_table_size(tbl)); \
  tbl->metadata_hashtable.keys = new_keys; \
  tbl->metadata_hashtable.values = new_values; \
    printf("rehashed keys\n"); \
  return result; \
} \
\
/* %%MPS: If we fail to find 'key' in the table, and if mps_ld_isstale \
 * returns true, then some of the keys in the table might have been \
 * moved by the garbage collector: in this case we need to re-hash the \
 * table. See topic/location. \
 */ \
int managed_obj_metadata_table_find(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, int add, size_t *b) \
{ \
    printf("find table: finding wanted key %p\n", key); \
  if (!managed_obj_metadata_buckets_find(state, tbl, tbl->metadata_hashtable.keys, key, add, b)) { \
    return 0; \
  } else if ((tbl->metadata_hashtable.keys->bucket[*b] == managed_obj_global_metadata_ids.unused \
              || tbl->metadata_hashtable.keys->bucket[*b] == managed_obj_global_metadata_ids.deleted) \
             && mps_ld_isstale(&tbl->metadata_hashtable.ld, state->arena, key)) { \
    return managed_obj_metadata_table_rehash(state, tbl, MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->length), key, b); \
  } else { \
    return 1; \
  } \
} \
\
managed_obj_t managed_obj_metadata_table_ref(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key) \
{ \
  size_t b; \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  if (managed_obj_metadata_table_find(state, tbl, key, 0, &b)) { \
    managed_obj_t k = tbl->metadata_hashtable.keys->bucket[b]; \
    if (k != managed_obj_global_metadata_ids.unused && k != managed_obj_global_metadata_ids.deleted) \
      return tbl->metadata_hashtable.values->bucket[b]; \
  } \
  return NULL; \
} \
\
int managed_obj_metadata_table_try_set(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, managed_obj_t value) \
{ \
  size_t b; \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  if (!managed_obj_metadata_table_find(state, tbl, key, 1, &b)) \
    return 0; \
  if (tbl->metadata_hashtable.keys->bucket[b] == managed_obj_global_metadata_ids.unused) { \
    tbl->metadata_hashtable.keys->bucket[b] = key; \
    tbl->metadata_hashtable.keys->used = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->used) + 1); \
  } else if (tbl->metadata_hashtable.keys->bucket[b] == managed_obj_global_metadata_ids.deleted) { \
    tbl->metadata_hashtable.keys->bucket[b] = key; \
    AVER(tbl->metadata_hashtable.keys->deleted > MANAGED_OBJECT_METADATA_TAG_COUNT(0)); \
    tbl->metadata_hashtable.keys->deleted \
      = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->deleted) - 1); \
  } \
  tbl->metadata_hashtable.values->bucket[b] = value; \
  return 1; \
} \
\
int managed_obj_metadata_table_full(managed_obj_t tbl) \
{ \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  return tbl->metadata_hashtable.keys->used >= tbl->metadata_hashtable.keys->length / 2; \
} \
\
void managed_obj_metadata_table_set(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key, managed_obj_t value) \
{ \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  if (managed_obj_metadata_table_full(tbl) || !managed_obj_metadata_table_try_set(state, tbl, key, value)) { \
    int res; \
    managed_obj_metadata_table_rehash(state, tbl, MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->length) * 2, NULL, NULL); \
    res = managed_obj_metadata_table_try_set(state, tbl, key, value); \
    AVER(res);                /* rehash should have made room */ \
  } \
} \
\
void managed_obj_metadata_table_delete(ManagedObjState * state, managed_obj_t tbl, managed_obj_t key) \
{ \
  size_t b; \
  AVER(MANAGED_OBJECT_TYPE(tbl) == MANAGED_OBJECT_TYPE_METADATA_HASH_TABLE); \
  if(managed_obj_metadata_table_find(state, tbl, key, 0, &b) \
     && tbl->metadata_hashtable.keys->bucket[b] != managed_obj_global_metadata_ids.unused \
     && tbl->metadata_hashtable.keys->bucket[b] != managed_obj_global_metadata_ids.deleted) \
  { \
    tbl->metadata_hashtable.keys->bucket[b] = managed_obj_global_metadata_ids.deleted; \
    tbl->metadata_hashtable.keys->deleted \
      = MANAGED_OBJECT_METADATA_TAG_COUNT(MANAGED_OBJECT_METADATA_UNTAG_COUNT(tbl->metadata_hashtable.keys->deleted) + 1); \
    tbl->metadata_hashtable.values->bucket[b] = NULL; \
  } \
} \
\
managed_obj_t managed_obj_metadata_intern_string(ManagedObjState * state, managed_obj_t tbl, managed_obj_t name) \
{ \
  AVER(MANAGED_OBJECT_TYPE(name) == MANAGED_OBJECT_TYPE_METADATA_STRING); \
  managed_obj_t symbol; \
  symbol = managed_obj_metadata_table_ref(state, tbl, name); \
  if(symbol == NULL) { \
    symbol = name; \
    managed_obj_metadata_table_set(state, tbl, name, symbol); \
  } \
  return symbol; \
} \
\
managed_obj_t managed_obj_metadata_intern(ManagedObjState * state, managed_obj_t tbl, const char *string) \
{ \
  return managed_obj_metadata_intern_string(state, tbl, managed_obj_metadata_make_string(state, strlen(string), string)); \
} \

#endif