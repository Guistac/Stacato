#pragma once


class File;

namespace Stacato::Application{

bool initialize();
bool terminate();

std::shared_ptr<File> openFile(std::filesystem::path path);

};



namespace Stacato::Gui{

void initialize();
void terminate();
void gui();

};
