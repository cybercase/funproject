#include <iostream>
#include <string>

void rotate_zero(std::string& s, int count)
{
    // Rotate a string using string iterators

    int len = s.size();
    if (count >= len)
        count %= len;
    std::string buf(s.begin(), s.begin()+count);
    s.replace(s.begin(), s.begin()+len-count, s.begin()+count, s.end());
    s.replace(s.begin()+len-count, s.end(), buf.begin(), buf.end());
}

void rotate_one(std::string& s, int count)
{
    // Rotate a string without using string iterators
    int len = s.size();
    if (count >= len)
        count %= len;
    std::string buf(s.c_str(), count);
    for (int i=0; i < len-count; ++i)
        s[i] = s[i+count];
    for (int i=0; i < count; ++i)
        s[i+len-count] = buf[i];
}

void rotate_two(std::string& s, int count)
{
    // Rotate a string using just 1 char buffer
    int len = s.size();
    if (count >= len)
        count %= len;
    char c;
    for (int i=0; i<count; ++i)
    {
        int j=i;
        c = s[j];
        while (j+count < len)
        {
            s[j] = s[j+count];
            j += count;
        }
        s[j] = c;
    }
}


void reverse(std::string& s, int i, int j)
{
    // Utility function for rotate three...
    // Reverse chars in s from position i to position j (excluded)
    if (i > j)
    {
        int t=i;
        i=j;
        j=t;
    }

    while(i < --j)
    {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
        ++i;
    }
}

void rotate_three(std::string& s, int count)
{
    // Reverse a string in place
    reverse(s, 0, count);
    reverse(s, count, s.size());
    reverse(s, 0, s.size());
}

int main(int argc, char** argv)
{
    // Rotate a string using several reverse function

    std::string str("01234567");
    std::cout << "Original:" << str << std::endl;
    
    rotate_zero(str, 2);
    std::cout << "Rotate_zero:" << str << std::endl;
    
    rotate_one(str, 2);
    std::cout << "Rotate_one:" << str << std::endl;
    
    rotate_two(str, 2);
    std::cout << "Rotate_two:" << str << std::endl;
    
    rotate_three(str, 2);
    std::cout << "Rotate_three:" << str << std::endl;
}
