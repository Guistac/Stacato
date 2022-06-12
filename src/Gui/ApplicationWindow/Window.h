#pragma once

#define SINGLETON_GET_METHOD(ClassName) \
static std::shared_ptr<ClassName> get(){ \
	static std::shared_ptr<ClassName> singleton = std::make_shared<ClassName>(); \
	return singleton; \
}\

class Window : public std::enable_shared_from_this<Window>{
public:
	
	Window(std::string name_, bool padding = true) : name(name_), b_padding(padding){}
	
	std::string name;
	bool b_open;
	bool b_padding;
	
	void addToDictionnary();
	void removeFromDictionnary();
	
	void open();
	void close(){ b_open = false; }
	bool isOpen(){ return b_open; }
	
	void draw();
	
	virtual void drawContent() = 0;
	virtual void onOpen(){}
	virtual void onClose(){}
	
};

class Popup : public std::enable_shared_from_this<Popup>{
public:
	
	Popup(std::string name_, bool modal, bool canClose) : name(name_), b_modal(modal), b_canClose(canClose) {}
	
	std::string name;
	bool b_open;
	bool b_modal;
	bool b_canClose;
		
	void open();
	void close();
	void draw();
	
	virtual void drawContent() = 0;
	virtual void onOpen(){}
	virtual void onClose(){}
	virtual glm::vec2 getSize(){ return glm::vec2(.0f,.0f); }
	
};
