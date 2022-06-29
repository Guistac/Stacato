#pragma once

#include "CommandHistory.h"

template <typename T>
class List : public std::enable_shared_from_this<List<T>>{
public:
	
	List(std::vector<T> content){
		elements = content;
	}
	std::vector<T>& get(){
		return elements;
	}
	
	int removedIndex;
	int movedUpIndex;
	int movedDownIndex;
	
	bool beginList(){
		removedIndex = -1;
		movedUpIndex = -1;
		movedDownIndex = -1;
		return true;
	}
	
	void endList(){
		if(removedIndex != -1) removeElement(removedIndex);
		if(movedUpIndex != -1) moveElement(movedUpIndex, movedUpIndex - 1);
		if(movedDownIndex != -1) moveElement(movedDownIndex, movedDownIndex + 1);
	}
	
	size_t size(){
		return elements.size();
	}
	
	
	T& beginElement(int index, bool removeButton = false, bool reorderButtons = false){
		ImGui::PushID(index);
		if(removeButton){
			if(ImGui::Button("X")) removedIndex = index;
			if(reorderButtons) ImGui::SameLine();
		}
		if(reorderButtons){
			ImGui::BeginDisabled(index == 0);
			if(ImGui::Button("Up")) movedUpIndex = index;
			ImGui::EndDisabled();
			ImGui::SameLine();
			ImGui::BeginDisabled(index == elements.size() - 1);
			if(ImGui::Button("Down")) movedDownIndex = index;
			ImGui::EndDisabled();
		}
		return elements[index];
	}
	
	void endElement(){
		ImGui::PopID();
	}
	
	void addElement(T element, int index = -1){
		int idx = index;
		if(index < 0 || index >= elements.size()) idx = elements.size();
		auto thisList = this->shared_from_this();
		auto command = std::make_shared<AddElementCommand>(element, thisList, idx);
		command->execute();
	}
	
	void removeElement(int index){
		if(index < 0 || index >= elements.size()) return;
		T element = elements[index];
		auto thisList = this->shared_from_this();
		std::make_shared<RemoveElementCommand>(element, thisList, index)->execute();
	}
	
	void moveElement(int oldIndex, int newIndex){
		if(oldIndex < 0 || oldIndex >= elements.size()) return;
		if(newIndex < 0 || newIndex >= elements.size()) return;
		auto thisList = this->shared_from_this();
		std::make_shared<MoveElementCommand>(thisList, oldIndex, newIndex)->execute();
	}
	
	
	
	
private:
	std::vector<T> elements;
	
	
	class AddElementCommand : public UndoableCommand{
	public:
		T element;
		std::shared_ptr<List> list;
		int index;
		
		AddElementCommand(T element_, std::shared_ptr<List> list_, int index_) :
		UndoableCommand("Added Element at index " + std::to_string(index)){
			element = element_;
			list = list_;
			index = index_;
		}
		
		virtual void onExecute(){
			list->elements.insert(list->elements.begin() + index, element);
		}
		
		virtual void onUndo(){
			list->elements.erase(list->elements.begin() + index);
		}
	};

	
	class RemoveElementCommand : public UndoableCommand{
	public:
		T element;
		std::shared_ptr<List> list;
		int index;
		
		RemoveElementCommand(T element_, std::shared_ptr<List> list_, int index_) :
		UndoableCommand("Removed Element at index " + std::to_string(index)){
			element = element_;
			list = list_;
			index = index_;
		}
		
		virtual void onExecute(){
			list->elements.erase(list->elements.begin() + index);
		}
		
		virtual void onUndo(){
			list->elements.insert(list->elements.begin() + index, element);
		}
	};
	
	class MoveElementCommand : public UndoableCommand{
	public:
		std::shared_ptr<List> list;
		int oldIndex;
		int newIndex;
		
		MoveElementCommand(std::shared_ptr<List> list_, int oldIndex_, int newIndex_) :
		UndoableCommand("Moved Element at index " + std::to_string(oldIndex) + " to index " + std::to_string(newIndex)){
			list = list_;
			oldIndex = oldIndex_;
			newIndex = newIndex_;
		}
		
		virtual void onExecute(){
			T tmp = list->elements[oldIndex];
			list->elements.erase(list->elements.begin() + oldIndex);
			list->elements.insert(list->elements.begin() + newIndex, tmp);
		}
		
		virtual void onUndo(){
			T tmp = list->elements[newIndex];
			list->elements.erase(list->elements.begin() + newIndex);
			list->elements.insert(list->elements.begin() + oldIndex, tmp);
		}
	};
	
};
