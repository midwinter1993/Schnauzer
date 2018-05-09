#ifndef __COMMON_MT_VECTOR_H__
#define __COMMON_MT_VECTOR_H__

#include <cassert>
#include <cstddef>
#include <thread>
#include <iostream>
#include <map>

template<typename T>
struct MtBlock {
    typedef T value_type;

    static const size_t ITEM_SZ = 2;
    MtBlock<T>(): nr_item_(0), next_(nullptr) { }

    bool full() { return nr_item_ == ITEM_SZ; }

    void push_back(const value_type& val) {
        assert(!full());
        item_arr_[nr_item_] = val;
        nr_item_ += 1;
    }

    value_type item_arr_[ITEM_SZ];
    size_t nr_item_;
    MtBlock<T> *next_;
};

template<typename T>
class MtVector {
public:
    typedef T value_type;

    MtVector<T>()
        : head_blk_(new MtBlock<value_type>())
        , curr_blk_(head_blk_) { }

    void push_back(const value_type &val) {
        curr_blk_->push_back(val);
        if (curr_blk_->full()) {
            curr_blk_->next_ = new MtBlock<value_type>();
            curr_blk_ = curr_blk_->next_;
        }
    }

    class Fiterator;

    Fiterator begin() { return Fiterator(head_blk_, 0, this); }
    Fiterator end() { return Fiterator(curr_blk_, curr_blk_->nr_item_, this);  }

private:
    MtBlock<value_type> *head_blk_;
    MtBlock<value_type> *curr_blk_;
};

template<typename T>
class MtVector<T>::Fiterator {
public:
    typedef T value_type;
    Fiterator(): curr_blk_(nullptr), curr_item_pos_(0), vec_(nullptr) { }
    Fiterator(MtBlock<T> *blk_, size_t nr, MtVector<T> *vec)
        : curr_blk_(blk_), curr_item_pos_(nr), vec_(vec) { }

    T& operator*() {
        return curr_blk_->item_arr_[curr_item_pos_];
    }

    T* operator->() {
        return &operator*();
    }

    void operator++() {
        curr_item_pos_ += 1;
        if (curr_item_pos_ == MtBlock<T>::ITEM_SZ) {
            assert(curr_blk_->next_);
            curr_blk_ = curr_blk_->next_;
            curr_item_pos_ = 0;
        }
    }

    bool operator==(const MtVector<T>::Fiterator &it) const {
        return curr_blk_ == it.curr_blk_ && curr_item_pos_ == it.curr_item_pos_;
    }

    bool operator!=(const MtVector<T>::Fiterator &it) const {
        return !operator==(it);
    }

    bool isEnd() const {
        return operator==(vec_->end());
    }
private:
    MtBlock<value_type> *curr_blk_;
    size_t curr_item_pos_;
    MtVector<value_type> *vec_;
};

template<typename T>
struct MtVectorMngr {
    typedef std::map<std::thread::id, MtVector<T>*> map_type;

    static MtVector<T>* get() {
        static __thread MtVector<T> *vec = nullptr;
        if (!vec) {
            vec = new MtVector<T>();
            // LOCK here?
            thread_ptr_map[std::this_thread::get_id()] = vec;
        }
        return vec;
    }

    static map_type& getAll() {
        return thread_ptr_map;
    }

    static map_type thread_ptr_map;
};
template<typename T>
typename MtVectorMngr<T>::map_type MtVectorMngr<T>::thread_ptr_map;

#endif /* ifndef __COMMON_MT_VECTOR_H__ */
