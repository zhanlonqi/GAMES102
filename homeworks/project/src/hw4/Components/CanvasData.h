#pragma once

#include <UGM/UGM.h>
#include"C:/lib/eigen-3.3.8/Eigen/Eigen"
struct CanvasData {
	std::vector<Ubpa::pointf2> points;
	std::vector<Ubpa::pointf2> assistant_points;
	std::vector<Ubpa::pointf2> M;
	Eigen::VectorXd parameter_x;

	Eigen::VectorXd parameter_y;

	Ubpa::valf2 scrolling{ 0.f,0.f };
	bool changing_point{ false };
	bool changing_assistant_point{ false };
	bool opt_enable_grid{ true };
	bool opt_enable_context_menu{ true };
	bool adding_line{ false };
	bool changing_G{ false };
	int index = { -1 };
	int assistant_index = { -1 };
	int parent;
	int ceta{ 1 };
	int max{ 5 };
	float lambda{ 0 };
	Ubpa::pointf2 derivative_positive;
	Ubpa::pointf2 derivative_negative;
	Eigen::VectorXd derivative_x_pos;
	Eigen::VectorXd derivative_x_neg;
	Eigen::VectorXd derivative_y_pos;
	Eigen::VectorXd derivative_y_neg;

	std::map<std::string, bool> continuity{ std::pair<std::string,bool>("G2",true),std::pair<std::string,bool>("G1",true)  };
	std::map<std::string, bool > fit_ways{ std::pair<std::string,bool>("interpolation",false),std::pair<std::string,bool>("MinE",false),std::pair<std::string,bool>("Ridge",false),std::pair<std::string,bool>("Cubic_spline",false) };
	std::map<std::string, bool > bases{ std::pair<std::string,bool>("exponiential",false),std::pair<std::string,bool>("gauss",false) };
	std::map<std::string, bool> parameterization{ std::pair<std::string,bool>("Equidistant",false),std::pair<std::string,bool>("Chordal",false) };
};

#include "details/CanvasData_AutoRefl.inl"

