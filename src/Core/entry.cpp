#include <pch.h>

#include "Gui/ApplicationWindow/ApplicationWindow.h"
#include "Nodes/NodeFactory.h"
#include "Project/Project.h"
#include "Environnement/Environnement.h"

void test();

#ifdef STACATO_WIN32_APPLICATION
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
#else
int main(int argcount, const char ** args){
#endif
	
    //initialize application
	ApplicationWindow::init();
	
	test();
	
	//initialize node factory modules
	NodeFactory::load();
	
	//load environnement and plots, configure ethercat network interfaces
	Project::loadStartup();
	
	//load network interfaces, initialize networking, open ethercat network interface
	Environnement::initialize();
	
	//application gui runtime, function returns when application is quit
	ApplicationWindow::open(3500,1000);

	//stop hardware or simulation and terminate fieldbus
	Environnement::terminate();

	//terminate application
	ApplicationWindow::terminate();
}


enum class StructType{
	NONE,
	BOOLEAN,
	INTEGER,
	DOUBLE,
	SPECIAL
};

struct Special{
	glm::dvec4 a;
	glm::dvec4 b;
	glm::dvec4 c;
	glm::dvec4 d;
};

class AbstractStruct{
public:
	virtual StructType getType(){ return StructType::NONE; }
};

class BoolStruct : public AbstractStruct{
public:
	BoolStruct(bool val) : value(val){}
	virtual StructType getType(){ return StructType::BOOLEAN; }
	bool value;
};

class IntStruct : public AbstractStruct{
public:
	IntStruct(int val) : value(val){}
	virtual StructType getType(){ return StructType::INTEGER; }
	int value;
};

class DoubleStruct : public AbstractStruct{
public:
	DoubleStruct(double val) : value(val){}
	virtual StructType getType(){ return StructType::DOUBLE; }
	double value;
};

class SpecialStruct : public AbstractStruct{
public:
	SpecialStruct(Special val) : value(val){}
	virtual StructType getType(){ return StructType::SPECIAL; }
	Special value;
};


std::shared_ptr<AbstractStruct> getStruct(int i){
	if(i == 0) return std::make_shared<BoolStruct>(false);
	else if(i == 1) return std::make_shared<IntStruct>(333);
	else if(i == 2) return std::make_shared<DoubleStruct>(333.333);
	else {
		Special test = {
			.a = glm::dvec4(1, 2, 3, 4),
			.b = glm::dvec4(5, 6, 7, 8),
			.c = glm::dvec4(9, 10, 11, 12),
			.d = glm::dvec4(13, 14, 15, 16)
		};
		return std::make_shared<SpecialStruct>(test);
	}
}
	
void test(){
	
	std::shared_ptr<AbstractStruct> a = getStruct(0);
	std::shared_ptr<AbstractStruct> b = getStruct(1);
	std::shared_ptr<AbstractStruct> c = getStruct(2);
	std::shared_ptr<AbstractStruct> d = getStruct(3);
	
	
}
