#pragma once

#include "../cache_base.h"
#include "ArcLRUPart.h"
#include "ArcLFUPart.h"
#include <memory>

namespace CacheSpace {
	template <typename Key, typename Value>
	class ArcCache : public CacheBase<Key, Value> {
	private:
		size_t capacity;
		size_t transformThreshold;
		std::unique_ptr<ArcLRUPart<Key, Value>> lruPart;
		std::unique_ptr<ArcLFUPart<Key, Value>> lfuPart;

	public:
		explicit ArcCache(size_t capacity = 10, size_t transformThreshold = 2)
			: capacity(capacity), transformThreshold(transformThreshold),
			  lruPart(std::make_unique<ArcLRUPart<Key, Value>>(capacity, transformThreshold)),
			  lfuPart(std::make_unique<ArcLFUPart<Key, Value>>(capacity, transformThreshold)) {
		}

		~ArcCache() override = default;
		void put(Key key, Value val) override {
			checkGhostCaches(key);
			bool inlfu = lfuPart->contain(key);
			lruPart->put(key, val);
			if (inlfu) {
				lfuPart->put(key, val);
			}
		}
		bool get(Key key, Value &val) override {
			checkGhostCaches(key);
			bool shouldTransform = false;
			if (lruPart->get(key, val, shouldTransform)) {
				if (shouldTransform) {
					lfuPart->put(key, val);
				}
				return true;
			}
			return lfuPart->get(key, val);
		}
		Value get(Key key) override {
			Value val{};
			get(key, val);
			return val;
		}

	private:
		bool checkGhostCaches(Key key) {
			bool inghost = false;
			if (lruPart->checkGhost(key)) {
				if (lfuPart->decreaseCapacity()) {
					lruPart->increaseCapacity();
				}
				inghost = true;
			} else if (lfuPart->checkGhost(key)) {
				if (lruPart->decreaseCapacity()) {
					lfuPart->increaseCapacity();
				}
				inghost = true;
			}
			return inghost;
		};
	};
}
