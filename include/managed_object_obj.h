#ifndef MANAGED_OBJECT____OBJ_H
#define MANAGED_OBJECT____OBJ_H

#ifdef __cplusplus
extern "C" {
#endif

#include <mps.h>
#include <mpsacl.h>
#include <mpsavm.h>
#include <mpscamc.h>
#include <mpslib.h>

// the compiler is not allowed to optimize out functions that use volatile pointers
static inline void managed_obj_volatile_memset(void * ptr, char value, size_t length_in_bytes) {
  volatile char* s = (char*)(ptr);
  volatile char* e = s + length_in_bytes;
#ifdef __cplusplus
  std::fill(s, e, 0);
#else
  for(;s!=e;s++) *s = value;
#endif
}

typedef union managed_obj_u *managed_obj_t;

typedef int managed_obj_type_t;

struct ManagedObjState;
typedef struct ManagedObjState ManagedObjState;

#include "managed_object_obj_metadata.h"

// #ifndef MANAGED_OBJECT____OBJ_METADATA_H
// #define MANAGED_OBJECT_METADATA_ENUMS MANAGED_OBJECT_METADATA_ENUMS
// #define MANAGED_OBJECT_METADATA_UNIONS
// #define MANAGED_OBJECT_METADATA_FINALIZE
// #define MANAGED_OBJECT_METADATA_HEADER_FUNCTIONS
// #define MANAGED_OBJECT_METADATA_SOURCE_FUNCTIONS
// #define MANAGED_OBJECT_METADATA_SCAN
// #define MANAGED_OBJECT_METADATA_SKIP
// #define MANAGED_OBJECT_METADATA_SOURCE_PREINIT
// #define MANAGED_OBJECT_METADATA_SOURCE_INIT
// #define MANAGED_OBJECT_METADATA_SOURCE_DEINIT
// #define MANAGED_OBJECT_METADATA_STATE
// #endif

struct ManagedObjState {
    /* MPS globals                                                  %%MPS
    *
    * These are global variables holding MPS values for use by the
    * interpreter.  In a more sophisticated integration some of these might
    * be thread local.  See `main` for where these are set up.
    *
    * `arena` is the global state of the MPS, and there's usually only one
    * per process. See topic/arena.
    *
    * `pool` is the memory pool in which the Scheme objects are allocated.
    * It is an instance of the Automatic Mostly Copying (AMC) pool class, which
    * is a general-purpose garbage collector for use when there are formatted
    * objects in the pool, but ambiguous references in thread stacks and
    * registers. See pool/amc.
    *
    * `ap` is an Allocation Point that allows fast in-line non-locking
    * allocation in a memory pool.  This would usually be thread-local, but
    * this interpreter is single-threaded.  See `make_pair` etc. for how this
    * is used with the reserve/commit protocol.
    */

    mps_arena_t arena;       /* the arena */
    mps_pool_t pool;     /* pool for ordinary Scheme objects */
    mps_ap_t ap;         /* allocation point used to allocate objects */
    mps_chain_t chain;
    mps_fmt_t fmt;
    mps_thr_t thread;
    mps_root_t thread_stack_root;

    // object pinning support
    // an array is used to keep track of pinned objects
    managed_obj_t* pinned;
    size_t pinned_used;
    size_t pinned_capacity;
    mps_root_t pinned_root;

    MANAGED_OBJECT_METADATA_STATE
    
    size_t allocated_bytes;
    size_t freed_bytes;
    size_t allocated_obj_bytes;
    size_t allocated_aligned_obj_bytes;
    size_t freed_obj_bytes;
    size_t freed_aligned_obj_bytes;
};


/* DATA TYPES */

/* managed_obj_t -- scheme object type
 *
 * managed_obj_t is a pointer to a union, managed_obj_u, which has members for
 * each scheme representation.
 *
 * The managed_obj_u also has a "type" member.  Each representation
 * structure also has a "type" field first.  ANSI C guarantees
 * that these type fields correspond [section?].
 *
 * Objects are allocated by allocating one of the representation
 * structures and casting the pointer to it to type managed_obj_t.  This
 * allows objects of different sizes to be represented by the
 * same type.
 *
 * To access an object, check its type by reading MANAGED_OBJECT_TYPE(obj), then
 * access the fields of the representation, e.g.
 *   if(MANAGED_OBJECT_TYPE(obj) == MANAGED_OBJECT_TYPE_PAIR) fiddle_with(CAR(obj));
 */

enum {
  MANAGED_OBJECT_TYPE_EMPTY,
  MANAGED_OBJECT_TYPE_SCANNED_POINTER,
  MANAGED_OBJECT_METADATA_ENUMS,
  MANAGED_OBJECT_TYPE_FWD2,            /* two-word forwarding object */
  MANAGED_OBJECT_TYPE_FWD,             /* three words and up forwarding object */
  MANAGED_OBJECT_TYPE_PAD1,            /* one-word padding object */
  MANAGED_OBJECT_TYPE_PAD              /* two words and up padding object */
};


typedef struct managed_obj_empty_s {
  managed_obj_type_t type;      /* MANAGED_OBJECT_TYPE_EMPTY */
} managed_obj_empty_s;

typedef struct managed_obj_type_s {
  managed_obj_type_t type;
} managed_obj_type_s;

typedef mps_res_t (*managed_obj_scanned_pointer_scan_fn_t)(mps_ss_t ss, void*);
typedef void (*managed_obj_finalization_callback_t)(ManagedObjState * state, void*);

typedef struct managed_obj_scanned_pointer_s {
  managed_obj_type_t type;                                    /* MANAGED_OBJECT_TYPE_SCANNED_POINTER */
  void* pointer;                                              /* the pointer */
  managed_obj_scanned_pointer_scan_fn_t scanner;              /* tell the scanner how to scan our pointer */
  managed_obj_finalization_callback_t finalization_callback;  /* the finalization callback */
} managed_obj_scanned_pointer_s;


/* fwd2, fwd, pad1, pad -- MPS forwarding and padding objects        %%MPS
 *
 * These object types are here to satisfy the MPS Format Protocol.
 * See topic/format.
 *
 * The MPS needs to be able to replace any object with a forwarding
 * object or broken heart and since the smallest normal object defined
 * above is two words long, we have two kinds of forwarding objects:
 * FWD2 is exactly two words long, and FWD stores a size for larger
 * objects. There are cleverer ways to do this with bit twiddling, of
 * course.
 *
 * The MPS needs to be able to pad out any area of memory that's a
 * multiple of the pool alignment.  We've chosen an single word alignment
 * for this interpreter, so we have to have a special padding object, PAD1,
 * for single words.  For padding multiple words we use PAD objects with a
 * size field.
 *
 * See managed_obj_pad, managed_obj_fwd etc. to see how these are used.
 */

typedef struct managed_obj_fwd2_s {
  managed_obj_type_t type;                  /* MANAGED_OBJECT_TYPE_FWD2 */
  managed_obj_t fwd;                    /* forwarded object */
} managed_obj_fwd2_s;

typedef struct managed_obj_fwd_s {
  managed_obj_type_t type;                  /* MANAGED_OBJECT_TYPE_FWD */
  managed_obj_t fwd;                    /* forwarded object */
  size_t size;                  /* total size of this object */
} managed_obj_fwd_s;

typedef struct managed_obj_pad1_s {
  managed_obj_type_t type;                  /* MANAGED_OBJECT_TYPE_PAD1 */
} managed_obj_pad1_s;

typedef struct managed_obj_pad_s {
  managed_obj_type_t type;                  /* MANAGED_OBJECT_TYPE_PAD */
  size_t size;                  /* total size of this object */
} managed_obj_pad_s;

typedef union managed_obj_u {
  managed_obj_type_s type;                  /* one of MANAGED_OBJECT_TYPE_* */
  managed_obj_empty_s empty;
  managed_obj_scanned_pointer_s scanned_pointer;
  MANAGED_OBJECT_METADATA_UNIONS
  managed_obj_fwd2_s fwd2;
  managed_obj_fwd_s fwd;
  managed_obj_pad_s pad;
} managed_obj_s;

/* structure macros */

void * managed_obj_malloc(ManagedObjState * state, size_t s);
void managed_obj_free(ManagedObjState * state, void * p, size_t s);

// we require all objects that may contain references to other objects to be zero'd
// this should avoid keeping an object alive when it should be dead

#ifdef __cplusplus

} // end extern "C", C++ templates must have C++ linkage

template <typename T, class... Args>
static inline T* managed_obj_new(ManagedObjState * state, Args &&... args) {
  return new (managed_obj_malloc(state, sizeof(T))) T(std::forward<Args>(args)...);
}

template <typename T>
static inline void managed_obj_delete(ManagedObjState * state, void * ptr) {
  static_cast<T*>(ptr)->~T();
  managed_obj_volatile_memset(ptr, 0, sizeof(T));
  managed_obj_free(state, ptr, sizeof(T));
}

extern "C" {

#else

#define managed_obj_new(state, T) (T*)managed_obj_malloc(state, sizeof(T))

#define managed_obj_delete(state, T, ptr) \
  managed_obj_volatile_memset(ptr, 0, sizeof(T)); \
  managed_obj_free(state, ptr, sizeof(T))

#endif

#define MANAGED_OBJECT_FIX(ref) \
  do { \
    mps_addr_t _addr = (ref); /* copy to local to avoid type pun */ \
    printf("fixing address %p\n", _addr); \
    mps_res_t res = MPS_FIX12(ss, &_addr); \
    if (res != MPS_RES_OK) return res; \
    (ref) = _addr; \
  } while(0)

#define MANAGED_OBJECT_TYPE(obj) ((obj)->type.type)

managed_obj_t managed_obj_assert(managed_obj_t obj, managed_obj_type_t type);

/* a safe cast, asserts the pointer is actually a scanned pointer */
#define MANAGED_OBJECT_SCANNED_CAST(T, obj) ((T*)(managed_obj_assert((obj), MANAGED_OBJECT_TYPE_SCANNED_POINTER)->scanned_pointer.pointer))

/* an unsafe cast, assumes the pointer is actually a scanned pointer */

#define MANAGED_OBJECT_SCANNED_CAST_UNSAFE(T, obj) ((T*)((obj)->scanned_pointer.pointer))

#define MANAGED_OBJECT_ALIGNMENT _Alignof(managed_obj_s)

/* Align size upwards to the next multiple of the word size. */
#define MANAGED_OBJECT_ALIGN_WORD(size) (((size) + MANAGED_OBJECT_ALIGNMENT - 1) & ~(MANAGED_OBJECT_ALIGNMENT - 1))

/* Align size upwards to the next multiple of the word size, and
 * additionally ensure that it's big enough to store a forwarding
 * pointer. Evaluates its argument twice. */
#define MANAGED_OBJECT_ALIGN_OBJ(size) (MANAGED_OBJECT_ALIGN_WORD(size) >= MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd_s)) ? MANAGED_OBJECT_ALIGN_WORD(size) : MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd_s)))

typedef struct ManagedObjTime {
    int seconds;
    int milliseconds;
    int microseconds;
} ManagedObjTime;

ManagedObjTime managed_obj_convert_to_time(unsigned long stamp);

#define MANAGED_OBJECT_DEFAULT_ARENA_SIZE 32ul * 1024 * 1024

void managed_obj_init(ManagedObjState * state, size_t arena_size);
void managed_obj_init_with_user_memory(ManagedObjState * state, size_t arena_size, void * user_memory, size_t user_memory_size);
void managed_obj_error(const char *format, ...);
void managed_obj_describe(ManagedObjState * state);
managed_obj_t managed_obj_make(ManagedObjState * state, void * pointer);
managed_obj_t managed_obj_make_with_finalizer(ManagedObjState * state, void * pointer, managed_obj_finalization_callback_t finalization_callback);
managed_obj_t managed_obj_make_scanned(ManagedObjState * state, void * pointer, managed_obj_scanned_pointer_scan_fn_t scan_fn);
managed_obj_t managed_obj_make_scanned_with_finalizer(ManagedObjState * state, void * pointer, managed_obj_scanned_pointer_scan_fn_t scan_fn, managed_obj_finalization_callback_t finalization_callback);
managed_obj_t managed_obj_make_empty(ManagedObjState * state);
void managed_obj_pin(ManagedObjState * state, managed_obj_t);
void managed_obj_unpin(ManagedObjState * state, managed_obj_t);
void managed_obj_collect(ManagedObjState * state);
void managed_obj_deinit(ManagedObjState * state);

MANAGED_OBJECT_METADATA_HEADER_FUNCTIONS

#ifdef __cplusplus
}
#endif

#endif