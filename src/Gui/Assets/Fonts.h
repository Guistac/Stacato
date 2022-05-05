#pragma once

struct ImFont;

namespace Fonts {

	extern ImFont* sansRegular12;
	extern ImFont* sansBold12;
	extern ImFont* sansLight12;
	
	extern ImFont* sansRegular15;
	extern ImFont* sansBold15;
	extern ImFont* sansLight15;
	
	extern ImFont* sansRegular20;
	extern ImFont* sansBold20;
	extern ImFont* sansLight20;
	
	extern ImFont* sansRegular26;
	extern ImFont* sansBold26;
	extern ImFont* sansLight26;

	extern ImFont* sansRegular42;
	extern ImFont* sansBold42;
	extern ImFont* sansLight42;

	extern ImFont* mono14;

	void load(float scale);
};


