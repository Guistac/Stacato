#pragma once

class Window : public std::enable_shared_from_this<Window>{
public:
	
	Window(std::string name_, bool padding = true) : name(name_), b_padding(padding){}
	
	std::string name;
	bool b_open = false;
	bool b_padding;
	
	void addToDictionnary();
	void removeFromDictionnary();
	
	void open();
	void close();
	void draw();
	
	virtual void drawContent() = 0;
	virtual void onOpen(){}
	virtual void onClose(){}
	
};

class Popup : public std::enable_shared_from_this<Popup>{
public:
	
	Popup(std::string name_, bool modal) : name(name_), b_modal(modal) {}
	
	std::string name;
	bool b_modal;
	bool b_open = false;
		
	void open();
	void close();
	virtual void draw();
	
	virtual void drawContent() = 0;
	virtual void onOpen(){}
	virtual void onClose(){}
	
};
