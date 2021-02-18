#pragma once

#include <iostream>
#include <fstream>
#include <vector>

template<typename T>
std::ostream& serialize(std::ostream& os, std::vector<T> const& v)
{
    // this only works on built in data types (PODs)
    //static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value,
    //    "Can only serialize POD types with this function");

    auto size = v.size();
    os.write(reinterpret_cast<char const*>(&size), sizeof(size));
    os.write(reinterpret_cast<char const*>(v.data()), v.size() * sizeof(T));
    return os;
}

template<typename T>
std::istream& deserialize(std::istream& is, std::vector<T>& v)
{
    //static_assert(std::is_trivial<T>::value && std::is_standard_layout<T>::value,
    //    "Can only deserialize POD types with this function");

    decltype(v.size()) size;
    is.read(reinterpret_cast<char*>(&size), sizeof(size));
    v.resize(size);
    is.read(reinterpret_cast<char*>(v.data()), v.size() * sizeof(T));
    return is;
}
