// std::sort as libstdc++ implements it (introsort, median of three, insertion sort below 16
// elements). std::sort is not stable and the result for equal keys depends on the algorithm, so
// the port carries this copy to match the original on other standard libraries (libc++ on
// Android).
#pragma once

#include <cstddef>
#include <utility>

namespace stl {

namespace detail {

template <typename T, typename Less>
void MoveMedianToFirst(T *result, T *a, T *b, T *c, Less less) {
    if (less(*a, *b)) {
        if (less(*b, *c)) {
            std::swap(*result, *b);
        } else if (less(*a, *c)) {
            std::swap(*result, *c);
        } else {
            std::swap(*result, *a);
        }
    } else if (less(*a, *c)) {
        std::swap(*result, *a);
    } else if (less(*b, *c)) {
        std::swap(*result, *c);
    } else {
        std::swap(*result, *b);
    }
}

template <typename T, typename Less>
T *UnguardedPartition(T *first, T *last, T *pivot, Less less) {
    while (true) {
        while (less(*first, *pivot)) {
            ++first;
        }
        --last;
        while (less(*pivot, *last)) {
            --last;
        }
        if (!(first < last)) {
            return first;
        }
        std::swap(*first, *last);
        ++first;
    }
}

template <typename T, typename Less>
void AdjustHeap(T *first, std::ptrdiff_t hole, std::ptrdiff_t len, T value, Less less) {
    const std::ptrdiff_t top = hole;
    std::ptrdiff_t child = hole;
    while (child < (len - 1) / 2) {
        child = 2 * (child + 1);
        if (less(first[child], first[child - 1])) {
            child--;
        }
        first[hole] = std::move(first[child]);
        hole = child;
    }
    if ((len & 1) == 0 && child == (len - 2) / 2) {
        child = 2 * (child + 1);
        first[hole] = std::move(first[child - 1]);
        hole = child - 1;
    }
    std::ptrdiff_t parent = (hole - 1) / 2;
    while (hole > top && less(first[parent], value)) {
        first[hole] = std::move(first[parent]);
        hole = parent;
        parent = (hole - 1) / 2;
    }
    first[hole] = std::move(value);
}

// partial_sort(first, last, last): make_heap then sort_heap.
template <typename T, typename Less>
void HeapSort(T *first, T *last, Less less) {
    std::ptrdiff_t len = last - first;
    if (len >= 2) {
        for (std::ptrdiff_t parent = (len - 2) / 2;; parent--) {
            AdjustHeap(first, parent, len, std::move(first[parent]), less);
            if (parent == 0) {
                break;
            }
        }
    }
    while (last - first > 1) {
        --last;
        T value = std::move(*last);
        *last = std::move(*first);
        AdjustHeap(first, 0, last - first, std::move(value), less);
    }
}

template <typename T, typename Less>
void IntrosortLoop(T *first, T *last, int depth_limit, Less less) {
    while (last - first > 16) {
        if (depth_limit == 0) {
            HeapSort(first, last, less);
            return;
        }
        --depth_limit;
        T *mid = first + (last - first) / 2;
        MoveMedianToFirst(first, first + 1, mid, last - 1, less);
        T *cut = UnguardedPartition(first + 1, last, first, less);
        IntrosortLoop(cut, last, depth_limit, less);
        last = cut;
    }
}

template <typename T, typename Less>
void UnguardedLinearInsert(T *last, Less less) {
    T value = std::move(*last);
    T *next = last - 1;
    while (less(value, *next)) {
        *last = std::move(*next);
        last = next;
        --next;
    }
    *last = std::move(value);
}

template <typename T, typename Less>
void InsertionSort(T *first, T *last, Less less) {
    if (first == last) {
        return;
    }
    for (T *i = first + 1; i != last; ++i) {
        if (less(*i, *first)) {
            T value = std::move(*i);
            for (T *j = i; j != first; --j) {
                *j = std::move(*(j - 1));
            }
            *first = std::move(value);
        } else {
            UnguardedLinearInsert(i, less);
        }
    }
}

inline int Log2(std::size_t n) {
    int log = 0;
    while (n >>= 1) {
        log++;
    }
    return log;
}

}  // namespace detail

template <typename T, typename Less>
void Sort(T *first, T *last, Less less) {
    if (first == last) {
        return;
    }
    detail::IntrosortLoop(first, last, detail::Log2(static_cast<std::size_t>(last - first)) * 2, less);
    if (last - first > 16) {
        detail::InsertionSort(first, first + 16, less);
        for (T *i = first + 16; i != last; ++i) {
            detail::UnguardedLinearInsert(i, less);
        }
    } else {
        detail::InsertionSort(first, last, less);
    }
}

}  // namespace stl
