#pragma once

struct ImFont;

class Fonts {
public:

	static ImFont* robotoRegular15;
	static ImFont* robotoBold15;
	static ImFont* robotoLight15;

	static ImFont* robotoRegular20;
	static ImFont* robotoBold20;
	static ImFont* robotoLight20;

	static ImFont* robotoRegular42;
	static ImFont* robotoBold42;
	static ImFont* robotoLight42;

	static void load(float scale);
};


