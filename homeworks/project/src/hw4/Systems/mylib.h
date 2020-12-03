
#include<iostream>
#include"C:/lib/eigen-3.3.8/Eigen/Eigen"
#include "../Components/CanvasData.h"
#include<imgui/imgui.h>
#include<vector>
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
	CanvasData* data;
	double total_k = 0;
	double k = 0;
	ImVec4 col = ImVec4(255, 255, 255, 255);
	adopt(CanvasData& data, std::string method, std::string fit_way, std::string parameterization) :method(method), fit_way(fit_way), data(&data), parameterization(parameterization) {
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
				k = (i == 0 ? 0 : sqrt(pow(X[i] - X[i - 1], 2) + pow(Y[i] - Y[i - 1], 2)));
				t.push_back(i == 0 ? 0 : t[i - 1] + k / total_k);
			}
		}
		//t[t.size() - 1] = 1;
		if (fit_way == "interpolation") {
			parameter_X = calculate_interpolation(t, X);
			parameter_Y = calculate_interpolation(t, Y);
			col.x = 0;

		}
		else if (fit_way == "MinE") {
			parameter_X = calculate_MinE(t, X);
			parameter_Y = calculate_MinE(t, Y);
			col.x = 160;
		}
		else if (fit_way == "Ridge") {
			parameter_X = calculate_Ridge(t, X);
			parameter_Y = calculate_Ridge(t, Y);
			col.x = 240;
		}
		else if (fit_way == "Cubic_spline") {
			parameter_X = calculate_Cubic_spline(t, X);
			parameter_Y = calculate_Cubic_spline(t, Y);
			if (data.parameter_x.size()==0 || data.continuity["G2"]) {
				data.parameter_x = parameter_X;

				data.parameter_y = parameter_Y;

			}
			if (!data.continuity["G2"]) {
				getDerivative();
				ImGui::Begin("fuck");
				for (int i = 0; i < t.size(); i++) {
					ImGui::Text("%d: ",data.derivative_x_neg[i]);
				}
				ImGui::End();
			}
			if (data.index != -1) {
				
				if (data.index == t.size() - 1)
					data.derivative_positive =pointf2(0.f,0.f);
				else 
				data.derivative_positive = pointf2(getResult(t[data.index]+0.001)[0] - getResult(t[data.index] )[0], getResult(t[data.index]+0.001)[1] - getResult(t[data.index] )[1]);
				if (data.index == 0)
					data.derivative_negative = pointf2(0.f, 0.f);
				else
				data.derivative_negative = pointf2(getResult(t[data.index]-0.001)[0] - getResult(t[data.index] )[0], getResult(t[data.index]-0.001)[1] - getResult(t[data.index])[1]);
			}

			col.x = 80;
		}
		else {
			terminate;
		}

	};

	pointf2 getResult(float x) {
		float result_x = 0;
		float result_y = 0;
		int size = t.size();
		if (fit_way == "interpolation") {
			 size = t.size();
		}
		else if (fit_way == "MinE") {
			size = t.size() > data->max ? data->max : t.size();
		}
		else if (fit_way == "Ridge") {
			size = t.size() > data->max ? data->max + 1 : t.size();
		}
		else if (fit_way == "Cubic_spline") {
			return getResultCubicSpline(x);
		}
		else {
			terminate();
		}

		for (int i = 0; i < size; i++) {
			result_x += parameter_X(i) * base(x, i);
			result_y += parameter_Y(i) * base(x, i);
		}
		return pointf2(result_x, result_y);
	}

	pointf2 getResultCubicSpline(float x) {
		if (!data->continuity["G2"]) {
			return Hermite(x);
		}
		int part = 0;
		while (part<t.size()-1&&x >= t[part]) { //shall pay attention to this part!
			part++;
		}
		part--;

		float M_i;
		float M_i1;
		if (data->continuity["G2"]) {
			 M_i= parameter_X[part];
			 M_i1= parameter_X[part + 1];
		}
		else  {
			M_i = data->parameter_x[part];
			M_i1 = data->parameter_x[part + 1];
		}
		float h_i = t[part + 1] - t[part];
		float result_x = M_i / (6 * h_i) * pow((t[part+1] - x),3)  + M_i1 / (6 * h_i) * pow((x - t[part]),3)+(X[part+1]/h_i-(M_i1*h_i)/6)*(x-t[part])+(X[part]/h_i-M_i*h_i/6)*(t[part+1]-x);
		if (data->continuity["G2"]) {
			M_i = parameter_Y[part];
			M_i1 = parameter_Y[part + 1];
		}
		
		float result_y = M_i / (6 * h_i) * pow((t[part + 1] - x), 3) + M_i1 / (6 * h_i) * pow((x - t[part]), 3) + (Y[part + 1] / h_i - (M_i1 * h_i) / 6) * (x - t[part]) + (Y[part] / h_i - M_i * h_i / 6) * (t[part + 1] - x);

		return pointf2(result_x, result_y);
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
		k = (temp_x.transpose() * temp_x).inverse() * temp_x.transpose() * temp_y;
		//k = temp_x.colPivHouseholderQr().solve(temp_y);
		return k;
	}

	Eigen::VectorXd calculate_MinE(std::vector<float> x, std::vector<float> y) {
		int m = x.size() > data->max ? data->max : x.size();
		Eigen::MatrixXd temp_x(m, m);
		Eigen::VectorXd temp_y(m);
		for (int i = 0; i < m; i++) {
			for (int j = 0; j < m; j++) {
				double temp = 0;
				for (int index = 0; index < x.size(); index++) {
					temp += (double)base(x[index], i) * base(x[index], j);
				}
				temp_x(i, j) = temp;
			}
			double temp = 0;
			for (int index = 0; index < x.size(); index++) {
				temp += ((double)y[index]) * base(x[index], i);
			}
			temp_y(i) = temp;
		}
		Eigen::VectorXd k;
		k = temp_x.colPivHouseholderQr().solve(temp_y);
		return k;
	}

	Eigen::VectorXd calculate_Ridge(std::vector<float> x, std::vector<float> y) {
		int len = x.size() - 1 < data->max ? x.size() - 1 : data->max;
		Eigen::VectorXd xs(len + 1);
		//if (x.size() < 2)return xs;
		Eigen::MatrixXd mat(x.size(), len + 1);
		Eigen::VectorXd b(x.size());
		Eigen::MatrixXd ATA_add(len + 1, len + 1);
		Eigen::MatrixXd I = Eigen::MatrixXd::Identity(len + 1, len + 1);
		Eigen::VectorXd ATb(len + 1);

		for (int i = 0; i < x.size(); i++) {
			for (int j = 0; j < len + 1; j++)
				mat(i, j) = base(x[i], j);//pow(points[i][0], j);
			b(i) = y[i];//points[i][1];
		}
		ATA_add = mat.transpose() * mat + data->lambda * I;
		ATb = mat.transpose() * b;
		xs = ATA_add.inverse() * ATb;
		return xs;
	}

	Eigen::VectorXd calculate_Cubic_spline(std::vector<float> x, std::vector<float> y) {
	int size = x.size()-2;
	if (size <=0) {
		Eigen::VectorXd temp(2);
		temp[0] = 0;
		temp[1] = 0;
		return temp;
	}
	Eigen::MatrixXd mat(size,size);
	mat.setZero();
	Eigen::VectorXd yy(size );
	yy.setZero();
	for (int i = 1; i < size+1; i++) {
		float h_i = x[i + 1] - x[i];
		float u_i = 2*(x[i + 1] - x[i - 1]);
		float b_i = 6.f / (x[i + 1] - x[i]) * (y[i + 1] - y[i]);
		float v_i = 6.f / (x[i + 1] - x[i]) * (y[i + 1] - y[i]) - (6.f / (x[i ] - x[i-1]) * (y[i ] - y[i-1]));
		yy(i - 1) = v_i;
		mat(i - 1, i - 1) = u_i;
		if (i < size ) {
			mat(i - 1, i) = h_i;
			mat(i, i - 1) = h_i;
		}
	}

	Eigen::VectorXd temp = Chasing_method(mat, yy);//mat.lu().solve(yy);//
	Eigen::VectorXd temp2(temp.size() + 2);
	temp2[0] = 0;
	temp2[temp.size() + 1] = 0;
	for (int i = 1; i < temp.size() + 1; i++) {
		temp2[i] = temp[i - 1];
	}
	return temp2;
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

	Eigen::VectorXd  Chasing_method(Eigen::MatrixXd a, Eigen::VectorXd y)
	{
		Eigen::MatrixXd temp_A = a;
		int size = a.rows();
		Eigen::VectorXd b=y;
		Eigen::VectorXd x(size);

		Eigen::VectorXd d(size);
		Eigen::VectorXd l(size);
		Eigen::VectorXd u(size);
		for (int i = 0; i < size - 1; i++) {
			d(i) = temp_A(i, i);
			u(i) = temp_A(i, i + 1);
			l(i) = temp_A(i + 1, i);
		}
		d(size - 1) = temp_A(size - 1, size - 1);

		for (int i = 1; i < size; i++) {
			l(i - 1) = l(i - 1) / d(i - 1);
			d(i) = d(i) - u(i - 1) * l(i - 1);
			b(i) = b(i) - b(i - 1) * l(i - 1);
		}
		x(size - 1) = b(size - 1) / d(size - 1);
		for (int i = size - 2; i >= 0; i--) {
			x(i) = (b(i) - u(i) * x(i + 1)) / d(i);
		}
		return x;
	}

	pointf2 Hermite(float x) {
		float ans=0,ans2=0;
		for (int i = 0; i < t.size(); i++) {
			float temp = 0;
			float temp2 = 0;
			for (int j = 0; j < t.size(); j++) {
				if (i != j) {
					temp += 1 / (t[i] - t[j]);
					temp += 1 / (t[i] - t[j]);
				}
			}
			temp = temp * 2 * X[i] * (t[i] - x) + X[i];
			temp2 = temp2 * 2 * Y[i] * (t[i] - x) + Y[i];
			for (int j = 0; j < t.size(); j++) {
				if (i != j) {
					temp *= (x - t[j]) / (t[i] - t[j]);
					temp2*= (x - t[j]) / (t[i] - t[j]);
				}
			}
			ans += temp;
			ans2 += temp2;
		}
		return pointf2(ans, ans2);
	}

	void getDerivative() {
		Eigen::VectorXd temp1(t.size()), temp2(t.size());
		for (int i = 0; i < t.size() - 1; i++) {
		float M_i = data->parameter_x[i];
		float M_i1 = data->parameter_x[i + 1];
		float x_i1 = t[i + 1];
		float x_i = t[i];
		float h_i = x_i1 - x_i;
		temp1[i]= M_i / 2 * h_i + X[i + 1] / h_i - M_i1 * h_i / 6 - X[i] / h_i + M_i * h_i / 6;

			 M_i = data->parameter_y[i];
			 M_i1 = data->parameter_y[i + 1];
			 temp2[i] = M_i / 2 * h_i + Y[i + 1] / h_i - M_i1 * h_i / 6 - Y[i] / h_i + M_i * h_i / 6;
		}
		float M_i = data->parameter_x[t.size()-2];
		float M_i1 = data->parameter_x[t.size()-1];
		float x_i1 = t[t.size()-1];
		float x_i = t[t.size()-2];
		float h_i = x_i1 - x_i;
		temp1[t.size()-1]= M_i1 / 2 * h_i + X[t.size()-1] / h_i - M_i1 * h_i / 6 - X[t.size()-2] / h_i + M_i * h_i / 6;
		M_i = data->parameter_y[t.size()-2];
		M_i1 = data->parameter_y[t.size()-1];
		temp2[t.size()-1] = M_i1 / 2 * h_i + Y[t.size()-1] / h_i - M_i1 * h_i / 6 - Y[t.size()-2] / h_i + M_i * h_i / 6;
		data->derivative_x_neg = data->derivative_x_pos = temp1;
		data->derivative_y_neg = data->derivative_y_pos = temp2;
	}

};


