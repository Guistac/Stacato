#pragma once

#include "Parameter.h"

template<typename T>
class ListParameter : public SerializableList<T>{
public:
	void insertElement(std::shared_ptr<T> element, int index = -1){}
	void removeElement(std::shared_ptr<T> element){}
	void moveElement(int oldIndex, int newIndex){}
};
