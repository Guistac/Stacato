#pragma once

#include "Legato/Editor/Serializable.h"

struct ImGuiWindow;

#define SINGLETON_GET_METHOD(ClassName) \
static std::shared_ptr<ClassName> get(){ \
	static std::shared_ptr<ClassName> singleton = std::make_shared<ClassName>(); \
	return singleton; \
}\

namespace Legato{

	class Window : public std::enable_shared_from_this<Window>{
	public:
		
		Window(std::string name_, bool padding = true) : name(name_), b_hasPadding(padding){}
		
		void open();
		void close();
		void focus();
		void draw();
		
		bool isOpen();
		bool isFocused();
		
		void setName(std::string name_){ name = name_; }
		void setPadding(float padding = -1.0);
		
		std::string& getName(){ return name; }
		
		virtual void onOpen(){}
		virtual void onDraw() = 0;
		virtual void onClose(){}
		
		bool b_isOpen = false;
		bool b_isFocused = false;
		
		ImGuiWindow* imguiWindow = nullptr;
		
	private:
		std::string name;
		
		bool b_hasPadding = true;
	};

	class Popup : public std::enable_shared_from_this<Popup>{
	public:
		
		Popup(std::string name_, bool modal, bool canClose) : name(name_), b_isModal(modal), b_canClose(canClose) {}
		
		std::string name;
		bool b_isOpen = false;
		bool b_isModal = false;
		bool b_canClose = false;
		
		void open();
		void close();
		void draw();
		
		virtual void onOpen(){}
		virtual void onDraw() = 0;
		virtual void onClose(){}
		virtual glm::vec2 getSize(){ return glm::vec2(.0f,.0f); }
		
	};

}



/*
#define SINGLETON_GET_METHOD(ClassName) \
static std::shared_ptr<ClassName> get(){ \
	static std::shared_ptr<ClassName> singleton = std::make_shared<ClassName>(); \
	return singleton; \
}\

struct ImGuiWindow;

class Window : public std::enable_shared_from_this<Window>{
public:
	
	Window(std::string name_, bool padding = true) : name(name_), b_padding(padding){}
	
	std::string name;
	bool b_open = false;
	bool b_padding;
	
	ImGuiWindow* imguiWindow = nullptr;
	
	void addToDictionnary();
	void removeFromDictionnary();
	
	void open();
	void close();
	bool isOpen(){ return b_open; }
	
	void focus();
	bool isFocused();
	
	void draw();
	
	virtual void drawContent() = 0;
	
};

class Popup : public std::enable_shared_from_this<Popup>{
public:
	
	Popup(std::string name_, bool modal, bool canClose) : name(name_), b_modal(modal), b_canClose(canClose) {}
	
	std::string name;
	bool b_open = false;
	bool b_modal = false;
	bool b_canClose = false;
		
	void open();
	void close();
	void draw();
	
	virtual void drawContent() = 0;
	virtual void onPopupOpen(){}
	virtual glm::vec2 getSize(){ return glm::vec2(.0f,.0f); }
	
};

*/

/*

namespace WindowManager{

	std::vector<std::shared_ptr<Window>>& getWindowDictionnary();
	bool isInDictionnary(std::shared_ptr<Window> window);
	void addWindowToDictionnary(std::shared_ptr<Window> window);
	void removeWindowFromDictionnary(std::shared_ptr<Window> window);

	std::vector<std::shared_ptr<Window>>& getOpenWindows();
	void openWindow(std::shared_ptr<Window> window);
	void closeWindow(std::shared_ptr<Window> window);
	void closeAllWindows();

	void focusWindow(std::shared_ptr<Window> window);

	std::vector<std::shared_ptr<Popup>>& getOpenPopups();
	void openPopup(std::shared_ptr<Popup> popup);
	void closePopup(std::shared_ptr<Popup> popup);

	void manage();

};

*/
