#pragma once

#include <vector>
#include "glm/glm.hpp"

struct ScrollingBuffer {
public:

	void setMaxSize(size_t s) {
		data.reserve(s);
		s_maxSize = s;
	}

	void addPoint(glm::vec2 p) {
		if (data.size() < s_maxSize) {
			data.push_back(p);
		}
		else {
			data[s_offset] = p;
			s_offset = (s_offset + 1) % s_maxSize;
		}
	}

	void clear() {
		data.clear();
		s_offset = 0;
	}

	glm::vec2& front() {
		if (data.empty()) {
			static glm::vec2 dummy;
			return dummy;
		}
		return data.front();
	}

	glm::vec2& newest() {
		if (data.empty()) {
			static glm::vec2 dummy;
			return dummy;
		}
		if (s_offset == 0) return data.back();
		return data[s_offset - 1];
	}

	glm::vec2& oldest() {
		if (data.empty()) {
			static glm::vec2 dummy;
			return dummy;
		}
		if (s_offset == 0) return data.front();
		return data[s_offset];
	}

	size_t size() { return data.size(); }

	size_t maxSize() { return s_maxSize; }

	bool empty() { return data.empty(); }

	size_t offset() { return s_offset; }

	size_t stride() { return sizeof(glm::vec2); }

private:

	std::vector<glm::vec2> data;
	size_t s_maxSize;
	size_t s_offset = 0;
};
