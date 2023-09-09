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

    static std::shared_ptr<ManagedObjectHeap> & get_root();

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
        return push_value(value, [](void * p) { delete static_cast<T>(p); });
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && !std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, size_t>::type
    push_value(T value, std::function<void(void*)> destructor) {
        return heap->push(value, destructor);
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, size_t>::type
    push_value(T value, std::function<void(void*)> destructor) {
        return heap->push(value, true, destructor);
    }

    template <typename T>
    T & get_value_at(size_t index) {
        static_assert(sizeof(T) <= sizeof(uintptr_t), "T is too large, please use a pointer instead");
        return (T&)heap->memory->memory[index].ptr;
    }

    ManagedObject();

    ManagedObject(const char * tag);

    ManagedObject(const ManagedObject & other);

    ManagedObject & operator=(const ManagedObject & other);

    template <typename T>
    typename std::enable_if<std::is_base_of<ManagedObject, T>::value, size_t>::type
    byRef(const T & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("ASSIGN VARIABLE %s TO VARIABLE %s\n", other.heap->tag, heap->tag);
        get_root()->print();
#endif
        other.parent = this;
        size_t id = heap->push(other.heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(!heap->memory->seen);
        assert(!other.heap->memory->seen);
        printf("REMOVING VARIABLE %s\n", other.heap->tag);
#endif
        auto root = get_root();
        for (auto itr = root->memory->memory.rbegin(); itr != root->memory->memory.rend(); itr++) {
            if (!itr->is_ptr && itr->ref == other.heap) {
                for (auto itr2 = root->memory->indexes.rbegin(); itr2 != root->memory->indexes.rend(); itr2++) {
                    if (*itr2 == itr->index) {
                        auto it2 = std::next(itr2).base();
                        root->memory->indexes.erase(it2, std::next(it2));
                        break;
                    }
                }
                auto it = std::next(itr).base();
                root->memory->memory.erase(it, std::next(it));
                break;
            }
        }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(root->memory->memory.size() == root->memory->indexes.size());
        get_root()->print();
#endif
        return id;
    }

    template <typename T>
    typename std::enable_if<!std::is_base_of<ManagedObject, T>::value, size_t>::type
    byRef(const T & other) {
        return -1;
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, void>::type
    reroot(size_t index, const T & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("ASSIGN VARIABLE %s TO VARIABLE %s\n", other->heap->tag, heap->tag);
        get_root()->print();
#endif
        other->parent = this;
        heap->memory->memory[index].ptr = other;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(!heap->memory->seen);
        assert(!other->heap->memory->seen);
        printf("REMOVING VARIABLE %s\n", other->heap->tag);
#endif
        auto root = get_root();
        for (auto itr = root->memory->memory.rbegin(); itr != root->memory->memory.rend(); itr++) {
            if (!itr->is_ptr && itr->ref == other->heap) {
                for (auto itr2 = root->memory->indexes.rbegin(); itr2 != root->memory->indexes.rend(); itr2++) {
                    if (*itr2 == itr->index) {
                        auto it2 = std::next(itr2).base();
                        root->memory->indexes.erase(it2, std::next(it2));
                        break;
                    }
                }
                auto it = std::next(itr).base();
                root->memory->memory.erase(it, std::next(it));
                break;
            }
        }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(root->memory->memory.size() == root->memory->indexes.size());
        get_root()->print();
#endif
    }

    template <typename T>
    typename std::enable_if<std::is_pointer<T>::value && !std::is_base_of<ManagedObject, typename std::remove_pointer<T>::type>::value, void>::type
    reroot(size_t index, const T & other) {
        heap->memory->memory[index].ptr = other;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        assert(!heap->memory->seen);
        get_root()->print();
#endif
    }

    ~ManagedObject();

    void dealloc();
};

#endif