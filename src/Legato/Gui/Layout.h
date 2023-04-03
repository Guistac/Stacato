#pragma once

#include "Legato/Editor/Component.h"

class Layout : public Legato::Component{

	DECLARE_PROTOTYPE_IMPLENTATION_METHODS(Layout)
	
public:
	
	virtual bool onSerialization() override;
	virtual bool onDeserialization() override;
	virtual void onConstruction() override;
	virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override;
	
	void overwrite();
	
	std::string layoutString;
	std::vector<std::string> openWindowIds;
	
};
