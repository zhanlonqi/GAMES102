#include<iostream>
#include<algorithm>
#include<limits>
#include<cassert>
#include<cmath>
#include<ctime>
#include<cstdlib>
#include<vector>
#include<iomanip>
#include"C:/lib/eigen-3.3.8/Eigen/Eigen"
#define M_PI 3.1415926535897
using  namespace  std;

const  int  P = 100;         //��������������
vector< double > X(P);   //��������
Eigen::MatrixXd Y(P, 1);         //����������Ӧ���������
const  int  M = 10;          //���ز�ڵ���Ŀ
vector< double > center(M);        //M��Green��������������
vector< double > delta(M);         //M��Green��������չ����
Eigen::MatrixXd Green(P, M);          //Green����
Eigen::MatrixXd Weight(M, 1);        //Ȩֵ����

/*Hermit����ʽ����*/
inline  double  Hermit(double  x) {
    return  1.1 * (1 - x + 2 * x * x) * exp(-1 * x * x / 2);
}

/*����ָ�������Ͼ��ȷֲ��������*/
inline  double  uniform(double  floor, double  ceil) {
    return  floor + 1.0 * rand() / RAND_MAX * (ceil - floor);
}

/*��������[floor,ceil]�Ϸ�����̬�ֲ�N[mu,sigma]�������*/
inline  double  RandomNorm(double  mu, double  sigma, double  floor, double  ceil) {
    double  x, prob, y;
    do {
        x = uniform(floor, ceil);
        prob = 1 / sqrt(2 * M_PI * sigma) * exp(-1 * (x - mu) * (x - mu) / (2 * sigma * sigma));
        y = 1.0 * rand() / RAND_MAX;
    } while (y > prob);
    return  x;
}

/*������������*/
void  generateSample() {
    for (int i = 0; i < P; ++i) {
        double  in = uniform(-4, 4);
        X[i] = in;
        Y(i, 0) = Hermit(in) + RandomNorm(0, 0.1, -0.3, 0.3);
    }
}

/*Ѱ���������ĸ��������*/
int  nearest(const  vector< double >& center, double  sample) {
    int  rect = -1;
    double  dist = numeric_limits< double >::max();
    for (int i = 0; i < center.size(); ++i) {
        if (fabs(sample - center[i]) < dist) {
            dist = fabs(sample - center[i]);
            rect = i;
        }
    }
    return  rect;
}

/*����ص�����*/
double  calCenter(const  vector< double >& g) {
    int  len = g.size();
    double  sum = 0.0;
    for (int i = 0; i < len; ++i)
        sum += g[i];
    return  sum / len;
}

/*KMeans���෨������������*/
void  KMeans() {
    assert(P % M == 0);
    vector<vector< double > > group(M);           //��¼���������а�����Щ����
    double  gap = 0.001;        //�������ĵĸı���С��Ϊ��ֵʱ��������ֹ
    for (int i = 0; i < M; ++i) {    //��P���������������ѡP����Ϊ��ʼ��������
        center[i] = X[10 * i + 3];      //�����Ǿ��ȷֲ��ģ��������Ǿ��ȵ�ѡȡ
    }
    while (1) {
        for (int i = 0; i < M; ++i)
            group[i].clear();    //����վ�����Ϣ
        for (int i = 0; i < P; ++i) {        //���������������鵽��Ӧ�Ĵ�
            int  c = nearest(center, X[i]);
            group[c].push_back(X[i]);
        }
        vector< double > new_center(M);        //�洢�µĴ���
        for (int i = 0; i < M; ++i) {
            vector< double > g = group[i];
            new_center[i] = calCenter(g);
        }
        bool  flag = false;
        for (int i = 0; i < M; ++i) {        //���ǰ���������ĵĸı����Ƿ�С��gap
            if (fabs(new_center[i] - center[i]) > gap) {
                flag = true;
                break;
            }
        }
        center = new_center;
        if (!flag)
            break;
    }
}

/*����Green����*/
void  calGreen() {
    for (int i = 0; i < P; ++i) {
        for (int j = 0; j < M; ++j) {
            Green(i, j) = exp(-1.0 * (X[i] - center[j]) * (X[i] - center[j]) / (2 * delta[j] * delta[j]));
        }
    }
}

/*��һ�������α��*/
Eigen::MatrixXd getGereralizedInverse(const  Eigen::MatrixXd& matrix) {
    return  (matrix.transpose() * matrix).inverse() * (matrix.transpose());
}

/*������ѵ���õ������磬������õ����*/
double  getOutput(double  x) {
    double  y = 0.0;
    for (int i = 0; i < M; ++i)
        y += Weight(i, 0) * exp(-1.0 * (x - center[i]) * (x - center[i]) / (2 * delta[i] * delta[i]));
    return  y;
}

int  main(int  argc, char* argv[]) {
    //<br>����  srand(time(0));
    generateSample();        //��������Ͷ�Ӧ�������������
    KMeans();            //��������о��࣬������������
    sort(center.begin(), center.end());       //�Ծ������ģ�һά���ݣ���������

    //���ݾ������ļ�ľ��룬�������չ����
    delta[0] = center[1] - center[0];
    delta[M - 1] = center[M - 1] - center[M - 2];
    for (int i = 1; i < M - 1; ++i) {
        double  d1 = center[i] - center[i - 1];
        double  d2 = center[i + 1] - center[i];
        delta[i] = d1 < d2 ? d1 : d2;
    }

    calGreen();      //����Green����
    Weight = getGereralizedInverse(Green) * Y;       //����Ȩֵ����

    //������ѵ���õ����������������
    for (int x = -4; x < 5; ++x) {
        cout << x << "\t";
        cout << setprecision(8) << setiosflags(ios::left) << setw(15);
        cout << getOutput(x) << Hermit(x) << endl;       //���������Ԥ���ֵ���������ʵֵ
    }
    return  0;
}