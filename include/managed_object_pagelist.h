#ifndef MANAGED_OBJECT_PAGELIST_H
#define MANAGED_OBJECT_PAGELIST_H
template <typename T>
class ManagedObjectPage {

    struct Set {
        T value;
        bool set = false;
    };

    Set* sets = nullptr;
    size_t max = 0;
    size_t used_ = 0;
    bool empty_ = false;
    bool full_ = false;

    public:

    ManagedObjectPage() : ManagedObjectPage(2) {}

    ManagedObjectPage(size_t size) : max(size) {
        if (max != 0) {
            resize(max);
        }
    }

    size_t used() {
        return used_;
    }

    bool empty() {
        return empty_;
    }

    bool full() {
        return full_;
    }

    void resize(size_t s) {
        if (s == 0) {
            max = s;
            if (sets != nullptr) {
                delete[] sets;
                sets = nullptr;
            }
            used_ = 0;
            empty_ = true;
            full_ = false;
        } else if (sets == nullptr) {
            max = s;
            sets = new Set[max];
            used_ = 0;
            empty_ = true;
            full_ = false;
        } else {
            Set * n = new Set[s];
            if (sets == nullptr) {
                if (s < max) {
                    max = s;
                    if (used_ >= max) {
                        used_ = max;
                        full_ = true;
                    }
                } else {
                    // s >= max
                    max = s;
                    full_ = false;
                }
            } else {
                if (std::is_trivially_copyable<T>::value) {
                    if (s < max) {
                        memcpy(n, sets, s);
                        max = s;
                        if (used_ >= max) {
                            used_ = max;
                            full_ = true;
                        }
                    } else {
                        // s >= max
                        memcpy(n, sets, max);
                        max = s;
                        full_ = false;
                    }
                } else {
                    if (s < max) {
                        for (size_t i = 0; i < s; i++) {
                            n[i] = std::move(sets[i]);
                        }
                        max = s;
                        if (used_ >= max) {
                            used_ = max;
                            full_ = true;
                        }
                    } else {
                        // s >= max
                        for (size_t i = 0; i < max; i++) {
                            n[i] = std::move(sets[i]);
                        }
                        max = s;
                        full_ = false;
                    }
                }
                delete[] sets;
            }
            empty_ = used_ == 0;
            sets = n;
        }
    }

    ~ManagedObjectPage() {
        resize(0);
    }

    size_t set(T & value) {
        for (size_t i = 0; i < max; i++) {
            auto & set = sets[i];
            if (!set.set) {
                set.set = true;
                set.value = value;
                used_++;
                empty_ = false;
                if (used_ == max) {
                    full_ = true;
                }
                return i;
            }
        }
        return -1;
    }

    size_t set(const T & value) {
        for (size_t i = 0; i < max; i++) {
            auto & set = sets[i];
            if (!set.set) {
                set.set = true;
                set.value = value;
                empty_ = false;
                used_++;
                if (used_ == max) {
                    full_ = true;
                }
                return i;
            }
        }
        return -1;
    }

    void unset(size_t index) {
        auto & set = sets[index];
        #if MANAGED_OBJECT_HEAP_DEBUG
        assert(set.set);
        #endif
        if (set.set) {
            set.set = false;
            set.value.~T();
            used_--;
            empty_ = used_ == 0;
            full_ = false;
        }
    }
};

template <typename T>
class ManagedObjectPageList {

    std::vector<ManagedObjectPage<T>*> page_list;

    size_t page_size;

    public:

    ManagedObjectPageList() {
        page_size = 2;
    }

    ManagedObjectPageList(size_t page_size) : page_size(page_size) {}

    inline size_t chunks() {
        return page_list.size();
    }

    inline size_t get_chunk(size_t index) {
        return index / page_size;
    }

    inline size_t get_chunk_subindex(size_t index) {
        return index % page_size;
    }

    size_t set(T & value) {
        size_t offset = 0;
        for (ManagedObjectPage<T> * page : page_list) {
            if (page->full()) {
                offset += page_size;
            } else {
                return page->set(value) + offset;
            }
        }
        // all pages are full
        ManagedObjectPage<T> * page = new ManagedObjectPage<T>(page_size);
        page_list.emplace_back(page);
        return page->set(value) + offset;
    }
    
    size_t set(const T & value) {
        size_t offset = 0;
        for (ManagedObjectPage<T> * page : page_list) {
            if (page->full()) {
                offset += page_size;
            } else {
                return page->set(value) + offset;
            }
        }
        // all pages are full
        ManagedObjectPage<T> * page = new ManagedObjectPage<T>(page_size);
        page_list.emplace_back(page);
        return page->set(value) + offset;
    }

    void unset(size_t index) {
        size_t page_list_index = get_chunk(index);
        ManagedObjectPage<T> * page = page_list[page_list_index];
        page->unset(get_chunk_subindex(index));
        if (page->empty()) {
            page_list.erase(page_list.begin()+page_list_index);
            delete page;
        }
    }

    void print_chunk_and_index(size_t i) {
        printf("index: %zu, chunk: %zu, sub array index: %zu\n", i, get_chunk(i), get_chunk_subindex(i));
    }

    ~ManagedObjectPageList() {
        for (ManagedObjectPage<T> * page : page_list) {
            delete page;
        }
    }
};

#endif