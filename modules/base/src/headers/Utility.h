#ifndef IKNOW_BASE_UTILITY_H_
#define IKNOW_BASE_UTILITY_H_

#include <unordered_map>
#include <utility>
#include <type_traits>

struct pair_hash
{
  template <typename T1,typename T2>
  std::size_t operator() (const std::pair<T1,T2> &p) const {
    return std::hash<T1>()(p.first) ^ std::hash<T2>()(p.second);
  }
};

template <typename TIter>
std::size_t hash_range(TIter begin, TIter end) {
  size_t hash = 0;
  while (begin != end) {
    hash ^= std::hash<unsigned long>()(*begin++);
  }
  return hash;
}

template <typename TKey, typename TValue>
using PairUnorderedMap = std::unordered_map<TKey, TValue, pair_hash>;

#endif //IKNOW_BASE_UTILITY_H_
