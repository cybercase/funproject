#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdlib>
#include <stdexcept>

struct MyPoint
{
    MyPoint(int c, int x, int y, int d) :
             color(c), x(x), y(y), d(d) {}
    MyPoint() : color(0), x(0), y(0), d(0) {}

    bool operator==(const MyPoint& o) const
    {
        return color==o.color && x==o.x && y==o.y && d==o.d;
    }

    int color;
    int x;
    int y;
    int d;
};

std::ostream& operator << (std::ostream& out, const MyPoint& m)
{
    return out << "c=" << m.color << "\td=" << m.d << "\ty=" << m.y \
               << "\tx=" << m.x;
}

template <class T> void print(const T& v)
{
    for (int i=0, sz=int(v.size()); i<sz; ++i)
        std::cerr << v[i] << std::endl;
}


// Single Value sort functions
bool mypoint_x_sort_fn(const MyPoint& p0, const MyPoint& p1)
{
    return p0.x < p1.x;
}
bool mypoint_y_sort_fn(const MyPoint& p0, const MyPoint& p1)
{
    return p0.y < p1.y;
}
bool mypoint_d_sort_fn(const MyPoint& p0, const MyPoint& p1)
{
    return p0.d < p1.d;
}
bool mypoint_c_sort_fn(const MyPoint& p0, const MyPoint& p1)
{
    return p0.color < p1.color;
}

// Multiple value sort function
bool mypoint_all_sort_fn(const MyPoint& p0, const MyPoint& p1)
{
    if (p0.color < p1.color)
        return true;
    else if (p0.color == p1.color)
        if (p0.d < p1.d)
            return true;
        else if (p0.d == p1.d)
            if (p0.y < p1.y)
                return true;
            else if (p0.y == p1.y)
                if (p0.x < p1.x)
                    return true;
    return false;
}

// Order defining function
// Wrapped in a struct because functions doesn't support partial template
// specialization: a very clear example by Peter Dimov and Dave Abrahams
// is found at http://www.gotw.ca/publications/mill17.htm
template <typename T> int element(const T&, int i);
template <typename T, int I> struct Orderer
{
    static bool compare(const T& t0, const T& t1)
    {
        if (element(t0, I) < element(t1, I))
            return true;
        else if (element(t0, I) == element(t1, I))
            return Orderer<T, I-1>::compare(t0, t1);
        return false;
    }
};
template <typename T> struct Orderer<T, -1>
{
    static bool compare(const T& t0, const T& t1)
    {
        return false;
    }
};

template <> int element<MyPoint>(const MyPoint& p, int i)
{
    switch(i)
    {
        case 3:
            return p.color;
        case 2:
            return p.d;
        case 1:
            return p.y;
        case 0:
            return p.x;
        default:
            throw std::invalid_argument("Undefined element");
    }
}
bool mypoint_sort(const MyPoint& t0, const MyPoint& t1)
{
    return Orderer<MyPoint, 3>::compare(t0, t1);
}

int main(int argc, char **argv)
{
    int width = 100; // y
    int height = 100; // x
    int depth = 5;
    int colors = 2; // Colors

    int len = width * height * colors * depth;
    std::vector<MyPoint> pts;
    pts.reserve(len);

    for (int c=0; c<colors; ++c)
        for (int d=0; d<depth; ++d)
            for (int y=0; y<height; ++y)
                for (int x=0; x<width; ++x)
                    pts.push_back(MyPoint(c, x, y, d));
    // print(pts);
    
    std::vector<MyPoint> pts_sorted(pts);

    std::random_shuffle(pts.begin(), pts.end());
    
    std::vector<MyPoint> pts_shuffled(pts);

    // Using std::stable_sort
    std::stable_sort(pts.begin(), pts.end(), mypoint_x_sort_fn);
    std::stable_sort(pts.begin(), pts.end(), mypoint_y_sort_fn);
    std::stable_sort(pts.begin(), pts.end(), mypoint_d_sort_fn);    
    std::stable_sort(pts.begin(), pts.end(), mypoint_c_sort_fn);

    if (pts == pts_sorted)
        std::cerr << "Stable sort OK" << std::endl;
    else
        std::cerr << "Stable sort FAIL" << std::endl;
    
    // Using std::sort
    pts = pts_shuffled;

    std::sort(pts.begin(), pts.end(), mypoint_all_sort_fn);
    if (pts == pts_sorted)
        std::cerr << "Sort OK" << std::endl;
    else
        std::cerr << "Sort FAIL" << std::endl;

    // Templates world
    pts = pts_shuffled;
    
    std::sort(pts.begin(), pts.end(), mypoint_sort);
    if (pts == pts_sorted)
        std::cerr << "Sort OK" << std::endl;
    else
        std::cerr << "Sort FAIL" << std::endl;

    return 0;
}

