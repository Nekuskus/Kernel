#pragma once
#include <stddef.h>

#include "spinlock.hpp"

template <typename T>
struct NodeLink {
    T data;
    NodeLink<T>* next;
    int index;
};

template <typename T>
class NodeLinkIterator {
public:
    NodeLink<T>* link;

    explicit NodeLinkIterator(NodeLink<T>* link)
        : link(link) {
    }

    T& operator*() {
        return link->data;
    }

    void operator++() {
        if (link)
            link = link->next;
    }

    bool operator!=(NodeLinkIterator it) {
        return link != it.link;
    }
};

template <typename T>
class LinkedList {
    NodeLink<T>* list;
    size_t _length;

    void update() {
        _length = 0;

        if (list == nullptr)
            return;

        NodeLink<T>* last = list;
        int idx = 0;

        while (last != nullptr) {
            _length++;
            last->index = idx++;
            last = last->next;
        }
    }

public:
    LinkedList()
        : list(nullptr) {
    }

    ~LinkedList() {
        clear();
    }

    NodeLink<T>* find(int index) {
        NodeLink<T>* temp = list;

        while (temp && temp->index != index)
            temp = temp->next;

        return temp;
    }

    T* operator[](int index) {
        NodeLink<T>* node = find(index);

        return node ? &node->data : nullptr;
    }

    void push(T value) {
        NodeLink<T>* node = new NodeLink<T>;
        node->data = value;
        node->next = list;
        list = node;

        update();
    }

    void push_back(T value) {
        NodeLink<T>*node = new NodeLink<T>, *last = list;
        node->data = value;
        node->next = nullptr;

        if (list != nullptr) {
            while (last->next != nullptr)
                last = last->next;

            last->next = node;
        } else
            list = node;

        update();
    }

    bool insert_before(int index, T value) {
        NodeLink<T>*node = list, *last, *result = new NodeLink<T>;
        bool found = false;

        while (node != nullptr) {
            if (node->index == index) {
                found = true;

                break;
            }

            last = node;
            node = node->next;
        }

        if (found) {
            if (node == list) {
                push(value);

                delete result;
            } else {
                result->data = value;
                last->next = result;
                result->next = node;

                update();
            }

            return true;
        }

        delete result;

        return false;
    }

    bool insert_after(int index, T value) {
        NodeLink<T>*node = list, *last, *result = new NodeLink<T>;
        bool found = false;

        while (node != nullptr) {
            if (node->index == index) {
                found = true;
                break;
            }

            node = node->next;
        }

        if (found) {
            if (node->next == nullptr) {
                push_back(value);

                delete result;
            } else {
                result->data = value;
                last = node->next;
                node->next = result;
                result->next = last;

                update();

                return true;
            }
        }

        delete result;

        return false;
    }

    bool pop() {
        if (!list)
            return false;

        NodeLink<T>* node = list;
        list = list->next;

        delete node;

        update();

        return true;
    }

    bool pop(int index) {
        if (!list)
            return false;

        NodeLink<T>*node = list, *last;
        bool found = false;

        while (node != nullptr) {
            if (node->index == index) {
                found = true;

                break;
            }

            last = node;
            node = node->next;
        }

        if (found) {
            if (node == list)
                return false;

            if (node == list->next) {
                list = node;

                delete last;
            } else {
                find(last->index - 1)->next = node;

                delete last;
            }

            update();

            return true;
        }

        return false;
    }

    bool pop_back() {
        if (!list)
            return false;

        NodeLink<T>*node = list, *last = nullptr;

        if (node == nullptr || !node->next) {
            list = nullptr;

            delete node;
        } else {
            while (node->next != nullptr) {
                last = node;
                node = node->next;
            }

            if (last == nullptr)
                return false;

            last->next = nullptr;

            delete node;
        }
        return true;
    }

    void clear() {
        if (list == nullptr)
            return;

        while (list != nullptr)
            pop_back();
    }

    NodeLinkIterator<T> begin() {
        return NodeLinkIterator<T>(list);
    }

    NodeLinkIterator<T> end() {
        return NodeLinkIterator<T>(nullptr);
    }

    size_t length() {
        return _length;
    }
};