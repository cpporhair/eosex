//
// Created by 杨文宇 on 2018/10/26.
//
#pragma once

#include <boost/functional/hash.hpp>

template <typename T>
class hash_type {
public:
    hash_type(const T &x):_value{x}{}
    std::size_t hash() {
        boost::hash<T> hasher;
        return hasher(_value);
    }
private:
    T _value;
};


uint32_t djb_hash(const std::string& str)
{
    uint32_t hash = 5381;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = ((hash << 5) + hash) + str[i];
    }

    return hash;
}

uint32_t djb_hash(const char *str,std::size_t len) {
    uint32_t hash = 5381;

    for(std::size_t i = 0; i < len; i++)
    {
        hash = ((hash << 5) + hash) + str[i];
    }

    return hash;
}

uint32_t rs_hash(const std::string& str)
{
    uint32_t b    = 378551;
    uint32_t a    = 63689;
    uint32_t hash = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = hash * a + str[i];
        a    = a * b;
    }

    return hash;
}
/* End Of RS Hash Function */


uint32_t js_hash(const std::string& str)
{
    uint32_t hash = 1315423911;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash ^= ((hash << 5) + str[i] + (hash >> 2));
    }

    return hash;
}
/* End Of JS Hash Function */


uint32_t pjw_hash(const std::string& str)
{
    uint32_t BitsInUnsignedInt = (uint32_t)(sizeof(unsigned int) * 8);
    uint32_t ThreeQuarters     = (uint32_t)((BitsInUnsignedInt  * 3) / 4);
    uint32_t OneEighth         = (uint32_t)(BitsInUnsignedInt / 8);
    uint32_t HighBits          = (uint32_t)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth);
    uint32_t hash              = 0;
    uint32_t test              = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = (hash << OneEighth) + str[i];

        if((test = hash & HighBits)  != 0)
        {
            hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits));
        }
    }

    return hash;
}
/* End Of  P. J. Weinberger Hash Function */


unsigned int ELFHash(const std::string& str)
{
    unsigned int hash = 0;
    unsigned int x    = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = (hash << 4) + str[i];
        if((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
        }
        hash &= ~x;
    }

    return hash;
}
/* End Of ELF Hash Function */


unsigned int BKDRHash(const std::string& str)
{
    unsigned int seed = 131; // 31 131 1313 13131 131313 etc..
    unsigned int hash = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = (hash * seed) + str[i];
    }

    return hash;
}
/* End Of BKDR Hash Function */


unsigned int SDBMHash(const std::string& str)
{
    unsigned int hash = 0;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = str[i] + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}
/* End Of SDBM Hash Function */


unsigned int DJBHash(const std::string& str)
{
    unsigned int hash = 5381;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = ((hash << 5) + hash) + str[i];
    }

    return hash;
}
/* End Of DJB Hash Function */


unsigned int DEKHash(const std::string& str)
{
    unsigned int hash = static_cast<unsigned int>(str.length());

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = ((hash << 5) ^ (hash >> 27)) ^ str[i];
    }

    return hash;
}
/* End Of DEK Hash Function */


unsigned int BPHash(const std::string& str)
{
    unsigned int hash = 0;
    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash = hash << 7 ^ str[i];
    }

    return hash;
}
/* End Of BP Hash Function */


unsigned int FNVHash(const std::string& str)
{
    const unsigned int fnv_prime = 0x811C9DC5;
    unsigned int hash = 0;
    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash *= fnv_prime;
        hash ^= str[i];
    }

    return hash;
}
/* End Of FNV Hash Function */


unsigned int APHash(const std::string& str)
{
    unsigned int hash = 0xAAAAAAAA;

    for(std::size_t i = 0; i < str.length(); i++)
    {
        hash ^= ((i & 1) == 0) ? (  (hash <<  7) ^ str[i] * (hash >> 3)) :
                (~((hash << 11) + (str[i] ^ (hash >> 5))));
    }

    return hash;
}
/* End Of AP Hash Function */