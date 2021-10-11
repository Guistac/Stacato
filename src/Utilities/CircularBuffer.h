#pragma once

#include <vector>
#include "glm/glm.hpp"

struct CircularBuffer {
public:

	CircularBuffer(size_t s) {
		setMaxSize(s);
	}

	~CircularBuffer() {
		delete[] data;
	}

	void setMaxSize(size_t s) {
		mutex.lock();
		if (data == nullptr) delete[] data;
		s_maxSize = s;
		s_size = 0;
		s_capacity = s * 2;
		data = new glm::vec2[s_capacity];
		s_newest = -1;
		mutex.unlock();
	}

	void addPoint(const glm::vec2& p) {
		size_t newIndex = (s_newest + 1) % s_capacity;
		mutex.lock();
		data[newIndex] = p;
		data[(newIndex + s_maxSize) % s_capacity] = p;
		if (s_size < s_maxSize) s_size++;
		s_newest = newIndex;
		mutex.unlock();
	}

	void clear() {
		mutex.lock();
		s_size = 0;
		s_newest = -1;
		mutex.unlock();
	}

	size_t getBuffer(glm::vec2** output) {
		mutex.lock();
		if (s_size < s_maxSize) *output = data;
		else *output = &data[(s_newest + 1) % s_maxSize];
		mutex.unlock();
		return s_size;
	}

	size_t size() { return s_size; }

	size_t maxSize() { return s_maxSize; }

	bool empty() { return s_size == 0; }

	size_t stride() { return sizeof(glm::vec2); }

private:

	glm::vec2* data = nullptr;
	size_t s_size;		//number of elements currently stored
	size_t s_maxSize;	//maxnumber of elements stored
	size_t s_capacity;	//size of the buffer in elements (2*s_maxsize)
	size_t s_newest;

	std::mutex mutex;
};