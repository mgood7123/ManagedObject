#include <managed_object_heap.h>

#ifdef MANAGED_OBJECT_HEAP_DEBUG
#include <cassert>
#endif
#include <cstdio>

#include <mutex>

// std::recursive_mutex gc_lock;

// collect every 20 deallocations
//
uint8_t dealloc_limit = 10;

#define MANAGED_OBJECT_HEAP_COLOR_TO_STRING(color) color == MANAGED_OBJECT_HEAP_COLOR_WHITE ? "WHITE" : color == MANAGED_OBJECT_HEAP_COLOR_GRAY ? "GRAY" : "BLACK"

ManagedObjectHeap::Info::Info() {}

ManagedObjectHeap::Info::Info(size_t index) : index(index) {}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (void * ptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'VOID *' POINTER OBJECT %p\n", ptr);
#endif
    this->ptr = ptr;
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (uintptr_t ptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'UINTPTR_T' POINTER OBJECT\n");
#endif
    this->ptr = (void*)ptr;
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (ManagedObjectHeap copy) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'MANAGED_HEAP' COPY OBJECT\n");
#endif
    ManagedObjectHeap * s = new ManagedObjectHeap();
    *s = copy;
    this->ptr = s;
    ptr_is_ref = true;
    destructor = std::move(+[](void*p){
        #ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("DELETE MANAGED_HEAP '%s'\n", static_cast<ManagedObjectHeap*>(p)->tag);
        #endif
        delete static_cast<ManagedObjectHeap*>(p);
    });
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (ManagedObjectHeap * ref) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'MANAGED_HEAP' REFERENCE OBJECT\n");
#endif
    this->ptr = ref;
    ptr_is_ref = true;
    destructor = std::move(+[](void*p){
        #ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("DELETE MANAGED_HEAP '%s'\n", static_cast<ManagedObjectHeap*>(p)->tag);
        #endif
        delete static_cast<ManagedObjectHeap*>(p);
    });
    return *this;
}

ManagedObjectHeap::Info::~Info() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DEALLOCATING INFO OBJECT\n");
#endif
}

ManagedObjectHeap::Memory * ManagedObjectHeap::get_memory() {
    return (this->*get_memory_)();
}

ManagedObjectHeap::Memory * ManagedObjectHeap::get_memory1() {
    memory = new Memory();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATED MEMORY %p\n", memory);
#endif
    get_memory_ = &ManagedObjectHeap::get_memory2;
    return memory;
}

ManagedObjectHeap::Memory * ManagedObjectHeap::get_memory2() {
    return memory;
}

ManagedObjectHeap::ManagedObjectHeap() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE MANAGED_HEAP '%s'\n", tag);
#endif
    get_memory_ = &ManagedObjectHeap::get_memory1;
}

ManagedObjectHeap::ManagedObjectHeap(const char * tag) {
    this->tag = tag;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE MANAGED_HEAP '%s'\n", tag);
#endif
    get_memory_ = &ManagedObjectHeap::get_memory1;
}

ManagedObjectHeap::~ManagedObjectHeap() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DESTROY MANAGED_HEAP '%s'\n", tag);
#endif
    dealloc(get_root());
}

ManagedObjectHeap * ManagedObjectHeap::get_root() {
    static ManagedObjectHeap root = ManagedObjectHeap("root list");
    return & root;
}

bool inside_destructor = false;
size_t collect_count = 0;

void ManagedObjectHeap::dealloc(ManagedObjectHeap * root) {
    if (inside_destructor) return;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    if (root->get_memory()->seen) {
        return;
    }
    printf("DEALLOC MANAGED_HEAP '%s'\n", tag);
#endif
    deallocated = true;
    if (root == this) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("COLLECT ROOT\n");
#endif
        collect(root);

        // get_root()->dealloc() has been called, delete memory
        //
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("DELETE MEMORY %p\n", memory);
#endif
        delete memory;

#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("COLLECTED ROOT\n");
#endif
        return;
    } else {
        root->get_memory()->dealloc_count++;
        if (inside_destructor || (dealloc_limit != 0 && root->get_memory()->dealloc_count >= dealloc_limit)) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("DEALLOC COUNT: %d\n", root->get_memory()->dealloc_count);
            printf("DEALLOC LIMIT: %d\n", dealloc_limit);
            printf("COLLECT COUNT: %zu\n", collect_count);
#endif
            if (inside_destructor) {
                collect(root);
            } else {
                collect(root);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                printf("DEALLOC COUNT: %d\n", root->get_memory()->dealloc_count);
#endif
            }
        }
    }
}

size_t ManagedObjectHeap::collect() {
    return collect(get_root());
}

#include <chrono>

size_t ManagedObjectHeap::collect(ManagedObjectHeap * root) {
    if (root->get_memory()->memory.size() == 0) {
        return 0;
    }
    auto start = std::chrono::high_resolution_clock::now();
    if (inside_destructor) {
        printf("COLLECT INNER\n");
    } else {
        printf("COLLECT\n");
    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("indexes: %zu\n", get_memory()->indexes.size());
    printf("memories: %zu\n", get_memory()->memory.size());
    assert(get_memory()->indexes.size() == get_memory()->memory.size());
    printf("root indexes: %zu\n", root->get_memory()->indexes.size());
    printf("root memories: %zu\n", root->get_memory()->memory.size());
    assert(!root->get_memory()->seen);
    root->print();
    assert(!root->get_memory()->seen);
#endif
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("MARKING\n");
#endif
    root->mark();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!root->get_memory()->seen);
    printf("PRINTING MARKS\n");
    root->color();
    assert(!root->get_memory()->seen);
#endif
    size_t s = root->sweep_();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!root->get_memory()->seen);
#endif
    auto end = std::chrono::high_resolution_clock::now();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
#endif
    if (inside_destructor) {
        printf("COLLECTED (INNER) %zu objects in %f ms\n", s, std::chrono::duration<double, std::milli>(end - start).count());
    } else {
        printf("COLLECTED %zu objects (%zu total objects) in %f ms\n", s, collect_count, std::chrono::duration<double, std::milli>(end - start).count());
    }
    if (!inside_destructor) {
        root->get_memory()->dealloc_count = 0;
        collect_count = 0;
    }
    return s;
}

template <typename T>
void do_destroy(T & pair) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("RUNNING DESTRUCTOR - PRUNE PTR: %p\n", pair.first);
#endif
    pair.second(pair.first);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("RAN DESTRUCTOR\n");
#endif
}

template <typename T>
void do_memory_destroy(T & pair) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("RUNNING MEMORY DESTRUCTOR - PRUNE PTR: %p\n", pair.first);
#endif
    pair.second(pair.first);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("RAN MEMORY DESTRUCTOR\n");
#endif
}

size_t ManagedObjectHeap::sweep_() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SWEEPING\n");
    assert(!get_memory()->seen);
#endif
    size_t s = 0;
    {
        std::vector<std::pair<void*, void (*)(void*)>> list;
        std::vector<std::pair<void*, void (*)(void*)>> memory_list;

        do_sweep(list, memory_list);

        s = list.size() + memory_list.size();
        collect_count += s;

#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("RUNNING %zu DESTRUCTORS...\n", s);
#endif

        if (inside_destructor) {
            for (auto itr = list.rbegin(); itr != list.rend(); itr++) {
                do_destroy(*itr);
            }
            for (auto itr = memory_list.rbegin(); itr != memory_list.rend(); itr++) {
                do_memory_destroy(*itr);
            }
        } else {
            inside_destructor = true;
            for (auto itr = list.rbegin(); itr != list.rend(); itr++) {
                do_destroy(*itr);
            }
            for (auto itr = memory_list.rbegin(); itr != memory_list.rend(); itr++) {
                do_memory_destroy(*itr);
            }
            inside_destructor = false;
        }
    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("RAN %zu DESTRUCTORS\n", s);
#endif
    return s;
}

void ManagedObjectHeap::mark() {
    get_memory()->keep_memory = false;
    if (get_memory()->color != MANAGED_OBJECT_HEAP_COLOR_GRAY) {
        return;
    } else if (deallocated) {
        get_memory()->color = MANAGED_OBJECT_HEAP_COLOR_WHITE;
    } else {
        get_memory()->color = MANAGED_OBJECT_HEAP_COLOR_BLACK;
    }
    size_t m = get_memory()->memory.size();
    for (size_t i = 0; i < m; i++) {
        auto & info = get_memory()->memory[i];
        if (info.ptr_is_ref) {
            ManagedObjectHeap * ref = MANAGED_OBJECT_GET_HEAP(info);
            if (ref != nullptr) {
                ref->mark();
            }
        }
    }
}

void ManagedObjectHeap::do_sweep(std::vector<std::pair<void*, void (*)(void*)>> & list, std::vector<std::pair<void*, void (*)(void*)>> & memory_list) {
    if (get_memory()->seen) {
        return;
    }
    get_memory()->seen = true;
    auto & mem = get_memory()->memory;
    auto & indexes = get_memory()->indexes;
    if (get_memory()->color == MANAGED_OBJECT_HEAP_COLOR_BLACK) {
        get_memory()->color = MANAGED_OBJECT_HEAP_COLOR_GRAY;
    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(indexes.size() == mem.size());
#endif
    for (auto itr = mem.rbegin(); itr != mem.rend(); itr++) {
        auto & info = *itr;
        if (!info.ptr_is_ref) {
            if (get_memory()->color == MANAGED_OBJECT_HEAP_COLOR_WHITE) {
                for (auto itr2 = indexes.rbegin(); itr2 != indexes.rend(); itr2++) {
                    if (*itr2 == info.index) {
                        auto it2 = std::next(itr2).base();
                        indexes.erase(it2, std::next(it2));
                        break;
                    }
                }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                printf("QUEUE DESTRUCTOR - %p\n", info.ptr);
#endif
                list.emplace_back(std::move(info.ptr), std::move(info.destructor));
                info.ptr = nullptr;
                auto it = std::next(itr).base();
                mem.erase(it, std::next(it));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                assert(indexes.size() == mem.size());
#endif
            }
        } else {
            ManagedObjectHeap * ref = MANAGED_OBJECT_GET_HEAP(info);
            if (ref != nullptr) {
                if (ref->get_memory()->seen) {
                    // skip this reference, we are already processing it somewhere else so we dont want to touch it
                    continue;
                }
                ref->do_sweep(list, memory_list);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                assert(indexes.size() == mem.size());
#endif
                if (ref->get_memory()->keep_memory) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    printf("REMOVING OBJECT '%s' (ref %p, mem %p) WITHOUT REMOVING ITS MEMORY\n", ref->tag, ref, ref->get_memory());
#endif
                    for (auto itr2 = indexes.rbegin(); itr2 != indexes.rend(); itr2++) {
                        if (*itr2 == info.index) {
                            auto it2 = std::next(itr2).base();
                            indexes.erase(it2, std::next(it2));
                            break;
                        }
                    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    printf("QUEUE DESTRUCTOR - %p\n", info.ptr);
#endif
                    list.emplace_back(std::move(info.ptr), std::move(info.destructor));
                    info.ptr = nullptr;
                    auto it = std::next(itr).base();
                    mem.erase(it, std::next(it));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    assert(indexes.size() == mem.size());
#endif
                } else if (ref->get_memory()->color == MANAGED_OBJECT_HEAP_COLOR_WHITE) {
                    ref->get_memory()->keep_memory = true;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    printf("REMOVING OBJECT '%s' (ref %p, mem %p) AND ITS MEMORY\n", ref->tag, ref, ref->get_memory());
#endif
                    auto & mem2 = ref->get_memory()->memory;
                    auto & indexes2 = ref->get_memory()->indexes;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    assert(indexes.size() == mem.size());
                    assert(indexes2.size() == mem2.size());
#endif
                    for (auto itr2 = indexes2.rbegin(); itr2 != indexes2.rend(); itr2++) {
                        auto it2 = std::next(itr2).base();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                        printf("obtaining info2 at %zu\n", *it2);
#endif
                        auto & info2 = mem2[*it2];
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                        printf("QUEUE DESTRUCTOR - %p\n", info2.ptr);
#endif
                        list.emplace_back(std::move(info2.ptr), std::move(info2.destructor));
                        info2.ptr = nullptr;
                        for (auto itr3 = mem2.rbegin(); itr != mem2.rend(); itr3++) {
                            if (*itr2 == itr3->index) {
                                auto it3 = std::next(itr3).base();
                                mem2.erase(it3, std::next(it3));
                                break;
                            }
                        }
                        indexes2.erase(it2, std::next(it2));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                        assert(indexes2.size() == mem2.size());
#endif
                    }
                    for (auto itr2 = indexes.rbegin(); itr2 != indexes.rend(); itr2++) {
                        if (*itr2 == info.index) {
                            auto it2 = std::next(itr2).base();
                            indexes.erase(it2, std::next(it2));
                            break;
                        }
                    }

                    if (mem2.size() == 0) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                        printf("QUEUE MEMORY DESTRUCTOR - %p\n", ref->get_memory());
#endif
                        memory_list.emplace_back(std::move(ref->get_memory()), std::move(+[](void*p){
                            #ifdef MANAGED_OBJECT_HEAP_DEBUG
                                printf("DELETE MEMORY FOR MANAGED_HEAP\n");
                            #endif
                            delete static_cast<Memory*>(p);
                        }));
                    }

#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    printf("QUEUE DESTRUCTOR - %p\n", info.ptr);
#endif
                    list.emplace_back(std::move(info.ptr), std::move(info.destructor));
                    info.ptr = nullptr;

                    auto it = std::next(itr).base();
                    mem.erase(it, std::next(it));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    assert(indexes.size() == mem.size());
#endif
                }
            }
        }
    }
    get_memory()->seen = false;
}

void ManagedObjectHeap::color() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!get_memory()->seen);
#endif
    color([](){});
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!get_memory()->seen);
#endif
}

void ManagedObjectHeap::color(std::function<void()> prefix) {
    size_t m = get_memory()->memory.size();
    if (m == 0) {
        prefix();
        printf("[(ref %p, mem %p), %s [%s] ", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[ZERO SIZE] ]\n");
        return;
    }
    if (get_memory()->seen) {
        prefix();
        printf("[(ref %p, mem %p), %s [%s] ", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[CYCLIC DETECTED] ]\n");
        return;
    }
    if (deallocated) {
        prefix();
        printf("[(ref %p, mem %p), %s [%s] [deallocated] ]\n", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
    }
    get_memory()->seen = true;
    for (int i = 0; i < m; i++) {
        auto & info = get_memory()->memory[i];
        if (!info.ptr_is_ref) {
            prefix();
            printf("[(ref %p, mem %p), %s [%s] ", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
            if (deallocated) {
                printf("[deallocated] ");
            }
            printf("] memory[%d] = %p\n", i, info.ptr);
        } else {
            ManagedObjectHeap * ref = MANAGED_OBJECT_GET_HEAP(info);
            if (ref != nullptr) {
                ref->color([&, this]() {
                    prefix();
                    printf("[(ref %p, mem %p), %s [%s] ", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
                    if (deallocated) {
                        printf("[deallocated] ");
                    }
                    printf("] memory[%d] = ", i);
                });
            } else {
                prefix();
                printf("[(ref %p, mem %p), %s [%s] ", this, get_memory(), tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(get_memory()->color));
                if (deallocated) {
                    printf("[deallocated] ");
                }
                printf("] memory[%d] = %p\n", i, nullptr);
            }
        }
    }
    get_memory()->seen = false;
}

void ManagedObjectHeap::print() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!get_memory()->seen);
#endif
    print([](){});
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!get_memory()->seen);
#endif
}

void ManagedObjectHeap::print(std::function<void()> prefix) {
    size_t m = get_memory()->memory.size();
    if (m == 0) {
        prefix();
        printf("[(ref %p, mem %p), %s ", this, get_memory(), tag);
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[ZERO SIZE] ]\n");
        return;
    }
    if (get_memory()->seen) {
        prefix();
        printf("[(ref %p, mem %p), %s ", this, get_memory(), tag);
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[CYCLIC DETECTED] ]\n");
        return;
    }
    if (deallocated) {
        prefix();
        printf("[(ref %p, mem %p), %s [deallocated] ]\n", this, get_memory(), tag);
        return;
    }
    get_memory()->seen = true;
    for (int i = 0; i < m; i++) {
        auto & info = get_memory()->memory[i];
        if (!info.ptr_is_ref) {
            prefix();
            printf("[(ref %p, mem %p), %s] memory[%d] = %p\n", this, get_memory(), tag, i, info.ptr);
        } else {
            ManagedObjectHeap * ref = MANAGED_OBJECT_GET_HEAP(info);
            if (ref != nullptr) {
                ref->print([&, this](){
                    prefix();
                    if (info.ptr_is_ref) {
                        printf("[(ref %p, mem %p), %s] memory[%d] = (ref %p, mem %p) ", this, get_memory(), tag, i, ref, ref->get_memory());
                    } else {
                        printf("[(ref %p, mem %p), %s] memory[%d] = ", this, get_memory(), tag, i);
                    }
                });
            } else {
                prefix();
                printf("[(ref %p, mem %p), %s] memory[%d] = %p\n", this, get_memory(), tag, i, nullptr);
            }
        }
    }
    get_memory()->seen = false;
}

size_t ManagedObjectHeap::push() {
    Memory * m = get_memory();
    size_t index = m->memory.size();
    m->indexes.push_back(index);
    m->memory.emplace_back(index);
    return index;
}

size_t ManagedObjectHeap::push(ManagedObjectHeap * value) {
    Memory * m = get_memory();
    size_t index = m->memory.size();
    m->indexes.push_back(index);
    m->memory.emplace_back(index);
    auto & info = m->memory.back();
    info = value;
    return index;
}
