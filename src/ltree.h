#ifndef LTREE_H
#define LTREE_H

#include <algorithm>
#include "query.h"

template <class T>
class LeafNode {

public:
    typedef Interval<T> Tinterval;
    typedef LeafNode<T> Tnode;
    typedef Query<T> Tquery;
    Tnode * left;
    Tnode * right;
    Tnode * parent;
    T max;

    Tinterval interval;
    /*
        Intervals has to works as queries and range of leaf node
    */
    // The pair is composed by a pointer to the query and the current range
    // that belongs to the query.
    vector <Tquery *> queries;

    LeafNode(Tinterval interval) {
        this->interval = interval;
        T size = interval.max - interval.min;
        left = NULL;
        right = NULL;
        parent = NULL;
    }

    void update_weights() {
        if (this->parent != NULL && this->max > parent->max) {
            parent->max = this->max;
            parent->update_weights();
        } else {
            max = interval.max;
        }
    }

    // void printHashMap() {
    //     for (int i = 0; i < hashmap.size(); i += 1) {
    //         cout << *(hashmap[i].first) << "*" << hashmap[i].second << "*" << ", ";
    //     }
    // }

    // unsigned long long int checksum() {
    //     unsigned long long int value = 0;
    //     for (int i = 0; i < hashmap.size(); i += 1) {
    //         value += hashmap[i].second.checksum();
    //     }

    //     return value;
    // }

    void print() {
        printf("[%d, %d](%d)", interval.min, interval.max, max);
    }
};


template <class Tr>
class LeafTree {

public:
    typedef typename Tr::T T;
    typedef typename Tr::Tinterval Tinterval;
    typedef typename Tr::Tquery Tquery;
    typedef LeafNode<T> Tnode;
    Tnode * root;

    LeafTree() {
        root = NULL;
    }

    void assign(Tquery * query) {
        queue<Tinterval> pending;
        pending.push(query->interval);
        while (!pending.empty()) {
            Tnode ** visitor = &(this->root);
            Tinterval interval = pending.front();
            pending.pop();
            while ((*visitor) != NULL) {
                if (interval.min >= (*visitor)->interval.max) {
                    visitor = &((*visitor)->right);
                } else if (interval.max <= (*visitor)->interval.min) {
                    visitor = &((*visitor)->left);
                } else {
                    T left, right;
                    if (interval.min < (*visitor)->interval.min) {
                        left = (*visitor)->interval.min;
                        Tinterval tempInterval = Tinterval(interval.min, (*visitor)->interval.min);
                        pending.push(tempInterval);
                    } else {
                        left = interval.min;
                    }
                    if (interval.max > (*visitor)->interval.max) {
                        right = (*visitor)->interval.max;
                        Tinterval tempInterval = Tinterval((*visitor)->interval.max, interval.max);
                        pending.push(tempInterval);
                    } else {
                        right = interval.max;
                    }
                    (*visitor)->queries.push_back(query);
                    break;
                }
            }
        }
    }

    // void assign(Tinterval * query) {
    //     queue<Tinterval> pending;
    //     pending.push(*query);
    //     while (!pending.empty()) {
    //         Tnode ** visitor = &(this->root);
    //         Tinterval interval = pending.front();
    //         pending.pop();
    //         while ((*visitor) != NULL) {
    //             if (interval.min >= (*visitor)->interval.max) {
    //                 visitor = &((*visitor)->right);
    //             } else if (interval.max <= (*visitor)->interval.min) {
    //                 visitor = &((*visitor)->left);
    //             } else {
    //                 T left, right;
    //                 if (interval.min < (*visitor)->interval.min) {
    //                     left = (*visitor)->interval.min;
    //                     Tinterval tempInterval = Tinterval(interval.min, (*visitor)->interval.min);
    //                     pending.push(tempInterval);
    //                 } else {
    //                     left = interval.min;
    //                 }
    //                 if (interval.max > (*visitor)->interval.max) {
    //                     right = (*visitor)->interval.max;
    //                     Tinterval tempInterval = Tinterval((*visitor)->interval.max, interval.max);
    //                     pending.push(tempInterval);
    //                 } else {
    //                     right = interval.max;
    //                 }
    //                 (*visitor)->hashmap.push_back(make_pair(query, Tinterval(left, right)));
    //                 break;
    //             }
    //         }
    //     }
    // }

    Tnode ** search(Tinterval interval, Tnode * & parent = NULL) {
        Tnode ** visitor = &(this->root);
        while ((*visitor) != NULL) {
            parent = *visitor;

            if (interval == (*visitor)->interval) {
                break;
            } else if (interval < (*visitor)->interval) {
                visitor = &((*visitor)->left);
            } else {
                visitor = &((*visitor)->right);
            }
        }

        return visitor;
    }

    bool insert(Tinterval & key) {
        Tnode * parent = NULL;
        Tnode ** searchNode = this->search(key, parent);
        if ((*searchNode) != NULL) {
            return false;
        }
        (*searchNode) = new Tnode(key);
        (*searchNode)->parent = parent;
        (*searchNode)->update_weights();

        return true;
    }

    void find(T key, vector <Tinterval *> & results, Tnode * visitor) {
        if (visitor == NULL) {
            return;
        }
        cout << "interval: " << visitor->interval << endl;
        if (visitor->right) {
            cout << "-->" << visitor->right->max << endl;
        }
        if (visitor->interval.intersects(key)) {
            results.push_back(&(visitor->interval));
        }
        if (visitor->left && key < visitor->left->max) {
            find(key, results, visitor->left);
        }
        if (visitor->right && key < visitor->right->max) {
            find(key, results, visitor->left);
        }
    }

    void find(T key, vector <Tinterval *> & results) {
        find(key, results, root);
    }

    void print() {
        print(this->root);
        printf("\n");
    }

    void print(Tnode * visitor) {
        if (visitor != NULL) {
            visitor->print();
            printf(" -> ");
            print(visitor->left);
            print(visitor->right);
        } else {
            printf("nil | ");
        }
    }

    void graphviz(Tnode *node, string & tree, string iter="") {
        if (node != NULL) {
            if (node->parent == NULL && node->left == NULL && node->right == NULL) {
                tree += node->interval.to_graphviz(iter);
                for (int i = 0; i < node->hashmap.size(); i += 1) {
                    tree += node->interval.to_graphviz(iter); + " -> " + node->hashmap[i].first->to_graphviz(iter) + " ";
                }
            }
            if (node->left != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->left->interval.to_graphviz(iter) + " ";
            }
            if (node->right != NULL) {
                tree += node->interval.to_graphviz(iter) + " -> " + node->right->interval.to_graphviz(iter) + " ";
            }

            graphviz(node->left, tree, iter);
            graphviz(node->right, tree, iter);
        }
    }

    string graphviz(string iter=""){
        // string str = "digraph G {\n";
        string tree = "";
        graphviz(root, tree, iter);
        // str += tree + "}";
        return tree;
    }

    void get_nodes(Tnode * visitor, vector<Tnode *> & nodes) {
        if (visitor != NULL) {
            nodes.push_back(visitor);
            get_nodes(visitor->left, nodes);
            get_nodes(visitor->right, nodes);
        }
    }

    void getNodeCheckSum(Tnode * visitor, unsigned long long int & val) {
        if (visitor != NULL) {
            unsigned long long int chk = visitor->checksum();
            val += chk;
            getNodeCheckSum(visitor->left, val);
            getNodeCheckSum(visitor->right, val);
        }
    }

    unsigned long long int checksum() {
        unsigned long long int value = 0;
        getNodeCheckSum(root, value);

        return value;
    }

    void getLeafHash(Tnode * visitor) {
        if (visitor != NULL) {
            cout << "==>" << visitor->interval << " - ";
            visitor->printHashMap();
            cout << endl;
            getLeafHash(visitor->left);
            getLeafHash(visitor->right);
        }
    }

    void getLeafHash() {
        getLeafHash(root);
    }

    vector<Tnode *> nodes () {
        vector<Tnode *> result;
        get_nodes(root, result);

        return result;
    }

    long numIndexedQueries() {
        vector<Tnode *> nodes = this->nodes();
        long indexed = 0;

        for (int i = 0; i < nodes.size(); i += 1) {
            indexed += nodes[i]->hashmap.size();
        }

        return indexed;
    }

};

template <class T>
bool compareInterval(LeafNode<T> * i1, LeafNode<T> * i2)
{
    return (i1->interval.min < i2->interval.min);
}

#endif // LTREE_H
