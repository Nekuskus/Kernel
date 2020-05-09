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
    Spinlock lock;

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
        : list(nullptr), _length(0), lock() {
    }

    ~LinkedList() {
        clear();
    }

    NodeLink<T>* find(int index) {
        lock.lock();

        auto temp = list;

        while (temp && temp->index != index)
            temp = temp->next;

        lock.release();

        return temp;
    }

    T* operator[](int index) {
        auto node = find(index);

        return node ? &node->data : nullptr;
    }

    void push(T value) {
        lock.lock();

        auto node = new NodeLink<T>;
        node->data = value;
        node->next = list;
        list = node;

        update();
        lock.release();
    }

    void push_back(T value) {
        lock.lock();

        NodeLink<T> *node = new NodeLink<T>, *last = list;
        node->data = value;
        node->next = nullptr;

        if (list != nullptr) {
            while (last->next != nullptr)
                last = last->next;

            last->next = node;
        } else
            list = node;

        update();
        lock.release();
    }

    bool insert_before(int index, T value) {
        lock.lock();

        NodeLink<T> *node = list, *last, *result = new NodeLink<T>;
        bool found = false;

        while (node != nullptr) {
            if (node->index == index) {
                lock.release();

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

            lock.release();

            return true;
        }

        delete result;

        lock.release();

        return false;
    }

    bool insert_after(int index, T value) {
        lock.lock();

        NodeLink<T> *node = list, *last, *result = new NodeLink<T>;
        bool found = false;

        while (node != nullptr) {
            if (node->index == index) {
                lock.release();

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
                lock.release();

                return true;
            }
        }

        delete result;

        lock.release();

        return false;
    }

    bool pop() {
        if (!list)
            return false;

        lock.lock();

        auto node = list;
        list = list->next;

        delete node;

        update();
        lock.release();

        return true;
    }

    bool pop(int index) {
        if (!list)
            return false;

        lock.lock();

        NodeLink<T> *node = list, *last;
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
                lock.release();

                return false;
            }

            if (node == list->next)
                list = node;
            else
                find(last->index - 1)->next = node;

            delete last;

            update();
            lock.release();

            return true;
        }

        lock.release();

        return false;
    }

    bool pop_back() {
        if (!list)
            return false;

        lock.lock();

        NodeLink<T> *node = list, *last = nullptr;

        if (node == nullptr || !node->next)
            list = nullptr;
        else {
            while (node->next != nullptr) {
                last = node;
                node = node->next;
            }

            if (last == nullptr) {
                lock.release();

                return false;
            }

            last->next = nullptr;
        }

        delete node;

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