#pragma once
#include <stddef.h>
#include "spinlock.hpp"

template <typename T>
class LinkedList {
    size_t _length;
    Spinlock lock;

public:
    template <typename entry_T>
    class LinkedListEntry {
    public:
        entry_T item;
        LinkedListEntry<entry_T>* next;
        LinkedListEntry<entry_T>* prev;

        LinkedListEntry()
            : item(entry_T()), next(nullptr), prev(nullptr) {
        }
    };

    template <typename iterator_T>
    class LinkedListIterator {
    public:
        LinkedListEntry<iterator_T>* entry;

        explicit LinkedListIterator(LinkedListEntry<iterator_T>* entry)
            : entry(entry) {
        }

        iterator_T& operator*() {
            return entry->item;
        }

        void operator++() {
            if (entry)
                entry = entry->next;
        }

        bool operator!=(LinkedListIterator it) {
            return entry != it.entry;
        }
    };

    LinkedListEntry<T>* head;
    LinkedListEntry<T>* tail;

    constexpr LinkedList() noexcept
        : _length(0), lock(Spinlock()), head(nullptr), tail(nullptr) {
    }

    ~LinkedList() {
        auto current = head;

        while (current) {
            auto prev = current;
            current = current->next;

            delete prev;
        }
    }

    T* push_back(T entry) {
        lock.lock();

        auto* new_entry = new LinkedListEntry<T>;
        new_entry->item = entry;
        new_entry->next = nullptr;

        if (_length == 0) {
            head = new_entry;
            tail = new_entry;
            new_entry->prev = nullptr;
            _length++;

            lock.release();

            return &new_entry->item;
        }

        tail->next = new_entry;
        new_entry->prev = tail;
        tail = new_entry;
        _length++;

        lock.release();

        return &new_entry->item;
    }

    LinkedListEntry<T>* get_entry_for_item(T* entry) {
        for (LinkedListEntry<T>* item = head; item != nullptr; item = item->next)
            if (&item->item == entry)
                return item;

        return nullptr;
    }

    LinkedListIterator<T> get_iterator_for_item(T* item) {
        LinkedListEntry<T>* entry = get_entry_for_item(item);

        return LinkedListIterator<T>(entry);
    }

    LinkedListIterator<T> begin() {
        return LinkedListIterator<T>(head);
    }

    LinkedListIterator<T> end() {
        return LinkedListIterator<T>(nullptr);
    }

    size_t length() {
        return _length;
    }
};