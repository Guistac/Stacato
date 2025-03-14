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
	std::string getClassName() override {								\
		return #Typename;												\
	}																	\
private:																\
	virtual Ptr<Component> instanciateComponent() override{				\
		return make();													\
	}																	\


template<typename T>
using Ptr = std::shared_ptr<T>;
namespace tinyxml2{ class XMLElement; }

namespace Legato{

	class Component : public std::enable_shared_from_this<Component>{
	public:
		
		void addChild(Ptr<Component> child){
			child->parentComponent = shared_from_this();
			childComponents.push_back(child);
		}
		
		virtual bool serialize();
		virtual bool deserialize();
		
		bool setIdentifier(std::string input);
		
		template<typename T>
		Ptr<T> cast(){ return std::dynamic_pointer_cast<T>(shared_from_this()); }
		
	protected:
		Ptr<Component> duplicateComponent();
		virtual void onConstruction(){}
		virtual void copyFrom(Ptr<Component> source){}
		virtual bool onSerialization(){ return true; }
		virtual bool onDeserialization(){ return true; }
		virtual void onPostLoad(){ return true; }
		
	private:
		virtual Ptr<Component> instanciateComponent() = 0;
		virtual std::string getClassName() = 0;
		
		Ptr<Component> parentComponent = nullptr;
		std::vector<Ptr<Component>> childComponents = {};
		
		tinyxml2::XMLElement* xmlElement = nullptr;
		std::string identifier = "DefaultIdentifier";
		bool b_hasValidIdentifier = false;
	};

}

