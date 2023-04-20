#pragma once


#define DECLARE_SHARED_CONTRUCTION(Typename) \
public:\
	static std::shared_ptr<Typename> createInstance(){\
		std::shared_ptr<Typename> newInstance = std::shared_ptr<Typename>(new Typename());\
		newInstance->onConstruction();\
		return newInstance;\
	}\
protected:\
	Typename(){}\


namespace Legato{

	//First a common base class
	//of course, one should always virtually inherit from it.
	class BaseSharedObject : public std::enable_shared_from_this<BaseSharedObject>{
	public:
		virtual ~BaseSharedObject(){}
	};

	template<class T>
	class SharedObject : virtual public BaseSharedObject{
	public:
		std::shared_ptr<T> shared_from_this(){
			return std::dynamic_pointer_cast<T>(BaseSharedObject::shared_from_this());
		}
		//Utility method to easily downcast.
		//Useful when a child doesn't inherit directly from enable_shared_from_this
		//but wants to use the feature.
		template <class DownCastedClass>
		std::shared_ptr<DownCastedClass> downcasted_shared_from_this(){
			return std::dynamic_pointer_cast<DownCastedClass>(BaseSharedObject::shared_from_this());
		}
		
		virtual void onConstruction() = 0;
		
	};

}

//———————————————— USAGE EXAMPLE ———————————————————

/*
	class BaseObject1 : public SharedObject<BaseObject1>{
	public:
		void foo(){
			std::shared_ptr<BaseObject1> base = shared_from_this();
		}
		virtual void onConstruction() override {}
	};

	class BaseObject2 : public SharedObject<BaseObject2>{
	public:
		void foo(){
			std::shared_ptr<BaseObject2> base = shared_from_this();
		}
		virtual void onConstruction() override {}
	};

	class DerivedObject : public BaseObject1, public BaseObject2{
	public:
		void bar(){
			std::shared_ptr<DerivedObject> derivedFromBase1 = BaseObject1::downcasted_shared_from_this<DerivedObject>();
			std::shared_ptr<DerivedObject> derivedFromBase2 = BaseObject2::downcasted_shared_from_this<DerivedObject>();
			std::shared_ptr<BaseObject1> base1 = BaseObject1::shared_from_this();
			std::shared_ptr<BaseObject2> base2 = BaseObject2::shared_from_this();
		}
		virtual void onConstruction() override {
			BaseObject1::onConstruction();
			BaseObject2::onConstruction();
		}
	};
 */
