#ifndef MANAGED_OBJECT_STACK_H
#define MANAGED_OBJECT_STACK_H

#include "managed_object_bits.h"

#include <functional>
#include <memory>
#include <vector>

#define MANAGED_OBJECT_HEAP_COLOR_WHITE 0
#define MANAGED_OBJECT_HEAP_COLOR_GRAY 1
#define MANAGED_OBJECT_HEAP_COLOR_BLACK 2

struct ManagedObjectHeap {
    struct HeapHolder {
        mutable std::shared_ptr<ManagedObjectHeap> heap;
    };

    static std::shared_ptr<ManagedObjectHeap> & get_root();

    struct Info {
        size_t index = 0;
        void * ptr = nullptr;
        std::function<void(void*)> destructor;
        bool is_ptr = false;
        bool ptr_is_ref = false;

        std::shared_ptr<ManagedObjectHeap> ref;

        Info();

        Info(size_t index);

        Info & operator = (void * ptr);

        Info & operator = (uintptr_t ptr);

        Info & operator = (ManagedObjectHeap copy);

        Info & operator = (std::shared_ptr<ManagedObjectHeap> & ref);

        virtual ~Info();
    };

    struct Memory {
        std::vector<size_t> indexes;
        std::vector<Info> memory;
        bool seen  = false;
        uint8_t color = MANAGED_OBJECT_HEAP_COLOR_WHITE;
        bool sweep = false;
        uint8_t dealloc_count = 0;
    };

    const char * tag;
    std::shared_ptr<Memory> memory = std::make_shared<Memory>();
    bool sweep = false;
    bool deallocated = false;

    ManagedObjectHeap();

    ManagedObjectHeap(const char * tag);

    virtual ~ManagedObjectHeap();

    void dealloc(std::shared_ptr<ManagedObjectHeap> & root);
    
    void collect();
    
    void collect(std::shared_ptr<ManagedObjectHeap> & root);

    void print();

    void print(std::function<void()> prefix);

    void clear_marks();

    void mark();

    void do_sweep();

    void mark_prune();

    void prune();

    void prune(std::function<void()> prefix);

    void do_prune(std::vector<std::pair<void*, std::function<void(void*)>>> & list);

    void color();

    void color(std::function<void()> prefix);

    template <typename T>
    size_t push(T & value) {
        size_t index = memory->memory.size();
        memory->indexes.push_back(index);
        memory->memory.emplace_back(index);
        auto & info = memory->memory.back();
        info = value;
        info.destructor = [](void*p){};
        return index;
    }

    template <typename T>
    size_t push(std::shared_ptr<T> & value) {
        size_t index = memory->memory.size();
        memory->indexes.push_back(index);
        memory->memory.emplace_back(index);
        auto & info = memory->memory.back();
        info = value;
        info.destructor = [](void*p){};
        return index;
    }

    template <typename T>
    size_t push(T * & value, std::function<void(void*)> destructor) {
        size_t index = memory->memory.size();
        memory->indexes.push_back(index);
        memory->memory.emplace_back(index);
        auto & info = memory->memory.back();
        info = value;
        info.destructor = destructor;
        return index;
    }

    template <typename T>
    size_t push(T * & value, bool is_ref, std::function<void(void*)> destructor) {
        size_t index = memory->memory.size();
        memory->indexes.push_back(index);
        memory->memory.emplace_back(index);
        auto & info = memory->memory.back();
        info = value;
        info.destructor = destructor;
        info.ptr_is_ref = true;
        return index;
    }

    size_t push();

    static std::shared_ptr<ManagedObjectHeap> make();
    static std::shared_ptr<ManagedObjectHeap> make(const char * tag);

};

#endif
