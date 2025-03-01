#pragma once

#define COMPONENT_INTERFACE(Typename)									\
public:																	\
	Ptr<Typename> duplicate(){											\
		Ptr<Component> componentCopy = duplicateComponent();			\
		return std::static_pointer_cast<Typename>(componentCopy);		\
	}																	\
protected:																\
	Typename() = default;												\



#define COMPONENT_IMPLEMENTATION(Typename)								\
COMPONENT_INTERFACE(Typename)											\
public:																	\
	static Ptr<Typename> make(){										\
		auto newInstance = Ptr<Typename>(new Typename());				\
		newInstance->onConstruction();									\
		return newInstance;												\
	}																	\
private:																\
	virtual Ptr<Component> instanciateComponent() override{				\
		return make();													\
	}																	\



template<typename T>
using Ptr = std::shared_ptr<T>;

#include <tinyxml2.h>

namespace Test{

	class Component : public std::enable_shared_from_this<Component>{
	public:
		
		void addChildComponent(Ptr<Component> child){
			child->parentComponent = shared_from_this();
			childComponents.push_back(child);
		}
		
		virtual bool serialize(){
			xmlElement = parentComponent->xmlElement->InsertNewChildElement(saveString.c_str());
			onSerialization();
			for(auto child : childComponents) child->serialize();
			return true;
		}
		virtual bool deserialize(){
			xmlElement = parentComponent->xmlElement->FirstChildElement(saveString.c_str());
			//this deserializes and creates all necessary ChildComponents
			onDeserialization();
			//this deserializes all child components
			for(auto child : childComponents) child->deserialize();
			//this gets called after all children are deserialized,
			//in case the Component needs to initialize stuff after children have loaded
			onPostLoad();
			return true;
		}
		
		template<typename T>
		Ptr<T> cast(){ return std::dynamic_pointer_cast<T>(shared_from_this()); }
		
	protected:
		Ptr<Component> duplicateComponent(){
			//this makes a new instance of the component, with data at defaults
			Ptr<Component> copy = instanciateComponent();
			//this makes sure the copy has all the same ChildComponents as the original
			//this also copies fields that are not ChildComponents
			copy->copyFrom(shared_from_this());
			//now we make all ChildComponents copy the data of the original ChildComponents
			for(int i = 0; i < childComponents.size(); i++){
				//we probably need a better lookup mecanism instead of relying on the fact that the ChildComponents will be in the same order as the original
				copy->childComponents[i]->copyFrom(childComponents[i]);
			}
			//this gets called after all children are duplicated,
			//in case the Component needs to initialize stuff after children have finished setting up
			onPostLoad();
			//in theory we now have a deep copy of the original
			return copy;
		}
		virtual void onConstruction() {}
		virtual void copyFrom(Ptr<Component> source) {}
		virtual bool onSerialization(){ return true; }
		virtual bool onDeserialization(){ return true; }
		virtual void onPostLoad(){ return true; }
	private:
		virtual Ptr<Component> instanciateComponent() = 0;
		Ptr<Component> parentComponent = nullptr;
		std::vector<Ptr<Component>> childComponents = {};
		tinyxml2::XMLElement* xmlElement = nullptr;
		std::string saveString;
	};

}

//what are child components?
//parameters
//lists of other stuff

