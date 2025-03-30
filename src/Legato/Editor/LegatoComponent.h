#pragma once


#define COMPONENT_INTERFACE_NOADDCHILD(Typename)							\
public:																		\
	Ptr<Typename> duplicate(){												\
		Ptr<Component> componentCopy = duplicateComponent();				\
		return std::static_pointer_cast<Typename>(componentCopy);			\
	}																		\
	static std::string getStaticClassName(){ return #Typename; }			\
protected:																	\
	Typename() = default;													\
	


#define COMPONENT_IMPLEMENTATION_NOADDCHILD(Typename)							\
COMPONENT_INTERFACE_NOADDCHILD(Typename)										\
public:																			\
	static Ptr<Typename> make(){												\
		auto newInstance = Ptr<Typename>(new Typename());						\
		newInstance->onConstruction();											\
		return newInstance;														\
	}																			\
	std::string getClassName() override { return #Typename; }					\
private:																		\
	virtual Ptr<Component> instanciateComponent() override{	return make(); }	\



//add this to each user created component class that is an uninstanciable interface
#define COMPONENT_INTERFACE(Typename)		\
COMPONENT_INTERFACE_NOADDCHILD(Typename)	\
public:										\
	using Component::addChild;				\


//add this to each user create component derived class that is a final implementation
#define COMPONENT_IMPLEMENTATION(Typename)		\
COMPONENT_IMPLEMENTATION_NOADDCHILD(Typename)	\
public:											\
	using Component::addChild;					\



//This makes stuff less verbose
template<typename T>
using Ptr = std::shared_ptr<T>;

//forward declaration to avoid including tinyxml2 in a header file
namespace tinyxml2{ class XMLElement; }



namespace Legato{


	bool sanitizeXmlIdentifier(const std::string input, std::string& output);


	class File;
	class Directory;
	class Project;


	class Component : public std::enable_shared_from_this<Component>{
	public:
		
		template<typename T> //the template applies to class List<T> only
		friend class ListComponent;
		friend class File;
		friend class Directory;
		friend class Project;
		
		const std::vector<Ptr<Component>>& getChildren(){ return childComponents; }
		bool hasChild(Ptr<Component> input);
		bool hasChildren(){ return !childComponents.empty(); }
		
		void setIdentifier(std::string input);
		std::string getIdentifier(){ return identifier; }
		
		std::string getIdentifierPath();
		
		virtual bool serialize();
		virtual bool deserialize();
		
		template<typename T>
		Ptr<T> cast(){ return std::dynamic_pointer_cast<T>(shared_from_this()); }
		
		virtual std::string getClassName() = 0;
		
	protected:
		
		virtual void onConstruction();
		virtual void copyFrom(Ptr<Component> source){
			setIdentifier(source->identifier);
		}
		virtual bool onSerialization(){ 	return true; }
		virtual bool onDeserialization(){ 	return true; }
		virtual bool onPostLoad(){ 			return true; }
		
		bool serializeBoolAttribute(const std::string idString, bool data);
		bool serializeIntAttribute(const std::string idString, int data);
		bool serializeLongAttribute(const std::string idString, long long data);
		bool serializeDoubleAttribute(const std::string idString, double data);
		bool serializeStringAttribute(const std::string idString, const std::string& data);
		bool deserializeBoolAttribute(const std::string idString, bool& data);
		bool deserializeIntAttribute(const std::string idString, int& data);
		bool deserializeLongAttribute(const std::string idString, long long& data);
		bool deserializeDoubleAttribute(const std::string idString, double& data);
		bool deserializeStringAttribute(const std::string idString, std::string& data);
		bool checkAttributeSerializationError(std::string& ID);
		bool checkAttributeDeserializationError(std::string& ID);
		bool checkAttributeDeserializationResult(int result, std::string& ID);
		
		std::string identifier = "DefaultIdentifier";
		tinyxml2::XMLElement* xmlElement = nullptr;
		
		virtual Ptr<Component> addChild(Ptr<Component> child);
		void addChildDependencies(Ptr<Component> child);
		std::vector<Ptr<Component>> childComponents = {};
		
		Ptr<Component> parentComponent = nullptr; //use to serialize and deserialize
		Ptr<File> parentFile = nullptr; //used to construct complete paths
		Ptr<Project> parentProject = nullptr; //used to execute undaoble actions
		
		virtual Ptr<Component> instanciateComponent() = 0;
		Ptr<Component> duplicateComponent();
		
	};

}
