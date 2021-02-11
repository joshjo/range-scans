#ifndef CITREE_H
#define CITREE_H

#include "includes.h"
#include "interval.h"
#include "query.h"

using namespace std;

template <class T>
T getMedian(vector<T> & s) {
    size_t ss = s.size();
    size_t middle = (size_t) ss / 2;

    if (ss % 2) {
        return s[middle];
    }

    return (float) (s[middle] + s[middle - 1]) / 2;
}

template <class T>
vector<T> getEndpoints(vector <Query<T> * > & queries) {
    vector<T> s;
    for (size_t i = 0; i < queries.size(); i++) {
        s.push_back(queries[i]->interval.min);
        s.push_back(queries[i]->interval.max);
    }
    return s;
}

template <class T>
class CINode {
    public:
    typedef Interval<T> Tinterval;
    typedef CINode<T> Tnode;
    typedef Query<T> Tquery;

    Tnode * left;
    Tnode * right;
    Tnode * parent;
    T value;
    vector<Tquery *> centers;

    CINode(T val) {
        this->value = val;
        left = NULL;
        right = NULL;
        parent = NULL;
    }

    string to_graphviz(string iter = "") {
        string str = "\"[" + to_string(value) + "]" + "{" + to_string(centers.size()) + "}" + "\"";
        return str;
    }
};

template <class T>
class CITree {
    public:
    typedef Interval<T> Tinterval;
    typedef CINode<T> Tnode;
    typedef Query<T> Tquery;

    Tnode * root;

    CITree() {
        root = NULL;
    }

    void __build(vector <Tquery *> & queries, Tnode * & node) {
        if (! queries.size()) {
            return;
        }
        vector <T> ee = getEndpoints(queries);
        T M = getMedian(ee);
        node = new Tnode(M);
        vector<Tquery *> lefts, rights;
        for (size_t i = 0; i < queries.size(); i++) {
            // Interval<T> * t = &(intervals[i]);
            if (queries[i]->interval.intersects(M)) {
                // Centered intervals
                node->centers.push_back(queries[i]);
            } else if (queries[i]->interval.max <= M) {
                // Intervals that goes to the left
                lefts.push_back(queries[i]);
            } else if (queries[i]->interval.length() > 0) {
                // Go to right, the if exclude intervals with 0=length
                rights.push_back(queries[i]);
            }
        }

        __build(lefts, node->left);
        __build(rights, node->right);
    }

    void insert(vector <Tquery *> & queries) {
        __build(queries, root);
    }

    vector <Tquery * > find(T key) {
        vector<Tquery *> result;
        priority_queue<Tnode *> q;
        q.push(root);
        while (!q.empty()) {
            Tnode * top = q.top();
            q.pop();
            if (top == NULL) {
                continue;
            }
            for (int i = 0; i < top->centers.size(); i++) {
                if (top->centers[i]->interval.intersects(key)) {
                    result.push_back(top->centers[i]);
                }
            }

            q.push(top->left);
            q.push(top->right);
        }

        return result;
    }

    void graphviz(Tnode *node, string & tree, string iter="") {
        if (node != NULL) {
            tree += node->to_graphviz(iter);
            if (node->parent == NULL && node->left == NULL && node->right == NULL) {
                tree += node->to_graphviz(iter);
            }
            if (node->left != NULL) {
                tree += node->to_graphviz(iter) + " -> " + node->left->to_graphviz(iter) + " ";
            }
            if (node->right != NULL) {
                tree += node->to_graphviz(iter) + " -> " + node->right->to_graphviz(iter) + " ";
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

    size_t size() {
        size_t count = 0;
        priority_queue<Tnode *> q;
        q.push(root);

        while (!q.empty()) {
            Tnode * top = q.top();
            q.pop();
            if (top == NULL) {
                continue;
            }

            count += top->centers.size();

            q.push(top->left);
            q.push(top->right);
        }

        return count;
    }
};


#endif // CITREE_H
