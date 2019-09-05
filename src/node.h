#ifndef NODE_H
#define NODE_H
#include "interval.h"

using namespace std;


template <class T>
class Node {
public:
    typedef Node<T> Tnode;
    typedef Interval<T> Tinterval;

    Node() {
        left = NULL;
        right = NULL;
    }

    Node(Tinterval interval) {
        this->interval = interval;
        top = interval.right;
        left = NULL;
        right = NULL;
    }

    bool is_interval() {
        return (interval.left != interval.right);
    }

    void update_interval(T elem) {
        if (elem > interval.left) {
            interval.right = elem;
            top = elem;
        } else {
            interval.left = elem;
            top = interval.left;
        }
    }

    void split() {
        Tinterval left_interval, right_interval;
        interval.split(left_interval, right_interval);
        left = new Node(left_interval);
        right = new Node(right_interval);
        left->parent = this;
        right->parent = this;
    }

    bool is_leaf() {
        return ((left == NULL) && (right == NULL));
    }

    void update_weights() {
        if (this->parent != NULL) {
            if (this->interval.left < parent->interval.left) {
                parent->interval.left = this->interval.left;
            }
            if (this->interval.right > parent->interval.right) {
                parent->interval.right = this->interval.right;
            }
            parent->update_weights();
        }
    }

    void expand(Tinterval & other_interval) {
        this->interval.expand(other_interval);
        this->top = this->interval.right;
    }

    void print() {
        // printf("[%d, %d](%d)", interval.left, interval.right, top);
        std::cout << interval;
        // std::cout << interval;
    }

    Tnode * left;
    Tnode * right;
    Tnode * parent;
    Tinterval interval;
    T top;
};


#endif // NODE_H
