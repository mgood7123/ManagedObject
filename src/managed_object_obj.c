#include <managed_object_obj.h>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include <mpm.h>
#include <mpscams.h>
#include <ring.h>

#include <gc/gc.h>

int debug_pool = 0;

void managed_obj_print_stats(ManagedObjState * state);


managed_obj_t managed_obj_assert(managed_obj_t obj, managed_obj_type_t type) {
  AVER(obj);
  AVER(MANAGED_OBJECT_TYPE(obj) == type);
  return obj;
}

#define MANAGED_OBJ_MESSAGE_MAX_LENGTH          ((size_t)255)   /* max length of error message */

static char managed_obj_error_message[MANAGED_OBJ_MESSAGE_MAX_LENGTH+1];

void managed_obj_error(const char *format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf(managed_obj_error_message, sizeof managed_obj_error_message, format, args);
  va_end(args);

    fflush(stdout);
    fprintf(stderr, "Fatal error: %s\n", managed_obj_error_message);
    abort();
}

/* MPS Format                                                   %%MPS
 *
 * These functions describe Scheme objects to the MPS. See topic/format.
 *
 * In general, MPS format methods are performance critical, as they're used
 * on the MPS critical path. See topic/critical.
 *
 * Format methods might also be called at any time from the MPS, including
 * in signal handlers, exception handlers, interrupts, or other special
 * contexts.  They must avoid touching any memory except the object they're
 * asked about, and possibly some static volatile data.
 *
 * Because these methods are critical, there are considerable gains in
 * performance if you mix them with the MPS source code and allow the
 * compiler to optimize globally.  See guide/build.
 */

/* managed_obj_scan -- object format scanner                            %%MPS
 *
 * The job of the scanner is to identify references in a contiguous
 * group of objects in memory, by passing them to the "fix" operation.
 * This code is highly performance critical. See topic/scanning.
 */

static mps_res_t managed_obj_scan(mps_ss_t ss, mps_addr_t base, mps_addr_t limit)
{
  mps_res_t res;
  MPS_SCAN_BEGIN(ss) {
    while (base < limit) {
      managed_obj_t obj = (managed_obj_t)base;
      switch (MANAGED_OBJECT_TYPE(obj)) {
      case MANAGED_OBJECT_TYPE_EMPTY:
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_empty_s));
        break;
      case MANAGED_OBJECT_TYPE_SCANNED_POINTER:
        MPS_FIX_CALL(ss, res = obj->scanned_pointer.scanner(ss, obj->scanned_pointer.pointer));
        if (res != MPS_RES_OK)
            return res;
        base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_scanned_pointer_s));
        break;
      MANAGED_OBJECT_METADATA_SCAN
      case MANAGED_OBJECT_TYPE_FWD2:
        base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd2_s));
        break;
      case MANAGED_OBJECT_TYPE_FWD:
        base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(obj->fwd.size);
        break;
      case MANAGED_OBJECT_TYPE_PAD1:
        base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_pad1_s));
        break;
      case MANAGED_OBJECT_TYPE_PAD:
        base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(obj->pad.size);
        break;
      default:
        managed_obj_error("Unexpected object on the heap\n");
      }
    }
  } MPS_SCAN_END(ss);
  return MPS_RES_OK;
}


/* managed_obj_skip -- object format skip method                        %%MPS
 *
 * The job of `managed_obj_skip` is to return the address where the next object would
 * be allocated.  This isn't quite the same as the size of the object,
 * since there may be some rounding according to the memory pool alignment
 * chosen. This interpreter has chosen to align to single words. See
 * topic/format.
 */

static mps_addr_t managed_obj_skip(mps_addr_t base)
{
  managed_obj_t obj = (managed_obj_t)base;
  switch (MANAGED_OBJECT_TYPE(obj)) {
  case MANAGED_OBJECT_TYPE_EMPTY:
    base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_empty_s));
    break;
  case MANAGED_OBJECT_TYPE_SCANNED_POINTER:
    base = (char *)base + MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_scanned_pointer_s));
    break;
  MANAGED_OBJECT_METADATA_SKIP
  case MANAGED_OBJECT_TYPE_FWD2:
    base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd2_s));
    break;
  case MANAGED_OBJECT_TYPE_FWD:
    base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(obj->fwd.size);
    break;
  case MANAGED_OBJECT_TYPE_PAD:
    base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(obj->pad.size);
    break;
  case MANAGED_OBJECT_TYPE_PAD1:
    base = (char *)base + MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_pad1_s));
    break;
  default:
    managed_obj_error("Unexpected object on the heap\n");
  }
  return base;
}


/* managed_obj_isfwd -- object format forwarded test                    %%MPS
 *
 * The job of `managed_obj_isfwd` is to detect whether an object has been replaced
 * by a forwarding object, and return the address of the new copy if it has,
 * otherwise NULL.  See topic/format.
 */

static mps_addr_t managed_obj_isfwd(mps_addr_t addr)
{
  managed_obj_t obj = (managed_obj_t)addr;
  switch (MANAGED_OBJECT_TYPE(obj)) {
  case MANAGED_OBJECT_TYPE_FWD2:
    return obj->fwd2.fwd;
  case MANAGED_OBJECT_TYPE_FWD:
    return obj->fwd.fwd;
  }
  return NULL;
}

/* managed_obj_isfwd -- object format forwarded test                    %%MPS
 *
 * The job of `managed_obj_isfwd` is to detect whether an object has been replaced
 * by a forwarding object, and return the address of the new copy if it has,
 * otherwise NULL.  See topic/format.
 */

static mps_bool_t managed_obj_ispinned(mps_addr_t addr)
{
  managed_obj_t obj = (managed_obj_t)addr;
  return obj->type.pinned;
}


/* managed_obj_fwd -- object format forwarding method                   %%MPS
 *
 * The job of `managed_obj_fwd` is to replace an object by a forwarding object that
 * points at a new copy of the object.  The object must be detected by
 * `managed_obj_isfwd`.  In this case, we have to be careful to replace two-word
 * objects with a `FWD2` object, because the `FWD` object won't fit. See
 * topic/format.
 */

static void managed_obj_fwd(mps_addr_t old, mps_addr_t new)
{
  managed_obj_t obj = (managed_obj_t)old;
  mps_addr_t limit = managed_obj_skip(old);
  size_t size = (size_t)((char *)limit - (char *)old);
  AVER_CRITICAL(size >= MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd2_s)));
  if (size == MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_fwd2_s))) {
    MANAGED_OBJECT_TYPE(obj) = MANAGED_OBJECT_TYPE_FWD2;
    obj->fwd2.fwd = (managed_obj_t)new;
  } else {
    MANAGED_OBJECT_TYPE(obj) = MANAGED_OBJECT_TYPE_FWD;
    obj->fwd.fwd = (managed_obj_t)new;
    obj->fwd.size = size;
  }
  printf("object moved from address %p to address %p\n", old, new);
}

/* managed_obj_pad -- object format padding method                      %%MPS
 *
 * The job of `managed_obj_pad` is to fill in a block of memory with a padding
 * object that will be skipped by `managed_obj_scan` or `managed_obj_skip` but does
 * nothing else.  Because we've chosen to align to single words, we may
 * have to pad a single word, so we have a special single-word padding
 * object, `PAD1` for that purpose.  Otherwise we can use multi-word
 * padding objects, `PAD`. See topic/format.
 */

static void managed_obj_pad(mps_addr_t addr, size_t size)
{
  managed_obj_t obj = (managed_obj_t)addr;
  AVER_CRITICAL(size >= MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_pad1_s)));
  if (size == MANAGED_OBJECT_ALIGN_WORD(sizeof(managed_obj_pad1_s))) {
    MANAGED_OBJECT_TYPE(obj) = MANAGED_OBJECT_TYPE_PAD1;
  } else {
    MANAGED_OBJECT_TYPE(obj) = MANAGED_OBJECT_TYPE_PAD;
    obj->pad.size = size;
  }
  // printf("object padded with address %p\n", addr);
}

ManagedObjTime managed_obj_convert_to_time(unsigned long stamp) {

    ManagedObjTime t;
    t.seconds = 0;
    t.milliseconds = 0;
    t.microseconds = 0;

    mps_clock_t cps = mps_clocks_per_sec();
    unsigned long s = stamp;
    if (cps >= 1000000) {
        // cps is microsecond resolution
        if (s >= 1000000) {
            // stamp exceeds 1000000 microseconds (1000 milliseconds)
            t.seconds = stamp / 1000000;
            t.milliseconds = stamp - (t.seconds * 1000000);
            // stamp_ms must exceed 1000 microseconds
            unsigned long stamp_ms = t.milliseconds;
            t.milliseconds = stamp_ms / 1000;
            t.microseconds = stamp_ms - (t.milliseconds * 1000);
        } else if (s >= 1000) {
            // stamp exceeds 1000 microseconds
            t.milliseconds = stamp / 1000;
            t.microseconds = stamp - (t.milliseconds * 1000);
        } else {
            // stamp does not exceed 1000 microseconds
            t.microseconds = stamp;
        }
    } else if (cps >= 1000) {
        // cps is millisecond resolution, do not convert to microseconds
        if (s >= 1000) {
            // stamp exceeds 1000 milliseconds
            t.seconds = stamp / 1000;
            t.milliseconds = stamp - (t.seconds * 1000);
        } else {
            // stamp does not exceed 1000 milliseconds
            t.milliseconds = stamp;
        }
    } else {
        // cps is in seconds resolution, do not convert to milliseconds
        t.seconds = stamp;
    }
    return t;
}

/* managed_object_mps_chat -- get and display MPS messages                     %%MPS
 *
 * The MPS message protocol allows the MPS to communicate various things
 * to the client code.  Because the MPS may run asynchronously the client
 * must poll the MPS to pick up messages.  This function shows how this
 * is done. See topic/message and topic/finalization.
 */

int nStart = 0;
int nComplete = 0;

void managed_obj_mps_chat(ManagedObjState * state) {
  mps_message_type_t type;

  while (mps_message_queue_type(&type, state->arena)) {
    mps_message_t message;
    mps_bool_t b;
    b = mps_message_get(&message, state->arena, type);
    AVER(b); /* we just checked there was one */

    if (type == mps_message_type_gc_start()) {
      printf("\nCollection start %d due to '%s'\n", ++nStart, mps_message_gc_start_why(state->arena, message));

    } else if (type == mps_message_type_gc()) {

      // should these be part of stats?
      // we currently dont know exactly what they mean in terms of GC profiling

      size_t live, condemned, not_condemned;

      live = mps_message_gc_live_size(state->arena, message);
      condemned = mps_message_gc_condemned_size(state->arena, message);
      not_condemned = mps_message_gc_not_condemned_size(state->arena, message);

      printf("\nCollection complete %d\n", ++nComplete);
      // printf("  live %zu\n", live);
      // printf("  condemned %zu\n", condemned);
      // printf("  not_condemned %zu\n", not_condemned);

     } else if (type == mps_message_type_finalization()) {
      /* A finalization message is received when an object registered earlier
        with `mps_finalize` would have been recycled if it hadn't been
        registered. This means there are no other references to the object.
        Note, however, that finalization isn't reliable or prompt.
        Treat it as an optimization. See topic/finalization. */
      
      managed_obj_t obj;

      mps_message_finalization_ref((mps_addr_t*)&obj, state->arena, message);

      //printf("object %p is being finalized.\n", obj);

      if(MANAGED_OBJECT_TYPE(obj) == MANAGED_OBJECT_TYPE_EMPTY) {
        state->freed_obj_bytes += sizeof(managed_obj_empty_s);
        state->freed_aligned_obj_bytes += MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_empty_s));
      } else if(MANAGED_OBJECT_TYPE(obj) == MANAGED_OBJECT_TYPE_SCANNED_POINTER) {
        state->freed_obj_bytes += sizeof(managed_obj_scanned_pointer_s);
        state->freed_aligned_obj_bytes += MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_scanned_pointer_s));
        if (obj->scanned_pointer.pointer) {
          if (obj->scanned_pointer.finalization_callback) {
            printf("object %p has pointer %p.\n", obj, obj->scanned_pointer.pointer);

            // TODO: the user could resurrect the object at field 'obj->scanned_pointer.pointer'
            // TODO: the field 'obj->scanned_pointer.pointer' could contain a GC object not finalized yet
            // TODO: the field 'obj->scanned_pointer.pointer' could contain a GC object that is finalized but not yet processed

            obj->scanned_pointer.finalization_callback(state, obj->scanned_pointer.pointer);
          }
          printf("object %p with pointer %p has been freed, setting pointer to zero.\n", obj, obj->scanned_pointer.pointer);
          obj->scanned_pointer.pointer = NULL;
        } else {
          // this could happen if a user explicitly sets a field to NULL
          printf("WARNING: object %p has already been freed.\n", obj);
        }
      }
      MANAGED_OBJECT_METADATA_FINALIZE
    // } else {
      // printf("Unknown message from MPS!\n");
    }

    mps_message_discard(state->arena, message);
  }
}

/* managed_obj_gen_params -- initial setup for generational GC          %%MPS
 *
 * Each structure in this array describes one generation of objects. The
 * two members are the capacity of the generation in kilobytes, and the
 * mortality, the proportion of objects in the generation that you expect
 * to survive a collection of that generation.
 *
 * These numbers are *hints* to the MPS that it may use to make decisions
 * about when and what to collect: nothing will go wrong (other than
 * suboptimal performance) if you make poor choices.
 *
 * Note that these numbers have deliberately been chosen to be small,
 * so that the MPS is forced to collect often so that you can see it
 * working. Don't just copy these numbers unless you also want to see
 * frequent garbage collections! See topic/collection.
 */

static mps_gen_param_s managed_obj_gen_params[] = {
  { 150, 0.85 },
  { 170, 0.45 }
};

#ifdef MPS_ADDRESS_SANITIZER
#include <sanitizer/asan_interface.h>
#endif

void managed_obj_init(ManagedObjState * state, size_t arena_size) {
    if (state == NULL) {
        managed_obj_error("state cannot be null");
    }
    MANAGED_OBJECT_METADATA_SOURCE_PREINIT
    memset(state, 0, sizeof(ManagedObjState));

    mps_res_t res;

    /* Create an MPS arena.  There is usually only one of these in a process.
        It holds all the MPS "global" state and is where everything happens. */
    MPS_ARGS_BEGIN(args) {
        MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE, arena_size);
        res = mps_arena_create_k(&state->arena, mps_arena_class_vm(), args);
    } MPS_ARGS_END(args);
    if (res == MPS_RES_MEMORY) managed_obj_error("Couldn't create arena, not enough memory ('arena_size' was too small)");
    if (res != MPS_RES_OK) managed_obj_error("Couldn't create arena");

    // pick up gc stats
    mps_message_type_enable(state->arena, mps_message_type_gc());
    mps_message_type_enable(state->arena, mps_message_type_gc_start());

    /* Make sure we can pick up finalization messages. */
    mps_message_type_enable(state->arena, mps_message_type_finalization());


  /* Create the object format. This gathers together the methods that
     the MPS uses to interrogate your objects via the Format Protocol.
     See topic/format. */
  MPS_ARGS_BEGIN(args) {
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ALIGN, MANAGED_OBJECT_ALIGNMENT);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SCAN, managed_obj_scan);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SKIP, managed_obj_skip);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_FWD, managed_obj_fwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ISFWD, managed_obj_isfwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ISPINNED, managed_obj_ispinned);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_PAD, managed_obj_pad);
    res = mps_fmt_create_k(&state->fmt, state->arena, args);
  } MPS_ARGS_END(args);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj format");

  /* Create a chain controlling GC strategy. */
  /* TODO: Brief explanation with link to manual. */
  res = mps_chain_create(&state->chain,
                         state->arena,
                         (sizeof(managed_obj_gen_params) / sizeof(managed_obj_gen_params[0])),
                         managed_obj_gen_params);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj chain");

  /* Create an Automatic Mostly-Copying (AMC) pool to manage the Scheme
     objects.  This is a kind of copying garbage collector. */
  MPS_ARGS_BEGIN(args) {
    // MPS_ARGS_ADD(args, MPS_KEY_CHAIN, state->chain);
    MPS_ARGS_ADD(args, MPS_KEY_FORMAT, state->fmt);
    res = mps_pool_create_k(&state->pool, state->arena, debug_pool ? mps_class_ams_debug() : mps_class_ams(), args);
  } MPS_ARGS_END(args);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj pool");

  /* Create an allocation point for fast in-line allocation of objects
     from the `managed_obj_pool`.  You'd usually want one of these per thread
     for your primary pools.  This interpreter is single threaded, though,
     so we just have it in a global. See topic/allocation. */
  res = mps_ap_create_k(&state->ap, state->pool, mps_args_none);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj allocation point");


    res = mps_thread_reg(&state->thread, state->arena);
    if (res != MPS_RES_OK) managed_obj_error("Couldn't register thread");

    // if running under ASAN any attempt to get a stack address via a local variable will
    // point to ASAN's fake stack
    // use BOEHM to obtain the real stack (actual stack) address

    struct MANAGED_STACK_ADDRESS_BOEHM_GC_stack_base sb_end;
    memset(&sb_end, 0, sizeof(struct MANAGED_STACK_ADDRESS_BOEHM_GC_stack_base));

    int gcres = MANAGED_STACK_ADDRESS_BOEHM_GC_get_stack_base(&sb_end, 1);
    if (gcres == MANAGED_STACK_ADDRESS_BOEHM_GC_SUCCESS) {
    } else if (gcres == MANAGED_STACK_ADDRESS_BOEHM_GC_UNIMPLEMENTED) {
      managed_obj_error("GC_get_stack_base(start) unimplemented\n");
    } else {
      managed_obj_error("GC_get_stack_base(start) failed: %d\n", gcres);
    }

    printf("thread stack bottom: %p\n", sb_end.mem_base);

    void *marker = sb_end.mem_base;

    res = mps_root_create_thread_tagged(
      &state->thread_stack_root, state->arena,
      mps_rank_ambig(), (mps_rm_t)0,
      state->thread, mps_scan_area_tagged,
      sizeof(mps_word_t) - 1, (mps_word_t)0,
      marker
    );

    if (res != MPS_RES_OK) printf("Couldn't create root");

  // depreciated - uses extra memory, as opposed setting an object flag
  //
  // state->pushed_pinned_used = 0;
  // state->pushed_pinned_capacity = 2;
  // state->pushed_pinned = calloc(sizeof(managed_obj_t), state->pushed_pinned_capacity);
  // if (state->pushed_pinned == NULL) managed_obj_error("Couldn't allocate pinned memory");

  // res = mps_root_create_area_tagged(
  //     &state->pushed_pinned_root, state->arena, mps_rank_ambig(),
  //     (mps_rm_t)0, state->pushed_pinned, state->pushed_pinned+state->pushed_pinned_capacity,
  //     mps_scan_area_tagged, sizeof(mps_word_t) - 1, (mps_word_t)0
  // );
  // if (res != MPS_RES_OK) managed_obj_error("Couldn't create pinned root");

    MANAGED_OBJECT_METADATA_SOURCE_INIT
}

void managed_obj_init_with_user_memory(ManagedObjState * state, size_t arena_size, void * user_memory, size_t user_memory_size) {
    if (state == NULL) {
        managed_obj_error("state cannot be null");
    }
    MANAGED_OBJECT_METADATA_SOURCE_PREINIT
    memset(state, 0, sizeof(ManagedObjState));

    mps_res_t res;

    /* Create an MPS arena.  There is usually only one of these in a process.
        It holds all the MPS "global" state and is where everything happens. */
    MPS_ARGS_BEGIN(args) {
        MPS_ARGS_ADD(args, MPS_KEY_ARENA_SIZE, arena_size);
        res = mps_arena_create_k(&state->arena, mps_arena_class_vm(), args);
    } MPS_ARGS_END(args);
    if (res == MPS_RES_MEMORY) managed_obj_error("Couldn't create arena, not enough memory ('arena_size' was too small)");
    if (res != MPS_RES_OK) managed_obj_error("Couldn't create arena");

    // pick up gc stats
    mps_message_type_enable(state->arena, mps_message_type_gc());
    mps_message_type_enable(state->arena, mps_message_type_gc_start());

    /* Make sure we can pick up finalization messages. */
    mps_message_type_enable(state->arena, mps_message_type_finalization());
  if (user_memory != NULL || user_memory_size != 0) {
    memset(user_memory, 0, user_memory_size);
    res = mps_root_create_area_tagged(
        &state->thread_stack_root, state->arena, mps_rank_exact(),
        (mps_rm_t)0, user_memory, user_memory+user_memory_size,
        mps_scan_area_tagged, sizeof(mps_word_t) - 1, (mps_word_t)0
    );
    if (res != MPS_RES_OK) managed_obj_error("Couldn't create user memory root");
  }

  /* Create the object format. This gathers together the methods that
     the MPS uses to interrogate your objects via the Format Protocol.
     See topic/format. */
  MPS_ARGS_BEGIN(args) {
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ALIGN, MANAGED_OBJECT_ALIGNMENT);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SCAN, managed_obj_scan);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_SKIP, managed_obj_skip);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_FWD, managed_obj_fwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ISFWD, managed_obj_isfwd);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_ISPINNED, managed_obj_ispinned);
    MPS_ARGS_ADD(args, MPS_KEY_FMT_PAD, managed_obj_pad);
    res = mps_fmt_create_k(&state->fmt, state->arena, args);
  } MPS_ARGS_END(args);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj format");

  /* Create a chain controlling GC strategy. */
  /* TODO: Brief explanation with link to manual. */
  res = mps_chain_create(&state->chain,
                         state->arena,
                         (sizeof(managed_obj_gen_params) / sizeof(managed_obj_gen_params[0])),
                         managed_obj_gen_params);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj chain");

  /* Create an Automatic Mostly-Copying (AMC) pool to manage the Scheme
     objects.  This is a kind of copying garbage collector. */
  MPS_ARGS_BEGIN(args) {
    // MPS_ARGS_ADD(args, MPS_KEY_CHAIN, state->chain);
    MPS_ARGS_ADD(args, MPS_KEY_FORMAT, state->fmt);
    res = mps_pool_create_k(&state->pool, state->arena, debug_pool ? mps_class_ams_debug() : mps_class_ams(), args);
  } MPS_ARGS_END(args);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj pool");

  /* Create an allocation point for fast in-line allocation of objects
     from the `managed_obj_pool`.  You'd usually want one of these per thread
     for your primary pools.  This interpreter is single threaded, though,
     so we just have it in a global. See topic/allocation. */
  res = mps_ap_create_k(&state->ap, state->pool, mps_args_none);
  if (res != MPS_RES_OK) managed_obj_error("Couldn't create obj allocation point");

  // depreciated - uses extra memory, as opposed setting an object flag
  //
  // state->pushed_pinned_used = 0;
  // state->pushed_pinned_capacity = 2;
  // state->pushed_pinned = calloc(sizeof(managed_obj_t), state->pushed_pinned_capacity);
  // if (state->pushed_pinned == NULL) managed_obj_error("Couldn't allocate pinned memory");

  // res = mps_root_create_area_tagged(
  //     &state->pushed_pinned_root, state->arena, mps_rank_ambig(),
  //     (mps_rm_t)0, state->pushed_pinned, state->pushed_pinned+state->pushed_pinned_capacity,
  //     mps_scan_area_tagged, sizeof(mps_word_t) - 1, (mps_word_t)0
  // );
  // if (res != MPS_RES_OK) managed_obj_error("Couldn't create pinned root");

  MANAGED_OBJECT_METADATA_SOURCE_INIT
}

MANAGED_OBJECT_METADATA_SOURCE_FUNCTIONS

static mps_res_t managed_obj_no_scan_callback(mps_ss_t ss, void * p) {
  UNUSED(ss);
  UNUSED(p);
  return MPS_RES_OK;
}

static void managed_obj_no_finalization_callback(ManagedObjState * state, void * p) {
  free(p);
}

void * managed_obj_malloc(ManagedObjState * state, size_t s) {
  void * p = malloc(s);
  if (p != NULL) {
    state->allocated_bytes += s;
  }
  return p;
}
void managed_obj_free(ManagedObjState * state, void * p, size_t s) {
  if (p != NULL) {
    state->freed_bytes += s;
  }
  free(p);
}


managed_obj_t managed_obj_make(ManagedObjState * state, void * pointer) {
  return managed_obj_make_scanned_with_finalizer(state, pointer, managed_obj_no_scan_callback, managed_obj_no_finalization_callback);
}

managed_obj_t managed_obj_make_scanned(ManagedObjState * state, void * pointer, managed_obj_scanned_pointer_scan_fn_t scanner) {
  return managed_obj_make_scanned_with_finalizer(state, pointer, scanner, managed_obj_no_finalization_callback);
}

managed_obj_t managed_obj_make_with_finalizer(ManagedObjState * state, void * pointer, managed_obj_finalization_callback_t finalization_callback)
{
  return managed_obj_make_scanned_with_finalizer(state, pointer, managed_obj_no_scan_callback, finalization_callback);
}

// incorrect vs correct usage example:
// https://memory-pool-system.readthedocs.io/en/latest/topic/allocation.html#example-inserting-into-a-doubly-linked-list
//
managed_obj_t managed_obj_make_scanned_with_finalizer(ManagedObjState * state, void * pointer, managed_obj_scanned_pointer_scan_fn_t scanner, managed_obj_finalization_callback_t finalization_callback)
{
  managed_obj_t obj;
  size_t size = MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_scanned_pointer_s));
  while(1) {
    mps_res_t res = mps_reserve((mps_addr_t*)&obj, state->ap, size);
    if (res != MPS_RES_OK) managed_obj_error("out of memory in make_pointer");
    obj->scanned_pointer.type = MANAGED_OBJECT_TYPE_SCANNED_POINTER;
    obj->scanned_pointer.pinned = FALSE;
    obj->scanned_pointer.pointer = pointer;
    obj->scanned_pointer.scanner = scanner;
    obj->scanned_pointer.finalization_callback = finalization_callback;
    if (mps_commit(state->ap, obj, size)) {
      break;
    }
  }
  state->allocated_obj_bytes += sizeof(managed_obj_scanned_pointer_s);
  state->allocated_aligned_obj_bytes += size;

  mps_finalize(state->arena, (mps_addr_t*)&obj);

  return obj;
}

managed_obj_t managed_obj_make_empty(ManagedObjState * state)
{
  managed_obj_t obj;
  size_t size = MANAGED_OBJECT_ALIGN_OBJ(sizeof(managed_obj_empty_s));
  while(1) {
    mps_res_t res = mps_reserve((mps_addr_t*)&obj, state->ap, size);
    if (res != MPS_RES_OK) managed_obj_error("out of memory in make_pointer");
    obj->empty.type = MANAGED_OBJECT_TYPE_EMPTY;
    obj->empty.pinned = FALSE;
    if (mps_commit(state->ap, obj, size)) {
      break;
    }
  }
  state->allocated_obj_bytes += sizeof(managed_obj_empty_s);
  state->allocated_aligned_obj_bytes += size;

  mps_finalize(state->arena, (mps_addr_t*)&obj);

  return obj;
}

void managed_obj_describe(ManagedObjState * state) {
  printf("DESCRIBING ARENA:\n");
  ArenaDescribe(state->arena, mps_lib_get_stderr(), 1);
  printf("DESCRIBED ARENA\n");
  printf("DESCRIBING ARENA TRACTS:\n");
  ArenaDescribeTracts(state->arena, mps_lib_get_stderr(), 1);
  printf("DESCRIBED ARENA TRACTS\n");
}

inline void managed_obj_pin(ManagedObjState * state, managed_obj_t obj) {
  obj->type.pinned = TRUE;
  state->pinned_used++;
}

inline void managed_obj_unpin(ManagedObjState * state, managed_obj_t obj) {
  obj->type.pinned = FALSE;
  state->pinned_used--;
}

// depreciated - uses extra memory, as opposed setting an object flag
//
// void reroot(ManagedObjState * state, managed_obj_t * new_pinned, int new_capacity) {
//     mps_root_t new_pinned_root = NULL;
//     mps_res_t res = mps_root_create_area_tagged(
//         &new_pinned_root, state->arena, mps_rank_ambig(),
//         (mps_rm_t)0, new_pinned, new_pinned+new_capacity,
//         mps_scan_area_tagged, sizeof(mps_word_t) - 1, (mps_word_t)0
//     );
//     if (res != MPS_RES_OK) managed_obj_error("Couldn't create pinned root");
//     // both are pinned, we can safely destroy old pin
//     mps_root_destroy(state->pushed_pinned_root);
//     state->pushed_pinned_root = new_pinned_root;
//     free(state->pushed_pinned);
//     state->pushed_pinned = new_pinned;
//     state->pushed_pinned_capacity = new_capacity;
// }

// void managed_obj_push_pin(ManagedObjState * state, managed_obj_t obj) {
//   if (state->pushed_pinned_used == state->pushed_pinned_capacity) {
//     // reallocate
//     int new_capacity = state->pushed_pinned_capacity * 2;
//     managed_obj_t * new_pinned = calloc(sizeof(managed_obj_t), new_capacity);
//     if (new_pinned == NULL) managed_obj_error("Couldn't allocate pinned memory");
//     // copy old pinned to new pinned
//     memcpy(new_pinned, state->pushed_pinned, sizeof(managed_obj_t)*state->pushed_pinned_capacity);
//     reroot(state, new_pinned, new_capacity);
//   }
//   state->pushed_pinned[state->pushed_pinned_used++] = obj;
// }

// void managed_obj_pop_pin(ManagedObjState * state) {
//   state->pushed_pinned[--state->pushed_pinned_used] = NULL;
//   // always keep at least 2 elements
//   if (state->pushed_pinned_capacity > 2 && state->pushed_pinned_used == state->pushed_pinned_capacity / 2) {
//     // reallocate
//     int new_capacity = state->pushed_pinned_capacity / 2;
//     managed_obj_t * new_pinned = malloc(sizeof(managed_obj_t)*new_capacity);
//     if (new_pinned == NULL) managed_obj_error("Couldn't allocate pinned memory");
//     // copy old pinned to new pinned
//     memcpy(new_pinned, state->pushed_pinned, sizeof(managed_obj_t)*new_capacity);
//     reroot(state, new_pinned, new_capacity);
//   }
// }

void managed_obj_collect(ManagedObjState * state) {
    printf("\nCollection start\n");

    unsigned long collect_start = mps_clock();

    mps_arena_collect(state->arena);
    managed_obj_mps_chat(state);

    unsigned long collect_end = mps_clock();

    ManagedObjTime collect_duration_time = managed_obj_convert_to_time(collect_end - collect_start);

    printf("Collection end\n");
    printf("Total Collection duration: %d seconds, %d milliseconds, %d microseconds\n\n", collect_duration_time.seconds, collect_duration_time.milliseconds, collect_duration_time.microseconds);
}

static Res rootDestroy(Root root, void *p)
{
  /* we are inside a locked arena, we cannot */
  /* use 'mps_root_destroy' since it locks the arena */

  AVERT(Root, root);
  
  Arena arena;

  arena = RootArena(root);

  RootDestroy(root);

  return ResOK;
}

struct mps_thr_s;

// all mps ThreadStruct contain this layout

typedef struct mps_thr_s {      /* ANSI fake thread structure */
  Sig sig;                      /* design.mps.sig.field */
  Serial serial;                /* from arena->threadSerial */
  Arena arena;                  /* owning arena */
  RingStruct arenaRing;         /* attaches to arena */
  /* extra data per impl */
} ThreadStruct;

void destroy_roots(mps_arena_t mps_arena) {
  Arena arena = (Arena)mps_arena;

  /* lock the arena to prevent any roots from */
  /* being created while we are iterating them */

  ArenaEnter(arena);

  AVER(ArenaGlobals(arena)->clamped);          /* .assume.parked */
  AVER(arena->busyTraces == TraceSetEMPTY);    /* .assume.parked */

  Globals arenaGlobals = ArenaGlobals(arena);

  AVERT(Globals, arenaGlobals);

  RootsIterate(arenaGlobals, rootDestroy, 0);
  
  /* unregister all registered threads since */
  /* we have no roots associated with them */

  /*
  _`.req.register.multi`: It must be possible to register the same
  thread multiple times. (This is needed to support the situation where
  a program that does not use the MPS is calling into MPS-using code
  from multiple threads. On entry to the MPS-using code, the thread can
  be registered, but it may not be possible to ensure that the thread is
  deregistered on exit, because control may be transferred by some
  non-local mechanism such as an exception or ``longjmp()``. We don't
  want to insist that the client program keep a table of threads it has
  registered, because maintaining the table might require allocation,
  which might provoke a collection. See request.dylan.160252_.)


  *  Register/Deregister
  *
  *  Explicitly register/deregister a thread on the arena threadRing.
  *  Register returns a "Thread" value which needs to be used
  *  for deregistration.
  *
  *  Threads must not be multiply registered in the same arena.

  */

  Ring threadRing = ArenaThreadRing(arena);
  Ring node, next;
  AVERT(Ring, threadRing);
  RING_FOR(node, threadRing, next) {
    Thread thread = RING_ELT(Thread, arenaRing, node);

    AVER(ThreadCheckSimple(thread));
    
    AVER(arena == ThreadArena(thread)); /* is the thread arena always us? */

    ThreadDeregister(thread, arena);
  }

  ArenaLeave(arena);
}

void managed_obj_deinit(ManagedObjState * state) {
    mps_arena_park(state->arena);        /* ensure no collection is running */

    managed_obj_print_stats(state);

    printf("deinit - destroy all roots\n");

    destroy_roots(state->arena);

    printf("deinit - destroyed all roots\n");

    printf("deinit - collect\n");

    managed_obj_collect(state);

    printf("deinit - collected\n");

    managed_obj_print_stats(state);

    printf("deinit - shutdown\n");

    MANAGED_OBJECT_METADATA_SOURCE_DEINIT;

    mps_ap_destroy(state->ap);       /* destroy ap before pool */
    mps_pool_destroy(state->pool);   /* destroy pool before fmt */
    mps_fmt_destroy(state->fmt);     /* destroy fmt before chain */
    mps_chain_destroy(state->chain); /* destroy chain before arena */
    mps_arena_destroy(state->arena);     /* last of all */

    memset(state, 0, sizeof(ManagedObjState));
}

void managed_obj_print_stats(ManagedObjState * state) {
    
    printf("Stats:\n");
    printf("  Pinned:\n");
    printf("    pinned objects:   %zu\n", state->pinned_used);

    // depreciated - uses extra memory, as opposed setting an object flag
    //
    // printf("  Pinned (pushed):\n");
    // printf("    used:             %zu\n", state->pushed_pinned_used);
    // printf("    used (bytes):     %zu\n", sizeof(managed_obj_t)*(state->pushed_pinned_used));
    // printf("    capacity:         %zu\n", state->pushed_pinned_capacity);
    // printf("    capacity (bytes): %zu\n", sizeof(managed_obj_t)*(state->pushed_pinned_capacity));

    printf("  Memory:\n");
    printf("    Objects (managed_obj_malloc):\n");
    printf("      allocated (bytes): %zu\n", state->allocated_bytes);
    printf("      freed (bytes):     %zu\n", state->freed_bytes);
    printf("    Objects (gc):\n");
    printf("      allocated (bytes):           %zu\n", state->allocated_obj_bytes);
    printf("      freed (bytes):               %zu\n", state->freed_obj_bytes);
    printf("      (aligned) allocated (bytes): %zu\n", state->allocated_aligned_obj_bytes);
    printf("      (aligned) freed (bytes):     %zu\n", state->freed_aligned_obj_bytes);
    printf("    Pool Allocation Point:\n");
    printf("      init: %p\n", state->ap->init);
    printf("      alloc: %p\n", state->ap->alloc);
    printf("      limit: %p\n", state->ap->limit);
    printf("    Pool:\n");
    size_t size_total = mps_pool_total_size(state->pool);
    size_t size_free = mps_pool_free_size(state->pool);
    size_t size_used = size_total - size_free;
    printf("      Used:  %zu\n", size_used);
    printf("      Free:  %zu\n", size_free);
    printf("      Total: %zu\n", size_total);
    printf("    Arena:\n");
    printf("      Reserved:           %zu\n", mps_arena_reserved(state->arena));
    printf("      Commited:           %zu\n", mps_arena_committed(state->arena));
    if (mps_arena_commit_limit(state->arena) == -1) {
    printf("      Commit Limit:       %zu\n", mps_arena_reserved(state->arena));
    } else {
    printf("      Commit Limit:       %zu\n", mps_arena_commit_limit(state->arena));
    }
    printf("      Spare:              %g\n", mps_arena_spare(state->arena));
    printf("      Spare Commited:     %zu\n", mps_arena_spare_committed(state->arena));
    if (mps_arena_spare_commit_limit(state->arena) == -1) {
    printf("      Spare Commit Limit: Infinite\n");
    } else {
    printf("      Spare Commit Limit: %zu\n", mps_arena_spare_commit_limit(state->arena));
    }
    printf("      Pause Time:         %g\n", mps_arena_pause_time(state->arena));
}
