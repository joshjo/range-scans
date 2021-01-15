#ifndef INTERVAL_H
#define INTERVAL_H

#include "includes.h"

using namespace std;

template <class T>
class Interval {
public:
    typedef Interval<T> Tinterval;
    T min, max;

    Interval() {
        min = 0;
        max = 0;
    }

    Interval(T a, T b) {
        if (a > b) {
            swap(a, b);
        }
        this->min = a;
        this->max = b;
    }

    Interval(const Interval & other) {
        min = other.min;
        max = other.max;
    }

    T length() const {
        /**
         * @return the length of the interval.
         */

        return max - min;
    }

    T checksum() {
        T greater = (max * (max - 1)) / 2;
        T lower = (min * (min - 1)) / 2;

        return greater - lower;
    }

    T midpoint() const {
        /**
         * @return the midpoint of the interval.
         */
        T d = length();

        return (d / 2) + min;
    }

    Tinterval intersection(const Interval & other) {
        /**
         * @return a new interval with the intersection.
         * In case there is no intersection return an empty Interval (0, 0)
         */

        Tinterval result(0, 0);

        if ((other.min <= min && min <= other.max) || (other.min <= max && max <= other.max) || (min <= other.min && other.min <= max) || (min <= other.max && other.max <= max)) {
            result.min = min > other.min ? min : other.min;
            result.max = max < other.max ? max : other.max;
        }

        return result;
    }

    bool intersects(const T & point) {
        /**
         * @return a boolean indicating if the interval intersects with a point.
         */

        return min <= point && point < max;
    }

    bool intersects(const Interval & other) {
        /**
         * @return a boolean indicating if the interval intersects with another one
         */

        Tinterval i = intersection(other);

        return i.length() > 0;
    }

    string to_graphviz(string iter = "") {
        if (iter == "") {
            string str = "\"" + to_string() + "\"";
            return str;
        }
        string str = "\"[" + iter + "]" + to_string() + "\"";
        return str;
    }

    string to_string() {
        string str = "[" + std::to_string(min) + "," + std::to_string(max) + ")";

        return str;
    }

    Tinterval operator + (const Tinterval & other) const {
        T a = min < other.min ? min : other.min;
        T b = max > other.max ? max : other.max;

        Tinterval I(a, b);

        return I;
    }

    bool operator < (const Tinterval & other) const {
        /**
         * Override operator '<'
         * @return a boolean with the comparison between two intervals.
         */
        if (other.min <= max) {
            T m = this->midpoint();
            T n = other.midpoint();

            return m < n;
        }
        return (max < other.min);
    }

    bool operator > (const Interval & other) const {
        /**
         * Override operator '<'
         * @return a boolean with the comparison between two intervals.
         */
        if (other.max >= min) {
            T m = this->midpoint();
            T n = other.midpoint();

            return m > n;
        }
        return (min > other.max);
    }

    bool operator == (const Interval & other) {
        /**
         * Override operator '=='
         * @return a boolean with the comparison between two intervals.
         */
        return (this->min == other.min) && (this->max == other.max);
    }

    friend ostream & operator << (ostream &out, const Interval & i) {
        out << '[' << i.min << ';' << i.max << ')';
        return out;
    }
};


#endif // INTERVAL_H
