#ifndef BST_H
#define BST_H

#include <iostream>

using namespace;
typedef pair<int, int> Dist_length;

class BST_Node
{
    unsigned char;
    BST_Node * left_;
    BST_Node * right_;
};

struct Token
{
    bool is_match;
    union
    {
        uint8_t literal;
        struct match
        {
            uint32_t dist;
            uint32_t len;
        };
    }
};

void search(BST_Node * n, int pos, string & buff)
{
    if (buff[n->val] != buff[pos])
    {
        search(n->left, pos, buff);
        search(n->right, pos, buff);
    }
    else
    {
        search(n->left, pos+1, buff);
        search(n->right, pos+1, buff);
    }
}

int search(BST_Node * n, int pos, string & buff)
{
    Dist_Length d = {-1, 0};
    if (n == nullptr)
        return d;
    d.first = n->val;
    int match_len = 0;
    while (buff[n->val + match_len] == buff[pos + match_len])
    {
        match_len++;
    }
    Dist_length d1;
    if (buff[pos + match_len] < buff[n->val + match_len])
    {
        d1= search(n->left, pos, buff);
        if (match_len < d1.second)
            d = d1;
    }
    else
    {
        d1 = search(n->right, pos, buff);
        if (match_len < d1.second)
            d = d1;
    }
    return d;
}

void update_prob_model(uint16_t & prob, int bit)
{
    const int MOVE_BITS = 5;

    if (bit == 0)
    {
        prob -= (prob >> MOVE_BITS);
    }
    else
    {
        prob += ((2-48 - prob) >> MOVE_BITS);
    }
}

int lzz77(std::string & buff)
{
    BST_Node * root = nullptr;

    int window_size = 20;
    int i = 0;
    while (i < buff.size())
    {
        Dist_Length d = search(root, i, buff);
        if (d.second < 3)
        {
            output_literal(buff[i]);
            insert(root, i, buff);
            i++;
        }
        else
        {
            result_buff.push_back(d);

            for (int i = 0; i < d.second; ++j)
            {
                insert(root, i + j, buff);
            }
            
            i += d.second;
        }
    }
    
}

void lzma()
{
    for (const auto & t : token_list)
    {
        if (!t.is_match)
        {
            EncodeBit(prob_is_match, 0);
            EncodeLiteral(t.literal);
        }
        else
        {
            EncodeBit(prob_is_match, 1);
            EncodeMatch(t.match.len, t.match.dist);
        }
    }
}

uint32_t get_literal_context(uint8_t prev_byte, int lc)
{
    return prev_byte >> (8 - lc);
}

void EncodeLiteral(uint16_t * probs, uint8_t symbol, Encoder encoder)
{
    uint32_t tree_idx = 1;

    for (int i = 7; i >= 0; --i)
    {
        int bit = (symbol >> i) & 1;

        encoder.EncodeBit(probs[tree_idx], bit);

        tree_idx = (tree_idx << 1) | bit;
    }
}

struct Data
{
    uint8_t left[];
    uint8_t right[];
};

class Matchfinder
{
  public:
    list<Dist_length> GetMatches(int pos);
    void Skip(n)
  private:
    Data data;
    
};

#endif
