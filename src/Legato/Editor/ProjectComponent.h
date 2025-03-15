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

	bool sanitizeXmlIdentifier(const std::string input, std::string& output);

	class Component : public std::enable_shared_from_this<Component>{
	public:
		
		template<typename T>
		friend class ListComponent;
		
		virtual void addChild(Ptr<Component> child){
			if(child == nullptr) return;
			child->parentComponent = shared_from_this();
			childComponents.push_back(child);
		}
		
		void setIdentifier(std::string input);
		virtual bool serialize();
		virtual bool deserialize();
		
		template<typename T>
		Ptr<T> cast(){ return std::dynamic_pointer_cast<T>(shared_from_this()); }
		
	protected:
		virtual std::string getClassName() = 0;
		Ptr<Component> duplicateComponent();
		virtual void onConstruction(){}
		virtual void copyFrom(Ptr<Component> source){}
		virtual bool onSerialization(){ return true; }
		virtual bool onDeserialization(){ return true; }
		virtual void onPostLoad(){ return true; }
		
		bool serializeAttribute(const std::string attributeSaveString, int data);
		bool serializeAttribute(const std::string attributeSaveString, double data);
		bool serializeAttribute(const std::string attributeSaveString, const std::string& data);
		bool deserializeAttribute(const std::string idString, int& data);
		bool deserializeAttribute(const std::string idString, double& data);
		bool deserializeAttribute(const std::string idString, std::string& data);
		
		tinyxml2::XMLElement* xmlElement = nullptr;
		std::string identifier = "DefaultIdentifier";
		bool b_hasValidIdentifier = false;
		
	private:
		virtual Ptr<Component> instanciateComponent() = 0;
		
		Ptr<Component> parentComponent = nullptr;
		std::vector<Ptr<Component>> childComponents = {};
		
		bool checkAttributeSerializationError(std::string& ID);
		bool checkAttributeDeserializationError(std::string& ID);
		bool checkAttributeDeserializationResult(int result, std::string& ID);
		
	};

}
