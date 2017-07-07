#ifndef __linked_list_h
#define __linked_list_h

template <typename T>
class LinkedList {
public:
    class Iterator {
    public:
        T* next() {
            if (!_ptr) {
                return nullptr;
            }
            T* itemPtr = &_ptr->_item;
            _ptr = _ptr->_next;
            return itemPtr;
        }

        Iterator(typename LinkedList<T>::Node* ptr): _ptr(ptr) {}
    private:
        typename LinkedList<T>::Node* _ptr;
    };

public:
    LinkedList(): _head(nullptr), _tail(nullptr) {}
    ~LinkedList() {
        Node* ptr = _head;
        Node* next = nullptr;
        while (ptr) {
            next = ptr->_next;
            delete ptr;
            ptr = next;
        }
    }

    void append(const T& item) {
        Node* newNode = new Node(item);
    
        if (_tail) {
            _tail->_next = newNode;
        }
        else {
            _head = newNode;
        }
        _tail = newNode;
    }

    Iterator iterator() const {
        return Iterator(_head);
    }

private:
    struct Node {
        T _item;
        Node* _next;

        Node(const T& item): _item(item), _next(nullptr) {}
    };

private:
    Node* _head;
    Node* _tail;
};

#endif // __linked_list_h
