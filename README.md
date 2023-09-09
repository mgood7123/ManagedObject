# ManagedObject

a minimal Managed Object implementation for C++

based on the `'object reference' system` that is used by Java, C# and other high-level language

# building

```
clear ; make CC=clang CXX=clang++ test_debug_asan
clear ; make CC=clang CXX=clang++ test_debug_valgrind
```

# usage

`ManagedObject` allows one to easily implement managed garbage collected resources in C++

garbage collection is implemented via `reference reachability` (which automatically handles `cyclic references`), and merged with a `mini-heap` to store the `structural data` required by the garbage collector

`it should be impossible to leak a reference that cannot be cleaned up by the garbage collector under normal conditions except for incorrect api usage`

as an example, below is an implementation of "boxing" to allow easy creation of managed objects in C++

# Boxed Primitive

`BoxedPrimitive<T>` allows for a primitive (`any object equal in size to a pointer, and is trivially copyable`) to be directly stored `without needing a seperate allocation`

this makes sense when you consider a `uint8_t` (`8 bits`) is smaller than a `pointer` (`32/64 bits`) and is `trivially copyable`

this means that `uint8_t` can be `directly stored and retrieved` since it `does not exceed the size of a pointer` and is `trivially copyable`

`ManagedObject` itself will let you know `at compile time` if you attempt to push or retrieve a `T` that exceeds the size of a `pointer`, and if `T` is not `trivially copyable`

`trivially copyable` means that `memcpy` or similar can be used to duplicate an object as often done in `C` language

see `std::is_trivially_copyable` for more information

`NOTE:` although it is possible for `BoxedPrimitive` to store a `T*`
 - it cannot correctly track `T` if `T` is (or is derived from) a `ManagedObject`
 - it cannot correctly instantiate cyclic data

use `BoxedVar<T>` instead as it handles the above

the following code can form a `cyclic leak` which is impossible to clean up since they form `self referencing roots`

```cpp
    BoxedPrimitive<ManagedObject*> cptr_a("cycle_a");
    BoxedPrimitive<ManagedObject*> cptr_b("cycle_b");

    cptr_a = new BoxedPrimitive<ManagedObject*>("cycle_a_value");
    cptr_b = new BoxedPrimitive<ManagedObject*>("cycle_b_value");
    
    **cptr_a = cptr_b;
    **cptr_b = cptr_a;
```

`for this reason alone`, storing pointers to `ManagedObject` is not allowed inside a `BoxedPrimitive`

we also supply the `MANAGED_OBJECT_STATIC_ASSERT_IS_PRIMITIVE_LIKE` macro to detect this

(`ManagedObject` itself (as in `ManagedObject foo;`) is not `trivially copyable` and will error if passed to `ManagedObject::push_value`)

this means we `do not` need to detect `trivially copyable` inside the `MANAGED_OBJECT_STATIC_ASSERT_IS_PRIMITIVE_LIKE` macro itself



this can be fixed with `BoxedVar<T>`
```cpp
    BoxedVar<ManagedObject> cptr_a("cycle_a");
    BoxedVar<ManagedObject> cptr_b("cycle_b");

    cptr_a = BoxedPrimitive<ManagedObject*>("cycle_a_value");
    cptr_b = BoxedPrimitive<ManagedObject*>("cycle_b_value");

    *cptr_a = cptr_b;
    *cptr_b = cptr_a;
```
although due to the above, `BoxedPrimitive<ManagedObject*>` itself is not allowed so we must use `BoxedVar` instead
```cpp

    BoxedVar<ManagedObject> cptr_a("cycle_a");
    BoxedVar<ManagedObject> cptr_b("cycle_b");

    cptr_a = BoxedVar<ManagedObject>("cycle_a_value");
    cptr_b = BoxedVar<ManagedObject>("cycle_b_value");

    *cptr_a = cptr_b;
    *cptr_b = cptr_a;
```

# Boxed Var

`BoxedVar<T>` is for when `T` is too large to fit inside a `BoxedPrimitive<T>`

do note that if `T` is a `ManagedObject` or derived then `BoxedVar` will only invoke its `operator=` if the type being assigned matches exactly

```cpp
BoxedVar<ManagedObject> b;

// invokes `ManagedObject::operator=` since `BoxedVar<ManagedObject> != ManagedObject` 
b = BoxedVar<ManagedObject>();

// invokes `BoxedVar<ManagedObject>::operator=` since `ManagedObject == ManagedObject` 
b = ManagedObject();

// since 'BoxedVar : ManagedObject' we CANNOT force `ManagedObject::operator=` to be invoked for '= ManagedObject();'
// since it makes no sense to do so
```

# Boxed Example

```cpp
typedef BoxedPrimitive<int> BoxedInt;
typedef BoxedVar<BoxedInt> IntegerHolder;
```

the types above can be used as follows

```cpp
    // debug tag, optional
    //
    BoxedInt a("a");

    // we can assign it integer types directly
    //
    a = 1;

    BoxedInt b("b");
    b = 2;

    // we can assign a BoxedInt object to another BoxedInt object
    //
    // at this point, there are no more references to 'a' since 'a' is assigned the reference 'b'
    //
    // think of this like:
    //   int * a = new int();
    //   int * b = new int();
    //   a = b;
    //
    //   the pointer 'a' is reassigned the pointer 'b', both 'a' and 'b' now point to the same allocation
    //
    //   the original allocation 'a' is now lost and since no other objects point to it, it is automatically collected
    //
    //   if we did 'BoxedInt c = a; a = b;'  then 'a' would still exist as 'c' refers to it, thus 'a' will not be GC'd when 'a = b'
    //
    a = b;

    // at end of scope, RAII kicks in and informs the GC that the reference no longer exists
    //
    // since 'a' and 'b' get RAII'd there are 0 references to 'b' original allocation and it gets collected
    //

    // equal to 'a = null' in a high level language
    //
    // 'dealloc' is provided by 'ManagedObject' since 'operator = (nullptr_t)' ('... = nullptr;') might be reused by derived classes
    //
    a.dealloc();
```

```cpp

// we can use the above to form managed structures, lets create a cyclic structure
//
// note that we do not use 'reroot' here
//
// this is because we directly track the ManagedObject itself
//
// tracking byRef is prefered over 'reroot' unless it is an allocated object
//
// you may need to experiment to see what works and what doesnt work when it comes to object tracking
//
// BoxedVar and BoxedPrimitive provide a stable foundation to easily manage objects
//

struct T_A;
struct T_B;

struct T_A : ManagedObject {
    BoxedVar<T_B> b = BoxedVar<T_B>("a.b");

    T_A() {
        // inform ManagedObject that the Managed Object 'b' is owned by us
        //
        byRef(b);
    }

    T_A(const char * id) : ManagedObject(id) {
        // inform ManagedObject that the Managed Object 'b' is owned by us
        //
        byRef(b);
    }
};

struct T_B : ManagedObject {
    BoxedVar<T_A> a = BoxedVar<T_A>("b.a");

    T_B() {
        // inform ManagedObject that the Managed Object 'a' is owned by us
        //
        byRef(a);
    }

    T_B(const char * id) : ManagedObject(id) {
        // inform ManagedObject that the Managed Object 'a' is owned by us
        //
        byRef(b);
    }
};

TEST(references, 10) {
    T_A a("a");
    T_B b("b");
    b.a = a;
    a.b = b;
}

```

below is their implementation

```cpp
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
```