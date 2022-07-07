#pragma once

class Console;
class ConsoleMapping;

namespace ConsoleHandler{

void initialize();
void terminate();

void applyMapping(std::shared_ptr<Console> console);

};
