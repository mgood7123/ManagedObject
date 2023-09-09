#include "gtest/gtest.h"

#include <managed_object.h>

/*

// kept for reference

void test_st() {
    auto root = ManagedObjectHeap::make("root", 2);
    auto root_ref = ManagedObjectHeap::make("root ref", 1);
    auto root_copy = ManagedObjectHeap::make("root copy", 1);
    auto root_real = ManagedObjectHeap::make("root real", 2);
    auto a_a = ManagedObjectHeap::make("a_a", 3);
    auto a_b = ManagedObjectHeap::make("a_b", 2);
    auto c = ManagedObjectHeap::make("c", 3);
    root->get_memory()[0] = a_a; // reference
    root->get_memory()[1] = c; // reference
    a_a->get_memory()[0] = 0x1;
    a_a->get_memory()[1] = 0x2;
    a_a->get_memory()[2] = a_b; // reference
    a_b->get_memory()[0] = 0x3;
    c->get_memory()[0] = 0x8;
    c->get_memory()[1] = a_a; // reference
    c->get_memory()[2] = *a_a; // copy
    a_b->get_memory()[1] = *c; // copy
    root_ref->get_memory()[0] = root; // reference
    root_copy->get_memory()[0] = *root; // copy
    root_real->get_memory()[0] = root_ref; // reference
    root_real->get_memory()[1] = root_copy; // reference
    root_real->print();
    printf("deallocating c\n");
    c->dealloc(root_real); // c = nullptr
    root_real->print();
}
*/

template <typename T>
class BoxedPrimitive : public ManagedObject {

    MANAGED_OBJECT_STATIC_ASSERT_IS_PRIMITIVE_LIKE(T, "T cannot be a pointer to a ManagedObject, use BoxedVar<T> instead");

    size_t id;

    public:

    BoxedPrimitive() {
        id = push_value<T>(T());
    }
    BoxedPrimitive(const char * id) : ManagedObject(id) {
        this->id = push_value<T>(T());
    }

    BoxedPrimitive(std::function<void(void*)> destructor) {
        id = push_value<T>(T(), destructor);
    }
    BoxedPrimitive(const char * id, std::function<void(void*)> destructor) : ManagedObject(id) {
        this->id = push_value<T>(T(), destructor);
    }

    BoxedPrimitive & operator=(const T & value) {
        ref() = value;
        return *this;
    }

    BoxedPrimitive & operator=(T && value) {
        ref() = std::move(value);
        return *this;
    }

    T * operator -> () {
        return &get_value_at<T>(id);
    }

    T & operator * () {
        return get_value_at<T>(id);
    }

    auto & ref() {
        return operator*();
    }

    auto value() {
        return operator*();
    }
};

template <typename T>
class BoxedVar : public ManagedObject {
    size_t id;

    public:

    BoxedVar() {
        id = push_value<T*>(nullptr);
    }
    BoxedVar(const char * id) : ManagedObject(id) {
        this->id = push_value<T*>(nullptr);
    }

    BoxedVar(std::function<void(void*)> destructor) {
        id = push_value<T*>(nullptr, destructor);
    }
    BoxedVar(const char * id, std::function<void(void*)> destructor) : ManagedObject(id) {
        this->id = push_value<T*>(nullptr, destructor);
    }

    BoxedVar<T> & operator=(const T & value) {
        ref() = value;
        return *this;
    }

    BoxedVar<T> & operator=(T && value) {
        ref() = std::move(value);
        return *this;
    }

    T * operator -> () {
        T * & ptr = get_value_at<T*>(id);
        if (ptr != nullptr) {
            return ptr;
        } else {
            // assigns the 'ptr' regardless of what T is
            // additionally, if T is derived from ManagedObject then it re-root's the allocation to our ManagedObject
            reroot(id, new T()); // needed to re-root allocation to us 
            return ptr;
        }
    }

    T & operator * () {
        return *operator->();
    }

    auto & ref() {
        return operator*();
    }

    auto value() {
        return operator*();
    }
};

typedef BoxedPrimitive<int> BoxedInt;
typedef BoxedVar<BoxedInt> IntegerHolder;

TEST(references, 1) {
    BoxedInt a("a");
    BoxedInt b("b");
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 1\n");
#endif
    a = 1;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("b = 2\n");
#endif
    b = 2;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 1);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("a = b\n");
#endif
    a = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 2);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
#endif
}

TEST(references, 2) {
    BoxedInt a("a");
    BoxedInt b("b");
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 1\n");
#endif
    a = 1;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("b = 2\n");
#endif
    b = 2;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 1);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
#endif
    a = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 2);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 3) {
    IntegerHolder h_a("holder_a");
    IntegerHolder h_b("holder_b");
    
    BoxedInt a("a");
    BoxedInt b("b");
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 1\n");
#endif
    a = 1;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("b = 2\n");
#endif
    b = 2;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 1);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("h_a = a\n");
#endif
    h_a.ref() = a;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("h_b = b\n");
#endif
    h_b.ref() = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(h_a.ref().value(), 1);
    ASSERT_EQ(h_b.ref().value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("a = b\n");
#endif
    a = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 2);
    ASSERT_EQ(b.value(), 2);
    ASSERT_EQ(h_a.ref().value(), 1);
    ASSERT_EQ(h_b.ref().value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 4) {
    IntegerHolder h_a("holder_a");
    IntegerHolder h_b("holder_b");
    
    BoxedInt a("a");
    BoxedInt b("b");
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 1\n");
#endif
    a = 1;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("b = 2\n");
#endif
    b = 2;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 1);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("h_a = a\n");
#endif
    h_a.ref() = a;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("h_b = b\n");
#endif
    h_b.ref() = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(h_a.ref().value(), 1);
    ASSERT_EQ(h_b.ref().value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("a = b\n");
#endif
    a = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 88\n");
#endif
    a = 88;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 88);
    ASSERT_EQ(b.value(), 88);
    ASSERT_EQ(h_a.ref().value(), 1);
    ASSERT_EQ(h_b.ref().value(), 88);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 5) {
    IntegerHolder h_a("holder_a");
    IntegerHolder h_b("holder_b");

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("h_a = h_b\n");
#endif
    h_a = h_b;
    
    BoxedInt a("a");
    BoxedInt b("b");
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 1\n");
#endif
    a = 1;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("b = 1\n");
#endif
    b = 2;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 1);
    ASSERT_EQ(b.value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("h_a = a\n");
#endif
    h_a.ref() = a;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("h_b = b\n");
#endif
    h_b.ref() = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(h_a.ref().value(), 2);
    ASSERT_EQ(h_b.ref().value(), 2);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("a = b\n");
#endif
    a = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("a = 88\n");
#endif
    a = 88;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 88);
    ASSERT_EQ(b.value(), 88);
    ASSERT_EQ(h_a.ref().value(), 88);
    ASSERT_EQ(h_b.ref().value(), 88);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

void set_int(BoxedInt var) {
    var = 5;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts\n");
#endif
    ASSERT_EQ(var.value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts done\n");
    ManagedObject::get_root()->print();
#endif
} 

void set_int(IntegerHolder var) {
    var.ref() = 5;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts\n");
#endif
    ASSERT_EQ(var.ref().value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts done\n");
    ManagedObject::get_root()->print();
#endif
} 

void assign_int(BoxedInt var) {
    var = BoxedInt("tmp");
    var = 5;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts\n");
#endif
    ASSERT_EQ(var.value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts done\n");
    ManagedObject::get_root()->print();
#endif
} 

void assign_int(IntegerHolder var) {
    var = IntegerHolder("tmp");
    var.ref() = 5;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts\n");
#endif
    ASSERT_EQ(var.ref().value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("set int asserts done\n");
    ManagedObject::get_root()->print();
#endif
} 

TEST(references, 6) {
    BoxedInt a("a");
    a = 8;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("call set int\n");
#endif
    set_int(a);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 7) {
    IntegerHolder b("holder a");
    b.ref() = 8;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(b.ref().value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("call set int\n");
#endif
    set_int(b);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(b.ref().value(), 5);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 8) {
    BoxedInt a("a");
    a = 8;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("call set int\n");
#endif
    assign_int(a);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(a.value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    ManagedObject::get_root()->print();
#endif
}

TEST(references, 9) {
    IntegerHolder b("holder a");
    b.ref() = 8;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(b.ref().value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
    printf("call set int\n");
#endif
    assign_int(b);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts\n");
#endif
    ASSERT_EQ(b.ref().value(), 8);
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    printf("asserts done\n");
#endif

    BoxedVar<char> c;
    c = 'g';

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
}

struct T_A;
struct T_B;

struct T_A : ManagedObject {
    BoxedVar<T_B> b = BoxedVar<T_B>("a.b");

    T_A() {
        byRef(b);
    }

    T_A(const char * id) : ManagedObject(id) {
        byRef(b);
    }
};
struct T_B : ManagedObject {
    BoxedVar<T_A> a = BoxedVar<T_A>("b.a");

    T_B() {
        byRef(a);
    }

    T_B(const char * id) : ManagedObject(id) {
        byRef(a);
    }
};

TEST(references, 10) {
    T_A a("a");
    T_B b("b");
    b.a = a;
    a.b = b;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
}

typedef BoxedPrimitive<int*> BoxedIntPtr;

TEST(references, 11) {
    BoxedInt a("a");
    a = 8;
    BoxedInt b = a;
    BoxedInt c;
    c = a;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
    b.dealloc();
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
    b = a;
#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
    BoxedIntPtr d("ptr");
    d = new int();
    *d.ref() = 1;

    BoxedVar<BoxedPrimitive<int>> ptr_b("ptr_b");
    ptr_b = BoxedPrimitive<int>("ptr_b value");
    *ptr_b = 3;

    BoxedVar<BoxedPrimitive<int>> ptr_b_c("ptr_b_c");
    BoxedPrimitive<int> ptr_b_c_v("ptr_b_c value");
    ptr_b_c = ptr_b_c_v;
    *ptr_b_c = 3;

    auto ptr_c = BoxedPrimitive<int>("ptr_c value");
    ptr_c = 3;

    auto cptr_a = BoxedVar<ManagedObject>("cycle_a_value");
    auto cptr_b = BoxedVar<ManagedObject>("cycle_b_value");

    *cptr_a = cptr_b;
    *cptr_b = cptr_a;

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif

    ManagedObject x;
    *cptr_b = x;

#ifdef MANAGED_OBJECT_HEAP_DEBUG
    ManagedObject::get_root()->print();
#endif
}
