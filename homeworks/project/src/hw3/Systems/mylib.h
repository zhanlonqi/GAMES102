#include<iostream>
#include"C:/lib/eigen-3.3.8/Eigen/Eigen"
#include "../Components/CanvasData.h"
using namespace Ubpa;

class adopt {
public:
	std::string method;
	std::string fit_way="interpolation";
	std::vector<pointf2> points;
	std::vector<double> parameters;
	std::vector<float> X;
	std::vector<float> Y;
	std::vector<float> t;
	Eigen::VectorXd parameter_X;
	Eigen::VectorXd parameter_Y;
	int ceta = 1;
	int max = 5;
	adopt(std::vector<pointf2> point,std::string method,std::string fit_way,int ceta):points(point),method(method),ceta(ceta),fit_way(fit_way){
		for (int i = 0; i < points.size(); i++) {
			X.push_back(points.data()[i][0]);
			Y.push_back(points.data()[i][1]);
			t.push_back(1.f / (points.size()-1) * i);
		}
		t[t.size() - 1] = 1;
		if (fit_way == "interpolation") {
			parameter_X = calculate_interpolation(t, X);
			parameter_Y = calculate_interpolation(t, Y);
		}
		else {
			parameter_X = calculate_MinE(t, X);
			parameter_Y = calculate_MinE(t, Y);
		}
	};

	pointf2 getResult(float x) {
		float result_x=0;
		float result_y=0;
		int size = X.size();
		if(fit_way=="MinE")
		 size = X.size() > max ? max : X.size();
		
		for (int i = 0; i < size; i++) {
			result_x+=parameter_X(i)* base(x, i);
			result_y += parameter_Y(i) * base(x, i);
		}


		return pointf2(result_x,result_y);
	}

private:
	Eigen::VectorXd calculate_interpolation(std::vector<float> x, std::vector<float> y) {
		Eigen::MatrixXd temp_x(x.size(), x.size());
		Eigen::VectorXd temp_y(x.size());
		for (int i = 0; i < x.size(); i++) {
			for (int j = 0; j < x.size(); j++) {
				temp_x(i,j) = base(x[i],j);
			}
			temp_y(i) = y[i];
		}
		Eigen::VectorXd k(x.size());
		//k = (temp_x.transpose()*temp_x).inverse()*temp_x.transpose()*temp_y;
		k = temp_x.colPivHouseholderQr().solve(temp_y);
		return k;
	}

	Eigen::VectorXd calculate_MinE(std::vector<float> x, std::vector<float> y) {
		int m = x.size()>max?max:x.size();
		Eigen::MatrixXd temp_x(m, m);
		Eigen::VectorXd temp_y(m);
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				double temp = 0;
				for (int index = 0; index < x.size(); index++) {
					temp += (double)base(x[index],i)*base(x[index],j);
				}
				temp_x(i, j) = temp;
			}
			double temp = 0;
			for (int index = 0; index < x.size(); index++) {
				temp += ((double)y[index]) * base(x[index],i);
			}
			temp_y(i) = temp;
		}
		Eigen::VectorXd k;
		k = temp_x.colPivHouseholderQr().solve(temp_y);
		return k;
	}

	float base(int i, int j) {
		if (method == "exponiential") {
			return std::pow(i, j);
		}
		else if (method == "gauss") {
			if (j == 0)return 1.f;
			return exp(-1.f * (double)pow((i - t[j]), 2) * pow(ceta, 2) / 2.f);
		}
		return 0;
	}

};
