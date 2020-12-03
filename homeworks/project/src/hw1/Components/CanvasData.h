#pragma once

#include <UGM/UGM.h>
#include<map>
struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	int fuck{ 1 };
	std::map<std::string, bool > fit_ways{ std::pair<std::string,bool>("interpolation",false),std::pair<std::string,bool>("MinE",false),std::pair<std::string,bool>("Ridge",false) };
	std::map<std::string, bool > bases{ std::pair<std::string,bool>("exponiential",false),std::pair<std::string,bool>("gauss",false) };

};

#include "details/CanvasData_AutoRefl.inl"
