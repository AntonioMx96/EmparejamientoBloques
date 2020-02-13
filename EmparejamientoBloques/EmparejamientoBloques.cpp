#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/types.hpp>
using namespace cv;
using namespace std;

void Template(Mat frame, Mat T, struct Bbox bbox, int x, int  y);
void Area_Busqueda(Mat frame, struct Bbox bbox, struct A_Busqueda* a_busqueda, int x, int y, struct Path path);
void Buscar_Template(Mat frame, Mat T, Mat Tnew, Mat Rc, struct A_Busqueda* a_busqueda, struct Bbox bbox, 
					struct X_Y_Optima* xy_optima, struct Des_Prom* des_prom);
float Calcular_Rc(Mat T, Mat Tnew, Mat Rc);
void Calcular_Promedio_Destandar(Mat frame, Mat frame_new, Mat T, Mat Tnew, Mat Rc, struct Bbox bbox,
								struct A_Busqueda* a_busqueda, struct X_Y_Optima* xy_optima, struct Path path,
								struct Des_Prom* des_prom, float* sum_Rc_min);
void crear_vec(float* sum_Rc_min, int NoMuestra);
void Crear_Frame(Mat frame, struct Bbox bbox, struct A_Busqueda* a_busqueda, struct Path path, struct X_Y_Optima *xy_optima);

struct Bbox
{
	int x=50;
	int y=131;
	int w=36;
	int h=89;
};

struct Path
{
	char entrada[50] = "D:/cmt_dataset/";
	char salida[50] = "D:/cmt/cmt_est/";
	char carpeta[50] = "juice";
	char tipo[5] = "jpg";
	int Frames = 404;
	float rho = 1.5;
	int NoMuestra=404;
	char salidaaux[50] = "";
};

struct A_Busqueda
{
	float pos_in_x = 0.0f;
	float pos_in_y = 0.0f;
	float pos_fin_x = 0.0f;
	float pos_fin_y = 0.0f;
};

struct Centro
{
	float Sx = 0.0f;
	float Sy = 0.0f;
};

struct X_Y_Optima
{
	int x = 0;
	int y = 0;
};

struct Des_Prom
{
	float promedio = 0;
	float Desv_Estandar = 0;
	float sum_Rc_min = 0;
};
int main()
{
	struct Bbox bbox;
	struct Path path;
	struct A_Busqueda ap_a_busqueda;
	struct A_Busqueda *a_busqueda=&ap_a_busqueda;
	struct Centro ap_centro;
	struct Centro* centro=&ap_centro;
	struct X_Y_Optima ap_x_y_optima;
	struct X_Y_Optima *x_y_optima=&ap_x_y_optima;
	struct Des_Prom a_des_prom;
	struct Des_Prom *des_prom=&a_des_prom;
	Mat frame;
	Mat frame_new;
	Mat T;
	Mat Tnew;
	Mat Rc;
	float a = 0;
	float *sum_Rc_min=&a;
	char img_in[50] = "";
	char archivo[50] = "";
	

	sprintf_s(path.entrada, "%s%s/", path.entrada, path.carpeta);
	sprintf_s(path.salida, "%s%s/", path.salida, path.carpeta);
	sprintf_s(img_in, "%s00000001.%s", path.entrada, path.tipo);

	frame = imread(img_in,0);

	T.create(bbox.h, bbox.w, CV_8UC1);
	Tnew.create(bbox.h, bbox.w, CV_8UC1);
	Rc.create(bbox.h, bbox.w, CV_32FC1);

	Calcular_Promedio_Destandar(frame, frame_new, T, Tnew, Rc, bbox, a_busqueda, x_y_optima, path, des_prom, sum_Rc_min);
	
}

void Template(Mat frame, Mat T, struct Bbox bbox, int x, int  y)
{
	for (int i = 0; i < bbox.h; i++) {
		for (int j = 0; j <bbox.w; j++)
		{
			T.at<unsigned char>(i, j) = frame.at<unsigned char>(i + x, j + y);
		}
	}
}

void Area_Busqueda(Mat frame, struct Bbox bbox, struct A_Busqueda *a_busqueda, int x, int y, struct Path path)
{
	int Sx = 0;
	int Sy = 0;

	/*
	int x=50;
	int y=131;
	int w=36;
	int h=89;*/

	Sx = x + (bbox.h / 2);//=94.5
	Sy = y + (bbox.w / 2);//=149

	a_busqueda->pos_in_x = Sx - (path.rho * bbox.h);//=-39.5=0
	a_busqueda->pos_in_y = Sy - (path.rho * bbox.w);//=95
	a_busqueda->pos_fin_x = Sx + (path.rho * bbox.h);//=228
	a_busqueda->pos_fin_y = Sy + (path.rho * bbox.w);//=203

	if (a_busqueda->pos_in_y < 0) 
	{
		a_busqueda->pos_in_y = 0;
	}

	if (a_busqueda->pos_in_x < 0) 
	{
		a_busqueda->pos_in_x = 0;
	}
	if (a_busqueda->pos_fin_y > frame.cols) 
	{
		a_busqueda->pos_fin_y = (float)frame.cols;
	}
	if (a_busqueda->pos_fin_x > frame.rows) 
	{
		a_busqueda->pos_fin_x = (float)frame.rows;
	}
}

void Buscar_Template(Mat frame, Mat T, Mat Tnew, Mat Rc, struct A_Busqueda *a_busqueda,struct Bbox bbox, struct X_Y_Optima *xy_optima, struct Des_Prom* des_prom) 
{
	float suma_Rc = 0;
	float r_min = 255*255*bbox.h*bbox.w;
	int ii = 0;
	int jj = 0;
	printf("%f %f %f %f", a_busqueda->pos_in_x, a_busqueda->pos_fin_x, a_busqueda->pos_in_y, a_busqueda->pos_fin_y);

	for (int i = (int)a_busqueda->pos_in_x; i < (int)a_busqueda->pos_fin_x - bbox.h; i++)
	{
		for (int j = (int)a_busqueda->pos_in_y; j < (int)a_busqueda->pos_fin_y - bbox.w; j++)
		{
			Template(frame, Tnew, bbox, i, j);
			suma_Rc = Calcular_Rc(T, Tnew, Rc);
			if (suma_Rc <r_min)
			{
				r_min = suma_Rc;
				ii = i;
				jj = j;
			}
		}
	}
	xy_optima->x = ii;
	xy_optima->y = jj;
	des_prom->sum_Rc_min = r_min;
}

float Calcular_Rc(Mat T, Mat Tnew, Mat Rc) 
{
	int f = T.rows;
	int c = T.cols;
	float suma = 0;

	for (int i = 0; i < f; i++) 
	{
		for (int j = 0; j < c; j++)
		{
			Rc.at<float>(i, j) = (float)T.at<unsigned char>(i, j) - (float)Tnew.at<unsigned char>(i, j);
			Rc.at<float>(i, j) = Rc.at<float>(i, j) * Rc.at<float>(i, j);
			suma = suma + Rc.at<float>(i, j);
		}
	}
	return suma;
}

void Calcular_Promedio_Destandar( Mat frame, Mat frame_new, Mat T, Mat Tnew, Mat Rc,
	struct Bbox bbox, struct A_Busqueda *a_busqueda, struct X_Y_Optima *xy_optima, struct Path path,
	struct Des_Prom *des_prom, float *sum_Rc_min) 
{
	char img_in[50] = "";
	char img[20] = "";
	float promedio = 0;
	float varianza = 0;

	crear_vec(sum_Rc_min, path.NoMuestra);
	Area_Busqueda(frame, bbox, a_busqueda, bbox.x, bbox.y, path);
	Template(frame,T, bbox, bbox.x, bbox.y);

	for (int i = 0; i <= path.NoMuestra; i++) {
		sprintf_s(img, sizeof(img), "%08d", (i+1));
		sprintf_s(img_in, "%s%s.%s", path.entrada, img, path.tipo);
		sprintf_s(path.salidaaux, "%s%s.%s", path.salida, img, path.tipo);
		printf("\n%s\n", img_in);
		frame = imread(img_in, 0);
		frame_new = imread(img_in);
		Buscar_Template(frame, T, Tnew, Rc, a_busqueda,bbox, xy_optima, des_prom);
		Area_Busqueda(frame, bbox, a_busqueda, xy_optima->x, xy_optima->y, path);
		Crear_Frame(frame_new, bbox, a_busqueda, path, xy_optima);
		sum_Rc_min[i] = des_prom->sum_Rc_min;
		promedio = promedio + des_prom->sum_Rc_min;
	}
	des_prom->promedio = promedio;

	for (int i = 0; i < path.NoMuestra; i++) {
		varianza = varianza + (sum_Rc_min[i] -promedio)* (sum_Rc_min[i] - promedio);
	}
	varianza = varianza / (path.NoMuestra - 1);
	des_prom->Desv_Estandar = sqrt(varianza);
}


///////////////////////////////////////////////////////////////////////////
void crear_vec(float *sum_Rc_min, int NoMuestra) {
	sum_Rc_min = (float*)malloc(NoMuestra * sizeof(int));
	if (sum_Rc_min == NULL)
	{
		perror("problemas resercando memoria");
		exit(1);
	}
}

void Crear_Frame(Mat frame, struct Bbox bbox, struct A_Busqueda *a_busqueda, struct Path path, struct X_Y_Optima *xy_optima) 
{
	Scalar color(255, 0, 0);
	Scalar color2(0, 255, 0);
	Scalar color3(255, 255, 0);

	Point templatein(xy_optima->y, xy_optima->x);
	Point templatefin(xy_optima->y+ bbox.w, xy_optima->x + bbox.h);
	Point ini((int)a_busqueda->pos_in_y, (int)a_busqueda->pos_in_x);
	Point fin((int)a_busqueda->pos_fin_y, (int)a_busqueda->pos_fin_x);
	rectangle(frame, templatein, templatefin, color2, 2);
	rectangle(frame, ini, fin, color2, 2);
	cv::imwrite(path.salidaaux, frame);
}