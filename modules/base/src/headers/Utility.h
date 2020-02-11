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

namespace stable_hash {

	//see http://www.cse.yorku.ca/~oz/hash.html
	template<typename It>
	size_t dbm_hash(const It begin, const It end)
	{
		size_t hash = 5381;
		for (It i = begin; i != end; ++i) {
			hash = ((hash << 5) + hash) + *i;
		}
		return hash;
	}
	//Currently we only need short hash functions
	template<typename It>
	size_t hash_range(const It begin, const It end) {
		return dbm_hash(begin, end);
	}
	template<typename T>
	size_t hash_value(const T& t) {
		return hash_range(t.begin(), t.end());
	}
}
#endif //IKNOW_BASE_UTILITY_H_
