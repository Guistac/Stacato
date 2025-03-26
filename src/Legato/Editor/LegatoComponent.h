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



#define COMPONENT_INTERFACE(Typename)		\
COMPONENT_INTERFACE_NOADDCHILD(Typename)	\
public:										\
	using Component::addChild;				\



#define COMPONENT_IMPLEMENTATION(Typename)		\
COMPONENT_IMPLEMENTATION_NOADDCHILD(Typename)	\
public:											\
	using Component::addChild;					\



template<typename T>
using Ptr = std::shared_ptr<T>;


//forward declaration
namespace tinyxml2{ class XMLElement; }



namespace Legato{


	bool sanitizeXmlIdentifier(const std::string input, std::string& output);


	class File;
	class Directory;
	class Project;


	class Component : public std::enable_shared_from_this<Component>{
	public:
		
		template<typename T>
		friend class List;
		friend class File;
		friend class Directory;
		friend class Project;
		
		const std::vector<Ptr<Component>>& getChildren(){ return childComponents; }
		bool hasChildren(){ return !childComponents.empty(); }
		
		void setIdentifier(std::string input);
		std::string getIdentifier(){ return identifier; }
		virtual bool serialize();
		virtual bool deserialize();
		
		template<typename T>
		Ptr<T> cast(){ return std::dynamic_pointer_cast<T>(shared_from_this()); }
		
		virtual std::string getClassName() = 0;
		
	protected:
		Ptr<Component> duplicateComponent();
		virtual void onConstruction();
		virtual void copyFrom(Ptr<Component> source){}
		virtual bool onSerialization(){ return true; }
		virtual bool onDeserialization(){ return true; }
		virtual void onPostLoad(){ return true; }
		
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
		
		void addChildDependencies(Ptr<Component> child);
		
		tinyxml2::XMLElement* xmlElement = nullptr;
		std::string identifier = "DefaultIdentifier";
		std::string name = "Default Name";
		Ptr<Component> parentComponent = nullptr;
		std::vector<Ptr<Component>> childComponents = {};
		Ptr<Project> parentProject = nullptr;
		Ptr<Directory> parentDirectory = nullptr;
		Ptr<File> parentFile = nullptr;
		
		virtual void addChild(Ptr<Component> child);
		
	private:
		virtual Ptr<Component> instanciateComponent() = 0;
		
		bool checkAttributeSerializationError(std::string& ID);
		bool checkAttributeDeserializationError(std::string& ID);
		bool checkAttributeDeserializationResult(int result, std::string& ID);
		
	};



	class File : public Component{
		COMPONENT_INTERFACE(File)
	public:
		
		void onConstruction() override;
		void setFileName(std::filesystem::path input);
		
		virtual bool serialize() override;
		virtual bool deserialize() override;
		
		std::filesystem::path getFileName(){ return fileName; }
		std::filesystem::path getPath();
		
	private:
		std::filesystem::path fileName;
	};



	class Directory : public Component{
		COMPONENT_IMPLEMENTATION(Directory)
		friend class Project;
	public:
		static Ptr<Directory> make(std::filesystem::path directoryName_){
			auto newInstance = make();
			newInstance->setPath(directoryName_);
			return newInstance;
		}
		void setPath(std::filesystem::path input);
		std::filesystem::path getDirectoryName(){ return directoryName; }
		std::filesystem::path getPath();
	protected:
		virtual bool serialize() override;
		virtual bool deserialize() override;
	private:
		std::filesystem::path directoryName;
	};



	class Project : public Directory{
		COMPONENT_INTERFACE(Project)
	public:
		virtual bool serialize() override;
		virtual bool deserialize() override;
	};

}
