#pragma once

struct ImGuiWindow;

class NewWindow : public std::enable_shared_from_this<NewWindow>{
public:
	
	void open();
	void close();
	void focus();
	void draw();
	
	bool isOpen();
	bool isFocused();
	
	void setName(std::string name_){ name = name_; }
	void setPadding(float padding = -1.0);
	
	virtual void onOpen(){}
	virtual void onDraw() = 0;
	virtual void onClose(){}
	
	bool b_isOpen = false;
	bool b_isFocused = false;
	
private:
	std::string name;
	ImGuiWindow* imguiWindow = nullptr;
	
	bool b_hasPadding = true;
};
