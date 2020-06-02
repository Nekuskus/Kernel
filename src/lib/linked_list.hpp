#pragma once
#include <stddef.h>

template <typename T>
struct Node {
    T data;
    Node<T> *next;
};

template <typename T>
class NodeIterator {
public:
    Node<T> *link;

    explicit NodeIterator(Node<T> *link)
        : link(link) {
    }

    T &operator*() {
        return link->data;
    }

    void operator++() {
        if (link)
            link = link->next;
    }

    bool operator!=(NodeIterator it) {
        return link != it.link;
    }
};

template <typename T>
class LinkedList {
    Node<T> *list;
    size_t _length;

    void update() {
        _length = 0;

        if (!list)
            return;

        Node<T> *last = list;

        while (last) {
            _length++;
            last = last->next;
        }
    }

public:
    LinkedList()
        : list(nullptr), _length(0) {
    }

    ~LinkedList() {
        clear();
    }

    Node<T> *find(int index) {
        auto temp = list;

        while (temp && index--)
            temp = temp->next;

        return temp;
    }

    T *operator[](int index) {
        auto node = find(index);

        return node ? &node->data : nullptr;
    }

    void push(T value) {
        auto node = new Node<T>;
        node->data = value;
        node->next = list;
        list = node;

        update();
    }

    void push_back(T value) {
        Node<T> *node = new Node<T>, *last = list;
        node->data = value;
        node->next = nullptr;

        if (list) {
            while (last->next)
                last = last->next;

            last->next = node;
        } else
            list = node;

        update();
    }

    bool insert_before(int index, T value) {
        Node<T> *node = list, *last, *result = new Node<T>;

        while (node && index--) {
            last = node;
            node = node->next;
        }

        if (node) {
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
        Node<T> *node = list, *last, *result = new Node<T>;

        while (node && index--)
            node = node->next;

        if (node) {
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

        auto node = list;
        list = list->next;

        delete node;

        update();

        return true;
    }

    bool pop(int index) {
        if (!list)
            return false;

        Node<T> *node = list, *last;

        int idx = index;

        while (node && index--) {
            last = node;
            node = node->next;
        }

        if (node) {
            if (node == list)
                return false;

            if (node == list->next)
                list = node;
            else
                find(idx - 2)->next = node;

            delete last;

            update();

            return true;
        }

        return false;
    }

    bool pop_back() {
        if (!list)
            return false;

        Node<T> *node = list, *last = nullptr;

        if (!node || !node->next)
            list = nullptr;
        else {
            while (node->next) {
                last = node;
                node = node->next;
            }

            if (!last)
                return false;

            last->next = nullptr;
        }

        delete node;

        return true;
    }

    void clear() {
        if (!list)
            return;

        while (list)
            pop_back();
    }

    NodeIterator<T> begin() {
        return NodeIterator<T>(list);
    }

    NodeIterator<T> end() {
        return NodeIterator<T>(nullptr);
    }

    size_t length() {
        return _length;
    }
};