#pragma once

#include "ArcCacheNode.h"
#include <unordered_map>
#include <map>
#include <mutex>
#include <list>
namespace CacheSpace {
	template <typename Key, typename Value>
	class ArcLFUPart {
	public:
		using node_t = ArcNode<Key, Value>;
		using node_ptr = std::shared_ptr<node_t>;
		using node_map = std::unordered_map<Key, node_ptr>;
		using freq_map = std::map<size_t, std::list<node_ptr>>;

	private:
		size_t capacity;
		size_t ghostCapacity;
		size_t transformThreshold;
		size_t minFreq;
		std::mutex mtx;

		node_map mainCache;
		node_map ghostCache;
		freq_map freqMap;
		node_ptr ghostHead;
		node_ptr ghostTail;

	public:
		explicit ArcLFUPart(size_t capacity, size_t transformThershold) : capacity(capacity), ghostCapacity(capacity),
			transformThreshold(transformThershold), minFreq(0) {
			initializeLists();
		}
		bool put(Key key, Value val) {
			if (capacity == 0) {
				return false;
			}
			std::lock_guard<std::mutex> lock(mtx);
			auto it = mainCache.find(key);
			if (it != mainCache.end()) {
				return updateExistingNode(it->second, val);
			}
			return addNewNode(key, val);
		}
		bool get(Key key, Value &val) {
			std::lock_guard<std::mutex> lock(mtx);
			auto it = mainCache.find(key);
			if (it != mainCache.end()) {
				updateNodeFrequency(it->second);
				val = it->second->getValue();
				return true;
			}
			return false;
		}
		bool contain(Key key) {
			return mainCache.count(key) > 0;
		}
		bool checkGhost(Key key) {
			auto it = ghostCache.find(key);
			if (it != ghostCache.end()) {
				removeFromGhost(it->second);
				ghostCache.erase(it);
				return true;
			}
			return false;
		}
		void increaseCapacity() {
			capacity++;
		}
		bool decreaseCapacity() {
			if (capacity <= 0) {
				return false;
			}
			if (mainCache.size() == capacity) {
				evictLeastFrequent();
			}
			--capacity;
			return true;
		}

	private:
		void initializeLists() {
			ghostHead = std::make_shared<node_t>();
			ghostTail = std::make_shared<node_t>();
			ghostHead->next = ghostTail;
			ghostTail->prev = ghostHead;
		}
		bool updateExistingNode(node_ptr node, const Value &val) {
			node->setValue(val);
			updateNodeFrequency(node);
			return true;
		}
		bool addNewNode(const Key &key, const Value &val) {
			if (mainCache.size() >= capacity) {
				evictLeastFrequent();
			}
			node_ptr newNode = std::make_shared<node_t>(key, val);
			mainCache[key] = newNode;
			if (freqMap.count(1) == 0) {
				freqMap[1] = std::list<node_ptr>();
			}
			freqMap[1].push_back(newNode);
			minFreq = 1;
			return true;
		}
		void updateNodeFrequency(node_ptr node) {
			size_t oldFreq = node->getAccessCnt();
			node->incrementAccessCnt();
			size_t newFreq = node->getAccessCnt();
			auto &oldList = freqMap[oldFreq];
			oldList.remove(node);
			if (oldList.empty()) {
				freqMap.erase(oldFreq);
				if (oldFreq == minFreq) {
					minFreq = newFreq;
				}
			}
			if (freqMap.count(newFreq) == 0) {
				freqMap[newFreq] = std::list<node_ptr>();
			}
			freqMap[newFreq].push_back(node);
		}
		void evictLeastFrequent() {
			if (freqMap.empty()) {
				return;
			}
			auto &minFreqList = freqMap[minFreq];
			if (minFreqList.empty()) {
				return;
			}
			node_ptr leastNode = minFreqList.front();
			minFreqList.pop_front();

			if (minFreqList.empty()) {
				freqMap.erase(minFreq);
				if (!freqMap.empty()) {
					minFreq = freqMap.begin()->first;
				}
			}
			if (ghostCache.size() >= ghostCapacity) {
				removeOldestGhost();
			}
			addToGhost(leastNode);
			mainCache.erase(leastNode->getKey());
		}
		void removeFromGhost(node_ptr node) {
			if (!node->prev.expired() && node->next) {
				auto prev = node->prev.lock();
				prev->next = node->next;
				node->next->prev = node->prev;
				node->next = nullptr;
			}
		}
		void addToGhost(node_ptr node) {
			node->next = ghostTail;
			node->prev = ghostTail->prev;
			if (!ghostTail->prev.expired()) {
				ghostTail->prev.lock()->next = node;
			}
			ghostTail->prev = node;
			ghostCache[node->getKey()] = node;
		}
		void removeOldestGhost() {
			node_ptr oldestGhost = ghostHead->next;
			if (oldestGhost != ghostTail) {
				removeFromGhost(oldestGhost);
				ghostCache.erase(oldestGhost->getKey());
			}
		}
	};
}
