#include <managed_object.h>

std::shared_ptr<ManagedObjectHeap> & ManagedObject::get_root() {
    return ManagedObjectHeap::get_root();
}

ManagedObject::ManagedObject() : ManagedObject("no name") {}

ManagedObject::ManagedObject(const char * tag) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE VARIABLE %p, %s\n", this, tag);
#endif
    heap = ManagedObjectHeap::make(tag);
    get_root()->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    get_root()->print();
#endif
}

ManagedObject::ManagedObject(const ManagedObject & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("CREATE VARIABLE %s\n", other.heap->tag);
#endif
    heap = ManagedObjectHeap::make(other.heap->tag);
    heap->memory = other.heap->memory; // copy of reference
    heap->deallocated = other.heap->deallocated; // copy deallocated flag
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!heap->memory->seen);
#endif
    get_root()->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    get_root()->print();
#endif
}

ManagedObject & ManagedObject::operator=(const ManagedObject & other) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("ASSIGN VARIABLE %s TO VARIABLE %s\n", other.heap->tag, heap->tag);
#endif
    auto root = get_root();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    root->print();
#endif
    auto old = heap;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("copying memory reference\n");
#endif
    heap = ManagedObjectHeap::make(old->tag);
    heap->memory = other.heap->memory; // copy of reference
    heap->deallocated = other.heap->deallocated; // copy deallocated flag
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    assert(!heap->memory->seen);
#endif
    if (parent != nullptr) {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
#endif
        parent->heap->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
#endif
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        printf("DEALLOC VARIABLE %s\n", old->tag);
#endif
        old->dealloc(root);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
#endif
    } else {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
#endif
        root->push(heap); // reference
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
        printf("DEALLOC VARIABLE %s\n", old->tag);
#endif
        old->dealloc(root);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
        root->print();
#endif
    }
    return *this;
}

ManagedObject::~ManagedObject() {
    dealloc();
}

void ManagedObject::dealloc() {
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("DEALLOC VARIABLE %s\n", heap->tag);
#endif
    heap->dealloc(get_root());
}
