#ifndef MANAGED_OBJECT_H
#define MANAGED_OBJECT_H

#include <managed_object_heap.h>
#ifdef MANAGED_OBJECT_HEAP_DEBUG
#include <cassert>
#endif
#include <cstdio>

#define MANAGED_OBJECT_STATIC_ASSERT_IS_PRIMITIVE_LIKE(T, msg) static_assert(!std::is_pointer<T>::value || std::is_pointer<T>::value && !std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, msg);

struct ManagedObject : ManagedObjectHeap::HeapHolder {

    mutable ManagedObject * parent = nullptr;

    static ManagedObjectHeap * get_root();

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value, size_t>::type
    push_value(T & value) {
        static_assert(sizeof(T) <= sizeof(uintptr_t), "T is too large, please use a pointer instead");
        return heap->push(value);
    }

    template <typename T>
    typename std::enable_if<!std::is_pointer<T>::value && std::is_trivially_copyable<T>::value, size_t>::type
    push_value(const T & value) {
        static_assert(sizeof(T) <= sizeof(uintptr_t), "T is too large, please use a pointer instead");
        return heap->push(value);
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value, size_t>::type
    push_value(T value) {
        return push_value(value, +[](void * p) { delete static_cast<T>(p); });
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && !std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, size_t>::type
    push_value(T value, void (*destructor)(void*)) {
        return heap->push(value, destructor);
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, size_t>::type
    push_value(T value, void (*destructor)(void*)) {
        return heap->push(value, true, destructor);
    }

    template <typename T>
    T & get_value_at(size_t index) {
        static_assert(sizeof(T) <= sizeof(uintptr_t), "T is too large, please use a pointer instead");
        return (T&)heap->get_memory()->memory[index].ptr;
    }

    ManagedObject();

    ManagedObject(const char * tag);

    ManagedObject(const ManagedObject & other);

    ManagedObject & operator=(const ManagedObject & other);

    template <typename T>
    typename std::enable_if<std::is_base_of<ManagedObject, T>::value, size_t>::type
    byRef(const T & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("BYREF: ASSIGN VARIABLE '%s' TO VARIABLE '%s'\n", other.heap->tag, heap->tag);
        get_root()->print();
#endif
        other.parent = this;
        size_t id = heap->push(other.heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(!heap->get_memory()->seen);
        assert(!other.heap->get_memory()->seen);
        printf("REMOVING VARIABLE '%s'\n", other.heap->tag);
#endif
        auto root = get_root();
        for (auto itr = root->get_memory()->memory.rbegin(); itr != root->get_memory()->memory.rend(); itr++) {
            if (MANAGED_OBJECT_GET_HEAP((*itr)) == other.heap) {
                for (auto itr2 = root->get_memory()->indexes.rbegin(); itr2 != root->get_memory()->indexes.rend(); itr2++) {
                    if (*itr2 == itr->index) {
                        auto it2 = std::next(itr2).base();
                        root->get_memory()->indexes.erase(it2, std::next(it2));
                        break;
                    }
                }
                auto it = std::next(itr).base();
                root->get_memory()->memory.erase(it, std::next(it));
                break;
            }
        }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(root->get_memory()->memory.size() == root->get_memory()->indexes.size());
        get_root()->print();
#endif
        return id;
    }

    template <typename T>
    typename std::enable_if<!std::is_base_of<ManagedObject, T>::value, size_t>::type
    byRef(const T & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("BYREF: CANNOT ASSIGN T WHICH IS NOT A MANAGED OBJECT\n");
#endif
        return -1;
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, T &>::type
    reroot(size_t index, const T & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("REROOT: ASSIGN VARIABLE '%s' TO VARIABLE '%s'\n", other->heap->tag, heap->tag);
        get_root()->print();
#endif
        other->parent = this;
        auto & info = heap->get_memory()->memory[index];
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(info.ptr == nullptr && "can only reroot to a nullptr index");
#endif
        info = other;
        heap->push(other->heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(!heap->get_memory()->seen);
        assert(!other->heap->get_memory()->seen);
        printf("REMOVING VARIABLE '%s'\n", other->heap->tag);
#endif
        auto root = get_root();
        for (auto itr = root->get_memory()->memory.rbegin(); itr != root->get_memory()->memory.rend(); itr++) {
            if (MANAGED_OBJECT_GET_HEAP((*itr)) == other->heap) {
                for (auto itr2 = root->get_memory()->indexes.rbegin(); itr2 != root->get_memory()->indexes.rend(); itr2++) {
                    if (*itr2 == itr->index) {
                        auto it2 = std::next(itr2).base();
                        root->get_memory()->indexes.erase(it2, std::next(it2));
                        break;
                    }
                }
                auto it = std::next(itr).base();
                root->get_memory()->memory.erase(it, std::next(it));
                break;
            }
        }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(root->get_memory()->memory.size() == root->get_memory()->indexes.size());
        get_root()->print();
#endif
        // pushing the heap may have invalidated our previous info, re-obtain it
        return (T&)heap->get_memory()->memory[index].ptr;
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && !std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, T &>::type
    reroot(size_t index, const T & other) {
        auto & info = heap->get_memory()->memory[index];
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(info.ptr == nullptr && "can only reroot to a nullptr index");
#endif
        info = other;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        get_root()->print();
#endif
        return (T&)info.ptr;
    }

    ~ManagedObject();

    void dealloc();
};

#endif