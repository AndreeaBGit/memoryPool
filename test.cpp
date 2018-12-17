#include <iostream>
#include <functional>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <vector>

#include "memPool.h"

using std::ref;
using std::string;
using std::thread;
using std::vector;

void threadFunction(MemoryPool<int>& memPool, int i) {
	int* elem = memPool.alloc();
	*elem = i;
	memPool.free(elem);
}

// threads access the pool one by one
TEST(MemoryPoolTest, threadSerial) {
	MemoryPool<int> memPool;
	int i = 1;
	thread t1(threadFunction, ref(memPool), i);
	t1.join();
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());

	i++;
	thread t2(threadFunction, ref(memPool), i);
	t2.join();
	
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// multiple threads access the poll at once 
TEST(MemoryPoolTest, threadParallel) {
	MemoryPool<int> memPool;
	int i = 1;
	thread t1(threadFunction, ref(memPool), i);
	i++;

	thread t2(threadFunction, ref(memPool), i);
	i++;
	thread t3(threadFunction, ref(memPool), i);

	t1.join();
	t2.join();
	t3.join();

	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// testing memory is freed for multiple memory access events 
TEST(MemoryPoolTest, manyThreads) {
	MemoryPool<int> memPool;
	vector<thread> threads;

	for (size_t index = 0; index < 200; index++) {
		threads.push_back(thread(threadFunction, ref(memPool), index));	
	}
	for (size_t index = 0; index < 200; index++) {
		threads[index].join();
	}
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// test address queue functionallity 
TEST(MemoryPoolTest, queueAccess) {
	MemoryPool<int> memPool;
	int* elem = memPool.alloc();
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize()-1);
	
	memPool.free(elem);	
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// memory pool with non primitive type
TEST(MemoryPoolTest, stringPool) {
	MemoryPool<string> memPool;
	string* elem = memPool.alloc();
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize()-1);
	*elem = "testing";
	
	memPool.free(elem);	
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// memory pool request when memory full 
TEST(MemoryPoolTest, memoryFull) {
	MemoryPool<int> memPool(2);

	int* elem1 = memPool.alloc();
	EXPECT_EQ(memPool.getAvailableCount(), 1);
	int* elem2 = memPool.alloc();
	EXPECT_EQ(memPool.getAvailableCount(), 0);
	try {
	 int*elem3 = memPool.alloc();
	}
	catch(const MemoryPoolException& except)  {
		EXPECT_EQ(except.what(), string("Failed to allocate. Memory pool depleted"));
	}

	memPool.free(elem1);
	memPool.free(elem2);
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// calling free on an element that is not part of the memory pool
TEST(MemoryPoolTest, freeOutsideElement) {
	MemoryPool<int> memPool;
	int* elem1 = memPool.alloc();
	int* elem2 = new int(5);

	try {
		memPool.free(elem2);
	}
	catch(const MemoryPoolException& except)  {
		EXPECT_EQ(except.what(), string("Unable to free. Element is not part of the memory"));
	}

	memPool.free(elem1);
	EXPECT_EQ(memPool.getAvailableCount(), memPool.getPoolSize());
}

// test object reinitialization
TEST(MemoryPoolTest, ObjectReinitialization) {
	MemoryPool<string> memPool(1);
	string* elem = memPool.alloc();
	*elem = "testing";

	memPool.free(elem);
	elem = memPool.alloc();
	EXPECT_NE(*elem, string("testing"));
}
