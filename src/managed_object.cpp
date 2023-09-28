#include <managed_object.h>

ManagedObjectHeap * ManagedObject::get_root() {
    return ManagedObjectHeap::get_root();
}

ManagedObject::ManagedObject() : ManagedObject("no name") {}

ManagedObject::ManagedObject(const char * tag) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE VARIABLE %p, '%s'\n", this, tag);
#endif
    heap = new ManagedObjectHeap(tag);
    get_root()->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    get_root()->print();
#endif
}

ManagedObject::ManagedObject(const ManagedObject & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE VARIABLE %p, '%s'\n", this, other.heap->tag);
#endif
    heap = new ManagedObjectHeap(other.heap->tag);
    heap->memory = other.heap->memory; // copy of reference
    heap->get_memory_ = other.heap->get_memory_;
    heap->deallocated = other.heap->deallocated; // copy deallocated flag
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!heap->get_memory()->seen);
#endif
    get_root()->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    get_root()->print();
#endif
}

ManagedObject & ManagedObject::operator=(const ManagedObject & other) {
    if (this == &other) {
        return *this;
    }
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("ASSIGN VARIABLE '%s' TO VARIABLE '%s'\n", other.heap->tag, heap->tag);
#endif
    auto root = get_root();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    root->print();
#endif

    // offload heap to root
    auto old = new ManagedObjectHeap(heap->tag);
    *old = std::move(*heap);
    root->push(old);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("copying memory reference\n");
#endif
    *heap = *other.heap; // copy
    heap->tag = old->tag;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    get_root()->print();
    printf("DEALLOC OLD VARIABLE '%s'\n", old->tag);
#endif
    old->dealloc(root);

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    root->print();
#endif

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!heap->get_memory()->seen);
#endif
    return *this;
}

ManagedObject::~ManagedObject() {
    dealloc();
}

void ManagedObject::dealloc() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DEALLOC VARIABLE '%s'\n", heap->tag);
#endif
    heap->dealloc(get_root());
}
