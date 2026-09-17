#ifndef LFU_CACHE_HPP
#define LFU_CACHE_HPP

#include <cstddef>
#include <unordered_map>
#include <list>
#include <iostream>

typedef int value_t;

// hash_map_1 key->record
// hash_map freq -> dlist of records

#ifdef VERBOSE_ON
    #define VERBOSE(code) code
#else
    #define VERBOSE(code)
#endif

template <typename T, typename KeyT> struct Record // put in class?
{
	KeyT key;
	T freq;
	value_t value;
};

template <typename T, typename KeyT>
class LFUCache
{
	public:
		LFUCache(int capacity) : capacity_(capacity), min_freq_(1) { }

		using node_iter = std::list<Record<T, KeyT>>::iterator;
		using dlist = std::list<Record<T, KeyT>>;

		template <typename F>
 		bool lookup_update(KeyT key, F slow_get_page);

		std::size_t max_capacity() const { return capacity_; }
		bool is_full() const { return (capacity_ == cache_map_.size()); }

		void cache_dump();

	private:
		std::unordered_map<KeyT, node_iter> cache_map_;	// here may be a problem
		std::unordered_map<KeyT, dlist> freq_to_list_map_;
		std::size_t capacity_;
		int min_freq_;
};


// check AI version and website version
template <typename T, typename KeyT>
template<typename F>
bool LFUCache<T, KeyT>::lookup_update(KeyT key, F slow_get_page)
{
	if (max_capacity() == 0)
		return false;

	if (auto it = cache_map_.find(key); it != cache_map_.end())
	{
		VERBOSE(std::cout << "found requested element" << std::endl;)

		Record<T, KeyT> node = *(it->second);
		freq_to_list_map_[node.freq].erase(it->second);
		node.freq += 1;
		freq_to_list_map_[node.freq].push_front(node);
		cache_map_[key] = freq_to_list_map_[node.freq].begin();
		if (freq_to_list_map_[min_freq_].empty()) min_freq_++;

		return true;
	}
	else
	{
		VERBOSE(std::cout << "could bot find requested element" << std::endl;)

		T val = slow_get_page(key);
		Record<T, KeyT> rec {.key = key, .value = val, .freq = 1};

		if (is_full())
		{
			node_iter evicted_node_iter = --(freq_to_list_map_[min_freq_].end());
			cache_map_.erase(evicted_node_iter->key);
			freq_to_list_map_[min_freq_].pop_back();
		}

		min_freq_ = 1;
		freq_to_list_map_[min_freq_].push_front(rec);
		cache_map_[key] = freq_to_list_map_[min_freq_].begin();

		return false;
	}
}

template <typename T, typename KeyT>
void LFUCache<T, KeyT>::cache_dump()
{
	std::cout << "CACHE DUMP" << std::endl;
	std::cout << "Capacity: " << capacity_ << std::endl;
	std::cout << "Used cache space: " <<  cache_map_.size()  << std::endl;
	std::cout << "Min frequency: " << min_freq_ << std::endl;

	std::cout << "freq_to_list_map_ dump: " << std::endl;
	for (auto it = freq_to_list_map_.begin(); it != freq_to_list_map_.end(); ++it)
	{
		int freq = it->first;
		std::cout << "[" << freq << "]: ";
		for (auto list_it = freq_to_list_map_[freq].begin();
							list_it != freq_to_list_map_[freq].end(); ++list_it)
		{
			std::cout << list_it->value << " ";
		}
		std::cout << std::endl;
	}

	std::cout << "Cache map dump: " << std::endl;
	for (auto it = cache_map_.begin(); it != cache_map_.end(); ++it)
	{
		node_iter cur_node = it->second;

		std::cout << cur_node->value << " ";
	}
	std::cout << std::endl;
}




#endif