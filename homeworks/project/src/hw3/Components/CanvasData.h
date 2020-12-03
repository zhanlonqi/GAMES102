#pragma once

#include <UGM/UGM.h>

struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool changing_point{ false };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	int index = { -1 };
	int ceta{ 1 };
	int max{ 5 };
	float lambda{ 0 };
	std::map<std::string, bool > fit_ways{ std::pair<std::string,bool>("interpolation",false),std::pair<std::string,bool>("MinE",false),std::pair<std::string,bool>("Ridge",false) };
	std::map<std::string, bool > bases{ std::pair<std::string,bool>("exponiential",false),std::pair<std::string,bool>("gauss",false) };
	std::map<std::string, bool> parameterization{ std::pair<std::string,bool>("Equidistant",false),std::pair<std::string,bool>("Chordal",false) };
};

#include "details/CanvasData_AutoRefl.inl"
