#include <queue>
#include <mutex>
#include <new>
#include <stdexcept>

using std::bad_alloc;
using std::queue;
using std::lock_guard;
using std::mutex;
using std::runtime_error;
using std::out_of_range;
using std::size_t;

class MemoryPoolException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// default no of obejctes the memory pool can contain
static const size_t DEFAULT_POOL_SIZE = 256;

template <class ELEM_TYPE>
class MemoryPool { 
public:
	MemoryPool(size_t poolSize = DEFAULT_POOL_SIZE);
	MemoryPool(const MemoryPool<ELEM_TYPE>&) = delete;
	const MemoryPool& operator=(const MemoryPool<ELEM_TYPE>&) = delete;
	~MemoryPool();
	ELEM_TYPE* alloc(); 
	void free(ELEM_TYPE*);
	size_t getPoolSize() const;
	size_t getAvailableCount() const;

private:
	ELEM_TYPE* memPool_;
	size_t poolSize_;
	
	queue<ELEM_TYPE*> memQueue_;	
	mutable mutex queueMutex_;
};


template <class ELEM_TYPE>
MemoryPool<ELEM_TYPE>::MemoryPool(size_t poolSize) : memPool_(nullptr), queueMutex_(),
										  poolSize_(poolSize) {
	// new operator will throw a bad_alloc exception if it's unable to allocate memory										  	
	memPool_ = new ELEM_TYPE[poolSize];
	for (size_t i =0; i < poolSize; i++) {
		memQueue_.push(&memPool_[i]);
	}
}

template <class ELEM_TYPE>
MemoryPool<ELEM_TYPE>::~MemoryPool() {
	delete [] memPool_;
	memPool_ = nullptr;
}

template <class ELEM_TYPE>
ELEM_TYPE* MemoryPool<ELEM_TYPE>::alloc() {
	lock_guard<mutex> lock(queueMutex_);
	if (!memQueue_.size()) {
		throw MemoryPoolException("Failed to allocate. Memory pool depleted");
	}
	ELEM_TYPE* elem = memQueue_.front();
	memQueue_.pop();

	return elem;
}

template <class ELEM_TYPE>
void MemoryPool<ELEM_TYPE>::free(ELEM_TYPE* elem) {
	if (elem < &memPool_[0] || elem > &memPool_[poolSize_-1]) {
		throw MemoryPoolException("Unable to free. Element is not part of the memory");
	}
	lock_guard<mutex> lock(queueMutex_);
	if (elem != nullptr) {
		// reinitialize element
		elem->~ELEM_TYPE();
		*elem = std::move(ELEM_TYPE());
		memQueue_.push(elem);		
	}
}

template <class ELEM_TYPE>
size_t MemoryPool<ELEM_TYPE>::getPoolSize() const {
	return poolSize_;
}

template <class ELEM_TYPE>
size_t MemoryPool<ELEM_TYPE>::getAvailableCount() const {
	lock_guard<mutex> lock(queueMutex_);
	return memQueue_.size();
}
