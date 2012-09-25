/* Input:
* You have a 10-million-lines file and each line contains
* a positive 7 digit integer.
*
* Problem:
* Sort the integers in input file.
*
* Assumption:
* Integers are unique inside the entire file.
*
* Bibliography: Jon Bentley - Programming pearls - second edition
*/

#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

#define DEBUG

struct Bitmap
{
    char* bm;
    int size;
    
    Bitmap(int sz) : size(sz)
    {
#ifdef DEBUG
        if (!sz)
            throw std::runtime_error("Invalid size for Bitmap");
#endif
        bm = new char[(sz+sizeof(char)-1)/sizeof(char)];
        memset(bm, 0, (sz+sizeof(char)-1)/sizeof(char));
    }

    ~Bitmap()
    {
        delete [] bm;
    }
    
    void set(int i)
    {
#ifdef DEBUG
        if (i >= size)
            throw std::out_of_range("Out of range");
        if (get(i))
            throw std::logic_error("Duplicate entry");
#endif
        bm[i/sizeof(char)] |= 1 << (i % sizeof(char));
    }

    int get(int i)
    {
#ifdef DEBUG
        if (i >= size)
            throw std::out_of_range("Out of range");
#endif
        return bm[i/sizeof(char)] && 1 << (i % sizeof(char));
    }

#ifdef DEBUG
    void dump()
    {
        std::cout << "BM: ";
        for (int i=0; i < size; ++i)
            std::cout << (bm[i/sizeof(char)]
            && 1 << ( (i % sizeof(char)) ) ? 1 : 0 );
        std::cout << std::endl;
    }
#endif

private:
    // Don't want use them because of char* bm
    Bitmap(const Bitmap &);
    Bitmap& operator=(const Bitmap&);
};

void generate_random_source(int len)
{
    // Generate in reversed order all the integers
    // from 0 to LEN-1
    std::cout << "Generating... ";
    std::fstream f("input.txt", std::ios_base::out);
    Bitmap bitmap(len);
#ifdef DEBUG
    int counter=0;
#endif
    for (int i=0; i<len; ++i)
    {
        unsigned int tmp=0;
        while(1) // Find an unique random number
        {
#ifdef DEBUG
            ++counter;
#endif
            // RAND_MAX is granted to be at least 32767 (16bit)
            // So i'm making sure for generating at least 2^32 integers
            tmp = (unsigned int)(rand() << 16 | rand()) % len;
            if (bitmap.get(tmp) == 0)
            {
                bitmap.set(tmp);
                break;
            }
        }
        f << tmp << std::endl;
    }
    f.close();
#ifdef DEBUG
    std::cout << "Iterations: " << counter << std::endl;
#endif
    std::cout << "Done!" << std::endl;
}

static const int LEN = 1000000;

int main(int argc, char **argv)
{
    // Try open/generate input.txt
    std::fstream f("input.txt", std::ios_base::in);
    if (f.fail())
    {
        f.close();
        generate_random_source(LEN);
        f.open("input.txt", std::ios_base::in);
    }
    if (f.fail())
    {
        std::cerr << "Can't create/access to input file" << std::endl;
        return -1;
    }
    
    // Create the bitmap
    Bitmap bitmap(LEN);

    // Read from file filling the bitmap
    int tmp;
    f >> tmp;
    while (!f.eof() && !f.fail())
    {
        bitmap.set(tmp);
        f >> tmp;
    }
    f.close();

    // Write sorted output
    f.open("output.txt", std::ios_base::out);
    for (int i=0; i < LEN; ++i)
        if (bitmap.get(i))
            f << i << std::endl;
    
    return 0;
};
