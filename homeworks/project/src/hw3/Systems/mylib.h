
#include<iostream>
#include"C:/lib/eigen-3.3.8/Eigen/Eigen"
#include "../Components/CanvasData.h"
#include<imgui/imgui.h>
using namespace Ubpa;

static int getNearest1(std::vector<pointf2>& points, pointf2 point, float dis) {
	int nearestIndex = -1;
	float nearestDis = dis;
	float distance2;
	for (int i = 0; i < points.size(); i++) {
		distance2 = pow(points[i][0] - point[0], 2) + pow(points[i][1] - point[1], 2);
		if (distance2 < nearestDis) {
			nearestIndex = i;
			nearestDis = distance2;
		}
	}
	return nearestIndex;
}

class adopt {
public:
	std::string method;
	std::string fit_way;
	std::string parameterization;
	std::vector<double> parameters;
	std::vector<float> X;
	std::vector<float> Y;
	std::vector<float> t;
	Eigen::VectorXd parameter_X;
	Eigen::VectorXd parameter_Y;
	CanvasData *data;
	double total_k = 0;
	double k = 0;
	ImVec4 col = ImVec4(255, 255, 255, 255);
	adopt(CanvasData &data,std::string method,std::string fit_way,std::string parameterization):method(method),fit_way(fit_way),data(&data),parameterization(parameterization){
		for (int i = 0; i < data.points.size(); i++) {
			X.push_back(data.points[i][0]);
			Y.push_back(data.points[i][1]);	
			if (parameterization == "Equidistant")//don't need to calculate total_k
			{
				col.z = 200;
				t.push_back(1.f / (data.points.size() - 1) * i);
			}
			else
				total_k += (i == 0 ? 0 : sqrt(pow(data.points[i][0] - data.points[i - 1][0], 2) + pow(data.points[i][1] - data.points[i - 1][1], 2)));
		}
			 if (parameterization == "Chordal") {
				 col.z = 20;
				 for (int i = 0; i < data.points.size(); i++) {
					  k= (i == 0 ? 0 : sqrt(pow(X[i] - X[i - 1], 2) + pow(Y[i] - Y[i-1], 2)));
					 t.push_back(i==0?0:t[i-1]+ k/total_k);
				}
			}
		//t[t.size() - 1] = 1;
		if (fit_way == "interpolation") {
			parameter_X = calculate_interpolation(t, X);
			parameter_Y = calculate_interpolation(t, Y);
			col.x = 0;

		}
		else if(fit_way=="MinE"){
			parameter_X = calculate_MinE(t, X);
			parameter_Y = calculate_MinE(t, Y);
			col.x = 160;
		}
		else if (fit_way == "Ridge") {
			parameter_X = calculate_Ridge(t, X);
			parameter_Y = calculate_Ridge(t, Y);
			col.x = 240;
		}
		else {
			terminate;
		}
		
	};

	pointf2 getResult(float x) {
		float result_x=0;
		float result_y=0;
		int size =t.size();
		if (fit_way == "interpolation") {
			int size = t.size();
		}
		if (fit_way == "MinE") {
			size = t.size() > data->max ? data->max : t.size();
		}
		else if (fit_way == "Ridge") {
			size = t.size() > data->max ? data->max + 1 : t.size();
		}
		else {
			terminate;
		}

		for (int i = 0; i < size; i++) {
			result_x += parameter_X(i) * base(x, i);
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
				temp_x(i, j) = base(x[i], j);
			}
			temp_y(i) = y[i];
		}
		Eigen::VectorXd k(x.size());
		k = (temp_x.transpose()*temp_x).inverse()*temp_x.transpose()*temp_y;
		//k = temp_x.colPivHouseholderQr().solve(temp_y);
		return k;
	}

	Eigen::VectorXd calculate_MinE(std::vector<float> x, std::vector<float> y) {
		int m = x.size()>data->max?data->max:x.size();
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

	Eigen::VectorXd calculate_Ridge(std::vector<float> x, std::vector<float> y) {
		int len = x.size()-1 < data->max ? x.size()-1 : data->max;
		Eigen::VectorXd xs(len+1);
		//if (x.size() < 2)return xs;
		Eigen::MatrixXd mat(x.size(), len+1 );
		Eigen::VectorXd b(x.size());
		Eigen::MatrixXd ATA_add(len+1 , len+1 );
		Eigen::MatrixXd I = Eigen::MatrixXd::Identity(len+1, len+1);
		Eigen::VectorXd ATb(len+1);
	
		for (int i = 0; i < x.size(); i++) {
			for (int j = 0; j < len+1; j++)
				mat(i, j) = base(x[i], j);//pow(points[i][0], j);
			b(i) = y[i];//points[i][1];
		}
		ATA_add = mat.transpose() * mat + data->lambda * I;
		ATb = mat.transpose() * b;
		xs = ATA_add.inverse() * ATb;
		return xs;
	}

	float base(float i, int j) {
		if (method == "exponiential") {
			col.y = 20;
			
			return std::pow(i, j);
		}
		else if (method == "gauss") {
			col.y = 240;
			
			if (j == 0)return 1.f;
			return exp(-1.f * (double)pow((i - t[j]), 2) * pow(data->ceta, 2) / 2.f);
		}
		return 0;
	}

};


