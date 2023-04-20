#pragma once

#include "SharedObject.h"

/*————————————————————————————————————————————————————————————————
 
DESCRIPTION
 
 A base for any object that is managed through a shared pointer and can be duplicated
 
USAGE
 
 Create instances of the implemented type using createInstance()
 Create copies of instances using duplicate()
 
IMPLEMENTATION
 
 each class implementation derived from Prototype has to implement the following
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
	void copyFrom(std::shared_ptr<Prototype> original)
		In this method the argument is cast to the implemented type using std::static_pointer_cast
		Then all fields are copied from the original
		When nesting subclasses, the method should call ParentClass::copyFrom()

IMPLEMENTATION EXAMPLE
 
	class PrototypeImplementation : public Prototype{
		DECLARE_PROTOTYPE_BASE_METHODS(PrototypeImplementation)
 
	protected:
 
		virtual void onConstruction() override {
			ParentClass::onConstruction(); //optional
			field = 1238;
		}
 
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) override {
			ParentClass::copyFrom(source); //optional
			auto original = std::static_pointer_cast<PrototypeImplementation>(source);
			field = field->number;
		}
 
	private:
		int field;
	};

 ————————————————————————————————————————————————————————————————*/


#define DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Typename) \
	DECLARE_SHARED_CONTRUCTION(Typename)\
private:\
	std::shared_ptr<Prototype> createPrototypeInstance() override{\
		return Typename::createInstance();\
	};\

namespace Legato{

	class Prototype : public SharedObject<Prototype>{
	public:
		
		template<class ClassName>
		std::shared_ptr<ClassName> duplicate(){
			std::shared_ptr<Prototype> prototypeCopy = duplicatePrototype();
			return prototypeCopy->downcasted_shared_from_this<ClassName>();
		}
		
		virtual void onCopyFrom(std::shared_ptr<Prototype> source) {}
		
	private:
		
		std::shared_ptr<Prototype> duplicatePrototype(){
			std::shared_ptr<Prototype> copy = createPrototypeInstance();
			copy->onCopyFrom(shared_from_this());
			return copy;
		}
		
		virtual std::shared_ptr<Prototype> createPrototypeInstance() = 0;
	};

}
