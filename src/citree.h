#ifndef CITREE_H
#define CITREE_H

#include "includes.h"
#include "interval.h"

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
vector<T> getEndpoints(vector <Interval<T> > & intervals) {
    vector<T> allEndpoints;
    for (size_t i = 0; i < intervals.size(); i++) {
        allEndpoints.push_back(intervals[i].min);
        allEndpoints.push_back(intervals[i].max);
    }

    set<T> s(allEndpoints.begin(), allEndpoints.end());

    vector <T> result (s.begin(), s.end());
    return result;
}

template <class T>
class CINode {
    public:
    typedef Interval<T> Tinterval;
    typedef CINode<T> Tnode;
    Tnode * left;
    Tnode * right;
    Tnode * parent;
    T value;
    vector<Tinterval *> centers;

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

    Tnode * root;

    CITree() {
        root = NULL;
    }

    void build(vector <Tinterval> &intervals, Tnode * & node) {
        if (! intervals.size()) {
            return;
        }
        vector <T> ee = getEndpoints(intervals);
        T M = getMedian(ee);
        node = new Tnode(M);
        vector<Tinterval> lefts, rights;
        for (size_t i = 0; i < intervals.size(); i++) {
            Interval<T> * t = &(intervals[i]);
            if (t->intersects(M)) {
                // Centered intervals
                node->centers.push_back(new Tinterval(*t));

                // if (M == 731436) {
                //     // cout << "t: " << *(node->centers[i]) << endl;
                // }
            } else if (t->max <= M) {
                // Intervals that goes to the left
                lefts.push_back(*t);
            } else if (t->length() > 0) {
                // Go to right, the if exclude intervals with 0=length
                rights.push_back(*t);
            }
        }

        build(lefts, node->left);
        build(rights, node->right);
    }

    void insert(vector <Tinterval> & intervals) {
        build(intervals, root);

        // vector<T> endpoints = getEndpoints(intervals);

        // T ee = getMedian(endpoints);

        // for(int i = 0; i < endpoints.size(); i++) {
        //     cout << endpoints[i] << " ";
        // }
    }

    Tnode * findMiddle(T val) {
        Tnode * result = root;
        priority_queue<Tnode *> q;
        q.push(root);

        while (!q.empty()) {
            Tnode * top = q.top();
            q.pop();
            if (top == NULL) {
                continue;
            }
            if (top->value == val) {
                result = top;
                break;
            }
            q.push(top->left);
            q.push(top->right);
        }

        return result;
    }

    vector <Tinterval * > find(T key) {
        vector<Tinterval *> result;
        priority_queue<Tnode *> q;
        q.push(root);
        while (!q.empty()) {
            Tnode * top = q.top();
            q.pop();
            if (top == NULL) {
                continue;
            }
            for (int i = 0; i < top->centers.size(); i++) {
                if (top->centers[i]->intersects(key)) {

                    // cout << top->value << endl;
                    // cout << "centers " << *(top->centers[i]) << endl;
                    // cout << "centers " << top->centers[i] << endl;
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

    size_t numberOfIntervals() {
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
