#include <managed_object_heap.h>

#ifdef MANAGED_OBJECT_HEAP_DEBUG
#include <cassert>
#endif
#include <cstdio>

#include <mutex>

// std::recursive_mutex gc_lock;

// collect every 20 deallocations
//
uint8_t dealloc_limit = 20;

#define MANAGED_OBJECT_HEAP_COLOR_TO_STRING(color) color == MANAGED_OBJECT_HEAP_COLOR_WHITE ? "WHITE" : color == MANAGED_OBJECT_HEAP_COLOR_GRAY ? "GRAY" : "BLACK"

const std::shared_ptr<ManagedObjectHeap> & empty_heap() {
    static const std::shared_ptr<ManagedObjectHeap> m;
    return m;
}

ManagedObjectHeap::Info::Info() {}

ManagedObjectHeap::Info::Info(size_t index) : index(index) {}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (void * ptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'VOID *' POINTER OBJECT %p\n", ptr);
#endif
    ref.reset();
    this->ptr = ptr;
    is_ptr = true;
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (uintptr_t ptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'UINTPTR_T' POINTER OBJECT\n");
#endif
    ref.reset();
    this->ptr = (void*)ptr;
    is_ptr = true;
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (ManagedObjectHeap copy) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'MANAGED_HEAP' COPY OBJECT\n");
#endif
    ref.reset();
    is_ptr = false;
    ref = std::make_shared<ManagedObjectHeap>(copy);
    return *this;
}

ManagedObjectHeap::Info & ManagedObjectHeap::Info::operator = (std::shared_ptr<ManagedObjectHeap> & ref) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SETTING A 'MANAGED_HEAP' REFERENCE OBJECT\n");
#endif
    this->ref.reset();
    is_ptr = false;
    this->ref = ref;
    return *this;
}

ManagedObjectHeap::Info::~Info() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DEALLOCATING INFO OBJECT\n");
#endif
}


ManagedObjectHeap::ManagedObjectHeap() {}

ManagedObjectHeap::ManagedObjectHeap(const char * tag) {
    this->tag = tag;
}

ManagedObjectHeap::~ManagedObjectHeap() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DESTROY MANAGED_HEAP\n");
#endif
    dealloc(get_root());
}

std::shared_ptr<ManagedObjectHeap> & ManagedObjectHeap::get_root() {
    static std::shared_ptr<ManagedObjectHeap> root = ManagedObjectHeap::make("root list");
    return root;
}

void ManagedObjectHeap::dealloc(std::shared_ptr<ManagedObjectHeap> & root) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    if (root->memory->seen) {
        return;
    }
    printf("DEALLOC MANAGED_HEAP\n");
#endif
    deallocated = true;
    if (root.get() == this) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("COLLECT ROOT\n");
#endif
        collect(root);
        root->memory->dealloc_count = 0;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("COLLECTED ROOT\n");
#endif
    } else {
        root->memory->dealloc_count++;
        if (dealloc_limit != 0) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("DEALLOC COUNT: %d\n", root->memory->dealloc_count);
            printf("DEALLOC LIMIT: %d\n", dealloc_limit);
#endif
            if (root->memory->dealloc_count == dealloc_limit) {
                collect(root);
                root->memory->dealloc_count = 0;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                printf("DEALLOC COUNT: %d\n", root->memory->dealloc_count);
#endif
            }

            if (root->memory->dealloc_count > dealloc_limit) {
                collect(root);
            }
        }
    }
}

void ManagedObjectHeap::collect() {
    collect(get_root());
}

void ManagedObjectHeap::collect(std::shared_ptr<ManagedObjectHeap> & root) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    if (root->memory->dealloc_count > dealloc_limit) {
        printf("COLLECT INNER\n");
    } else {
        printf("COLLECT\n");
    }
    printf("indexes: %zu\n", memory->indexes.size());
    printf("memories: %zu\n", memory->memory.size());
    assert(memory->indexes.size() == memory->memory.size());
    printf("root indexes: %zu\n", root->memory->indexes.size());
    printf("root memories: %zu\n", root->memory->memory.size());
    assert(!root->memory->seen);
    root->print();
    assert(!root->memory->seen);
    printf("ERASING MARKS\n");
#endif
    root->clear_marks();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!root->memory->seen);
    printf("MARKING\n");
#endif
    root->mark();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!root->memory->seen);
    printf("PRINTING MARKS\n");
    root->color();
    assert(!root->memory->seen);
#endif
    root->do_sweep();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!root->memory->seen);
#endif
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    if (root->memory->dealloc_count > dealloc_limit) {
        printf("COLLECTED INNER\n");
    } else {
        printf("COLLECTED\n");
    }
#endif
}

void ManagedObjectHeap::print() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
    print([](){});
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
}

void ManagedObjectHeap::print(std::function<void()> prefix) {
    size_t m = memory->memory.size();
    if (m == 0) {
        prefix();
        printf("[%p, %s ", this, tag);
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[ZERO SIZE] ]\n");
        return;
    }
    if (memory->seen) {
        prefix();
        printf("[%p, %s ", this, tag);
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[CYCLIC DETECTED] ]\n");
        return;
    }
    if (deallocated) {
        prefix();
        printf("[%p, %s [deallocated] ]\n", this, tag);
        return;
    }
    memory->seen = true;
    for (int i = 0; i < m; i++) {
        auto & info = memory->memory[i];
        if (info.is_ptr && !info.ptr_is_ref) {
            prefix();
            printf("[%p, %s] memory[%d] = %p\n", this, tag, i, info.ptr);
        } else {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                ref->print([&, this](){
                    prefix();
                    if (info.ptr_is_ref) {
                        printf("[%p, %s] memory[%d] = (ref %p) ", this, tag, i, info.ptr);
                    } else {
                        printf("[%p, %s] memory[%d] = ", this, tag, i);
                    }
                });
            } else {
                prefix();
                printf("[%p, %s] memory[%d] = %p\n", this, tag, i, nullptr);
            }
        }
    }
    memory->seen = false;
}

void ManagedObjectHeap::clear_marks() {
    if (memory->seen) {
        return;
    }
    memory->color = MANAGED_OBJECT_HEAP_COLOR_WHITE;
    memory->sweep = false;
    memory->seen = true;
    size_t m = memory->memory.size();
    for (size_t i = 0; i < m; i++) {
        auto & info = memory->memory[i];
        if (!info.is_ptr || info.ptr_is_ref) {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                ref->clear_marks();
            }
        }
    }
    memory->seen = false;
}

void ManagedObjectHeap::mark() {
    size_t m = memory->memory.size();
    if (m == 0 || memory->seen || memory->color == MANAGED_OBJECT_HEAP_COLOR_BLACK) {
        return;
    }
    if (deallocated) {
        memory->color = MANAGED_OBJECT_HEAP_COLOR_WHITE;
    } else {
        memory->color = MANAGED_OBJECT_HEAP_COLOR_GRAY;
        memory->seen = true;
        for (size_t i = 0; i < m; i++) {
            auto & info = memory->memory[i];
            if (!info.is_ptr || info.ptr_is_ref) {
                auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
                if (ref.get() != nullptr) {
                    ref->mark();
                }
            }
        }
        memory->seen = false;
        memory->color = MANAGED_OBJECT_HEAP_COLOR_BLACK;
    }
}

void ManagedObjectHeap::do_sweep() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("SWEEPING\n");
    assert(!memory->seen);
#endif
    mark_prune();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
    prune();
    assert(!memory->seen);
    printf("PRUNING\n");
    assert(!memory->seen);
#endif
    {
        std::vector<std::pair<void*, std::function<void(void*)>>> list;

        do_prune(list);

#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("RUNNING %zu DESTRUCTORS...\n", list.size());
#endif

        for(auto & p : list) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("RUNNING DESTRUCTOR - PRUNE PTR: %p\n", p.first);
#endif
            p.second(p.first);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
            printf("RAN DESTRUCTOR\n");
#endif
        }

#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("RAN %zu DESTRUCTORS\n", list.size());
#endif
    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
    prune();
    assert(!memory->seen);
#endif
}

void ManagedObjectHeap::mark_prune() {
    if (memory->color == MANAGED_OBJECT_HEAP_COLOR_WHITE) {
        memory->sweep = true;
    }
    if (memory->color == MANAGED_OBJECT_HEAP_COLOR_BLACK && deallocated) {
        sweep = true;
    }
    if (memory->seen) {
        return;
    }
    memory->seen = true;
    size_t m = memory->memory.size();
    for (size_t i = 0; i < m; i++) {
        auto & info = memory->memory[i];
        if (!info.is_ptr || info.ptr_is_ref) {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                ref->mark_prune();
            }
        }
    }
    memory->seen = false;
}

void ManagedObjectHeap::prune() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
    prune([](){});
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
}

void ManagedObjectHeap::prune(std::function<void()> prefix) {
    size_t m = memory->memory.size();
    if (m == 0) {
        prefix();
        printf("[%p, %s [%s] ", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[ZERO SIZE] ]\n");
        return;
    }
    if (memory->seen) {
        prefix();
        printf("[%p, %s [%s] ", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[CYCLIC DETECTED] ]\n");
        return;
    }
    if (deallocated) {
        prefix();
        printf("[%p, %s [%s] [deallocated] ]\n", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
    }
    memory->seen = true;
    for (int i = 0; i < m; i++) {
        auto & info = memory->memory[i];
        if (info.is_ptr && !info.ptr_is_ref) {
            prefix();
            printf("[%p, %s [%s] ", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
            if (deallocated) {
                printf("[deallocated] ");
            }
            printf("] memory[%d] = %p\n", i, info.ptr);
        } else {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                ref->prune([&, this]() {
                    prefix();
                    printf("[%p, %s [%s] ", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
                    if (deallocated) {
                        printf("[deallocated] ");
                    }
                    if (info.ptr_is_ref) {
                        printf("] memory[%d] = (ref %p) ", i, info.ptr);
                    } else {
                        printf("] memory[%d] = ", i);
                    }
                });
            } else {
                prefix();
                printf("[%p, %s [%s] ", this, tag, sweep ? "SWEEP OBJECT ONLY" : memory->sweep ? "SWEEP OBJECT AND MEMORY" : "KEEP");
                if (deallocated) {
                    printf("[deallocated] ");
                }
                printf("] memory[%d] = %p\n", i, nullptr);
            }
        }
    }
    memory->seen = false;
}

void ManagedObjectHeap::do_prune(std::vector<std::pair<void*, std::function<void(void*)>>> & list) {
    if (memory->seen) {
        return;
    }
    memory->seen = true;
    auto & mem = memory->memory;
    auto & indexes = memory->indexes;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(indexes.size() == mem.size());
#endif
    for (auto itr = mem.rbegin(); itr != mem.rend(); itr++) {
        auto & info = *itr;
        if (!info.is_ptr || info.ptr_is_ref) {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                if (ref->memory->seen) {
                    // skip this reference, we are already processing it somewhere else so we dont want to touch it
                    continue;
                }
                ref->do_prune(list);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                assert(indexes.size() == mem.size());
#endif
                if (ref->sweep) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    printf("REMOVING OBJECT '%s' ONLY\n", ref->tag);
#endif
                    for (auto itr2 = indexes.rbegin(); itr2 != indexes.rend(); itr2++) {
                        if (*itr2 == info.index) {
                            auto it2 = std::next(itr2).base();
                            indexes.erase(it2, std::next(it2));
                            break;
                        }
                    }
                    auto it = std::next(itr).base();
                    mem.erase(it, std::next(it));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    assert(indexes.size() == mem.size());
#endif
                } else if (ref->memory->sweep) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    if (info.ptr_is_ref) {
                        printf("REMOVING OBJECT '%s' (ref %p) AND ITS MEMORY\n", ref->tag, info.ptr);
                    } else {
                        printf("REMOVING OBJECT '%s' AND ITS MEMORY\n", ref->tag);
                    }
#endif
                    auto & mem2 = ref->memory->memory;
                    auto & indexes2 = ref->memory->indexes;
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
                        if (info2.is_ptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                            printf("QUEUE DESTRUCTOR\n");
#endif
                            list.emplace_back(std::move(info2.ptr), std::move(info2.destructor));
                            info2.ptr = nullptr;
                        }
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
                    if (info.ptr_is_ref) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                        printf("QUEUE DESTRUCTOR\n");
#endif
                        list.emplace_back(std::move(info.ptr), std::move(info.destructor));
                        info.ptr = nullptr;
                    }
                    auto it = std::next(itr).base();
                    mem.erase(it, std::next(it));
#ifdef MANAGED_OBJECT_HEAP_DEBUG
                    assert(indexes.size() == mem.size());
#endif
                }
            }
        }
    }
    memory->seen = false;
}

void ManagedObjectHeap::color() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
    color([](){});
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!memory->seen);
#endif
}

void ManagedObjectHeap::color(std::function<void()> prefix) {
    size_t m = memory->memory.size();
    if (m == 0) {
        prefix();
        printf("[%p, %s [%s] ", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[ZERO SIZE] ]\n");
        return;
    }
    if (memory->seen) {
        prefix();
        printf("[%p, %s [%s] ", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
        if (deallocated) {
            printf("[deallocated] ");
        }
        printf("[CYCLIC DETECTED] ]\n");
        return;
    }
    if (deallocated) {
        prefix();
        printf("[%p, %s [%s] [deallocated] ]\n", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
    }
    memory->seen = true;
    for (int i = 0; i < m; i++) {
        auto & info = memory->memory[i];
        if (info.is_ptr && !info.ptr_is_ref) {
            prefix();
            printf("[%p, %s [%s] ", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
            if (deallocated) {
                printf("[deallocated] ");
            }
            printf("] memory[%d] = %p\n", i, info.ptr);
        } else {
            auto & ref = info.ptr_is_ref ? (info.ptr == nullptr ? empty_heap() : static_cast<ManagedObjectHeap::HeapHolder*>(info.ptr)->heap) : info.ref;
            if (ref.get() != nullptr) {
                ref->color([&, this]() {
                    prefix();
                    printf("[%p, %s [%s] ", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
                    if (deallocated) {
                        printf("[deallocated] ");
                    }
                    if (info.ptr_is_ref) {
                        printf("] memory[%d] = (ref %p) ", i, info.ptr);
                    } else {
                        printf("] memory[%d] = ", i);
                    }
                });
            } else {
                prefix();
                printf("[%p, %s [%s] ", this, tag, MANAGED_OBJECT_HEAP_COLOR_TO_STRING(memory->color));
                if (deallocated) {
                    printf("[deallocated] ");
                }
                printf("] memory[%d] = %p\n", i, nullptr);
            }
        }
    }
    memory->seen = false;
}

size_t ManagedObjectHeap::push() {
    size_t index = memory->memory.size();
    memory->indexes.push_back(index);
    memory->memory.emplace_back(index);
    return index;
}

std::shared_ptr<ManagedObjectHeap> ManagedObjectHeap::make() {
    return std::make_shared<ManagedObjectHeap>();
}

std::shared_ptr<ManagedObjectHeap> ManagedObjectHeap::make(const char * tag) {
    return std::make_shared<ManagedObjectHeap>(tag);
}
