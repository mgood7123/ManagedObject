#include "gtest/gtest.h"

#include <managed_object_pagelist.h>

#include <cstdbool> // bool
#include <cstdio>   // printf
#include <vector>   // vector

// TEST(ManagedObjectPageList, 0) {
//     ManagedObjectPageList<int> r(6);
//     for (int i = 0; i < 34; i++) {
//         r.print_chunk_and_index(r.set(1));
//     }
// }

// TEST(ManagedObjectPageList, 1) {
//     ManagedObjectPageList<int> r(6);
//     auto index_1 = r.set(1);
//     auto index_2 = r.set(2);
//     auto index_3 = r.set(3);
//     auto index_4 = r.set(4);
//     auto index_5 = r.set(4);
//     r.print_chunk_and_index(index_1);
//     r.print_chunk_and_index(index_2);
//     r.print_chunk_and_index(index_3);
//     r.print_chunk_and_index(index_4);
//     r.print_chunk_and_index(index_5);
//     r.unset(index_1);
//     r.unset(index_2);
//     r.unset(index_4);
//     r.unset(index_5);
//     r.unset(index_3);
// }

// template <typename w, typename g, typename b, typename n>
// struct Strategy {

//     using white_t = w;
//     using gray_t = g;
//     using black_t = b;
//     using node_t = n;

//     virtual node_t & get_root() = 0;

//     // node population, be explicit
//     //
//     virtual void fill_white_with_all_nodes(white_t & white) = 0;
//     virtual void fill_gray_with_all_root_nodes(gray_t & gray) = 0;

//     // node operations, be explicit
//     //
//     virtual bool is_gray_empty(gray_t & gray) = 0;
//     virtual node_t && remove_first_from_gray(gray_t & gray) = 0;
//     virtual void add_node_to_gray(gray_t & gray, node_t & node) = 0;
//     virtual void add_node_to_black(black_t & black, node_t & node) = 0;
//     virtual bool remove_if_white_contains_node(white_t & white, node_t & node) = 0;
// };

// struct MockNode {
//     std::vector<MockNode> refs;

//     struct Iterator {
//         MockNode * obj;
//         size_t next_i;
//         size_t max_i;

//         bool has_next() {
//             return next_i != max_i;
//         }

//         MockNode & next() {
//             return obj->refs[next_i++];
//         }
//     };

//     Iterator iterator() {
//         return Iterator {this, 0, refs.size()};
//     }
// };

// //                                   <white_t              , gray_t               , black_t              , node_t  >
// struct MockStrategy : public Strategy<std::vector<MockNode>, std::vector<MockNode>, std::vector<MockNode>, MockNode> {
//     node_t r;

//     virtual node_t & get_root() override {
//         return r;
//     }

//     void fill_white_with_all_nodes(white_t & white) override {

//     }

//     void fill_gray_with_all_root_nodes(gray_t & gray) override {
        
//     }

//     bool is_gray_empty(gray_t & gray) override {
//         return gray.size() == 0;
//     }

//     node_t && remove_first_from_gray(gray_t & gray) override {
//         return std::move(node_t());
//     }

//     void add_node_to_gray(gray_t & gray, node_t & node) override {

//     }

//     void add_node_to_black(black_t & black, node_t & node) override {

//     }

//     bool remove_if_white_contains_node(white_t & white, node_t & node) override {
//         return true;
//     }
// };

// struct BasicNode {
//     std::vector<BasicNode> refs;

//     struct Iterator {
//         BasicNode * obj;
//         size_t next_i;
//         size_t max_i;

//         bool has_next() {
//             return next_i != max_i;
//         }

//         BasicNode & next() {
//             return obj->refs[next_i++];
//         }
//     };

//     Iterator iterator() {
//         return Iterator {this, 0, refs.size()};
//     }
// };

// //                                    <white_t                , gray_t                 , black_t                , node_t  >
// struct BasicStrategy : public Strategy<std::vector<BasicNode*>, std::vector<BasicNode*>, std::vector<BasicNode*>, BasicNode> {
//     node_t r;

//     virtual node_t & get_root() override {
//         return r;
//     }

//     void fill_white_with_all_nodes(white_t & white) override {

//         std::vector<node_t*> stack;
//         stack.emplace_back(&r);
//         while (stack.size() != 0) {
//             node_t * current = stack.back();
//             stack.erase(--stack.end());
//             auto it = current->iterator();
//             while (it.has_next()) {
//                 node_t & next = it.next();
//                 white.emplace_back(&next);
//                 stack.emplace_back(&next);
//             }
//         }

//     }

//     void fill_gray_with_all_root_nodes(gray_t & gray) override {
        
//     }

//     bool is_gray_empty(gray_t & gray) override {
//         return gray.size() == 0;
//     }

//     node_t && remove_first_from_gray(gray_t & gray) override {
//         return std::move(node_t());
//     }

//     void add_node_to_gray(gray_t & gray, node_t & node) override {

//     }

//     void add_node_to_black(black_t & black, node_t & node) override {

//     }

//     bool remove_if_white_contains_node(white_t & white, node_t & node) override {
//         return true;
//     }
// };

// template <typename S>
// struct GC {
//     S strategy;

//     typename S::node_t create_node() {
//         return typename S::node_t();
//     }

//     typename S::node_t get_root() {
//         return strategy.get_root();
//     }

//     void collect() {
//         typename S::white_t white;
//         typename S::gray_t gray;
//         typename S::black_t black;

//         mark(white, gray, black);

//     }

//     void mark(typename S::white_t & white, typename S::gray_t & gray, typename S::black_t & black) {
//         strategy.fill_white_with_all_nodes(white);
//         strategy.fill_gray_with_all_root_nodes(gray);

//         while (!strategy.is_gray_empty(gray)) {
//             typename S::node_t node = std::move(strategy.remove_first_from_gray(gray));
//             strategy.add_node_to_black(black, node);
//             auto iterator = node.iterator();
//             while (iterator.has_next()) {
//                 typename S::node_t & current = iterator.next();
//                 if (strategy.remove_if_white_contains_node(white, current)) {
//                     strategy.add_node_to_gray(gray, current);
//                 }
//             }
//         }
//     }
// };

// TEST(PGC, 0) {
//     GC<MockStrategy> mock_gc;
//     mock_gc.collect();
// }

// TEST(PGC, 1) {
//     GC<BasicStrategy> basic_gc;
//     auto root = basic_gc.get_root();
//     root.refs.emplace_back();
//     basic_gc.collect();
// }

#include <managed_object_obj.h>
// #include <gc/gc.h>

// TEST(MPS, 0) {
//     ManagedObjState state;
//     managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

//     for (int i = 0; i < 1; i++) {
//         managed_obj_make(&state, malloc(500));
//     }

//     managed_obj_deinit(&state);
// }

// #include <gc.h>

// TEST(BOEHM_GC, GC_1) {
//     int i;

//     GC_INIT();
//     for (i = 0; i < 1; ++i) {
//         int **p = (int **) GC_MALLOC(sizeof(int *));
//         int *q = (int *) GC_MALLOC_ATOMIC(sizeof(int));
//         assert(*p == 0);
//         *p = (int *) GC_REALLOC(q, 2 * sizeof(int));
//         if (i % 100000 == 0)
//             printf("Heap size = %lu bytes\n",
//                    (unsigned long)GC_get_heap_size());
//         *p = nullptr;
//         p = nullptr;
//         q = nullptr;
//         {
//         auto gc_start = mps_clock();
//         GC_gcollect();
//         auto gc_end = mps_clock();

//       ManagedObjTime gc_duration_time = managed_obj_convert_to_time(gc_end - gc_start);

//     //   printf("Collection start:    %d seconds, %d milliseconds, %d microseconds\n", gc_start_time.seconds, gc_start_time.milliseconds, gc_start_time.microseconds);
//     //   printf("Collection end:      %d seconds, %d milliseconds, %d microseconds\n", gc_end_time.seconds, gc_end_time.milliseconds, gc_end_time.microseconds);
//       printf("Collection duration: %d seconds, %d milliseconds, %d microseconds\n", gc_duration_time.seconds, gc_duration_time.milliseconds, gc_duration_time.microseconds);
//         }
//         for (int i = 0; i < 100; i++)
//             GC_MALLOC(500);
//         {
//         auto gc_start = mps_clock();
//         GC_gcollect();
//         auto gc_end = mps_clock();

//       ManagedObjTime gc_duration_time = managed_obj_convert_to_time(gc_end - gc_start);

//     //   printf("Collection start:    %d seconds, %d milliseconds, %d microseconds\n", gc_start_time.seconds, gc_start_time.milliseconds, gc_start_time.microseconds);
//     //   printf("Collection end:      %d seconds, %d milliseconds, %d microseconds\n", gc_end_time.seconds, gc_end_time.milliseconds, gc_end_time.microseconds);
//       printf("Collection duration: %d seconds, %d milliseconds, %d microseconds\n", gc_duration_time.seconds, gc_duration_time.milliseconds, gc_duration_time.microseconds);
//         }
//     }
// }

// TEST(MPS_collect, 0) {
//     ManagedObjState state;
//     managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

//     for (int i = 0; i < 100; i++) {
//         for (int i = 0; i < 100; i++) {
//             managed_obj_make(&state, malloc(500));
//         }
//         managed_obj_collect(&state);
//     }

//     managed_obj_deinit(&state);
// }

// TEST(BOEHM_GC_collect, 0) {
//     int i;

//     GC_INIT();
//     for (int i = 0; i < 100; i++) {
//         for (int i = 0; i < 100; i++) {
//             GC_MALLOC(500);
//         }

//         auto gc_start = mps_clock();
//         GC_gcollect();
//         auto gc_end = mps_clock();
//         ManagedObjTime gc_duration_time = managed_obj_convert_to_time(gc_end - gc_start);
//         printf("Collection duration: %d seconds, %d milliseconds, %d microseconds\n", gc_duration_time.seconds, gc_duration_time.milliseconds, gc_duration_time.microseconds);
//     }
// }
/*
TEST(MPS_collect2, 1) {
    ManagedObjState state;

    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);
    
    managed_obj_t p1 = managed_obj_make(&state, malloc(500));
    managed_obj_t p2 = managed_obj_make(&state, malloc(500));

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should not collect p

    printf("if p has been collected then we have not managed to track it correctly\n");

    p1 = (managed_obj_t) 0x6; // no longer referenced
    p2 = (managed_obj_t) 0x6; // no longer referenced

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("if p has NOT been collected then we be collected when we destroy the gc\n");

    managed_obj_deinit(&state);

    printf("if p has NOT been collected then we have not managed to track it correctly\n");
}
*/

struct A {
    ~A() {
        printf("~A()\n");
    }
};

struct B_A {
    ~B_A() {
        printf("~B_A() (parent is A)\n");
    }
};

struct B {
    void * a = nullptr;
    ~B() {
        printf("~B() (child is B_A)\n");
    }
};

// TEST(MPS_finalize, 1) {
//     ManagedObjState state;
//     managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);
//     volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new A(), +[](ManagedObjState * state, void * p){ delete static_cast<A*>(p); });
//     managed_obj_deinit(&state);
// }

// TEST(MPS_finalize, 1_null) {
//     ManagedObjState state;
//     managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);
//     volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new A(), +[](ManagedObjState * state, void * p){ delete static_cast<A*>(p); });
//     p1 = nullptr;
//     printf("first collect, p may be collected\n");
//     managed_obj_collect(&state); // should collect the original pointer assigned to p
//     printf("second collect, p may be collected\n");
//     managed_obj_collect(&state); // should collect the original pointer assigned to p
//     managed_obj_deinit(&state);
// }

// TEST(MPS_finalize, 2) {
//     ManagedObjState state;
//     managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

//     volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
//     volatile managed_obj_t p2 = managed_obj_make_scanned_with_finalizer(
//         &state, managed_obj_new<B>(&state),
//         // scanner
//         +[](mps_ss_t ss, void*p) -> mps_res_t {
//             MPS_SCAN_BEGIN(ss) {
//                 MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
//             } MPS_SCAN_END(ss);
//             return MPS_RES_OK;
//         },
//         // finalizer
//         +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
//     );
//     MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

//     printf("second collect, p may be collected\n");
//     managed_obj_collect(&state); // should collect the original pointer assigned to p

//     managed_obj_deinit(&state);
// }
/*
void level_2(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });

    p1 = nullptr;
    p2 = nullptr;

    // p1 can be destroyed now, no references to it should exist
}

void level_1(ManagedObjState & state) {
    level_2(state);
    printf("first collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

TEST(MPS_finalize, 2_null) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

void level_2_(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    p1 = nullptr;
    p2 = nullptr;

    // p1 can be destroyed now, no references to it should exist
}

void level_1_(ManagedObjState & state) {
    level_2_(state);
    printf("\nfirst collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("\nsecond collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("\nthird collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
    printf("\n");
}

TEST(MPS_finalize, 2_null2) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1_(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

void level_2_1(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    p1 = nullptr;
    p2 = nullptr;

    // p1 can be destroyed now, no references to it should exist
}

void level_1_1(ManagedObjState & state) {
    level_2_1(state);
    printf("\nfirst collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("\nsecond collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("\nthird collect, p may be collected\n\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
    printf("\n");
}

TEST(MPS_finalize, 2_null3) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1_1(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

managed_obj_t level_2_lift(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });

    p1 = nullptr;

    // p1 can be destroyed now, no references to it should exist
    return p2;
}

void level_1_lift2(ManagedObjState & state) {
    volatile managed_obj_t p2 = level_2_lift(state);
    printf("first collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

void level_1_lift(ManagedObjState & state) {
    level_1_lift2(state);
    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

TEST(MPS_finalize, 2_null_lift) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1_lift(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

managed_obj_t level_22_lift(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    p1 = nullptr;

    // p1 can be destroyed now, no references to it should exist
    return p2;
}

void level_1_2lift2(ManagedObjState & state) {
    volatile managed_obj_t p2 = level_22_lift(state);
    printf("first collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

void level_1_2lift(ManagedObjState & state) {
    level_1_2lift2(state);
    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

TEST(MPS_finalize, 2_null_2lift) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1_2lift(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

void level_2_1_lift(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    p1 = nullptr;
    p2 = nullptr;

    // p1 can be destroyed now, no references to it should exist
}

managed_obj_t level_23_lift(ManagedObjState & state) {
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t p2 = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    p1 = nullptr;

    // p1 can be destroyed now, no references to it should exist
    return p2;
}

void level_1_3lift2(ManagedObjState & state) {
    volatile managed_obj_t p2 = level_23_lift(state);
    printf("first collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

void level_1_3lift(ManagedObjState & state) {
    level_1_3lift2(state);
    printf("third collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p
}

TEST(MPS_finalize, 2_null_3lift) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    level_1_3lift(state);

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}
*/
int mem_count = 2;
/*
TEST(MPS_finalize, 2_null_user_mem_1) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;
    memory[1] = nullptr;

    printf("forth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("sixth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

TEST(MPS_finalize, 2_null_user_mem_2) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, memory[1])->a = memory[0];

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;
    memory[1] = nullptr;

    printf("forth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("sixth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}
TEST(MPS_finalize, 2_null_user_mem_3) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, memory[1])->a = memory[0];

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;
    memory[1] = nullptr;

    printf("forth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("sixth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

TEST(MPS_finalize, 2_null_user_mem_4) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;

    printf("forth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("sixth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[1] = nullptr;

    printf("seventh collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("eigth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("ninth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

TEST(MPS_finalize, 2_null_user_mem_5) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    memory[1] = managed_obj_make_with_finalizer(&state, managed_obj_new<B>(&state), +[](ManagedObjState * state, void * p){ delete static_cast<B*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, memory[1])->a = memory[0];

    printf("first collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("third collect, p should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;

    printf("forth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fifth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("sixth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[1] = nullptr;

    printf("seventh collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("eigth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("ninth collect, p should be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}
*/

/*
TEST(MPS_finalize, 2_null_user_mem_6) {
    ManagedObjState state;

    managed_obj_t memory[mem_count];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, memory, sizeof(void*)*mem_count);

    managed_obj_collect(&state); // should collect the original pointer assigned to p
    memory[0] = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    managed_obj_collect(&state); // should collect the original pointer assigned to p
    memory[1] = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        //
        // if we do not scan 'a' then 'memory[0]' will be collected
        //   when 'memory[0]' is set to 0x0
        //
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    managed_obj_collect(&state); // should collect the original pointer assigned to p
    MANAGED_OBJECT_SCANNED_CAST(B, memory[1])->a = memory[0];
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[0] = nullptr;

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    memory[1] = nullptr;

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_collect(&state); // should collect the original pointer assigned to p

    // p1 should have been destroyed by now

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

*/

// TEST(MPS_hash_table, 1) {
//     ManagedObjState state;

//     managed_obj_t mem[1];

//     managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, mem, sizeof(void*)*1);

//     mem[0] = managed_obj_metadata_make_table(&state, 16, &managed_obj_metadata_string_hash, &managed_obj_metadata_string_equalp, 0, 1);

//     managed_obj_t string_obj = managed_obj_metadata_make_string(&state, strlen("hello"), "hello");

//     printf("intern 'bye' returns %p\n", managed_obj_metadata_intern(&state, mem[0], "bye"));
    

//     managed_obj_t ref = managed_obj_metadata_table_ref(&state, mem[0], string_obj);
//     printf("find key 'hello' returns %p\n", ref);

//     printf("insert key 'hello'\n");
//     managed_obj_metadata_table_set(&state, mem[0], string_obj, string_obj);

//     ref = managed_obj_metadata_table_ref(&state, mem[0], string_obj);
//     printf("find key 'hello' returns %p\n", ref);

//     managed_obj_deinit(&state); // p1 is actually destroyed here
// }

// TEST(MPS_hash_table, 2) {
//     ManagedObjState state;

//     managed_obj_t mem[3];

//     managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, mem, sizeof(void*)*3);

//     mem[0] = managed_obj_metadata_make_table(&state, 16, &managed_obj_metadata_string_hash, &managed_obj_metadata_string_equalp, 1, 1);
//     mem[1] = managed_obj_metadata_make_string(&state, strlen("hello"), "hello");
//     mem[2] = managed_obj_make_empty(&state);

//     managed_obj_t & key = mem[1];
//     managed_obj_t & value = mem[2];

//     printf("insert key 'hello' %p with value 'hashmap' %p\n", key, value);
//     managed_obj_metadata_table_set(&state, mem[0], key, value);

//     printf("finding key 'hello' %p\n", key);
//     managed_obj_t ref = managed_obj_metadata_table_ref(&state, mem[0], key);
//     printf("find key 'hello' returns %p\n", ref);
//     printf("collecting\n");
//     managed_obj_collect(&state);
//     printf("finding key 'hello' %p\n", key);
//     ref = managed_obj_metadata_table_ref(&state, mem[0], key);
//     printf("find key 'hello' after collect returns %p\n", ref);
//     key = NULL;
//     value = NULL;
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);

//     managed_obj_deinit(&state); // p1 is actually destroyed here
// }

// TEST(MPS_weak, 1) {
//     ManagedObjState state;

//     managed_obj_t mem[2];

//     managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, mem, sizeof(void*)*2);


//     mem[0] = managed_obj_metadata_make_buckets(&state, 1, state.weak_buckets_ap);
//     mem[1] = managed_obj_metadata_make_string(&state, strlen("hello"), "hello");

//     printf("collecting\n");
//     managed_obj_collect(&state);

//     printf("mem[0]->metadata_bucket.bucket[0] = mem[1]\n");
//     mem[0]->metadata_bucket.bucket[0] = mem[1];

//     printf("collecting\n");
//     managed_obj_collect(&state);

//     printf("mem[1] = NULL\n");
//     mem[1] = NULL;
//     printf("collecting\n");
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);
//     managed_obj_collect(&state);

//     managed_obj_deinit(&state); // p1 is actually destroyed here
// }

TEST(MPS_weak, 2) {
    ManagedObjState state;

    managed_obj_t mem[3];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, mem, sizeof(void*)*3);


    mem[0] = managed_obj_metadata_make_buckets(&state, 2, state.weak_buckets_ap);
    mem[1] = managed_obj_metadata_make_string(&state, strlen("hello"), "hello");
    mem[2] = managed_obj_metadata_make_string(&state, strlen("byebye"), "byebye");

    printf("collecting\n");
    managed_obj_collect(&state);

    printf("mem[0]->metadata_bucket.bucket[0] = mem[1]\n");
    mem[0]->metadata_bucket.bucket[0] = mem[1];
    printf("mem[0]->metadata_bucket.bucket[1] = mem[2]\n");
    mem[0]->metadata_bucket.bucket[1] = mem[2];

    printf("collecting\n");
    managed_obj_collect(&state);

    printf("mem[1] = NULL\n");
    mem[1] = NULL;
    printf("mem[2] = NULL\n");
    mem[2] = NULL;
    printf("collecting\n");
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

TEST(MPS_weak, 3) {
    ManagedObjState state;

    managed_obj_t mem[4];

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, mem, sizeof(void*)*4);


    mem[0] = managed_obj_metadata_make_buckets(&state, 1, state.weak_buckets_ap);
    mem[1] = managed_obj_metadata_make_buckets(&state, 1, state.weak_buckets_ap);
    mem[2] = managed_obj_metadata_make_string(&state, strlen("hello"), "hello");
    mem[3] = managed_obj_metadata_make_string(&state, strlen("byebye"), "byebye");

    mem[0]->metadata_bucket.dependent = (managed_obj_metadata_buckets_t) mem[1];
    mem[1]->metadata_bucket.dependent = (managed_obj_metadata_buckets_t) mem[0];

    printf("collecting\n");
    managed_obj_collect(&state);

    printf("mem[0]->metadata_bucket.bucket[0] = mem[2]\n");
    mem[0]->metadata_bucket.bucket[0] = mem[2];
    printf("mem[1]->metadata_bucket.bucket[0] = mem[3]\n");
    mem[1]->metadata_bucket.bucket[0] = mem[3];

    printf("collecting\n");
    managed_obj_collect(&state);

    printf("mem[2] = NULL\n");
    mem[2] = NULL;
    printf("mem[3] = NULL\n");
    mem[3] = NULL;
    printf("collecting\n");
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);
    managed_obj_collect(&state);

    managed_obj_deinit(&state); // p1 is actually destroyed here
}

TEST(MPS_alloc, 1) {
    ManagedObjState state;

    managed_obj_init_with_user_memory(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE, nullptr, 0);

    printf("allocating and pinning 10 thousand objects\n");

    int max = 1000000;
    unsigned long collect_start = mps_clock();
    for (int i = 0; i < max; i++) {
        auto obj = managed_obj_make_empty(&state);

        // unsigned long pin_collect_start = mps_clock();
        // managed_obj_pin(&state, obj);
        // unsigned long pin_collect_end = mps_clock();

        // ManagedObjTime pin_collect_duration_time = managed_obj_convert_to_time(pin_collect_end - pin_collect_start);
        // printf("object %i (pinned in %d seconds, %d milliseconds, %d microseconds)\n\n", i, pin_collect_duration_time.seconds, pin_collect_duration_time.milliseconds, pin_collect_duration_time.microseconds);
        // managed_obj_make_empty(&state);
    }
    unsigned long collect_end = mps_clock();

    ManagedObjTime collect_duration_time = managed_obj_convert_to_time(collect_end - collect_start);

    printf("allocated and pinned 10 thousand objects in %d seconds, %d milliseconds, %d microseconds\n\n", collect_duration_time.seconds, collect_duration_time.milliseconds, collect_duration_time.microseconds);
    managed_obj_deinit(&state); // p1 is actually destroyed here
}

#if false
#include <gc/env/common.h>

// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
#ifndef __GCENV_H__
#define __GCENV_H__

// The sample is to be kept simple, so building the sample
// in tandem with a standalone GC is currently not supported.
#ifdef BUILD_AS_STANDALONE
#undef BUILD_AS_STANDALONE
#endif // BUILD_AS_STANDALONE

#define FEATURE_NATIVEAOT

#if defined(_DEBUG)
#ifndef _DEBUG_IMPL
#define _DEBUG_IMPL 1
#endif
#define ASSERT(_expr) assert(_expr)
#else
#define ASSERT(_expr)
#endif

#ifndef _ASSERTE
#define _ASSERTE(_expr) ASSERT(_expr)
#endif

#include "gcenv.structs.h"
#include "gcenv.base.h"
#include "gcenv.os.h"
#include "gcenv.interlocked.h"
#include "gcenv.interlocked.inl"
#include "gcenv.object.h"
#include "gcenv.sync.h"
#include "gcenv.ee.h"
#include "volatile.h"

#ifdef TARGET_UNIX
#include "gcenv.unix.inl"
#else
#include "gcenv.windows.inl"
#endif

#define MAX_LONGPATH 1024

#ifdef _MSC_VER
#define SUPPRESS_WARNING_4127   \
    __pragma(warning(push))     \
    __pragma(warning(disable:4127)) /* conditional expression is constant*/
#define POP_WARNING_STATE       \
    __pragma(warning(pop))
#else // _MSC_VER
#define SUPPRESS_WARNING_4127
#define POP_WARNING_STATE
#endif // _MSC_VER

#define WHILE_0             \
    SUPPRESS_WARNING_4127   \
    while(0)                \
    POP_WARNING_STATE       \

#define LL_INFO10 4

#define STRESS_LOG_VA(level,msg)                                        do { } WHILE_0
#define STRESS_LOG0(facility, level, msg)                               do { } WHILE_0
#define STRESS_LOG1(facility, level, msg, data1)                        do { } WHILE_0
#define STRESS_LOG2(facility, level, msg, data1, data2)                 do { } WHILE_0
#define STRESS_LOG3(facility, level, msg, data1, data2, data3)          do { } WHILE_0
#define STRESS_LOG4(facility, level, msg, data1, data2, data3, data4)   do { } WHILE_0
#define STRESS_LOG5(facility, level, msg, data1, data2, data3, data4, data5)   do { } WHILE_0
#define STRESS_LOG6(facility, level, msg, data1, data2, data3, data4, data5, data6)   do { } WHILE_0
#define STRESS_LOG7(facility, level, msg, data1, data2, data3, data4, data5, data6, data7)   do { } WHILE_0
#define STRESS_LOG_PLUG_MOVE(plug_start, plug_end, plug_delta)          do { } WHILE_0
#define STRESS_LOG_ROOT_PROMOTE(root_addr, objPtr, methodTable)         do { } WHILE_0
#define STRESS_LOG_ROOT_RELOCATE(root_addr, old_value, new_value, methodTable) do { } WHILE_0
#define STRESS_LOG_GC_START(gcCount, Gen, collectClasses)               do { } WHILE_0
#define STRESS_LOG_GC_END(gcCount, Gen, collectClasses)                 do { } WHILE_0
#define STRESS_LOG_OOM_STACK(size)   do { } while(0)
#define STRESS_LOG_RESERVE_MEM(numChunks) do {} while (0)
#define STRESS_LOG_GC_STACK

#define LOG(x)

#define SVAL_IMPL_INIT(type, cls, var, init) \
    type cls::var = init

//
// Thread
//

struct alloc_context;

class Thread
{
    bool m_fPreemptiveGCDisabled;
    uintptr_t m_alloc_context[16]; // Reserve enough space to fix allocation context

    friend class ThreadStore;
    Thread * m_pNext;

public:
    Thread()
    {
    }

    bool PreemptiveGCDisabled()
    {
        return m_fPreemptiveGCDisabled;
    }

    void EnablePreemptiveGC()
    {
        m_fPreemptiveGCDisabled = false;
    }

    void DisablePreemptiveGC()
    {
        m_fPreemptiveGCDisabled = true;
    }

    alloc_context* GetAllocContext()
    {
        return (alloc_context *)&m_alloc_context;
    }

    void SetGCSpecial()
    {
    }
};

Thread * GetThread();

class ThreadStore
{
public:
    static Thread * GetThreadList(Thread * pThread);

    static void AttachCurrentThread();
};

// -----------------------------------------------------------------------------------------------------------
// Config file enumulation
//

class EEConfig
{
public:
    enum HeapVerifyFlags {
        HEAPVERIFY_NONE = 0,
        HEAPVERIFY_GC = 1,   // Verify the heap at beginning and end of GC
        HEAPVERIFY_BARRIERCHECK = 2,   // Verify the brick table
        HEAPVERIFY_SYNCBLK = 4,   // Verify sync block scanning

                                  // the following options can be used to mitigate some of the overhead introduced
                                  // by heap verification.  some options might cause heap verifiction to be less
                                  // effective depending on the scenario.

        HEAPVERIFY_NO_RANGE_CHECKS = 0x10,   // Excludes checking if an OBJECTREF is within the bounds of the managed heap
        HEAPVERIFY_NO_MEM_FILL = 0x20,   // Excludes filling unused segment portions with fill pattern
        HEAPVERIFY_POST_GC_ONLY = 0x40,   // Performs heap verification post-GCs only (instead of before and after each GC)
        HEAPVERIFY_DEEP_ON_COMPACT = 0x80    // Performs deep object verfication only on compacting GCs.
    };

    enum  GCStressFlags {
        GCSTRESS_NONE = 0,
        GCSTRESS_ALLOC = 1,    // GC on all allocs and 'easy' places
        GCSTRESS_TRANSITION = 2,    // GC on transitions to preemptive GC
        GCSTRESS_INSTR_JIT = 4,    // GC on every allowable JITed instr
        GCSTRESS_INSTR_NGEN = 8,    // GC on every allowable NGEN instr
        GCSTRESS_UNIQUE = 16,   // GC only on a unique stack trace
    };
};

#include "etmdummy.h"
#define ETW_EVENT_ENABLED(e,f) false

class ThreadStressLog
{
public:
    #include "../../inc/gcmsg.inl"
};

#endif // __GCENV_H__

// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.

#include "common.h"

#include "gcenv.h"
#include "gc.h"

EEConfig * g_pConfig;

bool CLREventStatic::CreateManualEventNoThrow(bool bInitialState)
{
    return false;
}

bool CLREventStatic::CreateAutoEventNoThrow(bool bInitialState)
{
    return false;
}

bool CLREventStatic::CreateOSManualEventNoThrow(bool bInitialState)
{
    return false;
}

bool CLREventStatic::CreateOSAutoEventNoThrow(bool bInitialState)
{
    return false;
}

void CLREventStatic::CloseEvent()
{
}

bool CLREventStatic::IsValid() const
{
    return false;
}

bool CLREventStatic::Set()
{
    return false;
}

bool CLREventStatic::Reset()
{
    return false;
}

uint32_t CLREventStatic::Wait(uint32_t dwMilliseconds, bool bAlertable)
{
    return 0;
}

thread_local Thread * pCurrentThread;

Thread * GetThread()
{
    return pCurrentThread;
}

Thread * g_pThreadList = NULL;

Thread * ThreadStore::GetThreadList(Thread * pThread)
{
    if (pThread == NULL)
        return g_pThreadList;

    return pThread->m_pNext;
}

void ThreadStore::AttachCurrentThread()
{
    // TODO: Locks

    Thread * pThread = new Thread();
    pThread->GetAllocContext()->init();
    pCurrentThread = pThread;

    pThread->m_pNext = g_pThreadList;
    g_pThreadList = pThread;
}

void GCToEEInterface::SuspendEE(SUSPEND_REASON reason)
{
    g_theGCHeap->SetGCInProgress(true);

    // TODO: Implement
}

void GCToEEInterface::RestartEE(bool bFinishedGC)
{
    // TODO: Implement

    g_theGCHeap->SetGCInProgress(false);
}

void GCToEEInterface::GcScanRoots(promote_func* fn,  int condemned, int max_gen, ScanContext* sc)
{
    // TODO: Implement - Scan stack roots on given thread
}

void GCToEEInterface::GcStartWork(int condemned, int max_gen)
{
}

void GCToEEInterface::BeforeGcScanRoots(int condemned, bool is_bgc, bool is_concurrent)
{
}

void GCToEEInterface::AfterGcScanRoots(int condemned, int max_gen, ScanContext* sc)
{
}

void GCToEEInterface::GcDone(int condemned)
{
}

bool GCToEEInterface::RefCountedHandleCallbacks(Object * pObject)
{
    return false;
}

bool GCToEEInterface::IsPreemptiveGCDisabled()
{
    Thread* pThread = ::GetThread();
    return pThread->PreemptiveGCDisabled();
}

bool GCToEEInterface::EnablePreemptiveGC()
{
    Thread* pThread = ::GetThread();
    if (pThread && pThread->PreemptiveGCDisabled())
    {
        pThread->EnablePreemptiveGC();
        return true;
    }

    return false;
}

void GCToEEInterface::DisablePreemptiveGC()
{
    Thread* pThread = ::GetThread();
    pThread->DisablePreemptiveGC();
}

Thread* GCToEEInterface::GetThread()
{
    return ::GetThread();
}

gc_alloc_context * GCToEEInterface::GetAllocContext()
{
    Thread* pThread = ::GetThread();
    return pThread->GetAllocContext();
}

void GCToEEInterface::GcEnumAllocContexts (enum_alloc_context_func* fn, void* param)
{
    Thread * pThread = NULL;
    while ((pThread = ThreadStore::GetThreadList(pThread)) != NULL)
    {
        fn(pThread->GetAllocContext(), param);
    }
}

uint8_t* GCToEEInterface::GetLoaderAllocatorObjectForGC(Object* pObject)
{
    return NULL;
}

void GCToEEInterface::SyncBlockCacheWeakPtrScan(HANDLESCANPROC /*scanProc*/, uintptr_t /*lp1*/, uintptr_t /*lp2*/)
{
}

void GCToEEInterface::SyncBlockCacheDemote(int /*max_gen*/)
{
}

void GCToEEInterface::SyncBlockCachePromotionsGranted(int /*max_gen*/)
{
}

void GCToEEInterface::DiagGCStart(int gen, bool isInduced)
{
}

void GCToEEInterface::DiagUpdateGenerationBounds()
{
}

void GCToEEInterface::DiagGCEnd(size_t index, int gen, int reason, bool fConcurrent)
{
}

void GCToEEInterface::DiagWalkFReachableObjects(void* gcContext)
{
}

void GCToEEInterface::DiagWalkSurvivors(void* gcContext, bool fCompacting)
{
}

void GCToEEInterface::DiagWalkUOHSurvivors(void* gcContext, int gen)
{
}

void GCToEEInterface::DiagWalkBGCSurvivors(void* gcContext)
{
}

void GCToEEInterface::StompWriteBarrier(WriteBarrierParameters* args)
{
}

void GCToEEInterface::EnableFinalization(bool gcHasWorkForFinalizerThread)
{
    // Signal to finalizer thread that there are objects to finalize
    // TODO: Implement for finalization
}

void GCToEEInterface::HandleFatalError(unsigned int exitCode)
{
    abort();
}

bool GCToEEInterface::EagerFinalized(Object* obj)
{
    // The sample does not finalize anything eagerly.
    return false;
}

bool GCToEEInterface::GetBooleanConfigValue(const char* privateKey, const char* publicKey, bool* value)
{
    return false;
}

bool GCToEEInterface::GetIntConfigValue(const char* privateKey, const char* publicKey, int64_t* value)
{
    return false;
}

bool GCToEEInterface::GetStringConfigValue(const char* privateKey, const char* publicKey, const char** value)
{
    return false;
}

void GCToEEInterface::FreeStringConfigValue(const char *value)
{
}

bool GCToEEInterface::IsGCThread()
{
    return false;
}

bool GCToEEInterface::WasCurrentThreadCreatedByGC()
{
    return false;
}

static MethodTable freeObjectMT;

MethodTable* GCToEEInterface::GetFreeObjectMethodTable()
{
    //
    // Initialize free object methodtable. The GC uses a special array-like methodtable as placeholder
    // for collected free space.
    //
    freeObjectMT.InitializeFreeObject();
    return &freeObjectMT;
}

bool GCToEEInterface::CreateThread(void (*threadStart)(void*), void* arg, bool is_suspendable, const char* name)
{
    return false;
}

void GCToEEInterface::WalkAsyncPinnedForPromotion(Object* object, ScanContext* sc, promote_func* callback)
{
}

void GCToEEInterface::WalkAsyncPinned(Object* object, void* context, void (*callback)(Object*, Object*, void*))
{
}

uint32_t GCToEEInterface::GetTotalNumSizedRefHandles()
{
    return -1;
}

inline bool GCToEEInterface::AnalyzeSurvivorsRequested(int condemnedGeneration)
{
    return false;
}

inline void GCToEEInterface::AnalyzeSurvivorsFinished(size_t gcIndex, int condemnedGeneration, uint64_t promoted_bytes, void (*reportGenerationBounds)())
{
}

void GCToEEInterface::VerifySyncTableEntry()
{
}

void GCToEEInterface::UpdateGCEventStatus(int currentPublicLevel, int currentPublicKeywords, int currentPrivateLevel, int currentPrivateKeywords)
{
}

uint32_t GCToEEInterface::GetCurrentProcessCpuCount()
{
    return GCToOSInterface::GetTotalProcessorCount();
}

void GCToEEInterface::DiagAddNewRegion(int generation, uint8_t* rangeStart, uint8_t* rangeEnd, uint8_t* rangeEndReserved)
{
}

void GCToEEInterface::LogErrorToHost(const char *message)
{
}

#include <gc/gc.h>
#include <gc/objecthandle.h>
#include <gc/gcdesc.h>

#ifdef TARGET_X86
#define LOCALGC_CALLCONV __cdecl
#else
#define LOCALGC_CALLCONV
#endif

//
// The fast paths for object allocation and write barriers is performance critical. They are often
// hand written in assembly code, etc.
//
Object * AllocateObject(MethodTable * pMT)
{
    alloc_context * acontext = GCToEEInterface::GetThread()->GetAllocContext();
    Object * pObject;

    size_t size = pMT->GetBaseSize();

    uint8_t* result = acontext->alloc_ptr;
    uint8_t* advance = result + size;
    if (advance <= acontext->alloc_limit)
    {
        acontext->alloc_ptr = advance;
        pObject = (Object *)result;
    }
    else
    {
        pObject = g_theGCHeap->Alloc(acontext, size, 0);
        if (pObject == NULL)
            return NULL;
    }

    pObject->RawSetMethodTable(pMT);

    return pObject;
}

#if defined(HOST_64BIT)
// Card byte shift is different on 64bit.
#define card_byte_shift     11
#else
#define card_byte_shift     10
#endif

#define card_byte(addr) (((size_t)(addr)) >> card_byte_shift)

inline void ErectWriteBarrier(Object ** dst, Object * ref)
{
    // if the dst is outside of the heap (unboxed value classes) then we
    //      simply exit
    if (((uint8_t*)dst < g_gc_lowest_address) || ((uint8_t*)dst >= g_gc_highest_address))
        return;

    // volatile is used here to prevent fetch of g_card_table from being reordered
    // with g_lowest/highest_address check above. See comments in StompWriteBarrier
    uint8_t* pCardByte = (uint8_t *)*(volatile uint8_t **)(&g_gc_card_table) + card_byte((uint8_t *)dst);
    if(*pCardByte != 0xFF)
        *pCardByte = 0xFF;
}

void WriteBarrier(Object ** dst, Object * ref)
{
    *dst = ref;
    ErectWriteBarrier(dst, ref);
}

extern "C" HRESULT LOCALGC_CALLCONV GC_Initialize(IGCToCLR* clrToGC, IGCHeap** gcHeap, IGCHandleManager** gcHandleManager, GcDacVars* gcDacVars);

int m() {
    //
    // Initialize system info
    //
    if (!GCToOSInterface::Initialize())
    {
        return -1;
    }

    //
    // Initialize GC heap
    //
    GcDacVars dacVars;
    IGCHeap *pGCHeap;
    IGCHandleManager *pGCHandleManager;
    if (GC_Initialize(nullptr, &pGCHeap, &pGCHandleManager, &dacVars) != S_OK)
    {
        return -1;
    }

    if (FAILED(pGCHeap->Initialize()))
        return -1;

    //
    // Initialize handle manager
    //
    if (!pGCHandleManager->Initialize())
        return -1;

    //
    // Initialize current thread
    //
    ThreadStore::AttachCurrentThread();

    //
    // Create a Methodtable with GCDesc
    //

    class My : Object {
    public:
        Object * m_pOther1;
        int dummy_inbetween;
        Object * m_pOther2;
    };

    static struct My_MethodTable
    {
        // GCDesc
        CGCDescSeries m_series[2];
        size_t m_numSeries;

        // The actual methodtable
        MethodTable m_MT;
    }
    My_MethodTable;

    // 'My' contains the MethodTable*
    uint32_t baseSize = sizeof(My);
    // GC expects the size of ObjHeader (extra void*) to be included in the size.
    baseSize = baseSize + sizeof(ObjHeader);
    // Add padding as necessary. GC requires the object size to be at least MIN_OBJECT_SIZE.
    My_MethodTable.m_MT.m_baseSize = max(baseSize, MIN_OBJECT_SIZE);

    My_MethodTable.m_MT.m_componentSize = 0;    // Array component size
    My_MethodTable.m_MT.m_flags = MTFlag_ContainsPointers;

    My_MethodTable.m_numSeries = 2;

    // The GC walks the series backwards. It expects the offsets to be sorted in descending order.
    My_MethodTable.m_series[0].SetSeriesOffset(offsetof(My, m_pOther2));
    My_MethodTable.m_series[0].SetSeriesCount(1);
    My_MethodTable.m_series[0].seriessize -= My_MethodTable.m_MT.m_baseSize;

    My_MethodTable.m_series[1].SetSeriesOffset(offsetof(My, m_pOther1));
    My_MethodTable.m_series[1].SetSeriesCount(1);
    My_MethodTable.m_series[1].seriessize -= My_MethodTable.m_MT.m_baseSize;

    MethodTable * pMyMethodTable = &My_MethodTable.m_MT;

    // Allocate instance of MyObject
    Object * pObj = AllocateObject(pMyMethodTable);
    if (pObj == NULL)
        return -1;

    // Create strong handle and store the object into it
    OBJECTHANDLE oh = HndCreateHandle(g_HandleTableMap.pBuckets[0]->pTable[GetCurrentThreadHomeHeapNumber()], HNDTYPE_DEFAULT, pObj);
    if (oh == NULL)
        return -1;

    for (int i = 0; i < 1000000; i++)
    {
        Object * pBefore = ((My *)HndFetchHandle(oh))->m_pOther1;

        // Allocate more instances of the same object
        Object * p = AllocateObject(pMyMethodTable);
        if (p == NULL)
            return -1;

        Object * pAfter = ((My *)HndFetchHandle(oh))->m_pOther1;

        // Uncomment this assert to see how GC triggered inside AllocateObject moved objects around
        // assert(pBefore == pAfter);

        // Store the newly allocated object into a field using WriteBarrier
        WriteBarrier(&(((My *)HndFetchHandle(oh))->m_pOther1), p);
    }

    // Create weak handle that points to our object
    OBJECTHANDLE ohWeak = HndCreateHandle(g_HandleTableMap.pBuckets[0]->pTable[GetCurrentThreadHomeHeapNumber()], HNDTYPE_WEAK_DEFAULT, HndFetchHandle(oh));
    if (ohWeak == NULL)
        return -1;

    // Destroy the strong handle so that nothing will be keeping out object alive
    HndDestroyHandle(HndGetHandleTable(oh), HNDTYPE_DEFAULT, oh);

    // Explicitly trigger full GC
    pGCHeap->GarbageCollect();

    // Verify that the weak handle got cleared by the GC
    assert(HndFetchHandle(ohWeak) == NULL);

    printf("Done\n");

    return 0;
}

TEST(dotnet_gc, GC_1) {
    m();
}
#endif
/*
TEST(MPS_finalize, 3) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    volatile managed_obj_t p2 = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    volatile managed_obj_t p1 = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, p2)->a = p1;

    printf("second collect, p may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_deinit(&state);
}

TEST(MPS_finalize, 4) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    volatile managed_obj_t b = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    volatile managed_obj_t b_a = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, b)->a = b_a;

    b_a = nullptr;

    printf("first collect, b should not be collected\n");
    printf("first collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("first collect, b should not be collected\n");
    printf("second collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    b = nullptr;

    printf("third collect, b may be collected\n");
    printf("third collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, b may be collected\n");
    printf("fourth collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_deinit(&state);
}

TEST(MPS_finalize, 5) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    volatile managed_obj_t b_a = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t b = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, b)->a = b_a;

    b_a = nullptr;

    printf("first collect, b should not be collected\n");
    printf("first collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("first collect, b should not be collected\n");
    printf("second collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    b = nullptr;

    printf("third collect, b may be collected\n");
    printf("third collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, b may be collected\n");
    printf("fourth collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_deinit(&state);
}

TEST(MPS_finalize, 6) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    volatile managed_obj_t b = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    volatile managed_obj_t b_a = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    MANAGED_OBJECT_SCANNED_CAST(B, b)->a = b_a;

    b = nullptr;

    printf("first collect, b may be collected\n");
    printf("first collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, b may be collected\n");
    printf("second collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    b_a = nullptr;

    printf("third collect, b may be collected\n");
    printf("third collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, b may be collected\n");
    printf("fourth collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_deinit(&state);
}

TEST(MPS_finalize, 7) {
    ManagedObjState state;
    managed_obj_init(&state, MANAGED_OBJECT_DEFAULT_ARENA_SIZE);

    volatile managed_obj_t b_a = managed_obj_make_with_finalizer(&state, new B_A(), +[](ManagedObjState * state, void * p){ delete static_cast<B_A*>(p); });
    volatile managed_obj_t b = managed_obj_make_scanned_with_finalizer(
        &state, managed_obj_new<B>(&state),
        // scanner
        +[](mps_ss_t ss, void*p) -> mps_res_t {
            MPS_SCAN_BEGIN(ss) {
                MANAGED_OBJECT_FIX(static_cast<B*>(p)->a);
            } MPS_SCAN_END(ss);
            return MPS_RES_OK;
        },
        // finalizer
        +[](ManagedObjState * state, void * p){ managed_obj_delete<B>(state, p); }
    );
    MANAGED_OBJECT_SCANNED_CAST(B, b)->a = b_a;

    b = nullptr;

    printf("first collect, b may be collected\n");
    printf("first collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("second collect, b may be collected\n");
    printf("second collect, b_a should not be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    b_a = nullptr;

    printf("third collect, b may be collected\n");
    printf("third collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    printf("fourth collect, b may be collected\n");
    printf("fourth collect, b_a may be collected\n");
    managed_obj_collect(&state); // should collect the original pointer assigned to p

    managed_obj_deinit(&state);
}
*/
// #include <managed_object.h>

// TEST(references, 0) {
//     ManagedObject object;
// }

// TEST(references, 01) {
//     ManagedObject object;
//     object.dealloc();
// }

// /*

// // kept for reference

// void test_st() {
//     auto root = ManagedObjectHeap::make("root", 2);
//     auto root_ref = ManagedObjectHeap::make("root ref", 1);
//     auto root_copy = ManagedObjectHeap::make("root copy", 1);
//     auto root_real = ManagedObjectHeap::make("root real", 2);
//     auto a_a = ManagedObjectHeap::make("a_a", 3);
//     auto a_b = ManagedObjectHeap::make("a_b", 2);
//     auto c = ManagedObjectHeap::make("c", 3);
//     root->get_memory()[0] = a_a; // reference
//     root->get_memory()[1] = c; // reference
//     a_a->get_memory()[0] = 0x1;
//     a_a->get_memory()[1] = 0x2;
//     a_a->get_memory()[2] = a_b; // reference
//     a_b->get_memory()[0] = 0x3;
//     c->get_memory()[0] = 0x8;
//     c->get_memory()[1] = a_a; // reference
//     c->get_memory()[2] = *a_a; // copy
//     a_b->get_memory()[1] = *c; // copy
//     root_ref->get_memory()[0] = root; // reference
//     root_copy->get_memory()[0] = *root; // copy
//     root_real->get_memory()[0] = root_ref; // reference
//     root_real->get_memory()[1] = root_copy; // reference
//     root_real->print();
//     printf("deallocating c\n");
//     c->dealloc(root_real); // c = nullptr
//     root_real->print();
// }
// */

// template <typename T>
// class BoxedPrimitive : public ManagedObject {

//     MANAGED_OBJECT_STATIC_ASSERT_IS_PRIMITIVE_LIKE(T, "T cannot be a pointer to a ManagedObject, use BoxedVar<T> instead");

//     size_t id;

//     public:

//     BoxedPrimitive() {
//         id = push_value<T>(T());
//     }
//     BoxedPrimitive(const char * id) : ManagedObject(id) {
//         this->id = push_value<T>(T());
//     }

//     BoxedPrimitive(std::function<void(void*)> destructor) {
//         id = push_value<T>(T(), destructor);
//     }
//     BoxedPrimitive(const char * id, std::function<void(void*)> destructor) : ManagedObject(id) {
//         this->id = push_value<T>(T(), destructor);
//     }

//     BoxedPrimitive & operator=(const T & value) {
//         ref() = value;
//         return *this;
//     }

//     BoxedPrimitive & operator=(T && value) {
//         ref() = std::move(value);
//         return *this;
//     }

//     T * operator -> () {
//         return &get_value_at<T>(id);
//     }

//     T & operator * () {
//         return get_value_at<T>(id);
//     }

//     auto & ref() {
//         return operator*();
//     }

//     auto value() {
//         return operator*();
//     }
// };

// template <typename T>
// class BoxedVar : public ManagedObject {
//     size_t id;

//     public:

//     BoxedVar() {
//         id = push_value<T*>(nullptr);
//     }
//     BoxedVar(const char * id) : ManagedObject(id) {
//         this->id = push_value<T*>(nullptr);
//     }

//     BoxedVar(std::function<void(void*)> destructor) {
//         id = push_value<T*>(nullptr, destructor);
//     }
//     BoxedVar(const char * id, std::function<void(void*)> destructor) : ManagedObject(id) {
//         this->id = push_value<T*>(nullptr, destructor);
//     }

//     BoxedVar<T> & operator=(const T & value) {
//         ref() = value;
//         return *this;
//     }

//     BoxedVar<T> & operator=(T && value) {
//         ref() = std::move(value);
//         return *this;
//     }

//     T * operator -> () {
//         T * & ptr = get_value_at<T*>(id);
//         if (ptr != nullptr) {
//             return ptr;
//         } else {
//             // assigns the 'ptr' regardless of what T is
//             // additionally, if T is derived from ManagedObject then it re-root's the allocation to our ManagedObject
//             // returns the given ptr, as the current ptr reference may been invalidated
//             return reroot(id, new T());
//         }
//     }

//     T & operator * () {
//         return *operator->();
//     }

//     auto & ref() {
//         return operator*();
//     }

//     auto value() {
//         return operator*();
//     }
// };

// typedef BoxedPrimitive<int> BoxedInt;
// typedef BoxedVar<BoxedInt> IntegerHolder;

// TEST(references, 0_1) {
//     BoxedInt object;
// }

// TEST(references, 0_2) {
//     IntegerHolder object;
// }

// TEST(references, 0_3) {
//     BoxedInt object;
//     *object;
// }

// TEST(references, 0_4) {
//     IntegerHolder object;
//     *object;
// }

// TEST(references, 1) {
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif
// }

// TEST(references, 2) {
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 3) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 4) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 88\n");
// #endif
//     a = 88;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 88);
//     ASSERT_EQ(b.value(), 88);
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 88);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 5) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_a = h_b\n");
// #endif
//     h_a = h_b;
    
//     BoxedInt a("a");
//     // BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     // a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 1\n");
// #endif
//     // b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     // ASSERT_EQ(a.value(), 1);
//     // ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     // printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     // h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     // ASSERT_EQ(h_a.ref().value(), 2);
//     // ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     // printf("a = b\n");
// #endif
//     // a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     // printf("a = 88\n");
// #endif
//     // a = 88;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     // ASSERT_EQ(a.value(), 88);
//     // ASSERT_EQ(b.value(), 88);
//     // ASSERT_EQ(h_a.ref().value(), 88);
//     // ASSERT_EQ(h_b.ref().value(), 88);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 5_1) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_a = h_b\n");
// #endif
//     h_a = h_b;
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 1\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 2);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 88\n");
// #endif
//     a = 88;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 88);
//     ASSERT_EQ(b.value(), 88);
//     ASSERT_EQ(h_a.ref().value(), 88);
//     ASSERT_EQ(h_b.ref().value(), 88);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// void set_int(BoxedInt var) {
//     var = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void set_int(IntegerHolder var) {
//     var.ref() = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void assign_int(BoxedInt var) {
//     var = BoxedInt("tmp");
//     var = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void assign_int(IntegerHolder var) {
//     var = IntegerHolder("tmp");
//     var.ref() = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// TEST(references, 6) {
//     BoxedInt a("a");
//     a = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     set_int(a);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 7) {
//     IntegerHolder b("holder a");
//     b.ref() = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     set_int(b);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 8) {
//     BoxedInt a("a");
//     a = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     assign_int(a);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 9) {
//     IntegerHolder b("holder a");
//     b.ref() = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     assign_int(b);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif

//     BoxedVar<char> c;
//     c = 'g';

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }

// struct T_A;
// struct T_B;

// struct T_A : ManagedObject {
//     BoxedVar<T_B> b = BoxedVar<T_B>("a.b");

//     T_A() {
//         byRef(b);
//     }

//     T_A(const char * id) : ManagedObject(id) {
//         byRef(b);
//     }
// };
// struct T_B : ManagedObject {
//     BoxedVar<T_A> a = BoxedVar<T_A>("b.a");

//     T_B() {
//         byRef(a);
//     }

//     T_B(const char * id) : ManagedObject(id) {
//         byRef(a);
//     }
// };

// TEST(references, 10) {
//     T_A a("a");
//     T_B b("b");
//     b.a = a;
//     a.b = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }

// typedef BoxedPrimitive<int*> BoxedIntPtr;

// TEST(references, 11) {
//     BoxedInt a("a");
//     a = 8;
//     BoxedInt b = a;
//     BoxedInt c;
//     c = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     b.dealloc();
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     b = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     BoxedIntPtr d("ptr");
//     d = new int();
//     *d.ref() = 1;

//     BoxedVar<BoxedPrimitive<int>> ptr_b("ptr_b");
//     ptr_b = BoxedPrimitive<int>("ptr_b value");
//     *ptr_b = 3;

//     BoxedVar<BoxedPrimitive<int>> ptr_b_c("ptr_b_c");
//     BoxedPrimitive<int> ptr_b_c_v("ptr_b_c value");
//     ptr_b_c = ptr_b_c_v;
//     *ptr_b_c = 3;

//     auto ptr_c = BoxedPrimitive<int>("ptr_c value");
//     ptr_c = 3;

//     auto cptr_a = BoxedVar<ManagedObject>("cycle_a_value");
//     auto cptr_b = BoxedVar<ManagedObject>("cycle_b_value");

//     *cptr_a = cptr_b;
//     *cptr_b = cptr_a;

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif

//     ManagedObject x;
//     *cptr_b = x;

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }

// //------------------------------------------------------------------------------
// // SGCL - a real-time Garbage Collector for C++
// // Copyright (c) 2022, 2023 Sebastian Nibisz
// // SPDX-License-Identifier: Zlib
// //------------------------------------------------------------------------------
// #ifndef SGCL_H
// #define SGCL_H

// #include <any>
// #include <array>
// #include <atomic>
// #include <cassert>
// #include <cstring>
// #include <chrono>
// #include <cstdlib>
// #include <memory>
// #include <thread>
// #include <type_traits>

// // the maximum sleep time of the GC thread in seconds
// #define SGCL_MAX_SLEEP_TIME_SEC 30
// // the percentage amount of allocations that will wake up the GC thread
// #define SGCL_TRIGER_PERCENTAGE 25

// #define SGCL_DEBUG

// #ifdef SGCL_DEBUG
// 	#define SGCL_LOG_PRINT_LEVEL 3
// #endif

// #if SGCL_LOG_PRINT_LEVEL
// 	#include <iostream>
// #endif

// //------------------------------------------------------------------------------
// // Reduces memory usage on x86-64 platforms by using two highest bits of pointer
// //
// // Warning!
// // User heap must be allocated in the low half of virtual address space
// //------------------------------------------------------------------------------
// #if defined(__x86_64__) || defined(_M_X64)
// 	// #define SGCL_ARCH_X86_64
// #endif

// namespace sgcl {
// 	template<class>
// 	class tracked_ptr;

// 	template<class T, size_t N>
// 	class tracked_ptr<T[N]>;

// 	template<class T, class ...A>
// 	auto make_tracked(A&&...);

// 	void terminate_collector() noexcept;

// 	struct metadata_base {
// 		const std::type_info& type;
// 		std::any& user_data;

// 	protected:
// 		metadata_base(const std::type_info& type, std::any& user_data)
// 		: type(type)
// 		, user_data(user_data) {
// 		}
// 	};

// 	template<class T>
// 	struct metadata : metadata_base {
// 		metadata()
// 		: metadata_base(typeid(T), _user_data) {
// 		}
// 		template<class U>

// 		static void set(U t) noexcept {
// 			_user_data = std::move(t);
// 		}

// 		static const std::any& get() noexcept {
// 			return _user_data;
// 		}

// 	private:
// 		inline static std::any _user_data = nullptr;
// 	};

// 	namespace Priv {
// 		static constexpr size_t SqrMaxTypeNumber = 64;
// 		[[maybe_unused]] static constexpr size_t MaxTypeNumber = SqrMaxTypeNumber * SqrMaxTypeNumber;
// 		static constexpr ptrdiff_t MaxStackOffset = 1024;
// 		static constexpr size_t PageSize = 4096;
// 		static constexpr size_t PageDataSize = PageSize - sizeof(uintptr_t);
// 		using Pointer = std::atomic<void*>;
// 		using Const_pointer = std::atomic<const void*>;
// 	}

// 	[[maybe_unused]] static constexpr size_t MaxAliasingDataSize = Priv::PageDataSize;

// 	namespace Priv {
// 		struct States {
// 			using Value = uint8_t;
// 			static constexpr Value Unused = std::numeric_limits<uint8_t>::max();
// 			static constexpr Value BadAlloc = Unused - 1;
// 			static constexpr Value AtomicReachable = BadAlloc - 1;
// 			static constexpr Value Reachable = 1;
// 			static constexpr Value Used = 0;
// 		};

// 		struct Array_metadata;
// 		struct Array_base {
// 			Array_base(size_t c) noexcept
// 			: count(c) {
// 			}

// 			~Array_base() noexcept;

// 			template<class T>
// 			static void destroy(void* data, size_t count) noexcept {
// 				for (size_t i = count; i > 0; --i) {
// 					std::destroy_at((T*)data + i - 1);
// 				}
// 			}

// 			std::atomic<Array_metadata*> metadata = {nullptr};
// 			const size_t count;

// 			tracked_ptr<void>& next() noexcept {
// 				return (tracked_ptr<void>&)_next;
// 			}

// 			tracked_ptr<void>& prev() noexcept {
// 				return (tracked_ptr<void>&)_prev;
// 			}

// 		private:
// #ifdef SGCL_ARCH_X86_64
// 			uintptr_t _prev;
// 			uintptr_t _next;
// #else
// 			uintptr_t _prev[2];
// 			uintptr_t _next[2];
// #endif
// 		};

// 		struct Page;

// 		template<class T>
// 		struct Page_info;

// 		template<class T>
// 		tracked_ptr<void> Clone(const void*);

// 		struct Metadata {
// 			template<class T>
// 			Metadata(T*)
// 			: pointer_offsets(Page_info<T>::pointer_offsets)
// 			, destroy(!std::is_trivially_destructible_v<T> || std::is_base_of_v<Array_base, T> ? Page_info<T>::destroy : nullptr)
// 			, free(Page_info<T>::Heap_allocator::free)
// 			, clone(!std::is_base_of_v<Array_base, T> ? Clone<T> : nullptr)
// 			, object_size(Page_info<T>::ObjectSize)
// 			, object_count(Page_info<T>::ObjectCount)
// 			, is_array(std::is_base_of_v<Array_base, T>)
// 			, public_metadata(Page_info<T>::public_metadata()) {
// 			}

// 			std::atomic<ptrdiff_t*>& pointer_offsets;
// 			void (*const destroy)(void*) noexcept;
// 			void (*const free)(Page*);
// 			tracked_ptr<void> (*const clone)(const void*);
// 			const size_t object_size;
// 			const unsigned object_count;
// 			bool is_array;
// 			metadata_base& public_metadata;
// 			Page* empty_page = {nullptr};
// 			Metadata* next = {nullptr};
// 		};

// 		class Tracked_ptr;
// 		struct Array_metadata {
// 			template<class T>
// 			Array_metadata(T*)
// 			: pointer_offsets(Page_info<T>::pointer_offsets)
// 			, destroy(!std::is_trivially_destructible_v<T> ? Array_base::destroy<T> : nullptr)
// 			, object_size(Page_info<T>::ObjectSize)
// 			, public_metadata(Page_info<T[]>::public_metadata())
// 			, tracked_ptrs_only(std::is_base_of_v<Tracked_ptr, T>) {
// 			}

// 			std::atomic<ptrdiff_t*>& pointer_offsets;
// 			void (*const destroy)(void*, size_t) noexcept;
// 			const size_t object_size;
// 			metadata_base& public_metadata;
// 			const bool tracked_ptrs_only;
// 		};

// 		Array_base::~Array_base() noexcept {
// 			auto metadata = this->metadata.load(std::memory_order_acquire);
// 			if (metadata && metadata->destroy) {
// 				metadata->destroy(this + 1, count);
// 			}
// 		}

// 		struct Block;
// 		struct Page {
// 			using State = States::Value;
// 			using Flag = uint64_t;
// 			static constexpr unsigned FlagBitCount = sizeof(Flag) * 8;

// 			struct Flags {
// 				Flag registered = {0};
// 				Flag reachable = {0};
// 				Flag marked = {0};
// 			};

// 			template<class T>
// 			Page(Block* block, T* data) noexcept
// 			: metadata(&Page_info<T>::private_metadata())
// 			, block(block)
// 			, data((uintptr_t)data)
// 			, multiplier((1ull << 32 | 0x10000) / metadata->object_size) {
// 				assert(metadata != nullptr);
// 				assert(data != nullptr);
// 				auto states = this->states();
// 				for (unsigned i = 0; i < metadata->object_count; ++i) {
// 					new(states + i) std::atomic<State>(States::Used);
// 				}
// 				auto flags = this->flags();
// 				auto count = this->flags_count();
// 				for (unsigned i = 0; i < count; ++i) {
// 					new (flags + i) Flags;
// 				}
// 			}

// 			~Page() {
// 				if constexpr(!std::is_trivially_destructible_v<std::atomic<State>>) {
// 					auto states = this->states();
// 					std::destroy(states, states + metadata->object_count);
// 				}
// 			}

// 			std::atomic<State>* states() const noexcept {
// 				return (std::atomic<State>*)(this + 1);
// 			}

// 			Flags* flags() const noexcept {
// 				auto states_size = (sizeof(std::atomic<State>) * metadata->object_count + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);
// 				return (Flags*)((uintptr_t)states() + states_size);
// 			}

// 			unsigned flags_count() const noexcept {
// 				return (metadata->object_count + FlagBitCount - 1) / FlagBitCount;
// 			}

// 			void clear_flags() noexcept {
// 				auto flags = this->flags();
// 				auto count = flags_count();
// 				for (unsigned i = 0; i < count; ++i) {
// 					flags[i].reachable = 0;
// 					flags[i].marked = 0;
// 				}
// 			}

// 			static constexpr unsigned flag_index_of(unsigned i) noexcept {
// 				return i / FlagBitCount;
// 			}

// 			static constexpr Flag flag_mask_of(unsigned i) noexcept {
// 				return Flag(1) << (i % FlagBitCount);
// 			}

// 			unsigned index_of(const void* p) noexcept {
// 				return ((uintptr_t)p - data) * multiplier >> 32;
// 			}

// 			uintptr_t data_of(unsigned index) noexcept {
// 				return data + index * metadata->object_size;
// 			}

// 			static Page* page_of(const void* p) noexcept {
// 				auto page = ((uintptr_t)p & ~(uintptr_t)(PageSize - 1));
// 				return *((Page**)page);
// 			}

// 			static Metadata& metadata_of(const void* p) noexcept {
// 				auto page = Page::page_of(p);
// 				return *page->metadata;
// 			}

// 			static void* base_address_of(const void* p) noexcept {
// 				auto page = page_of(p);
// 				auto index = page->index_of(p);
// 				return (void*)page->data_of(index);
// 			}

// 			static void set_state(const void* p, State s) noexcept {
// 				auto page = Page::page_of(p);
// 				auto index = page->index_of(p);
// 				auto &state = page->states()[index];
// 				state.store(s, std::memory_order_release);
// 			}

// 			Metadata* const metadata;
// 			Block* const block;
// 			const uintptr_t data;
// 			const uint64_t multiplier;
// 			bool reachable = {false};
// 			bool registered = {false};
// 			bool is_used = {true};
// 			std::atomic_bool on_empty_list = {false};
// 			Page* next_reachable = {nullptr};
// 			Page* next_registered = {nullptr};
// 			Page* next_empty = {nullptr};
// 			Page* next = {nullptr};
// 		};

// 		struct Pointer_pool_base {
// 			Pointer_pool_base(unsigned s, unsigned o)
// 			: _size(s)
// 			, _offset(o)
// 			, _position(s) {
// 			}
// 			void fill(void* data) {
// 				for(auto i = 0u; i < _size; ++i, data = (void*)((uintptr_t)data + _offset)) {
// 					_indexes[i] = data;
// 				}
// 				_position = 0;
// 			}
// 			void fill(Page* page) {
// 				auto data = page->data;
// 				auto object_size = page->metadata->object_size;
// 				auto states = page->states();
// 				auto count = page->metadata->object_count;
// 				for(int i = count - 1; i >= 0; --i) {
// 					if (states[i].load(std::memory_order_relaxed) == States::Unused) {
// 						_indexes[--_position] = (void*)(data + i * object_size);
// 						states[i].store(States::Used, std::memory_order_relaxed);
// 					}
// 				}
// 			}
// 			unsigned pointer_count() const noexcept {
// 				return _size - _position;
// 			}
// 			bool is_empty() const noexcept {
// 				return _position == _size;
// 			}
// 			bool is_full() const noexcept {
// 				return _position == 0;
// 			}
// 			void* alloc() noexcept {
// 				return _indexes[_position++];
// 			}
// 			void free(void* p) noexcept {
// 				_indexes[--_position] = p;
// 			}

// 		protected:
// 			void** _indexes;

// 		private:
// 			const unsigned _size;
// 			const unsigned _offset;
// 			unsigned _position;
// 		};

// 		template<unsigned Size, unsigned Offset>
// 		struct Pointer_pool : Pointer_pool_base {
// 			constexpr Pointer_pool()
// 			: Pointer_pool_base(Size, Offset) {
// 				Pointer_pool_base::_indexes = _indexes.data();
// 			}
// 			Pointer_pool(void* data)
// 			: Pointer_pool() {
// 				Pointer_pool_base::_indexes = _indexes.data();
// 				fill(data);
// 			}

// 			Pointer_pool* next = nullptr;

// 		private:
// 			std::array<void*, Size> _indexes;
// 		};

// 		template<class>
// 		struct Large_object_allocator;

// 		template<class>
// 		struct Small_object_allocator;

// 		template<class T>
// 		struct Page_info {
// 			static constexpr size_t ObjectSize = sizeof(std::remove_extent_t<std::conditional_t<std::is_same_v<T, void>, char, T>>);
// 			static constexpr size_t ObjectCount = std::max(size_t(1), PageDataSize / ObjectSize);
// 			static constexpr size_t StatesSize = (sizeof(std::atomic<Page::State>) * ObjectCount + sizeof(uintptr_t) - 1) & ~(sizeof(uintptr_t) - 1);
// 			static constexpr size_t FlagsCount = (ObjectCount + Page::FlagBitCount - 1) / Page::FlagBitCount;
// 			static constexpr size_t FlagsSize = sizeof(Page::Flags) * FlagsCount;
// 			static constexpr size_t HeaderSize = sizeof(Page) + StatesSize + FlagsSize;
// 			using Heap_allocator = std::conditional_t<ObjectSize <= PageDataSize, Small_object_allocator<T>, Large_object_allocator<T>>;

// 			static void destroy(void* p) noexcept {
// 				std::destroy_at((T*)p);
// 			}

// 			inline static auto& public_metadata() {
// 				static auto metadata = new sgcl::metadata<T>;
// 				return *metadata;
// 			}

// 			inline static auto& private_metadata() {
// 				static auto metadata = new Metadata((std::remove_extent_t<T>*)0);
// 				return *metadata;
// 			}

// 			inline static auto& array_metadata() {
// 				static auto metadata = new Array_metadata((std::remove_extent_t<std::conditional_t<std::is_same_v<T, void>, char, T>>*)0);
// 				return *metadata;
// 			}

// 			inline static std::atomic<ptrdiff_t*> pointer_offsets = nullptr;
// 		};

// 		template<size_t Size = 1>
// 		struct Array : Array_base {
// 			constexpr Array(size_t c) noexcept
// 			: Array_base(c) {
// 			}
// 			template<class T>
// 			void init() {
// 				if constexpr(!std::is_trivial_v<T>) {
// 					_init<T>();
// 				}
// 				using Info = Page_info<std::remove_cv_t<T>>;
// 				metadata.store(&Info::array_metadata(), std::memory_order_release);
// 			}
// 			template<class T>
// 			void init(const T& v) {
// 				_init<T>(v);
// 				using Info = Page_info<std::remove_cv_t<T>>;
// 				metadata.store(&Info::array_metadata(), std::memory_order_release);
// 			}
// 			char data[Size];

// 		private:
// 			template<class T, class... A>
// 			void _init(A&&... a);
// 		};

// 		template<>
// 		struct Page_info<Array<>> : public Page_info<Array<PageDataSize>> {
// 			using Heap_allocator = Large_object_allocator<Array<>>;

// 			static void destroy(void* p) noexcept {
// 				std::destroy_at((Array<>*)p);
// 			}
// 		};

// 		struct Block {
// 			static constexpr size_t PageCount = 15;

// 			Block() noexcept {
// 				void* data = this + 1;
// 				for (size_t i = 0; i < PageCount; ++i) {
// 					*((Block**)data) = this;
// 					data = (void*)((uintptr_t)data + PageSize);
// 				}
// 			}

// 			static void* operator new(size_t) {
// 				void* mem = ::operator new(sizeof(uintptr_t) + sizeof(Block) + PageSize * (PageCount + 1));
// 				uintptr_t addres = (uintptr_t)mem + sizeof(uintptr_t) + sizeof(Block) + PageSize;
// 				addres = addres & ~(PageSize - 1);
// 				void* ptr = (void*)addres;
// 				Block* block = (Block*)ptr - 1;
// 				*((void**)block - 1) = mem;
// 				return block;
// 			}

// 			static void operator delete(void* p, size_t) noexcept {
// 				::operator delete(*((void**)p - 1));
// 			}

// 			Block* next = {nullptr};
// 			unsigned page_count = {0};
// 		};

// 		struct Block_allocator {
// 			using Pointer_pool = Priv::Pointer_pool<Block::PageCount, PageSize>;

// 			~Block_allocator() noexcept {
// 				void* page = nullptr;
// 				while (!_pointer_pool.is_empty()) {
// 					auto data = _pointer_pool.alloc();
// 					*((void**)data + 1) = page;
// 					page = (void*)data;
// 				}
// 				if (page) {
// 					free(page);
// 				}
// 			}

// 			void* alloc() {
// 				if (_pointer_pool.is_empty()) {
// 					auto page = _empty_pages.load(std::memory_order_acquire);
// 					while(page && !_empty_pages.compare_exchange_weak(page, *((void**)page + 1), std::memory_order_relaxed, std::memory_order_acquire));
// 					if (page) {
// 						return page;
// 					} else {
// 						auto block = new Block;
// 						_pointer_pool.fill(block + 1);
// 					}
// 				}
// 				return _pointer_pool.alloc();
// 			}

// 			static void free(void* page) {
// 				auto last = page;
// 				while(*((void**)last + 1)) {
// 					last = *((void**)last + 1);
// 				}
// 				*((void**)last + 1) = _empty_pages.exchange(nullptr, std::memory_order_acquire);
// 				Block* block = nullptr;
// 				auto p = page;
// 				while(p) {
// 					Block* b = *((Block**)p);
// 					if (!b->page_count) {
// 						b->next = block;
// 						block = b;
// 					}
// 					++b->page_count;
// 					p = *((void**)p + 1);
// 				}
// 				void* prev = nullptr;
// 				p = page;
// 				while(p) {
// 					auto next = *((void**)p + 1);
// 					Block* b = *((Block**)p);
// 					if (b->page_count == Block::PageCount) {
// 						if (!prev) {
// 							page = next;
// 						} else {
// 							*((void**)prev + 1) = next;
// 						}
// 					} else {
// 						prev = p;
// 					}
// 					p = next;
// 				}
// 				_empty_pages.store(page, std::memory_order_release);
// 				while(block) {
// 					auto next = block->next;
// 					if (block->page_count == Block::PageCount) {
// 						delete block;
// 					} else {
// 						block->page_count = 0;
// 					}
// 					block = next;
// 				}
// 			}

// 		private:
// 			inline static std::atomic<void*> _empty_pages = {nullptr};
// 			Pointer_pool _pointer_pool;
// 		};

// 		struct Heap_allocator_base {
// 			virtual ~Heap_allocator_base() noexcept = default;
// 			inline static std::atomic<Page*> pages = {nullptr};
// 		};

// 		template<class T>
// 		struct Large_object_allocator : Heap_allocator_base {
// 			T* alloc(size_t size = 0) const {
// 				auto mem = ::operator new(sizeof(T) + size + sizeof(uintptr_t), std::align_val_t(PageSize));
// 				auto data = (T*)((uintptr_t)mem + sizeof(uintptr_t));
// 				auto hmem = ::operator new(Page_info<T>::HeaderSize);
// 				auto page = new(hmem) Page(nullptr, data);
// 				*((Page**)mem) = page;
// 				page->next = pages.load(std::memory_order_relaxed);
// 				while(!pages.compare_exchange_weak(page->next, page, std::memory_order_release, std::memory_order_relaxed));
// 				return data;
// 			}

// 			static void free(Page* pages) noexcept {
// 				Page* page = pages;
// 				while(page) {
// 					auto data = (void*)(page->data - sizeof(uintptr_t));
// 					::operator delete(data, std::align_val_t(PageSize));
// 					page->is_used = false;
// 					page = page->next_empty;
// 				}
// 			}
// 		};

// 		struct Small_object_allocator_base : Heap_allocator_base {
// 			Small_object_allocator_base(Block_allocator& ba, Pointer_pool_base& pa, std::atomic<Page*>& pb) noexcept
// 			: _block_allocator(ba)
// 			, _pointer_pool(pa)
// 			, _pages_buffer(pb) {
// 			}

// 			~Small_object_allocator_base() noexcept override {
// 				while (!_pointer_pool.is_empty()) {
// 					auto ptr = _pointer_pool.alloc();
// 					auto index = _current_page->index_of(ptr);
// 					_current_page->states()[index].store(States::Unused, std::memory_order_relaxed);
// 				}
// 			}

// 			void* alloc(size_t) {
// 				if  (_pointer_pool.is_empty()) {
// 					auto page = _pages_buffer.load(std::memory_order_acquire);
// 					while(page && !_pages_buffer.compare_exchange_weak(page, page->next_empty, std::memory_order_relaxed, std::memory_order_acquire));
// 					if (page) {
// 						_pointer_pool.fill(page);
// 						page->on_empty_list.store(false, std::memory_order_relaxed);
// 					} else {
// 						page = _alloc_page();
// 						_pointer_pool.fill((void*)(page->data));
// 						page->next = pages.load(std::memory_order_relaxed);
// 						while(!pages.compare_exchange_weak(page->next, page, std::memory_order_release, std::memory_order_relaxed));
// 					}
// 					_current_page = page;
// 				}
// 				assert(!_pointer_pool.is_empty());
// 				return (void*)_pointer_pool.alloc();
// 			}

// 		private:
// 			Block_allocator& _block_allocator;
// 			Pointer_pool_base& _pointer_pool;
// 			std::atomic<Page*>& _pages_buffer;
// 			Page* _current_page = {nullptr};

// 			virtual Page* _create_page_parameters(Block* block, void* data) = 0;

// 			Page* _alloc_page() {
// 				auto mem = _block_allocator.alloc();
// 				auto block = *((Block**)mem);
// 				auto data = (void*)((uintptr_t)mem + sizeof(uintptr_t));
// 				auto page = _create_page_parameters(block, data);
// 				*((Page**)mem) = page;
// 				return page;
// 			}

// 		protected:
// 			static void _remove_empty(Page*& pages, Page*& empty_pages) noexcept {
// 				auto page = pages;
// 				Page* prev = nullptr;
// 				while(page) {
// 					auto next = page->next_empty;
// 					auto states = page->states();
// 					auto object_count = page->metadata->object_count;
// 					auto unused_count = 0u;
// 					for (unsigned i = 0; i < object_count; ++i) {
// 						auto state = states[i].load(std::memory_order_relaxed);
// 						if (state == States::Unused) {
// 							++unused_count;
// 						}
// 					}
// 					if (unused_count == object_count) {
// 						page->next_empty = empty_pages;
// 						empty_pages = page;
// 						if (!prev) {
// 							pages = next;
// 						} else {
// 							prev->next_empty = next;
// 						}
// 					} else {
// 						prev = page;
// 					}
// 					page = next;
// 				}
// 			}

// 			static void _free(Page* pages) noexcept {
// 				Page* page = pages;
// 				void* empty = nullptr;
// 				while(page) {
// 					auto data = page->data - sizeof(uintptr_t);
// 					*((Block**)data) = page->block;
// 					page->is_used = false;
// 					*((void**)data + 1) = empty;
// 					empty = (void*)data;
// 					page = page->next_empty;
// 				}
// 				Block_allocator::free(empty);
// 			}

// 			static void _free(Page* pages, std::atomic<Page*>& pages_buffer) noexcept {
// 				Page* empty_pages = nullptr;
// 				_remove_empty(pages, empty_pages);
// 				pages = pages_buffer.exchange(pages, std::memory_order_relaxed);
// 				_remove_empty(pages, empty_pages);
// 				pages = pages_buffer.exchange(pages, std::memory_order_relaxed);
// 				if (pages) {
// 					auto last = pages;
// 					while(last->next_empty) {
// 						last = last->next_empty;
// 					}
// 					last->next_empty = pages_buffer.load(std::memory_order_relaxed);
// 					while(!pages_buffer.compare_exchange_weak(last->next_empty, pages, std::memory_order_release, std::memory_order_relaxed));
// 				}
// 				if (empty_pages) {
// 					_free(empty_pages);
// 				}
// 			}
// 		};

// 		template<class T>
// 		struct Small_object_allocator : Small_object_allocator_base {
// 			using Pointer_pool = Priv::Pointer_pool<Page_info<T>::ObjectCount, sizeof(std::conditional_t<std::is_same_v<T, void>, char, T>)>;

// 			constexpr Small_object_allocator(Block_allocator& a) noexcept
// 			: Small_object_allocator_base(a, _pointer_pool, _pages_buffer) {
// 			}

// 			static void free(Page* pages) {
// 				_free(pages, _pages_buffer);
// 			}

// 		private:
// 			inline static std::atomic<Page*> _pages_buffer = {nullptr};
// 			Pointer_pool _pointer_pool;

// 			Page* _create_page_parameters(Block* block, void* data) override {
// 				auto mem = ::operator new(Page_info<T>::HeaderSize);
// 				auto page = new(mem) Page(block, (T*)data);
// 				return page;
// 			}
// 		};

// 		struct Roots_allocator {
// 			static constexpr size_t PointerCount = PageSize / sizeof(Pointer);
// 			using Page = std::array<Pointer, PointerCount>;
// 			using Pointer_pool = Priv::Pointer_pool<PointerCount, sizeof(Pointer)>;

// 			struct Page_node {
// 				Page_node* next = {nullptr};
// 				Page page = {nullptr};
// 			};

// 			constexpr Roots_allocator() noexcept
// 			:_pointer_pool({nullptr, nullptr}) {
// 			}

// 			~Roots_allocator() noexcept {
// 				for(auto pointer_pool : _pointer_pool) {
// 					if (pointer_pool) {
// 						auto& pool = pointer_pool->is_empty() ? _global_empty_pointer_pool : _global_pointer_pool;
// 						pointer_pool->next = pool.load(std::memory_order_acquire);
// 						while(!pool.compare_exchange_weak(pointer_pool->next, pointer_pool, std::memory_order_release, std::memory_order_relaxed));
// 					}
// 				}
// 			}

// 			Pointer* alloc() {
// 				auto [pool1, pool2] = _pointer_pool;
// 				if (pool1 && !pool1->is_empty()) {
// 					return (Pointer*)pool1->alloc();
// 				}
// 				if (pool2 && !pool2->is_empty()) {
// 					_pointer_pool = {pool2, pool1};
// 					return (Pointer*)pool2->alloc();
// 				}
// 				auto new_pool = _global_pointer_pool.load(std::memory_order_acquire);
// 				while(new_pool && !_global_pointer_pool.compare_exchange_weak(new_pool, new_pool->next, std::memory_order_release, std::memory_order_acquire));
// 				if (!new_pool) {
// 					auto node = new Page_node;
// 					new_pool = new Pointer_pool(node->page.data());
// 					node->next = pages.load(std::memory_order_acquire);
// 					while(!pages.compare_exchange_weak(node->next, node, std::memory_order_release, std::memory_order_relaxed));
// 				}
// 				if (pool1) {
// 					if (pool2) {
// 						pool2->next = _global_empty_pointer_pool.load(std::memory_order_acquire);
// 						while(!_global_empty_pointer_pool.compare_exchange_weak(pool2->next, pool2, std::memory_order_release, std::memory_order_relaxed));
// 					}
// 					pool2 = pool1;
// 				}
// 				pool1 = new_pool;
// 				_pointer_pool = {pool1, pool2};
// 				return (Pointer*)pool1->alloc();
// 			}

// 			void free(Pointer* p) noexcept {
// 				auto [pool1, pool2] = _pointer_pool;
// 				if (pool1 && !pool1->is_full()) {
// 					pool1->free(p);
// 					return;
// 				}
// 				if (pool2 && !pool2->is_full()) {
// 					pool2->free(p);
// 					_pointer_pool = {pool2, pool1};
// 					return;
// 				}
// 				auto new_pool = _global_empty_pointer_pool.load(std::memory_order_acquire);
// 				while(new_pool && !_global_empty_pointer_pool.compare_exchange_weak(new_pool, new_pool->next, std::memory_order_release, std::memory_order_acquire));
// 				if (!new_pool) {
// 					new_pool = new Pointer_pool();
// 				}
// 				if (pool1) {
// 					if (pool2) {
// 						pool2->next = _global_pointer_pool.load(std::memory_order_acquire);
// 						while(!_global_pointer_pool.compare_exchange_weak(pool2->next, pool2, std::memory_order_release, std::memory_order_relaxed));
// 					}
// 					pool2 = pool1;
// 				}
// 				pool1 = new_pool;
// 				_pointer_pool = {pool1, pool2};
// 				pool1->free(p);
// 			}

// 			inline static std::atomic<Page_node*> pages = {nullptr};

// 		private:
// 			std::array<Pointer_pool*, 2> _pointer_pool;
// 			inline static std::atomic<Pointer_pool*> _global_pointer_pool = {nullptr};
// 			inline static std::atomic<Pointer_pool*> _global_empty_pointer_pool = {nullptr};
// 		};

// 		struct Stack_allocator {
// 			static constexpr unsigned PageCount = 256;
// 			static constexpr size_t PointerCount = PageSize / sizeof(Pointer);
// 			using Page = std::array<Pointer, PointerCount>;

// 			~Stack_allocator() noexcept {
// 				for (auto& page : pages) {
// 					delete page.load(std::memory_order_relaxed);
// 				}
// 			}

// 			Pointer* alloc(void* p) {
// 				auto index = ((uintptr_t)p / PageSize) % PageCount;
// 				auto page = pages[index].load(std::memory_order_relaxed);
// 				if (!page) {
// 					page = new Page{nullptr};
// 					pages[index].store(page, std::memory_order_release);
// 				}
// 				auto offset = ((uintptr_t)p % PageSize) / sizeof(Pointer);
// 				return &(*page)[offset];
// 			}

// 			std::atomic<Page*> pages[PageCount] = {nullptr};
// 		};

// 		struct Thread {
// 			struct Data {
// 				Data(Block_allocator* b, Stack_allocator* s) noexcept
// 				: block_allocator(b)
// 				, stack_allocator(s) {
// 				}
// 				std::unique_ptr<Block_allocator> block_allocator;
// 				std::unique_ptr<Stack_allocator> stack_allocator;
// 				std::atomic<bool> is_used = {true};
// 				Const_pointer recursive_alloc_pointer = {nullptr};
// 				const void* last_recursive_alloc_pointer = {nullptr};
// 				std::atomic<int64_t> alloc_count = {0};
// 				std::atomic<int64_t> alloc_size = {0};
// 				Data* next = {nullptr};
// 			};

// 			struct Alloc_state {
// 				std::pair<uintptr_t, uintptr_t> range;
// 				size_t size;
// 				Pointer** ptrs;
// 			};

// 			Thread()
// 			:	block_allocator(new Block_allocator)
// 			, stack_allocator(new Stack_allocator)
// 			, roots_allocator(new Roots_allocator)
// 			, _data(new Data{block_allocator, stack_allocator}){
// #if SGCL_LOG_PRINT_LEVEL >= 3
// 				std::cout << "[sgcl] start thread id: " << std::this_thread::get_id() << std::endl;
// #endif
// 				_data->next = threads_data.load(std::memory_order_acquire);
// 				while(!threads_data.compare_exchange_weak(_data->next, _data, std::memory_order_release, std::memory_order_relaxed));
// 			}

// 			~Thread();

// 			template<class T>
// 			auto& alocator() {
// 				if constexpr(std::is_same_v<typename Page_info<T>::Heap_allocator, Small_object_allocator<T>>) {
// 					static unsigned index = _type_index++;
// 					assert(index < MaxTypeNumber);
// 					auto ax = heaps.get();
// 					if (!ax) {
// 						ax = new std::array<std::unique_ptr<std::array<std::unique_ptr<Heap_allocator_base>, SqrMaxTypeNumber>>, SqrMaxTypeNumber>;
// 						heaps.reset(ax);
// 					}
// 					auto& ay = (*ax)[index / SqrMaxTypeNumber];
// 					if (!ay) {
// 						ay.reset(new std::array<std::unique_ptr<Heap_allocator_base>, SqrMaxTypeNumber>);
// 					}
// 					auto& alocator = (*ay)[index % SqrMaxTypeNumber];
// 					if (!alocator) {
// 						alocator.reset(new Small_object_allocator<T>(*block_allocator));
// 					}
// 					return static_cast<Small_object_allocator<T>&>(*alocator);
// 				}
// 				else {
// 					static Large_object_allocator<T> allocator;
// 					return allocator;
// 				}
// 			}

// 			bool set_recursive_alloc_pointer(const void* p) noexcept {
// 				if (!_data->recursive_alloc_pointer.load(std::memory_order_relaxed)) {
// 					_data->recursive_alloc_pointer.store(p, std::memory_order_relaxed);
// 					return true;
// 				}
// 				return false;
// 			}

// 			void clear_recursive_alloc_pointer() noexcept {
// 				_data->recursive_alloc_pointer.store(nullptr, std::memory_order_relaxed);
// 			}

// 			void update_allocated(size_t s) {
// 				auto v = _data->alloc_count.load(std::memory_order_relaxed);
// 				_data->alloc_count.store(v + 1, std::memory_order_relaxed);
// 				v = _data->alloc_size.load(std::memory_order_relaxed);
// 				_data->alloc_size.store(v + s, std::memory_order_relaxed);
// 			}

// 			Block_allocator* const block_allocator;
// 			Stack_allocator* const stack_allocator;
// 			const std::unique_ptr<Roots_allocator> roots_allocator;
// 			std::unique_ptr<std::array<std::unique_ptr<std::array<std::unique_ptr<Heap_allocator_base>, SqrMaxTypeNumber>>, SqrMaxTypeNumber>> heaps;
// 			Alloc_state alloc_state = {{0, 0}, 0, nullptr};

// 			inline static std::atomic<Data*> threads_data = {nullptr};
// 			inline static std::thread::id main_thread_id = {};

// 		private:
// 			Data* const _data;
// 			inline static std::atomic<int> _type_index = {0};
// 		};

// 		struct Main_thread_detector {
// 			Main_thread_detector() noexcept {
// 				Thread::main_thread_id = std::this_thread::get_id();
// 			}
// 		};
// 		static Main_thread_detector main_thread_detector;

// 		static Thread& Current_thread() {
// 			static thread_local Thread instance;
// 			return instance;
// 		}

// 		struct Collector {
// 			struct Counter {
// 				Counter operator+(const Counter& c) const noexcept {
// 					return {count + c.count, size + c.size};
// 				}
// 				Counter operator-(const Counter& c) const noexcept {
// 					return {count - c.count, size - c.size};
// 				}
// 				void operator+=(const Counter& c) noexcept {
// 					count += c.count;
// 					size += c.size;
// 				}
// 				void operator-=(const Counter& c) noexcept {
// 					count += c.count;
// 					size += c.size;
// 				}
// 				int64_t count = {0};
// 				int64_t size = {0};
// 			};

// 			Collector() {
// 				if (!aborted()) {
// 					_terminated.store(false, std::memory_order_relaxed);
// 					_collected.store(false, std::memory_order_relaxed);
// 					std::thread([this]{_main_loop();}).detach();
// 				}
// 			}

// 			~Collector() {
// 				abort();
// 			}

// 			inline static void collect() noexcept {
// 				using namespace std::chrono_literals;
//                 _collected.store(false, std::memory_order_relaxed);
// 				_collect.store(true, std::memory_order_relaxed);
// 				while (!collected()) {
// 					std::this_thread::sleep_for(1ms);
// 				}
// 			}

// 			inline static bool collect_requested() noexcept {
// 				return _collect.load(std::memory_order_relaxed);
// 			}

// 			inline static bool collected() noexcept {
// 				return _collected.load(std::memory_order_relaxed);
// 			}

// 			inline static void abort() noexcept {
// 				_aborted.store(true, std::memory_order_relaxed);
// 			}

// 			inline static bool aborted() noexcept {
// 				return _aborted.load(std::memory_order_relaxed);
// 			}

// 			inline static void terminate() noexcept {
// 				using namespace std::chrono_literals;
// 				abort();
// 				while (!terminated()) {
// 					std::this_thread::sleep_for(1ms);
// 				}
// 			}

// 			inline static bool terminated() noexcept {
// 				return _terminated.load(std::memory_order_relaxed);
// 			}

// 		private:
// 			Counter _alloc_counter() const {
// 				Counter allocated;
// 				auto data = Thread::threads_data.load(std::memory_order_acquire);
// 				while(data) {
// 					allocated.count += data->alloc_count.load(std::memory_order_relaxed);
// 					allocated.size += data->alloc_size.load(std::memory_order_relaxed);
// 					data = data->next;
// 				}
// 				return allocated + _allocated_rest;
// 			}

// 			bool _check_threads() noexcept {
// 				Thread::Data* prev = nullptr;
// 				auto data = Thread::threads_data.load(std::memory_order_acquire);
// 				while(data) {
// 					auto next = data->next;
// 					if (data->is_used.load(std::memory_order_acquire)) {
// 						auto ptr = data->recursive_alloc_pointer.load(std::memory_order_relaxed);
// 						if (ptr && ptr == data->last_recursive_alloc_pointer) {
// 							return false;
// 						}
// 						prev = data;
// 					} else {
// 						if (!prev) {
// 							auto rdata = data;
// 							if (!Thread::threads_data.compare_exchange_strong(rdata, next, std::memory_order_relaxed, std::memory_order_acquire)) {
// 								while(rdata->next != data) {
// 									rdata = rdata->next;
// 								}
// 								prev = rdata;
// 							}
// 						}
// 						if (prev) {
// 							prev->next = next;
// 						}
// 						_allocated_rest.count += data->alloc_count.load(std::memory_order_relaxed);
// 						_allocated_rest.size += data->alloc_size.load(std::memory_order_relaxed);
// 						delete data;
// 					}
// 					data = next;
// 				}
// 				data = Thread::threads_data.load(std::memory_order_acquire);
// 				while(data) {
// 					auto ptr = data->recursive_alloc_pointer.load(std::memory_order_relaxed);
// 					data->last_recursive_alloc_pointer = ptr;
// 					data = data->next;
// 				}
// 				return true;
// 			}

// 			struct Timer {
// 				Timer() noexcept {
// 					reset();
// 				}
// 				double duration() noexcept {
// 					auto clock = std::chrono::steady_clock::now();
// 					return std::chrono::duration<double, std::milli>(clock - _clock).count();
// 				}
// 				void reset() noexcept {
// 					_clock = std::chrono::steady_clock::now();
// 				}

// 			private:
// 				std::chrono::steady_clock::time_point _clock;
// 			};

// 			void _update_states() {
// 				static Timer timer;
// 				static double rest = 0;
// 				double duration = timer.duration();
// 				duration = States::AtomicReachable * duration / 100 + rest; // 100ms for AtomicReachable
// 				auto iduration = (unsigned)duration;
// 				if (iduration >= 1) {
// 					timer.reset();
// 					rest = duration - iduration;
// 				}
// 				auto time_value = (States::Value)std::min((unsigned)States::AtomicReachable, iduration);
// 				auto page = _registered_pages;
// 				while(page) {
// 					auto states = page->states();
// 					auto flags = page->flags();
// 					auto count = page->metadata->object_count;
// 					for (unsigned i = 0; i < count; ++i) {
// 						auto state = states[i].load(std::memory_order_relaxed);
// 						if (state >= States::Reachable && state <= States::AtomicReachable) {
// 							auto index = Page::flag_index_of(i);
// 							auto mask = Page::flag_mask_of(i);
// 							auto& flag = flags[index];
// 							if (flag.registered & mask) {
// 								state -= state == States::Reachable || state == States::AtomicReachable ? 1 : std::min(state, time_value);
// 								states[i].store(state, std::memory_order_relaxed);
// 							}
// 						}
// 					}
// 					page = page->next_registered;
// 				}
// 			}

// 			void _mark_live_objects() noexcept {
// 				Page* prev = nullptr;
// 				auto page = Heap_allocator_base::pages.load(std::memory_order_acquire);
// 				while(page) {
// 					auto next = page->next;
// 					if (!page->is_used) {
// 						if (!prev) {
// 							auto rpage = page;
// 							if (!Heap_allocator_base::pages.compare_exchange_strong(rpage, next, std::memory_order_relaxed, std::memory_order_acquire)) {
// 								while(rpage->next != page) {
// 									rpage = rpage->next;
// 								}
// 								prev = rpage;
// 							}
// 						}
// 						if (prev) {
// 							prev->next = next;
// 						}
// 						delete page;
// 					} else {
// 						page->clear_flags();
// 						auto states = page->states();
// 						auto flags = page->flags();
// 						auto count = page->metadata->object_count;
// 						for (unsigned i = 0; i < count; ++i) {
// 							auto state = states[i].load(std::memory_order_relaxed);
// 							if (state >= States::Reachable && state <= States::BadAlloc) {
// 								auto index = Page::flag_index_of(i);
// 								auto mask = Page::flag_mask_of(i);
// 								auto& flag = flags[index];
// 								if (!(flag.registered & mask)) {
// 									if (!page->registered) {
// 										page->registered = true;
// 										page->next_registered = _registered_pages;
// 										_registered_pages = page;
// 									}
// 									flag.registered |= mask;
// 									if (state != States::BadAlloc) {
// 										if (!page->reachable) {
// 											page->reachable = true;
// 											page->next_reachable = _reachable_pages;
// 											_reachable_pages = page;
// 										}
// 										flag.reachable |= mask;
// 									}
// 								}
// 							}
// 						}
// 						prev = page;
// 					}
// 					page = next;
// 				}
// 			}

// 			void _mark(const void* p) noexcept {
// 				if (p) {
// 					auto page = Page::page_of(p);
// 					auto index = page->index_of(p);
// 					auto flag_index = Page::flag_index_of(index);
// 					auto mask = Page::flag_mask_of(index);
// 					auto& flag = page->flags()[flag_index];
// 					if (!(flag.marked & mask)) {
// 						if (!page->reachable) {
// 							page->reachable = true;
// 							page->next_reachable = _reachable_pages;
// 							_reachable_pages = page;
// 						}
// 						flag.reachable |= mask;
// 					}
// 				}
// 			}

// 			void _mark_stack() noexcept {
// 				auto data = Thread::threads_data.load(std::memory_order_acquire);
// 				while(data) {
// 					for (auto& p: data->stack_allocator->pages) {
// 						auto page = p.load(std::memory_order_relaxed);
// 						if (page) {
// 							for (auto& p: *page) {
// 								_mark(p.load(std::memory_order_relaxed));
// 							}
// 						}
// 					}
// 					data = data->next;
// 				}
// 			}

// 			void _mark_roots() noexcept {
// 				auto node = Roots_allocator::pages.load(std::memory_order_acquire);
// 				while(node) {
// 					for (auto& p :node->page) {
// 						_mark(p.load(std::memory_order_relaxed));
// 					}
// 					node = node->next;
// 				}
// 			}

// 			void _mark_childs(Page* page, unsigned index) noexcept {
// 				auto data = page->data_of(index);
// 				auto offsets = page->metadata->pointer_offsets.load(std::memory_order_acquire);
// 				if (offsets) {
// 					unsigned size = (unsigned)offsets[0];
// 					for (unsigned i = 1; i <= size; ++i) {
// 						auto p = (Pointer*)(data + offsets[i]);
// 						_mark(p->load(std::memory_order_relaxed));
// 					}
// 				}
// 			}

// 			void _mark_array_childs(Page* page, unsigned index) noexcept {
// 				auto data = page->data_of(index);
// 				auto array = (Array_base*)data;
// 				auto metadata = array->metadata.load(std::memory_order_acquire);
// 				if (metadata) {
// 					auto offsets = metadata->pointer_offsets.load(std::memory_order_acquire);
// 					if (offsets) {
// 						unsigned size = (unsigned)offsets[0];
// 						if (size) {
// 							data += sizeof(Array_base);
// 							auto object_size = metadata->object_size;
// 							for (size_t c = 0; c < array->count; ++c, data += object_size) {
// 								for (unsigned i = 1; i <= size; ++i) {
// 									auto p = (Pointer*)(data + offsets[i]);
// 									_mark(p->load(std::memory_order_relaxed));
// 								}
// 							}
// 						}
// 					}
// 				}
// 			}

// 			void _mark_reachable() noexcept {
// 				auto page = _reachable_pages;
// 				_reachable_pages = nullptr;
// 				while(page) {
// 					auto flags = page->flags();
// 					auto count = page->flags_count();
// 					bool marked;
// 					do {
// 						marked = false;
// 						for (unsigned i = 0; i < count; ++i) {
// 							auto& flag = flags[i];
// 							while (flag.reachable) {
// 								for (unsigned j = 0; j < Page::FlagBitCount; ++j) {
// 									auto mask = Page::Flag(1) << j;
// 									if (flag.reachable & mask) {
// 										flag.marked |= mask;
// 										marked = true;
// 										auto index = i * Page::FlagBitCount + j;
// 										_mark_childs(page, index);
// 										if (page->metadata->is_array) {
// 											_mark_array_childs(page, index);
// 										}
// 									}
// 								}
// 								flag.reachable &= ~flag.marked;
// 							}
// 						}
// 					} while(marked);
// 					page->reachable = false;
// 					page = page->next_reachable;
// 					if (!page) {
// 						page = _reachable_pages;
// 						_reachable_pages = nullptr;
// 					}
// 				}
// 			}

// 			void _mark_updated() noexcept {
// 				auto page = _registered_pages;
// 				while(page) {
// 					bool reachable = false;
// 					auto states = page->states();
// 					auto flags = page->flags();
// 					auto count = page->metadata->object_count;
// 					for (unsigned i = 0; i < count; ++i) {
// 						auto state = states[i].load(std::memory_order_relaxed);
// 						if (state >= States::Reachable && state <= States::AtomicReachable) {
// 							auto index = Page::flag_index_of(i);
// 							auto mask = Page::flag_mask_of(i);
// 							auto& flag = flags[index];
// 							if ((flag.registered & mask) && !(flag.marked & mask)) {
// 								flag.reachable |= mask;
// 								reachable = true;
// 							}
// 						}
// 					}
// 					if (reachable && !page->reachable) {
// 							page->reachable = true;
// 							page->next_reachable = _reachable_pages;
// 							_reachable_pages = page;
// 					}
// 					page = page->next_registered;
// 				}
// 			}

// 			Counter _remove_garbage() noexcept {
// 				Counter released;
// 				auto page = _registered_pages;
// 				Metadata* metadata = nullptr;
// 				while(page) {
// 					bool deregistered = false;
// 					auto destroy = page->metadata->destroy;
// 					auto states = page->states();
// 					auto data = page->data;
// 					auto object_size = page->metadata->object_size;
// 					auto flags = page->flags();
// 					auto count = page->flags_count();
// 					bool on_free_list = false;
// 					for (unsigned i = 0; i < count; ++i) {
// 						auto& flag = flags[i];
// 						auto unreachable = flag.registered & ~flag.marked;
// 						if (unreachable) {
// 							for (unsigned j = 0; j < Page::FlagBitCount; ++j) {
// 								auto mask = Page::Flag(1) << j;
// 								if (unreachable & mask) {
// 									auto index = i * Page::FlagBitCount + j;
// 									auto state = states[index].load(std::memory_order_relaxed);
// 									if (state != States::BadAlloc) {
// 										if (destroy) {
// 											auto p = data + index * object_size;
// 											auto data = page->data_of(index);
// 											if (!page->metadata->is_array) {
// 												auto offsets = page->metadata->pointer_offsets.load(std::memory_order_acquire);
// 												if (offsets) {
// 													unsigned size = (unsigned)offsets[0];
// 													for (unsigned i = 1; i <= size; ++i) {
// 														auto p = (Pointer*)(data + offsets[i]);
// 														p->store(nullptr, std::memory_order_relaxed);
// 													}
// 												}
// 												destroy((void*)p);
// 											} else {
// 												auto array = (Array_base*)data;
// 												auto metadata = array->metadata.load(std::memory_order_acquire);
// 												if (!metadata->tracked_ptrs_only) {
// 													auto offsets = metadata->pointer_offsets.load(std::memory_order_acquire);
// 													if (offsets) {
// 														unsigned size = (unsigned)offsets[0];
// 														if (size) {
// 															auto data = (uintptr_t)array + sizeof(Array_base);
// 															auto object_size = metadata->object_size;
// 															for (size_t c = 0; c < array->count; ++c, data += object_size) {
// 																for (unsigned i = 1; i <= size; ++i) {
// 																	auto p = (Pointer*)(data + offsets[i]);
// 																	p->store(nullptr, std::memory_order_relaxed);
// 																}
// 															}
// 														}
// 													}
// 													destroy((void*)p);
// 												}
// 											}
// 										}
// 										released.count++;
// 										if (!page->metadata->is_array || object_size != sizeof(Array<PageDataSize>)) {
// 											released.size += object_size;
// 										} else {
// 											auto array = (Array_base*)data;
// 											auto metadata = array->metadata.load(std::memory_order_acquire);
// 											released.size += sizeof(Array<>) + metadata->object_size * array->count;
// 										}
// 									}
// 									flag.registered &= ~mask;
// 									if (!deregistered) {
// 										on_free_list = page->on_empty_list.exchange(true, std::memory_order_relaxed);
// 										deregistered = true;
// 									}
// 									states[index].store(States::Unused, std::memory_order_relaxed);
// 								}
// 							}
// 						}
// 					}
// 					if (deregistered && !on_free_list) {
// 						if (!page->metadata->empty_page) {
// 							page->metadata->next = metadata;
// 							metadata = page->metadata;
// 						}
// 						page->next_empty = page->metadata->empty_page;
// 						page->metadata->empty_page = page;
// 					}
// 					page = page->next_registered;
// 				}
// 				while(metadata) {
// 					metadata->free(metadata->empty_page);
// 					metadata->empty_page = nullptr;
// 					metadata = metadata->next;
// 				}
// 				Page* prev = nullptr;
// 				page = _registered_pages;
// 				while(page) {
// 					auto next = page->next_registered;
// 					if (!page->is_used) {
// 						if (!prev) {
// 							_registered_pages = next;
// 						} else {
// 							prev->next_registered = next;
// 						}
// 					} else {
// 						prev = page;
// 					}
// 					page = next;
// 				}
// 				return released;
// 			}

// 			void _main_loop() noexcept {
// 				static constexpr int64_t MinLiveSize = PageSize;
// 				static constexpr int64_t MinLiveCount = MinLiveSize / sizeof(uintptr_t) * 2;
// #if SGCL_LOG_PRINT_LEVEL
// 				std::cout << "[sgcl] start collector id: " << std::this_thread::get_id() << std::endl;
// #endif
// 				using namespace std::chrono_literals;
// 				int finalization_counter = 5;
// 				Counter allocated;
// 				Counter removed;
// 				do {
// #if SGCL_LOG_PRINT_LEVEL >= 2
// 					auto start = std::chrono::high_resolution_clock::now();
// #endif
// 					if (_check_threads()) {
// 						_update_states();
// 						_mark_live_objects();
// 						// _mark_stack();
// 						// _mark_roots();
// 						while(_reachable_pages) {
// 							_mark_reachable();
// 							// _mark_updated();
// 						}
// 						Counter last_removed = _remove_garbage();
// 						Counter last_allocated = _alloc_counter() - allocated;
// 						Counter live = allocated + last_allocated - (removed + last_removed);
// 						assert(live.count >= 0 && live.size >= 0);
// #if SGCL_LOG_PRINT_LEVEL >= 2
// 						auto end = std::chrono::high_resolution_clock::now();
// 						std::cout << "[sgcl] live objects: " << live.count << ", destroyed: " << last_removed.count << ", time: "
// 											<< std::chrono::duration<double, std::milli>(end - start).count() << "ms"
// 											<< std::endl;
// #endif
//                         if (collect_requested()) {
//                             _collect.store(false, std::memory_order_relaxed);
//                             _collected.store(true, std::memory_order_relaxed);
//                         }
                        
// 						Timer timer;
// 						do {
// 							if ((std::max(last_allocated.count, last_removed.count) * 100 / SGCL_TRIGER_PERCENTAGE >= live.count + MinLiveCount)
// 							 || (std::max(last_allocated.size, last_removed.size) * 100 / SGCL_TRIGER_PERCENTAGE >= live.size + MinLiveSize)
// 							 || aborted() || collect_requested()) {
// 								break;
// 							}
// 							std::this_thread::sleep_for(1ms);
// 							last_allocated = _alloc_counter() - allocated;
// 							live = allocated + last_allocated - (removed + last_removed);
// 						} while(!live.count || timer.duration() < SGCL_MAX_SLEEP_TIME_SEC * 1000);

// 						allocated += last_allocated;
// 						removed += last_removed;
// 						if (!last_removed.count && aborted()) {
// 							if (live.count) {
// 								--finalization_counter;
// 							} else {
// 								finalization_counter = 0;
// 							}
// 						}
// 					}
// 					if (!aborted()) {
// 						std::this_thread::yield();
// 					}
// 				} while(finalization_counter);
// #if SGCL_LOG_PRINT_LEVEL
// 				std::cout << "[sgcl] stop collector id: " << std::this_thread::get_id() << std::endl;
// #endif
// 				_terminated.store(true, std::memory_order_relaxed);
// 			}

// 			Page* _reachable_pages = {nullptr};
// 			Page* _registered_pages = {nullptr};
// 			Counter _allocated_rest;

// 			inline static std::atomic<bool> _collect = {false};
// 			inline static std::atomic<bool> _collected = {true};
// 			inline static std::atomic<bool> _aborted = {false};
// 			inline static std::atomic<bool> _terminated = {true};
// 		};

// 		Thread::~Thread() {
// 			_data->is_used.store(false, std::memory_order_release);
// 			if (std::this_thread::get_id() == main_thread_id) {
// 				Collector::abort();
// 			}
// #if SGCL_LOG_PRINT_LEVEL >= 3
// 			std::cout << "[sgcl] stop thread id: " << std::this_thread::get_id() << std::endl;
// #endif
// 		}

// 		class Tracked_ptr {
// #ifdef SGCL_ARCH_X86_64
// 			static constexpr uintptr_t StackFlag = uintptr_t(1) << 63;
// 			static constexpr uintptr_t ExternalHeapFlag = uintptr_t(1) << 62;
// #else
// 			static constexpr uintptr_t StackFlag = 1;
// 			static constexpr uintptr_t ExternalHeapFlag = 2;
// #endif
// 			static constexpr uintptr_t ClearMask = ~(StackFlag | ExternalHeapFlag);

// 		protected:
// 			Tracked_ptr(Tracked_ptr* p = nullptr) {
// 				uintptr_t this_addr = (uintptr_t)this;
// 				uintptr_t stack_addr = (uintptr_t)&this_addr;
// 				ptrdiff_t offset = this_addr - stack_addr;
// 				bool stack = -MaxStackOffset <= offset && offset <= MaxStackOffset;
// 				if (!stack) {
// 					auto& thread = Current_thread();
// 					auto& state = thread.alloc_state;
// 					bool root = !(state.range.first <= this_addr && this_addr < state.range.second);
// 					if (!root) {
// 						if (state.ptrs) {
// 							state.ptrs[state.size++] = &_ptr_value;
// 						}
// #ifdef SGCL_ARCH_X86_64
// 						_ref = nullptr;
// #else
// 						_ref = &_ptr_value;
// #endif
// 					} else {
// 						if (p && p->_allocated_on_external_heap()) {
// 							_ref = p->_ref;
// 						} else {
// 							auto ref = thread.roots_allocator->alloc();
// 							_ref = _set_flag(ref, ExternalHeapFlag);
// 						}
// 					}
// 				} else {
// 					auto ref = Current_thread().stack_allocator->alloc(this);
// 					_ref = _set_flag(ref, StackFlag);
// 				}
// 			}

// 			~Tracked_ptr() {
// 				if (_allocated_on_heap()) {
// 					_store(nullptr);
// 				} else if (_ref) {
// 					_store(nullptr);
// 					if (!_allocated_on_stack() && !Collector::aborted()) {
// 						Current_thread().roots_allocator->free(_ref);
// 					}
// 				}
//                 Collector::collect();
// 			}

// 			Tracked_ptr(const Tracked_ptr& r) = delete;
// 			Tracked_ptr& operator=(const Tracked_ptr&) = delete;

// 			void _clear_ref() noexcept {
// 				_ref = nullptr;
// 			}

// 			bool _allocated_on_heap() const noexcept {
// 				return !((uintptr_t)_ref & (StackFlag | ExternalHeapFlag));
// 			}

// 			bool _allocated_on_stack() const noexcept {
// 				return (uintptr_t)_ref & StackFlag;
// 			}

// 			bool _allocated_on_external_heap() const noexcept {
// 				return (uintptr_t)_ref & ExternalHeapFlag;
// 			}

// 			static void _update(const void* p) noexcept {
// 				if (p) {
// 					Page::set_state(p, States::Reachable);
// 				}
// 			}

// 			static void _update_atomic(const void* p) noexcept {
// 				if (p) {
// 					Page::set_state(p, States::AtomicReachable);
// 				}
// 			}

// 			void* _load() const noexcept {
// 				return _ptr()->load(std::memory_order_relaxed);
// 			}

// 			void* _load(const std::memory_order m) const noexcept {
// 				auto l = _ptr()->load(m);
// 				_update_atomic(l);
// 				return l;
// 			}

// 			void _store(std::nullptr_t) noexcept {
// 				_ptr()->store(nullptr, std::memory_order_relaxed);
// 			}

// 			void _store_no_update(const void* p) noexcept {
// 				_ptr()->store(const_cast<void*>(p), std::memory_order_relaxed);
// 			}

// 			void _store(const void* p) noexcept {
// 				_store_no_update(p);
// 				_update(p);
// 			}

// 			void _store(const void* p, const std::memory_order m) noexcept {
// 				_ptr()->store(const_cast<void*>(p), m);
// 				_update_atomic(p);
// 			}

// 			void* _exchange(const void* p, const std::memory_order m) noexcept {
// 				auto l = _load();
// 				_update_atomic(l);
// 				_update_atomic(p);
// 				l = _ptr()->exchange(const_cast<void*>(p), m);
// 				return l;
// 			}

// 			template<class F>
// 			bool _compare_exchange(void*& o, const void* n, F f) noexcept {
// 				auto l = _load();
// 				_update_atomic(l);
// 				_update_atomic(n);
// 				return f(_ptr(), o, n);
// 			}

// 			void* _base_address_of(const void* p) const noexcept {
// 				return Page::base_address_of(p);
// 			}

// 			template<class T>
// 			metadata_base& _metadata() const noexcept {
// 				if constexpr(std::is_array_v<T>) {
// 					return Page_info<T>::public_metadata();
// 				} else {
// 					auto p = _load();
// 					if (p) {
// 						return Page::metadata_of(p).public_metadata;
// 					} else {
// 						return Page_info<T>::public_metadata();
// 					}
// 				}
// 			}

// 		private:
// 			template<class T>
// 			static T _set_flag(T p, uintptr_t f) noexcept {
// 				auto v = (uintptr_t)p | f;
// 				return (T)v;
// 			}

// 			template<class T>
// 			static constexpr T _remove_flags(T p) noexcept {
// 				auto v = (uintptr_t)p & ClearMask;
// 				return (T)v;
// 			}

// #ifdef SGCL_ARCH_X86_64
// 			Pointer* _ptr() noexcept {
// 				if (_allocated_on_heap()) {
// 					return &_ptr_value;
// 				}
// 				return _remove_flags(_ref);
// 			}

// 			const Pointer* _ptr() const noexcept {
// 				if (_allocated_on_heap()) {
// 					return &_ptr_value;
// 				}
// 				return _remove_flags(_ref);
// 			}

// 			union {
// 				Pointer* _ref;
// 				Pointer _ptr_value;
// 			};
// #else
// 			Pointer* _ptr() noexcept {
// 				return _remove_flags(_ref);
// 			}

// 			const Pointer* _ptr() const noexcept {
// 				return _remove_flags(_ref);
// 			}

// 			Pointer* _ref;
// 			Pointer _ptr_value;
// #endif
// 			template<size_t> friend struct Array;
// 		};
// 		template<class T, class U = T, class ...A>
// 		static tracked_ptr<U> Make_tracked(size_t, A&&...);
// 	}; // namespace Priv

// 	template<class T>
// 	class tracked_ptr : Priv::Tracked_ptr {
// 	public:
// 		using element_type = std::remove_extent_t<T>;

// 		constexpr tracked_ptr()
// 		: Tracked_ptr() {
// 			_store(nullptr);
// 		}

// 		constexpr tracked_ptr(std::nullptr_t)
// 		: Tracked_ptr() {
// 			_store(nullptr);
// 		}

// 		template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
// 		explicit tracked_ptr(U* p)
// 		: Tracked_ptr() {
// 			_store(p);
// 		}

// 		tracked_ptr(const tracked_ptr& p)
// 		: Tracked_ptr() {
// 			_store(p.get());
// 		}

// 		template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
// 		tracked_ptr(const tracked_ptr<U>& p)
// 		: Tracked_ptr() {
// 			_store(static_cast<T*>(p.get()));
// 		}

// 		tracked_ptr(tracked_ptr&& p)
// 		: Tracked_ptr(&p) {
// 			if (_allocated_on_external_heap() && p._allocated_on_external_heap()) {
// 				p._clear_ref();
// 			} else {
// 				_store(p.get());
// 				p._store(nullptr);
// 			}
// 		}

// 		template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
// 		tracked_ptr(tracked_ptr<U>&& p)
// 		: Tracked_ptr(&p) {
// 			if (_allocated_on_external_heap() && p._allocated_on_external_heap()) {
// 				p._clear_ref();
// 			} else {
// 				_store(static_cast<T*>(p.get()));
// 				p._store(nullptr);
// 			}
// 		}

// 		tracked_ptr& operator=(std::nullptr_t) noexcept {
// 			_store(nullptr);
// 			return *this;
// 		}

// 		tracked_ptr& operator=(const tracked_ptr& p) noexcept {
// 			_store(p.get());
// 			return *this;
// 		}

// 		template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
// 		tracked_ptr& operator=(const tracked_ptr<U>& p) noexcept {
// 			_store(static_cast<T*>(p.get()));
// 			return *this;
// 		}

// 		tracked_ptr& operator=(tracked_ptr&& p) noexcept {
// 			_store(p.get());
// 			p._store(nullptr);
// 			return *this;
// 		}

// 		template<class U, std::enable_if_t<std::is_convertible_v<U*, T*>, int> = 0>
// 		tracked_ptr& operator=(tracked_ptr<U>&& p) noexcept {
// 			_store(static_cast<T*>(p.get()));
// 			p._store(nullptr);
// 			return *this;
// 		}

// 		explicit operator bool() const noexcept {
// 			return (get() != nullptr);
// 		}

// 		template <class U = T, std::enable_if_t<!std::disjunction_v<std::is_void<U>, std::is_array<U>>, int> = 0>
// 		U& operator*() const noexcept {
// 			assert(get() != nullptr);
// 			return *get();
// 		}

// 		template <class U = T, std::enable_if_t<!std::disjunction_v<std::is_void<U>, std::is_array<U>>, int> = 0>
// 		U* operator->() const noexcept {
// 			assert(get() != nullptr);
// 			return get();
// 		}

// 		template <class U = T, class E = element_type, std::enable_if_t<std::is_array_v<U>, int> = 0>
// 		E& operator[](size_t i) const noexcept {
// 			assert(get() != nullptr);
// 			return get()[i];
// 		}

// 		template <class U = T, std::enable_if_t<std::is_array_v<U>, int> = 0>
// 		size_t size() const noexcept {
// 			auto p = get();
// 			if (p) {
// 				auto array = (Priv::Array_base*)_base_address_of(p);
// 				return array->count;
// 			} else {
// 				return 0;
// 			}
// 		}

// 		template <class U = T, std::enable_if_t<std::is_array_v<U>, int> = 0>
// 		element_type* begin() const noexcept {
// 			return get();
// 		}

// 		template <class U = T, std::enable_if_t<std::is_array_v<U>, int> = 0>
// 		element_type* end() const noexcept {
// 			return begin() + size();
// 		}

// 		element_type* get() const noexcept {
// 			return (element_type*)_load();
// 		}

// 		void reset() noexcept {
// 			_store(nullptr);
// 		}

// 		void swap(tracked_ptr& p) noexcept {
// 			auto l = get();
// 			_store(p.get());
// 			p._store(l);
// 		}

// 		template <class U = T, std::enable_if_t<!std::is_array_v<U>, int> = 0>
// 		tracked_ptr<T> clone() const {
// 			auto metadata = Priv::Page::metadata_of(get());
// 			if (!metadata.is_array) {
// 				auto p = metadata.clone(get());
// 				return tracked_ptr<T>((T*)p.get());
// 			} else {
// 				return {nullptr};
// 			}
// 		}

// 		template<class U, std::enable_if_t<!std::is_array_v<U>, int> = 0>
// 		bool is() const noexcept {
// 			return type() == typeid(U);
// 		}

// 		template<class U, std::enable_if_t<!std::is_array_v<U>, int> = 0>
// 		tracked_ptr<U> as() const noexcept {
// 			if (is<U>()) {
// 				auto address = _base_address_of(get());
// 				return tracked_ptr<U>((U*)address);
// 			} else {
// 				return {nullptr};
// 			}
// 		}

// 		const std::type_info& type() const noexcept {
// 			return _metadata().type;
// 		}

// 		const std::any& metadata() const noexcept {
// 			return _metadata().user_data;
// 		}

// 		template<class U>
// 		const U& metadata() const noexcept {
// 			return *std::any_cast<U>(&_metadata().user_data);
// 		}

// 	private:
// 		// for make
// 		template<class F>
// 		explicit tracked_ptr(void* p, F f)
// 		: Tracked_ptr() {
// 			_store(p);
// 			f();
// 		}

// 		tracked_ptr(const tracked_ptr& p, const std::memory_order m)
// 		: Tracked_ptr() {
// 			auto r = p._load(m);
// 			_store_no_update(r);
// 		}

// 		auto _metadata() const noexcept {
// 			return Tracked_ptr::_metadata<T>();
// 		}

// 		template<class> friend class tracked_ptr;
// 		template<class> friend class atomic_ptr;
// 		template<class, class U, class ...A> friend tracked_ptr<U> Priv::Make_tracked(size_t, A&&...);
// 		friend std::atomic<tracked_ptr>;
// 	};

// 	template<class T, class U>
// 	inline bool operator==(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		return a.get() == b.get();
// 	}

// 	template<class T>
// 	inline bool operator==(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return !a;
// 	}

// 	template<class T>
// 	inline bool operator==(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return !a;
// 	}

// 	template<class T, class U>
// 	inline bool operator!=(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		return !(a == b);
// 	}

// 	template<class T>
// 	inline bool operator!=(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return (bool)a;
// 	}

// 	template<class T>
// 	inline bool operator!=(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return (bool)a;
// 	}

// 	template<class T, class U>
// 	inline bool operator<(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		using V = typename std::common_type<T*, U*>::type;
// 		return std::less<V>()(a.get(), b.get());
// 	}

// 	template<class T>
// 	inline bool operator<(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return std::less<T*>()(a.get(), nullptr);
// 	}

// 	template<class T>
// 	inline bool operator<(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return std::less<T*>()(nullptr, a.get());
// 	}

// 	template<class T, class U>
// 	inline bool operator<=(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		return !(b < a);
// 	}

// 	template<class T>
// 	inline bool operator<=(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return !(nullptr < a);
// 	}

// 	template<class T>
// 	inline bool operator<=(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return !(a < nullptr);
// 	}

// 	template<class T, class U>
// 	inline bool operator>(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		return (b < a);
// 	}

// 	template<class T>
// 	inline bool operator>(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return nullptr < a;
// 	}

// 	template<class T>
// 	inline bool operator>(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return a < nullptr;
// 	}

// 	template<class T, class U>
// 	inline bool operator>=(const tracked_ptr<T>& a, const tracked_ptr<U>& b) noexcept {
// 		return !(a < b);
// 	}

// 	template<class T>
// 	inline bool operator>=(const tracked_ptr<T>& a, std::nullptr_t) noexcept {
// 		return !(a < nullptr);
// 	}

// 	template<class T>
// 	inline bool operator>=(std::nullptr_t, const tracked_ptr<T>& a) noexcept {
// 		return !(nullptr < a);
// 	}

// 	template<class T, class U, std::enable_if_t<!std::is_array_v<T>, int> = 0>
// 	inline tracked_ptr<T> static_pointer_cast(const tracked_ptr<U>& r) noexcept {
// 		return tracked_ptr<T>(static_cast<T*>(r.get()));
// 	}

// 	template<class T, class U, std::enable_if_t<!std::is_array_v<T>, int> = 0>
// 	inline tracked_ptr<T> const_pointer_cast(const tracked_ptr<U>& r) noexcept {
// 		return tracked_ptr<T>(const_cast<T*>(r.get()));
// 	}

// 	template<class T, class U, std::enable_if_t<!std::is_array_v<T>, int> = 0>
// 	inline tracked_ptr<T> dynamic_pointer_cast(const tracked_ptr<U>& r) noexcept {
// 		return tracked_ptr<T>(dynamic_cast<T*>(r.get()));
// 	}

// 	template<class T>
// 	std::ostream& operator<<(std::ostream& s, const tracked_ptr<T>& p) {
// 		s << p.get();
// 		return s;
// 	}

// 	namespace Priv {
// 		template<class T>
// 		tracked_ptr<void> Clone(const void* p) {
// 			if constexpr (std::is_copy_constructible_v<T>) {
// 				return p ? make_tracked<T>(*((const T*)p)) : nullptr;
// 			} else {
// 				std::ignore = p;
// 				assert(!"[sgcl] clone(): no copy constructor");
// 				return nullptr;
// 			}
// 		}

// 		template<size_t S>
// 		template<class T, class... A>
// 		void Array<S>::_init(A&&... a) {
// 			using Info = Page_info<std::remove_cv_t<T>>;
// 			if constexpr(!std::is_trivial_v<T>) {
// 				auto offsets = Info::pointer_offsets.load(std::memory_order_acquire);
// 				if (!offsets || offsets[0]) {
// 					Pointer** ptrs = nullptr;
// 					if (!offsets) {
// 						ptrs = (Pointer**)(::operator new(sizeof(T)));
// 					}
// 					auto& thread = Current_thread();
// 					bool first_recursive_pointer = thread.set_recursive_alloc_pointer(this->data);
// 					auto& state = thread.alloc_state;
// 					auto old_state = state;
// 					state = {{(uintptr_t)(this->data), (uintptr_t)((T*)this->data + count)}, 0, ptrs};
// 					try {
// 						new(const_cast<char*>(this->data)) T(std::forward<A>(a)...);
// 					}
// 					catch (...) {
// 						if (ptrs) {
// 							::operator delete(ptrs);
// 						}
// 						state = old_state;
// 						if (first_recursive_pointer) {
// 							thread.clear_recursive_alloc_pointer();
// 						}
// 						throw;
// 					}
// 					if (ptrs) {
// 						auto offsets = new ptrdiff_t[state.size + 1];
// 						offsets[0] = state.size;
// 						for (size_t i = 0; i < state.size; ++i) {
// 							offsets[i + 1] = (uintptr_t)ptrs[i] - (uintptr_t)this->data;
// 						}
// 						::operator delete(ptrs);
// 						ptrdiff_t* old = nullptr;
// 						if (!Info::pointer_offsets.compare_exchange_strong(old, offsets)) {
// 							::operator delete(offsets);
// 						}
// 					}
// 					state.ptrs = nullptr;
// 					if constexpr(std::is_base_of_v<Tracked_ptr, T>) {
// 						for (size_t i = 1; i < count; ++i) {
// 							auto p = (Tracked_ptr*)const_cast<std::remove_cv_t<T>*>((T*)this->data + i);
// 							if constexpr(sizeof...(A)) {
// 								p->_ptr_value.store((a.get())..., std::memory_order_relaxed);
// 							} else {
// 								p->_ptr_value.store(nullptr, std::memory_order_relaxed);
// 							}
// 		#ifndef SGCL_ARCH_X86_64
// 							p->_ref = &p->_ptr_value;
// 		#endif
// 						}
// 					} else {
// 						for (size_t i = 1; i < count; ++i) {
// 							new(const_cast<std::remove_cv_t<T>*>((T*)this->data + i)) T(std::forward<A>(a)...);
// 						}
// 					}
// 					state = old_state;
// 					if (first_recursive_pointer) {
// 						thread.clear_recursive_alloc_pointer();
// 					}
// 				} else {
// 					for (size_t i = 0; i < count; ++i) {
// 						new(const_cast<std::remove_cv_t<T>*>((T*)this->data + i)) T(std::forward<A>(a)...);
// 					}
// 				}
// 			} else {
// 				for (size_t i = 0; i < count; ++i) {
// 					new(const_cast<std::remove_cv_t<T>*>((T*)this->data + i)) T(std::forward<A>(a)...);
// 				}
// 			}
// 		}

// 		void Collector_init() {
// 			static Collector collector_instance;
// 		}

// 		template<class T, class U, class ...A>
// 		static tracked_ptr<U> Make_tracked(size_t size, A&&... a) {
// 			using Info = Page_info<std::remove_cv_t<T>>;
// 			Collector_init();
// 			Pointer** ptrs = nullptr;
// 			if (!Info::pointer_offsets.load(std::memory_order_relaxed)) {
// 				ptrs = (Pointer**)(::operator new(sizeof(T)));
// 			}
// 			auto& thread = Current_thread();
// 			auto& allocator = thread.alocator<std::remove_cv_t<T>>();
// 			auto mem = (T*)allocator.alloc(size);
// 			bool first_recursive_pointer = thread.set_recursive_alloc_pointer(mem);
// 			auto& state = thread.alloc_state;
// 			auto old_state = state;
// 			state = {{(uintptr_t)(mem), (uintptr_t)(mem + 1)}, 0, ptrs};
// 			std::remove_cv_t<T>* ptr;
// 			try {
// 				ptr = const_cast<std::remove_cv_t<T>*>(new(mem) T(std::forward<A>(a)...));
// 				if constexpr(std::is_base_of_v<Array_base, T>) {
// 					static_assert(sizeof(decltype(ptr->next())) == sizeof(tracked_ptr<void>));
// 					new (&ptr->next()) tracked_ptr<void>();
// 					new (&ptr->prev()) tracked_ptr<void>();
// 				}
// 				thread.update_allocated(sizeof(T) + size);
// 			}
// 			catch (...) {
// 				Page::set_state(mem, States::BadAlloc);
// 				if (ptrs) {
// 					::operator delete(ptrs);
// 				}
// 				state = old_state;
// 				if (first_recursive_pointer) {
// 					thread.clear_recursive_alloc_pointer();
// 				}
// 				throw;
// 			}
// 			if (ptrs) {
// 				auto offsets = new ptrdiff_t[state.size + 1];
// 				offsets[0] = state.size;
// 				for (size_t i = 0; i < state.size; ++i) {
// 					offsets[i + 1] = (uintptr_t)ptrs[i] - (uintptr_t)ptr;
// 				}
// 				::operator delete(ptrs);
// 				ptrdiff_t* old = nullptr;
// 				if (!Info::pointer_offsets.compare_exchange_strong(old, offsets)) {
// 					::operator delete(offsets);
// 				}
// 			}
// 			state = old_state;
// 			if constexpr(!std::is_base_of_v<Array_base, T>) {
// 				return tracked_ptr<T>(ptr, [&]{
// 					if (first_recursive_pointer) {
// 						thread.clear_recursive_alloc_pointer();
// 					}
// 				});
// 			} else {
// 				return tracked_ptr<U>(ptr->data, [&]{
// 					if (first_recursive_pointer) {
// 						thread.clear_recursive_alloc_pointer();
// 					}
// 				});
// 			}
// 		}

// 		template<class T>
// 		struct Maker {
// 			template<class ...A>
// 			static tracked_ptr<T> make_tracked(A&&... a) {
// 				return Make_tracked<T>(0, std::forward<A>(a)...);
// 			}
// 		};

// 		template<class T>
// 		struct Maker<T[]> {
// 			static tracked_ptr<T[]> make_tracked(size_t count) {
// 				tracked_ptr<T[]> p;
// 				_make_tracked(count, p);
// 				return p;
// 			}
// 			static tracked_ptr<T[]> make_tracked(size_t count, const std::remove_extent_t<T>& v) {
// 				tracked_ptr<T[]> p;
// 				_make_tracked(count, p, &v);
// 				return p;
// 			}

// 		private:
// 			template<size_t N>
// 			static void _make(size_t count, tracked_ptr<T[]>& p) {
// 				if (sizeof(T) * count <= N && sizeof(Array<N>) <= PageDataSize) {
// 					p = Make_tracked<Array<N>, T[]>(0, count);
// 				} else {
// 					if constexpr(sizeof(Array<N>) < PageDataSize) {
// 						_make<N * 2>(count, p);
// 					} else {
// 						if (sizeof(T) * count <= PageDataSize - sizeof(Array_base)) {
// 							p = Make_tracked<Array<PageDataSize - sizeof(Array_base)>, T[]>(0, count);
// 						} else {
// 							p = Make_tracked<Array<>, T[]>(sizeof(T) * count, count);
// 						}
// 					}
// 				}
// 			}
// 			static void _make_tracked(size_t count, tracked_ptr<T[]>& p, const std::remove_extent_t<T>* v = nullptr) {
// 				if (count) {
// 					_make<1>(count, p);
// 					auto array = (Array<>*)((Array_base*)p.get() - 1);
// 					if (v) {
// 						array->template init<T>(*v);
// 					} else {
// 						array->template init<T>();
// 					}
// 				}
// 			}
// 		};
// 	} // namespace Priv

// 	template<class T, class ...A>
// 	auto make_tracked(A&&... a) {
// 		return Priv::Maker<T>::make_tracked(std::forward<A>(a)...);
// 	}

// 	void terminate_collector() noexcept {
// 		Priv::Collector::terminate();
// 	}
// } // namespace sgcl

// template<class T>
// struct std::atomic<sgcl::tracked_ptr<T>> {
// 	using value_type = sgcl::tracked_ptr<T>;

// 	atomic() = default;

// 	atomic(std::nullptr_t)
// 	: _ptr(nullptr) {
// 	}

// 	atomic(const value_type& p)
// 	: _ptr(p) {
// 	}

// 	atomic(const atomic&) = delete;
// 	atomic& operator =(const atomic&) = delete;

// 	static constexpr bool is_always_lock_free = std::atomic<T*>::is_always_lock_free;

// 	bool is_lock_free() const noexcept {
// 		return std::atomic<T*>::is_lock_free();
// 	}

// 	void operator=(const value_type& p) noexcept {
// 		store(p);
// 	}

// 	void store(const value_type& p, const memory_order m = std::memory_order_seq_cst) noexcept {
// 		_store(p, m);
// 	}

// 	value_type load(const memory_order m = std::memory_order_seq_cst) const {
// 		return _load(m);
// 	}

// 	void load(value_type& o, const memory_order m = std::memory_order_seq_cst) const noexcept {
// 		_load(o, m);
// 	}

// 	value_type exchange(const value_type& p, const memory_order m = std::memory_order_seq_cst) {
// 		return _exchange(p, m);
// 	}

// 	void exchange(value_type& o, const value_type& p, const memory_order m = std::memory_order_seq_cst) noexcept {
// 		_exchange(o, p, m);
// 	}

// 	bool compare_exchange_strong(value_type& o, const value_type& n, const memory_order m = std::memory_order_seq_cst) noexcept {
// 		return _compare_exchange_strong(o, n, m);
// 	}

// 	bool compare_exchange_strong(value_type& o, const value_type& n, const memory_order s, const memory_order f) noexcept {
// 		return _compare_exchange_strong(o, n, s, f);
// 	}

// 	bool compare_exchange_weak(value_type& o, const value_type& n, const memory_order m = std::memory_order_seq_cst) noexcept {
// 		return _compare_exchange_weak(o, n, m);
// 	}

// 	bool compare_exchange_weak(value_type& o, const value_type& n, const memory_order s, const memory_order f) noexcept {
// 		return _compare_exchange_weak(o, n, s, f);
// 	}

// 	operator value_type() const noexcept {
// 		return load();
// 	}

// private:
// 	value_type _load(const std::memory_order m = std::memory_order_seq_cst) const {
// 		return value_type(_ptr, m);
// 	}

// 	void _load(value_type& o, const std::memory_order m = std::memory_order_seq_cst) const noexcept {
// 		auto l = _ptr._load(m);
// 		o._store_no_update(l);
// 	}

// 	void _store(const value_type& p, const std::memory_order m = std::memory_order_seq_cst) noexcept {
// 		auto r = p.get();
// 		_ptr._store(r, m);
// 	}

// 	value_type _exchange(const value_type& p, const std::memory_order m = std::memory_order_seq_cst) {
// 		auto r = p.get();
// 		return value_type((T*)_ptr._exchange(r, m));
// 	}

// 	void _exchange(value_type& o, const value_type& p, const std::memory_order m = std::memory_order_seq_cst) noexcept {
// 		auto r = p.get();
// 		auto l = _ptr._exchange(r, m);
// 		o._store_no_update(l);
// 	}

// 	bool _compare_exchange_weak(value_type& o, const value_type& n, const std::memory_order m = std::memory_order_seq_cst) noexcept {
// 		return _compare_exchange(o, n, [m](auto p, auto& l, auto r){ return p->compare_exchange_weak(l, const_cast<void*>(r), m); });
// 	}

// 	bool _compare_exchange_weak(value_type& o, const value_type& n, std::memory_order s, std::memory_order f) noexcept {
// 		return _compare_exchange(o, n, [s, f](auto p, auto& l, auto r){ return p->compare_exchange_weak(l, const_cast<void*>(r), s, f); });
// 	}

// 	bool _compare_exchange_strong(value_type& o, const value_type& n, const std::memory_order m = std::memory_order_seq_cst) noexcept {
// 		return _compare_exchange(o, n, [m](auto p, auto& l, auto r){ return p->compare_exchange_strong(l, const_cast<void*>(r), m); });
// 	}

// 	bool _compare_exchange_strong(value_type& o, const value_type& n, std::memory_order s, std::memory_order f) noexcept {
// 		return _compare_exchange(o, n, [s, f](auto p, auto& l, auto r){ return p->compare_exchange_strong(l, const_cast<void*>(r), s, f); });
// 	}

// 	template<class F>
// 	bool _compare_exchange(value_type& o, const value_type& n, F f) noexcept {
// 		auto r = n.get();
// 		void* l = const_cast<std::remove_cv_t<typename value_type::element_type>*>(o.get());
// 		if (!_ptr._compare_exchange(l, r, f)) {
// 			o._store_no_update(l);
// 			return false;
// 		}
// 		return true;
// 	}

// 	sgcl::tracked_ptr<T> _ptr;
// };

// //------------------------------------------------------------------------------
// // Tracked containers
// //------------------------------------------------------------------------------

// #include <deque>
// #include <forward_list>
// #include <list>
// #include <map>
// #include <queue>
// #include <set>
// #include <stack>
// #include <unordered_map>
// #include <unordered_set>
// #include <vector>

// namespace sgcl {
// 	namespace Priv {
// 		template <class T>
// 		class Tracked_allocator {
// 			using Info = Page_info<std::remove_cv_t<T>>;

// 		public:
// 			using value_type         = T;
// 			using pointer            = value_type*;
// 			using const_pointer      = const value_type*;
// 			using void_pointer       = void*;
// 			using const_void_pointer = const void*;
// 			using difference_type    = ptrdiff_t;
// 			using size_type          = std::size_t;

// 			template <class U>
// 			struct rebind  {
// 				using other = Tracked_allocator<U>;
// 			};

// 			Tracked_allocator(tracked_ptr<void>& p) noexcept
// 			: _list(p) {
// 			}

// 			Tracked_allocator(const Tracked_allocator& a) noexcept
// 			: _list(a._list) {
// 			}

// 			template <class U>
// 			Tracked_allocator(const Tracked_allocator<U>& a) noexcept
// 			: _list(a._list) {
// 			}

// 			Tracked_allocator& operator=(const Tracked_allocator&) noexcept {
// 				return *this;
// 			}

// 			Tracked_allocator& operator=(Tracked_allocator&& a) noexcept {
// 				_list = std::move(a._list);
// 				return *this;
// 			}

// 			pointer allocate(size_type n)  {
// 				struct Data {
// 					char data[sizeof(T)];
// 				};
// 				auto p = make_tracked<Data[]>(n);
// 				auto data = p.get();
// 				std::memset(data, 0, sizeof(T) * n);
// 				auto array = (Array_base*)data - 1;
// 				array->metadata.store(&Info::array_metadata(), std::memory_order_release);
// 				if (_list) {
// 					array->next() = _list;
// 					array = (Array_base*)_list.get() - 1;
// 					array->prev() = p;
// 				}
// 				_list = p;
// 				return (pointer)data;
// 			}

// 			void deallocate(pointer p, size_type) noexcept {
// 				auto array = (Array_base*)p - 1;
// 				if (array->next()) {
// 					auto next = (Array_base*)array->next().get() - 1;
// 					next->prev() = array->prev();
// 				}
// 				if (array->prev()) {
// 					auto prev = (Array_base*)array->prev().get() - 1;
// 					prev->next() = array->next();
// 				} else {
// 					_list = array->next();
// 				}
// 			}

// 			pointer allocate(size_type n, const_void_pointer) {
// 				return allocate(n);
// 			}

// 			template <class ...A>
// 			void construct(T* p, A&& ...a)  {
// 				if constexpr(!std::is_trivial_v<T>) {
// 					auto offsets = Info::pointer_offsets.load(std::memory_order_acquire);
// 					if (!offsets || offsets[0]) {
// 						Pointer** ptrs = nullptr;
// 						if (!offsets) {
// 							ptrs = (Pointer**)(::operator new(sizeof(T)));
// 						}
// 						auto& thread = Current_thread();
// 						bool first_recursive_pointer = thread.set_recursive_alloc_pointer(p);
// 						auto& state = thread.alloc_state;
// 						auto old_state = state;
// 						state = {{(uintptr_t)p, (uintptr_t)((T*)p + 1)}, 0, ptrs};
// 						try {
// 							new(const_cast<std::remove_cv_t<T>*>(p)) T(std::forward<A>(a)...);
// 						}
// 						catch (...) {
// 							if (ptrs) {
// 								::operator delete(ptrs);
// 							}
// 							state = old_state;
// 							if (first_recursive_pointer) {
// 								thread.clear_recursive_alloc_pointer();
// 							}
// 							throw;
// 						}
// 						if (ptrs) {
// 							auto offsets = new ptrdiff_t[state.size + 1];
// 							offsets[0] = state.size;
// 							for (size_t i = 0; i < state.size; ++i) {
// 								offsets[i + 1] = (uintptr_t)ptrs[i] - (uintptr_t)p;
// 							}
// 							::operator delete(ptrs);
// 							ptrdiff_t* old = nullptr;
// 							if (!Info::pointer_offsets.compare_exchange_strong(old, offsets)) {
// 								::operator delete(offsets);
// 							}
// 						}
// 						state = old_state;
// 						if (first_recursive_pointer) {
// 							thread.clear_recursive_alloc_pointer();
// 						}
// 					} else {
// 						new(const_cast<std::remove_cv_t<T>*>(p)) T(std::forward<A>(a)...);
// 					}
// 				} else {
// 					new(const_cast<std::remove_cv_t<T>*>(p)) T(std::forward<A>(a)...);
// 				}
// 			}

// 			void destroy(T* p) noexcept {
// 				std::destroy_at(p);
// 			}

// 			size_type max_size() const noexcept {
// 				return std::numeric_limits<size_type>::max();
// 			}

// 			Tracked_allocator select_on_container_copy_construction() const  {
// 				return *this;
// 			}

// 			using propagate_on_container_copy_assignment = std::false_type;
// 			using propagate_on_container_move_assignment = std::true_type;
// 			using propagate_on_container_swap            = std::true_type;
// 			using is_always_equal                        = std::true_type;

// 		private:
// 			tracked_ptr<void>& _list;

// 			template <class> friend class Tracked_allocator;
// 		};

// 		template <class T, class U>
// 		inline bool operator==(const Tracked_allocator<T>& a, const Tracked_allocator<U>& b) noexcept  {
// 			return &a == &b;
// 		}

// 		template <class T, class U>
// 		inline bool operator!=(const Tracked_allocator<T>& a, const Tracked_allocator<U>& b) noexcept {
// 			return !(a == b);
// 		}

// 		struct Tracked_container_base {
// 			Tracked_container_base() = default;
// 			Tracked_container_base(const Tracked_container_base&) = delete;
// 			Tracked_container_base& operator=(const Tracked_container_base&) = delete;

// 		protected:
// 			tracked_ptr<void> _list;
// 		};

// 		template<class Container>
// 		class Tracked_container : Tracked_container_base, public Container {
// 			using Type = typename Container::value_type;

// 		public:
// 			template<class ...A>
// 			Tracked_container(A&&... a)
// 			: Container(std::forward<A>(a)..., Tracked_allocator<Type>(_list)) {
// 			}

// 			Tracked_container(Tracked_container&& c)
// 				: Container(Tracked_allocator<Type>(_list)) {
// 				*this = std::move(c);
// 			}

// 			Tracked_container& operator=(const Tracked_container& c) {
// 				(Container&)*this = c;
// 				return *this;
// 			}

// 			Tracked_container& operator=(Tracked_container&& c) {
// 				(Container&)*this = std::move(c);
// 				return *this;
// 			}
// 		};
// 	} // namespace Priv;

// 	template <class T>
// 	using tracked_allocator = Priv::Tracked_allocator<T>;

// 	template<class Container>
// 	using tracked_container = Priv::Tracked_container<Container>;

// 	template<class T>
// 	using tracked_vector = tracked_container<std::vector<T, tracked_allocator<T>>>;

// 	template<class T>
// 	using tracked_list = tracked_container<std::list<T, tracked_allocator<T>>>;

// 	template<class T>
// 	using tracked_forward_list = tracked_container<std::forward_list<T, tracked_allocator<T>>>;

// 	template<class K, class C = std::less<K>>
// 	using tracked_set = tracked_container<std::set<K, C, tracked_allocator<K>>>;

// 	template<class K, class C = std::less<K>>
// 	using tracked_multiset = tracked_container<std::multiset<K, C, tracked_allocator<K>>>;

// 	template<class K, class T, class C = std::less<K>>
// 	using tracked_map = tracked_container<std::map<K, T, C, tracked_allocator<std::pair<const K, T>>>>;

// 	template<class K, class T, class C = std::less<K>>
// 	using tracked_multimap = tracked_container<std::multimap<K, T, C, tracked_allocator<std::pair<const K, T>>>>;

// 	template<class K, class H = std::hash<K>, class E = std::equal_to<K>>
// 	using tracked_unordered_set = tracked_container<std::unordered_set<K, H, E, tracked_allocator<K>>>;

// 	template<class K, class H = std::hash<K>, class E = std::equal_to<K>>
// 	using tracked_unordered_multiset = tracked_container<std::unordered_multiset<K, H, E, tracked_allocator<K>>>;

// 	template<class K, class T, class H = std::hash<K>, class E = std::equal_to<K>>
// 	using tracked_unordered_map = tracked_container<std::unordered_map<K, T, H, E, tracked_allocator<std::pair<const K, T>>>>;

// 	template<class K, class T, class H = std::hash<K>, class E = std::equal_to<K>>
// 	using tracked_unordered_multimap = tracked_container<std::unordered_multimap<K, T, H, E, tracked_allocator<std::pair<const K, T>>>>;

// 	template<class T>
// 	using tracked_deque = tracked_container<std::deque<T, tracked_allocator<T>>>;

// 	template<class T, class Container = tracked_deque<T>>
// 	using tracked_queue = std::queue<T, Container>;

// 	template<class T, class Container = tracked_vector<T>, class Compare = std::less<typename Container::value_type>>
// 	using tracked_priority_queue = std::priority_queue<T, Container, Compare>;

// 	template<class T, class Container = tracked_deque<T>>
// 	using tracked_stack = std::stack<T, Container>;
// } // namespace sgcl

// #endif // SGCL_H

// template <typename T>
// class BoxedPrimitive2 {

//     sgcl::tracked_ptr<T> ptr;

//     public:

//     BoxedPrimitive2() {
//         ptr = sgcl::make_tracked<T>(1);
//     }
//     BoxedPrimitive2(const char * id) {
//         ptr = sgcl::make_tracked<T>(1);
//     }

//     BoxedPrimitive2(std::function<void(void*)> destructor) {
//         ptr = sgcl::make_tracked<T>(1, destructor);
//     }
//     BoxedPrimitive2(const char * id, std::function<void(void*)> destructor) {
//         ptr = sgcl::make_tracked<T>(1, destructor);
//     }

//     BoxedPrimitive2 & operator=(const T & value) {
//         ref() = value;
//         return *this;
//     }

//     BoxedPrimitive2 & operator=(T && value) {
//         ref() = std::move(value);
//         return *this;
//     }

//     T * operator -> () {
//         return ptr.get();
//     }

//     T & operator * () {
//         return *ptr.get();
//     }

//     auto & ref() {
//         return operator*();
//     }

//     auto value() {
//         return operator*();
//     }
// };

// /*
// template <typename T>
// class BoxedVar : public ManagedObject {
//     size_t id;

//     public:

//     BoxedVar() {
//         id = push_value<T*>(nullptr);
//     }
//     BoxedVar(const char * id) : ManagedObject(id) {
//         this->id = push_value<T*>(nullptr);
//     }

//     BoxedVar(std::function<void(void*)> destructor) {
//         id = push_value<T*>(nullptr, destructor);
//     }
//     BoxedVar(const char * id, std::function<void(void*)> destructor) : ManagedObject(id) {
//         this->id = push_value<T*>(nullptr, destructor);
//     }

//     BoxedVar<T> & operator=(const T & value) {
//         ref() = value;
//         return *this;
//     }

//     BoxedVar<T> & operator=(T && value) {
//         ref() = std::move(value);
//         return *this;
//     }

//     T * operator -> () {
//         T * & ptr = get_value_at<T*>(id);
//         if (ptr != nullptr) {
//             return ptr;
//         } else {
//             // assigns the 'ptr' regardless of what T is
//             // additionally, if T is derived from ManagedObject then it re-root's the allocation to our ManagedObject
//             reroot(id, new T()); // needed to re-root allocation to us 
//             return ptr;
//         }
//     }

//     T & operator * () {
//         return *operator->();
//     }

//     auto & ref() {
//         return operator*();
//     }

//     auto value() {
//         return operator*();
//     }
// };
// */

// template <typename T>
// using BoxedVar2 = BoxedPrimitive2<T>;

// typedef BoxedPrimitive2<int> BoxedInt2;
// typedef BoxedVar2<BoxedInt> IntegerHolder2;

// TEST(references2, 1) {
//     BoxedInt2 a("a");
//     BoxedInt2 b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif
// }
// /*
// TEST(references, 2) {
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 3) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 2);
//     ASSERT_EQ(b.value(), 2);
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 4) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 2\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 88\n");
// #endif
//     a = 88;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 88);
//     ASSERT_EQ(b.value(), 88);
//     ASSERT_EQ(h_a.ref().value(), 1);
//     ASSERT_EQ(h_b.ref().value(), 88);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 5) {
//     IntegerHolder h_a("holder_a");
//     IntegerHolder h_b("holder_b");

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_a = h_b\n");
// #endif
//     h_a = h_b;
    
//     BoxedInt a("a");
//     BoxedInt b("b");
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 1\n");
// #endif
//     a = 1;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("b = 1\n");
// #endif
//     b = 2;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 1);
//     ASSERT_EQ(b.value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("h_a = a\n");
// #endif
//     h_a.ref() = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("h_b = b\n");
// #endif
//     h_b.ref() = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(h_a.ref().value(), 2);
//     ASSERT_EQ(h_b.ref().value(), 2);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("a = b\n");
// #endif
//     a = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("a = 88\n");
// #endif
//     a = 88;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 88);
//     ASSERT_EQ(b.value(), 88);
//     ASSERT_EQ(h_a.ref().value(), 88);
//     ASSERT_EQ(h_b.ref().value(), 88);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// void set_int(BoxedInt var) {
//     var = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void set_int(IntegerHolder var) {
//     var.ref() = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void assign_int(BoxedInt var) {
//     var = BoxedInt("tmp");
//     var = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// void assign_int(IntegerHolder var) {
//     var = IntegerHolder("tmp");
//     var.ref() = 5;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts\n");
// #endif
//     ASSERT_EQ(var.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("set int asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// } 

// TEST(references, 6) {
//     BoxedInt a("a");
//     a = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     set_int(a);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 7) {
//     IntegerHolder b("holder a");
//     b.ref() = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     set_int(b);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 5);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 8) {
//     BoxedInt a("a");
//     a = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     assign_int(a);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(a.value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     ManagedObject::get_root()->print();
// #endif
// }

// TEST(references, 9) {
//     IntegerHolder b("holder a");
//     b.ref() = 8;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
//     printf("call set int\n");
// #endif
//     assign_int(b);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts\n");
// #endif
//     ASSERT_EQ(b.ref().value(), 8);
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     printf("asserts done\n");
// #endif

//     BoxedVar<char> c;
//     c = 'g';

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }

// struct T_A;
// struct T_B;

// struct T_A : ManagedObject {
//     BoxedVar<T_B> b = BoxedVar<T_B>("a.b");

//     T_A() {
//         byRef(b);
//     }

//     T_A(const char * id) : ManagedObject(id) {
//         byRef(b);
//     }
// };
// struct T_B : ManagedObject {
//     BoxedVar<T_A> a = BoxedVar<T_A>("b.a");

//     T_B() {
//         byRef(a);
//     }

//     T_B(const char * id) : ManagedObject(id) {
//         byRef(a);
//     }
// };

// TEST(references, 10) {
//     T_A a("a");
//     T_B b("b");
//     b.a = a;
//     a.b = b;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }

// typedef BoxedPrimitive<int*> BoxedIntPtr;

// TEST(references, 11) {
//     BoxedInt a("a");
//     a = 8;
//     BoxedInt b = a;
//     BoxedInt c;
//     c = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     b.dealloc();
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     b = a;
// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
//     BoxedIntPtr d("ptr");
//     d = new int();
//     *d.ref() = 1;

//     BoxedVar<BoxedPrimitive<int>> ptr_b("ptr_b");
//     ptr_b = BoxedPrimitive<int>("ptr_b value");
//     *ptr_b = 3;

//     BoxedVar<BoxedPrimitive<int>> ptr_b_c("ptr_b_c");
//     BoxedPrimitive<int> ptr_b_c_v("ptr_b_c value");
//     ptr_b_c = ptr_b_c_v;
//     *ptr_b_c = 3;

//     auto ptr_c = BoxedPrimitive<int>("ptr_c value");
//     ptr_c = 3;

//     auto cptr_a = BoxedVar<ManagedObject>("cycle_a_value");
//     auto cptr_b = BoxedVar<ManagedObject>("cycle_b_value");

//     *cptr_a = cptr_b;
//     *cptr_b = cptr_a;

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif

//     ManagedObject x;
//     *cptr_b = x;

// #ifdef MANAGED_OBJECT_HEAP_DEBUG
//     ManagedObject::get_root()->print();
// #endif
// }
// */