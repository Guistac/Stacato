#pragma once

#include "LegatoParameter.h"

namespace Legato{

	class NamedComponent : public Component{
		COMPONENT_INTERFACE(NamedComponent)
	public:
		std::string& getName(){ return name->getValue(); }
		String name = makeString("Default Name", "Name", "Name");
	protected:
		virtual void onConstruction() override{
			Component::onConstruction();
			addChild(name);
		}
		virtual bool onSerialization() override{
			return Component::onSerialization();
		}
		virtual bool onDeserialization() override{
			return Component::onDeserialization();
		}
		virtual bool onPostLoad() override{
			return Component::onPostLoad();
		}
		virtual void copyFrom(std::shared_ptr<Component> source) override{
			Component::copyFrom(source);
			name->overwrite("Copy of " + source->cast<NamedComponent>()->name->getValue());
		}
	};

};

