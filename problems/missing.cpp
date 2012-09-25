/* Input:
* Shuffled vector of unsigned integers comprised by 0 and SIZE having 
* one or more elements missing.
* (So vector size is SIZE - number_of_missing_elements)
* 
* Problem:
* Find one of the missing element in vector in O(N)
*
* Assumption:
* Integers are unique inside the entire vector.
*
* Bibliography: Jon Bentley - Programming pearls - second edition
*/

#include <iostream>
#include <vector>
#include <cassert>
#include <cstdlib>

unsigned int on_search(std::vector<unsigned int>& input, unsigned int max_val)
{
    int shift = 0;
    while(max_val >> shift)
    {
        ++shift;
    };

    unsigned int missing=0;
    unsigned int mask=0;

    while (shift--)
    {
        mask = 1 << shift;

        std::vector<unsigned int> left, right;
        // Max length for each vector
        left.reserve(mask); right.reserve(mask);

        // Scan
        for (int i=0; i<input.size(); ++i)
            if (input[i] & mask)
                right.push_back(input[i]);
            else
                left.push_back(input[i]);

        assert(right.size() <= max_val - mask); // Right uniqueness
        assert(input.size()-left.size() <= mask); // Left uniqueness
        assert(mask-right.size() != 0 ||
               max_val - mask - left.size() != 0); // Any missing

        if (mask-left.size() >= max_val - mask - right.size())
        {
            missing = missing << 1;
            input.swap(left);
            max_val=mask;
        }
        else
        {
            missing = missing << 1 | 1;
            input.swap(right);
            max_val-=mask;
        }
    }
    return missing;
}


#define SIZE 1000000 // Try to increase
int main(int argc, char** argv)
{
    std::vector<unsigned int> in(SIZE);
    
    // Unordered fill
    std::cout << "Filling vector with " << SIZE << " elements...";
    for (int i=0; i < SIZE/2; ++i)
    {
        in[i] = i;
        in[i+SIZE/2] = i+SIZE/2;
        if (rand()>RAND_MAX/2)
            std::swap(in[i], in[i+SIZE/2]);
    }
    if(SIZE & 1)
        in[SIZE-1] = SIZE-1;
    std::cout << "Done!" << std::endl;

    // Removing one
    int k = 7;
    std::cout << "Removing " << *(in.begin()+k)
              << " at position " <<k << std::endl;
    in.erase(in.begin()+k);

    // Search
    std::cout << "on_search..." << std::endl;
    std::cout << "Element " << on_search(in, SIZE)
              << " is missing" << std::endl;
    return 0;
}
