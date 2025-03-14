#pragma once

/*————————————————————————————————————————————————————————————————
 
DESCRIPTION
 
 A base for any object that is managed through a shared pointer and can be duplicated
 
USAGE
 
 Create instances of the implemented type using createInstance()
 Create copies of instances using duplicate()
 
IMPLEMENTATION
 
 each class implementation derived from PrototypeBase has to implement the following
 -One of two macros:
	DECLARE_PROTOTYPE_IMPLENTATION_METHODS()
		Only use in a completely implemented type, no virtual classes and interfaces
		Declares the basic methods which allow duplication and force shared_ptr instancing
	DECLARE_PROTOTYPE_INTERFACE_METHODS()
		Only use in incomplete interface types
		Allows duplication at the interface level
 
 -Optional protected virtual methods:
	void onConstruction()
		In this method the object can initialize itself
		When nesting subclasses, the method should call ParentClass::onConstruction()
	void copyFrom(std::shared_ptr<PrototypeBase> original)
		In this method the argument is cast to the implemented type using std::static_pointer_cast
		Then all fields are copied from the original
		When nesting subclasses, the method should call ParentClass::copyFrom()

IMPLEMENTATION EXAMPLE
 
	class PrototypeImplementation : public PrototypeBase{
		DECLARE_PROTOTYPE_BASE_METHODS(PrototypeImplementation)
 
	protected:
 
		virtual void onConstruction() override {
			ParentClass::onConstruction(); //optional
			field = 1238;
		}
 
		virtual void copyFrom(std::shared_ptr<PrototypeBase> source) override {
			ParentClass::copyFrom(source); //optional
			auto original = std::static_pointer_cast<PrototypeImplementation>(source);
			field = field->number;
		}
 
	private:
		int field;
	};

 ————————————————————————————————————————————————————————————————*/

#define DECLARE_PROTOTYPE_INTERFACE_METHODS(Typename)\
public:\
	std::shared_ptr<Typename> duplicate(){\
		return std::static_pointer_cast<Typename>(duplicatePrototype());\
	}\
protected:\
	Typename(){}\



#define DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Typename) \
public:\
	static std::shared_ptr<Typename> createInstance(){\
		std::shared_ptr<Typename> newInstance = std::shared_ptr<Typename>(new Typename());\
		newInstance->onConstruction();\
		return newInstance;\
	}\
	std::shared_ptr<Typename> duplicate(){\
		return std::static_pointer_cast<Typename>(duplicatePrototype());\
	}\
protected:\
Typename(){}\
private:\
	std::shared_ptr<PrototypeBase> createPrototypeInstance_private() override{\
		std::shared_ptr<Typename> newPrototypeInstance = std::shared_ptr<Typename>(new Typename());\
		newPrototypeInstance->onConstruction();\
		return newPrototypeInstance;\
	};\


class PrototypeBase : public std::enable_shared_from_this<PrototypeBase>{
protected:
	
	std::shared_ptr<PrototypeBase> duplicatePrototype(){
		std::shared_ptr<PrototypeBase> copy = createPrototypeInstance_private();
		copy->onCopyFrom(shared_from_this());
		return copy;
	}
	
	virtual void onConstruction() {}
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) {}
	
private:
	virtual std::shared_ptr<PrototypeBase> createPrototypeInstance_private() = 0;
};


#include "ProjectComponent.h"

class InterfaceThing : public Legato::Component{
	COMPONENT_INTERFACE(InterfaceThing)
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	virtual void copyFrom(Ptr<Component> source) override {
		Component::copyFrom(source);
	}
	virtual bool onSerialization() override {
		Component::onSerialization();
		return true;
	}
	virtual bool onDeserialization() override {
		Component::onDeserialization();
		return true;
	}
};

 
class ChildThing : public Legato::Component{
	COMPONENT_IMPLEMENTATION(ChildThing)
	virtual void onConstruction() override {
		Component::onConstruction();
	}
	virtual void copyFrom(Ptr<Component> source) override {
		Component::copyFrom(source);
		auto src = source->cast<ChildThing>();
		value = src->value;
	}
	virtual bool onSerialization() override {
		Component::onSerialization();
		return true;
	}
	virtual bool onDeserialization() override {
		Component::onDeserialization();
		return true;
	}
public:
	int value = 0;
};


class RealThing : public InterfaceThing{
	COMPONENT_IMPLEMENTATION(RealThing)
	
	virtual void onConstruction() override {
		InterfaceThing::onConstruction();
		childThing = ChildThing::make();
		addChild(childThing);
	}
	virtual void copyFrom(Ptr<Component> source) override {
		InterfaceThing::copyFrom(source);
	}
	virtual bool onSerialization() override {
		return InterfaceThing::onSerialization();
	}
	virtual bool onDeserialization() override {
		return InterfaceThing::onDeserialization();
	}
public:
	Ptr<ChildThing> childThing;
	
};
