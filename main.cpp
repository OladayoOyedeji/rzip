#include <iostream>
#include <vector>
#include <bitset>
#include "File.h"
#include "TapeArchiver.h"

using namespace std;

int mode = 1;

std::ostream & operator<<(std::ostream & cout, const vector<int> &v)
{
    cout << "print: " << v.size() << endl;
    for (int i = 0; i < v.size(); ++i)
    {
        // cout << "i: " << i << " whats going on  " <<bitset<32>(v[i]) << ' ' << v[i] << '\n';
        int i0 = v[i] & ((1 << 9) - 1);
        int i1 = (v[i] >> 8) & ((1 << 9) - 1);
        int i2 = ((v[i] >> 8) >> 8) & ((1 << 9) - 1);
        int i3 = (((v[i] >> 8) >> 8) >> 8) & ((1 << 9) - 1);
        cout << char(i0) << ' '
             << char(i1) << ' '
             << char(i2) << ' '
             << char(i3) << '\n';
    }
    return cout;
}

void print_buffer(unsigned char * buff, int size)
{
    for (int i = 0; i < size; ++i)
    {
        if (mode == 1)
            cout << buff[i] << ' ';
        else
        {
            int x = buff[i];
            cout << x << ' ';
        }
    }
}

void buffer_to_bin_seq(unsigned char * buff, int size)
{
    vector<int> x(size/4, 0);

    cout << x.size() << endl;
    for (int j = 0; j < size; j += 4)
    {
        int i = j;
        int c;
        unsigned char * p = (unsigned char *) &c;
        cout << i << ' ' << i / 4 << endl;
        cout << buff[i];
        *p = buff[i++];
        cout << buff[i];
        *(p+1) = buff[i++];
        cout << buff[i];
        *(p+2) = buff[i++];
        cout << buff[i] << endl;;
        *(p+3) = buff[i++];
        x[j/4] = c;
        cout << bitset<32>(c) << endl;
    }
    cout << x.size() << endl;
    cout << x << endl;
}

std::vector< freq_sym_pair > get_freq(const std::string & buff)
{
    std::map< char, float > ht;
    
    for (int i = 0; i < buff.size(); ++i)
    {
        ht[buff[i]]++;
    }

    std::vector< freq_sym_pair > xs;
    for (auto p: ht)
    {
        xs.push_back(freq_sym_pair(p.second, p.first));
    }

    return xs;
}

void build_huffman_tree(std::vector< freq_sym_pair > & xs)
{
    auto h = build_head(xs);
    int n = xs.size();

    for (int i = 0; i < n; ++i)
    {
        freq_sym_pair a = h.root();
        h.extract_root();
        freq_sym_pair b = h.root();
        h.extract_root();
        // binary tree t -
        //     root = (frequency=a.frequency +  b.frequency, -)
        Binary_node t(a.first + b.first, '-', a, b);
        h.insert(t);
    }

    return h.root();
}

int main()
{
    // file: xyz
    // std::string name;
    // std::cin >> name;
    // cout << name << endl;

    // File f(name);

    // unsigned char buff[2024];
    // ssize_t s = 1024;
    // // try
    // // {
    //     ssize_t size = f.myread(buff, s);
    //     cout << buff << endl;
    
    //     f.myclose();
    //     buffer_to_bin_seq(buff, size);
    // }
    // catch (ReadError & e)
    // {
    //     cout << "what happened nihhh";
    // }
    // for (int i = 0; i < 8; ++i)
    // {
    //     buff[i] = 128 + i;
    // }
    
    Tar t;
    t.addtoTar("xyz");

    std::vector< freq_sym_pair > xs = get_freq(buff);
    
    cout << t.buff << endl;
    
    // print_buffer(buff, 8);
    
    return 0;
}
