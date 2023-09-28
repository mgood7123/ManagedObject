#ifndef MANAGED_OBJECT_STACK_H
#define MANAGED_OBJECT_STACK_H

#include "managed_object_bits.h"

#include <functional>
#include <memory>
#include <vector>

#define MANAGED_OBJECT_HEAP_COLOR_WHITE 0
#define MANAGED_OBJECT_HEAP_COLOR_GRAY 1
#define MANAGED_OBJECT_HEAP_COLOR_BLACK 2

#define MANAGED_OBJECT_GET_HEAP(info) (info.ptr_is_ref ? (info.ptr == nullptr ? nullptr : static_cast<ManagedObjectHeap*>(info.ptr)) : nullptr)

struct ManagedObjectHeap {
    struct HeapHolder {
        mutable ManagedObjectHeap * heap = nullptr;
    };

    static ManagedObjectHeap * get_root();

    struct Info {
        size_t index = 0;
        void * ptr = nullptr;
        void (*destructor)(void*);
        bool ptr_is_ref = false;

        Info();

        Info(size_t index);

        Info & operator = (void * ptr);

        Info & operator = (uintptr_t ptr);

        Info & operator = (ManagedObjectHeap copy);

        Info & operator = (ManagedObjectHeap * ref);

        virtual ~Info();
    };

    struct Memory {
        std::vector<size_t> indexes;
        std::vector<Info> memory;
        uint8_t color = MANAGED_OBJECT_HEAP_COLOR_GRAY;
        uint8_t dealloc_count = 0;
        bool seen  = false;
        bool keep_memory = false;
    };

    const char * tag = "no name";
    Memory * memory = nullptr;
    bool deallocated = false;
    Memory * (ManagedObjectHeap::*get_memory_)();

    ManagedObjectHeap();

    ManagedObjectHeap(const char * tag);

    virtual ~ManagedObjectHeap();

    void dealloc(ManagedObjectHeap * root);
    
    size_t collect();
    
    size_t collect(ManagedObjectHeap * root);

    void print();

    void print(std::function<void()> prefix);

    void mark();

    size_t sweep_();

    void do_sweep(std::vector<std::pair<void*, void (*)(void*)>> & list, std::vector<std::pair<void*, void (*)(void*)>> & memory_list);

    void color();

    void color(std::function<void()> prefix);

    
    Memory * get_memory();
    Memory * get_memory1();
    Memory * get_memory2();

    template <typename T>
    size_t push(T & value) {
        Memory * m = get_memory();
        size_t index = m->memory.size();
        m->indexes.push_back(index);
        m->memory.emplace_back(index);
        auto & info = m->memory.back();
        info = value;
        info.destructor = +[](void*){};
        return index;
    }

    // template <typename T>
    // size_t push(std::shared_ptr<T> & value) {
    //     Memory * m = get_memory();
    //     size_t index = m->memory.size();
    //     m->indexes.push_back(index);
    //     m->memory.emplace_back(index);
    //     auto & info = m->memory.back();
    //     info = value;
    //     return index;
    // }

    template <typename T>
    size_t push(T * value, void (*destructor)(void*)) {
        Memory * m = get_memory();
        size_t index = m->memory.size();
        m->indexes.push_back(index);
        m->memory.emplace_back(index);
        auto & info = m->memory.back();
        info = value;
        info.destructor = destructor;
        return index;
    }

    template <typename T>
    size_t push(T * value, bool is_ref, void (*destructor)(void*)) {
        Memory * m = get_memory();
        size_t index = m->memory.size();
        m->indexes.push_back(index);
        m->memory.emplace_back(index);
        auto & info = m->memory.back();
        info = value;
        info.destructor = destructor;
        return index;
    }

    size_t push(ManagedObjectHeap * value);

    size_t push();
};

#endif
