#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
using namespace cv;
using namespace std;


void Template(cv::Mat img, cv::Mat T, float x, float y, int w, int h);
void Buscar_Template(cv::Mat img, cv::Mat T, cv::Mat Tnew, cv::Mat Rc, int w, int h, int* pos[], float* sum_Rc_min, int* i_op, int* j_op);
float Suma(cv::Mat T);
void NormalizacionRc(cv::Mat T, cv::Mat Tnew, cv::Mat Rc);
void Calcular_promedio_Destandar(cv::Mat img, cv::Mat T, cv::Mat Tnew, cv::Mat Rc, int w, int h, float* vec_sum_Rc_min[],
	int* pos[], int NoMuestra, float* promedio, float* DesvEst, int* pos_i, int* pos_j);


struct BBbox
{
	float x;
	float y;
	int w;
	int h;
};



int main()
{
	//configuramos las rutas de entradas y salidas
	char entrada[50] = "D:/data/Entradas/";
	char salida[50] = "D:/data/Salidas/";
	char salidaimg[50] = "";

	//
	FILE* fp;
	char img_in[50] = "";
	char archivo[50] = "";

	//configuramos la carpeta
	char carpeta[50] = "Bike";
	char tipo[5] = "jpg";
	int Frames = 600;
	float rho = 1.5;

	//BBox
	BBbox bbox = BBbox();
	bbox.x = 214;
	bbox.y = 302;
	bbox.w = 132;
	bbox.h = 430;

	//opencv
	cv::Mat FrameI;
	cv::Mat Framenew;
	cv::Mat T;
	cv::Mat Tnew;
	cv::Mat Rc;
	cv::Scalar color(255, 0, 0);
	cv::Scalar color2(0, 255, 0);
	cv::Scalar color3(255, 255, 0);

	//calculan las posiciones (x, y) del template
	float Sx0 = 0.0f;
	float Sy0 = 0.0f;
	float Sx1 = 0.0f;
	float Sy1 = 0.0f;

	//posisciones iniciales y finales del area de busqueda
	float pos_in_x = 0.0f;
	float pos_in_y = 0.0f;
	float pos_fin_x = 0.0f;
	float pos_fin_y = 0.0f;

	////
	sprintf_s(entrada, "%s%s/", entrada, carpeta);
	sprintf_s(salida, "%s%s/", salida, carpeta);
	sprintf_s(archivo, "%ssalida.txt", salida);
	sprintf_s(img_in, "%s00001.%s", entrada, tipo);
	//fp = fopen(archivo, "r+");
	FrameI = cv::imread(img_in, 0);


	//se calcula el centro de la figura
	Sx0 = bbox.x + (bbox.h / 2);
	Sy0 = bbox.y + (bbox.w / 2);


	//se sacan la posiciones iniciales y finales
	pos_in_x = (Sx0 - (rho * bbox.h));
	pos_in_y = (Sy0 - (rho * bbox.w));
	pos_fin_x = (Sx0 + (rho * bbox.h));//largo
	pos_fin_y = (Sy0 + (rho * bbox.w));//ancho
	if (pos_in_y < 0) {
		pos_in_y = 0;
	}
	if (pos_in_x < 0) {
		pos_in_x = 0;
	}
	if (pos_fin_y > FrameI.cols) {
		pos_fin_y = FrameI.cols;
	}
	if (pos_fin_x > FrameI.rows) {
		pos_fin_x = FrameI.rows;
	}

}



void Template(cv::Mat img, cv::Mat T, float x, float y, int w, int h)
{
	for (int i = 0; i < h; i++)
	{
		for (int j = 0; j < w; j++)
		{
			T.at<unsigned char>(i, j) = img.at<unsigned char>(i + x, j + y);
		}
	}
}

void Buscar_Template(cv::Mat img, cv::Mat T, cv::Mat Tnew, cv::Mat Rc, int w, int h, int* pos[], float* sum_Rc_min, int* i_op, int* j_op)
{
	for (unsigned int i = *pos[0]; i > * pos[1]; i++)
	{
		for (unsigned int j = *pos[2]; j < *pos[3]; j++)
		{
			Template(img, Tnew, w, h, i, j);
			NormalizacionRc(T, Tnew, Rc);
			float sum_Rc = Suma(Rc);
			if (sum_Rc <= *sum_Rc_min) {
				*sum_Rc_min = sum_Rc;
				*i_op = i;
				*j_op = j;
			}
		}
	}
}

void NormalizacionRc(cv::Mat T, cv::Mat Tnew, cv::Mat Rc)
{
	int row = T.rows;
	int cols = T.cols;

	for (int i = 0; i < row; i++)
	{
		for (int j = 0; j < cols; j++)
		{
			Rc.at<float>(i, j) = (float)T.at<unsigned char>(i, j) - (float)Tnew.at<unsigned char>(i, j);
			Rc.at<float>(i, j) = Rc.at<float>(i, j) * Rc.at<float>(i, j);
		}
	}

}

float Suma(cv::Mat T)
{
	int rows = T.rows;
	int cols = T.cols;
	float suma = 0;

	for (int i = 0; i < rows; i++)
	{
		for (int j = 0; j < cols; j++) {
			suma = suma + T.at<float>(i, j);
		}
	}

	return suma;
}

void Calcular_promedio_Destandar(cv::Mat img, cv::Mat T, cv::Mat Tnew, cv::Mat Rc, int w, int h, float* vec_sum_Rc_min[],
	int* pos[], int NoMuestra, float* promedio, float* DesvEst, int* pos_i, int* pos_j)
{
	float* sum_Rc_min = 0;
	float varianza = 0.0;
	for (int t = 0; t < NoMuestra; t++)
	{
		Buscar_Template(img, T, Tnew, Rc, w, h, pos, sum_Rc_min, pos_i, pos_j);
		vec_sum_Rc_min[t] = sum_Rc_min;
		*promedio = *promedio + *sum_Rc_min;
	}

	*promedio = *promedio / NoMuestra;

	for (int i = 0; i < NoMuestra; i++)
	{
		varianza = varianza + (vec_sum_Rc_min[i] - promedio) * (vec_sum_Rc_min[i] - promedio);
	}
	varianza = varianza / (NoMuestra - 1);
	*DesvEst = sqrt(varianza);

}

