#pragma once

#include "Workspace/Project.h"


 class StacatoProject : public NewProject{
	 
	 DECLARE_PROTOTYPE_IMPLENTATION_METHODS(StacatoProject)
	 
protected:
	 
	 virtual void onConstruction() override {
		 Component::onConstruction();
		 //initialize object
	 }
	 
	 virtual void onCopyFrom(std::shared_ptr<PrototypeBase> source) override {
		 Component::onCopyFrom(source);
		 //copy project content
	 }
	 
	 virtual bool onWriteFile() override {
		 return false;
	 };
	 
	 virtual bool onReadFile() override {
		 return false;
	 };
	 
	 virtual bool canClose() override {
		 return true;
	 }
	 
	 virtual void onOpen() override {
		 //initialize some project variables?
	 }
	 
	 virtual void onClose() override {
		 //deinitialize some project variables?
	 };
	 
 };
 
