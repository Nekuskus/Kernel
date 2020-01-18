#pragma once
#include <hardware/mm/mm.hpp>

namespace Firework::FireworkKernel {
    template <typename T>
    class LinkedList {
        size_t length;

    public:
        template <typename entry_T>
        class LinkedListEntry {
        public:
            entry_T item;
            LinkedListEntry<entry_T>* next;
            LinkedListEntry<entry_T>* prev;

            LinkedListEntry()
                : item(entry_T()), prev(nullptr), next(nullptr) {
            }
        };

        template <typename iterator_T>
        class LinkedListIterator {
        public:
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

            LinkedListEntry<iterator_T>* entry;
        };

        LinkedListEntry<T>* head;
        LinkedListEntry<T>* tail;

        constexpr LinkedList() noexcept
            : length(0), head(nullptr), tail(nullptr) {
        }

        ~LinkedList() {
            if (head != nullptr && tail != nullptr && length != 0)
                for (T& e : *this)
                    free((void*)&e);
        }

        T* push_back(T entry) {
            LinkedListEntry<T>* new_entry = (LinkedListEntry<T>*)calloc(sizeof(LinkedListEntry<T>));
            new_entry->item = entry;
            new_entry->next = nullptr;

            if (length == 0) {
                head = new_entry;
                tail = new_entry;
                new_entry->prev = nullptr;
                length++;

                return &new_entry->item;
            }

            tail->next = new_entry;
            new_entry->prev = tail;
            tail = new_entry;
            length++;

            return &new_entry->item;
        }

        LinkedListEntry<T>* get_entry_for_item(T* entry) {
            for (LinkedListEntry<T>* item = head; item != nullptr; item = item->next)
                if (item->item == entry)
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
    };
}  // namespace Firework::FireworkKernel