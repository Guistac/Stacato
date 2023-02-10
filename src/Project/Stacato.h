#pragma once



namespace Stacato::Application{

bool initialize();
bool terminate();

bool openFile(std::filesystem::path path);

};



namespace Stacato::Gui{

void initialize();
void terminate();
void gui();

};
