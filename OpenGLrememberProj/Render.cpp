#include "Render.h"

#include <sstream>
#include <iostream>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;		// (Н) Эта переменная отвечает за смену текстур

//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния камеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(1, 1, 3);
	}


	//рисует сферу и линии под источником света
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//линия от источника света до окружности
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//рисуем окруность
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света




//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//двигаем свет по плоскости, в точку где мышь
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}
	if (key == 'E')
	{
		textureReplace = !textureReplace;
	}
}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;
GLuint texId2;

//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray;

	//массив символов
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray2;

	//массив символов
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("texture2.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId2);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId2);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}


	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}





double N_Vector_X(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[0];
}

double N_Vector_Y(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[1];
}

double N_Vector_Z(double A[], double B[], double C[], double height) {




	//Счиатем А и В по входным точкам
	double Vector_AB[3] = { B[0] - A[0], B[1] - A[1], B[2] - A[2] };
	double Vector_AC[3] = { C[0] - A[0], C[1] - A[1], C[2] - A[2] };

	//Создаем вектор N
	double N_X = Vector_AB[1] * Vector_AC[2] - Vector_AC[1] * Vector_AB[2];
	double N_Y = -Vector_AB[0] * Vector_AC[2] + Vector_AC[0] * Vector_AB[2];
	double N_Z = Vector_AB[0] * Vector_AC[1] - Vector_AC[0] * Vector_AB[1];

	double N_Vector[] = { N_X,N_Y,N_Z };
	double Abs_Vector = sqrt(N_X * N_X + N_Y * N_Y + N_Z * N_Z);

	N_Vector[0] = N_X / Abs_Vector;
	N_Vector[1] = N_Y / Abs_Vector;
	N_Vector[2] = N_Z / Abs_Vector;
	return N_Vector[2];
}




void circle() {
	glBindTexture(GL_TEXTURE_2D, texId);
	glBegin(GL_POLYGON);
	for (double i = 0; i <= 2; i += 0.01)
	{
		double x = 9 * cos(i * 3.141593);
		double y = 9 * sin(i * 3.141593);

		double tx = cos(i * 3.141593) * 0.5 + 0.5;
		double ty = sin(i * 3.141593) * 0.5 + 0.5;

		glColor3d(0.5f, 0.5f, 0.5f);
		glNormal3d(0, 0, 1);
		glTexCoord2d(tx, ty);
		glVertex3d(x, y, 0);

	}
	glEnd();
}



void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);
	if (textureReplace) {
		glBindTexture(GL_TEXTURE_2D, texId);
	}
	if (!textureReplace) {
		glBindTexture(GL_TEXTURE_2D, texId2);
	}


	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	GLfloat sh = 0.1f * 256;


	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);

	//===================================
	//Прогать тут  

	double A[] = { 5, 7, 0 };
	double B[] = { 4, 14, 0 };
	double C[] = { 9, 12, 0 };
	double D[] = { 12, 15, 0 };
	double E[] = { 18, 11, 0 };
	double F[] = { 11, 10, 0 };
	double G[] = { 12, 5, 0 };
	double H[] = { 10, 9, 0 };

	double A1[] = { 5, 7, 5 };
	double B1[] = { 4, 14, 5 };
	double C1[] = { 9, 12, 5 };
	double D1[] = { 12, 15, 5 };
	double E1[] = { 18, 11, 5 };
	double F1[] = { 11, 10, 5 };
	double G1[] = { 12, 5, 5 };
	double H1[] = { 10, 9, 5 };


	glColor3d(0.7, 0.7, 0.7);;

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.5f, 0.5f, 0.5f);

	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(H);

	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.3f, 0.3f, 0.3f);

	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(H);

	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.3f, 0.3f, 0.3f);

	glVertex3dv(C);
	glVertex3dv(F);
	glVertex3dv(H);

	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.3f, 0.3f, 0.3f);

	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(F);

	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.3f, 0.3f, 0.3f);

	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor3d(0.3f, 0.3f, 0.3f);

	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(H);

	glEnd();


	double N_X = N_Vector_X(A, A1, B, 5);
	double N_Y = N_Vector_Y(A, A1, B, 5);
	double N_Z = N_Vector_Z(A, A1, B, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 0);
	glVertex3dv(B);
	glEnd();

	N_X = N_Vector_X(B, B1, C, 5);
	N_Y = N_Vector_Y(B, B1, C, 5);
	N_Z = N_Vector_Z(B, B1, C, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);							// (Н) Это накладывается текстура (x, y) это углы текстуры, пишутся поверх углов квадратов
	glVertex3dv(B);
	glTexCoord2d(0, 1);
	glVertex3dv(B1);
	glTexCoord2d(1, 1);
	glVertex3dv(C1);
	glTexCoord2d(1, 0);
	glVertex3dv(C);
	glEnd();

	N_X = N_Vector_X(C, C1, D, 5);
	N_Y = N_Vector_Y(C, C1, D, 5);
	N_Z = N_Vector_Z(C, C1, D, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(C);
	glTexCoord2d(1, 0);
	glVertex3dv(C1);
	glTexCoord2d(1, 1);
	glVertex3dv(D1);
	glTexCoord2d(0, 1);
	glVertex3dv(D);
	glEnd();

	N_X = N_Vector_X(E, E1, F, 5);
	N_Y = N_Vector_Y(E, E1, F, 5);
	N_Z = N_Vector_Z(E, E1, F, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(E);
	glTexCoord2d(1, 0);
	glVertex3dv(E1);
	glTexCoord2d(1, 1);
	glVertex3dv(F1);
	glTexCoord2d(0, 1);
	glVertex3dv(F);
	glEnd();

	N_X = N_Vector_X(F, F1, G, 5);
	N_Y = N_Vector_Y(F, F1, G, 5);
	N_Z = N_Vector_Z(F, F1, G, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(F);
	glTexCoord2d(1, 0);
	glVertex3dv(F1);
	glTexCoord2d(1, 1);
	glVertex3dv(G1);
	glTexCoord2d(0, 1);
	glVertex3dv(G);
	glEnd();


	N_X = N_Vector_X(G, G1, H, 5);
	N_Y = N_Vector_Y(G, G1, H, 5);
	N_Z = N_Vector_Z(G, G1, H, 5);

	glBegin(GL_QUADS);
	glColor3d(0.3f, 0.3f, 0.3f);
	glNormal3d(N_X, N_Y, N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(G);
	glTexCoord2d(1, 0);
	glVertex3dv(G1);
	glTexCoord2d(1, 1);
	glVertex3dv(H1);
	glTexCoord2d(0, 1);
	glVertex3dv(H);
	glEnd();

	N_X = N_Vector_X(A, A1, H, 5);
	N_Y = N_Vector_Y(A, A1, H, 5);
	N_Z = N_Vector_Z(A, A1, H, 5);

	glBegin(GL_QUADS);
	glColor3f(0.3f, 0.3f, 0.3f);
	glNormal3d(-N_X, -N_Y, -N_Z);
	glTexCoord2d(0, 0);
	glVertex3dv(A);
	glTexCoord2d(0, 1);
	glVertex3dv(A1);
	glTexCoord2d(1, 1);
	glVertex3dv(H1);
	glTexCoord2d(1, 0);
	glVertex3dv(H);
	glEnd();

	double x0 = 9;
	double y0 = 12;

	double tx0 = 0;
	double ty0 = 0;


	double Det[] = { 0,0,0 };
	double Det1[] = { 0,0,5 };
	double N[] = { 0,0,0 };
	double Nl;

	for (double i = -0.192; i <= 0.9; i += 0.0001)
	{
		double x = 15 + 3.535534 * cos(i * 3.141593);
		double y = 13 + 3.535534 * sin(i * 3.141593);

		double tx = tx0 + 1.0 / 7000.0;
		double ty = ty0 + 1.0 / 7000.0;
		if (i != -0.192) {
			

			glBegin(GL_POLYGON);
			glNormal3d(0, 0, -1);
			glColor3d(0.5f, 0.5f, 0.5f);
			glVertex3dv(C);
			glVertex3d(x0, y0, 0);
			glVertex3d(x, y, 0);
			glEnd();

			glBegin(GL_QUADS);
			glColor3d(0.3f, 0.3f, 0.3f);
			Det[0] = { x - (15 + 3.535534 * cos((i - 0.0001) * 3.141593)) };
			Det[1] = { y - (13 + 3.535534 * sin((i - 0.0001) * 3.141593)) };
			N[0] = { Det[1] * Det1[2] - Det1[1] * Det[2] };
			N[1] = { -Det[0] * Det1[2] + Det1[0] * Det[2] };
			N[2] = { Det[0] * Det1[1] - Det1[0] * Det[1] };
			Nl = sqrt(N[0] * N[0] + N[1] * N[1] + N[2] * N[2]);
			N[0] = { N[0] / Nl };
			N[1] = { N[1] / Nl };
			N[2] = { N[2] / Nl };
			glNormal3d(N[0], N[1], N[2]);

			glTexCoord2d(tx0, 0);
			glVertex3d(x0, y0, 0);
			glTexCoord2d(tx, 0);
			glVertex3d(x, y, 0);
			glTexCoord2d(ty, 1);
			glVertex3d(x, y, 5);
			glTexCoord2d(ty0, 1);
			glVertex3d(x0, y0, 5);
			glEnd();
		}
		x0 = x;
		y0 = y;
		tx0 = tx;
		ty0 = ty;
	}

	glEnable(GL_BLEND);											// (Н) Здесь строится крышка. Чтоб Альфа наложение хорошо работало, то объект, который должен быть прозрачным(для этого альфа и существует)
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);			//Должен строится в последнию очередь, иначе он будет просвечиваться "насквозь"
	x0 = 9;
	y0 = 12;


	for (double i = -0.192; i <= 0.9; i += 0.0001)
	{
		double x = 15 + 3.535534 * cos(i * 3.141593);
		double y = 13 + 3.535534 * sin(i * 3.141593);

		glBegin(GL_POLYGON);
		glNormal3d(0, 0, 1);
		glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
		glVertex3dv(C1);
		glVertex3d(x0, y0, 5);
		glVertex3d(x, y, 5);
		glEnd();

		x0 = x;
		y0 = y;
	}


	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(A1);
	glVertex3dv(B1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(B1);
	glVertex3dv(C1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(C1);
	glVertex3dv(F1);
	glVertex3dv(H1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(C1);
	glVertex3dv(D1);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(D1);
	glVertex3dv(E1);
	glVertex3dv(F1);
	glEnd();

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4f(0.5f, 0.5f, 0.5f, 0.7f);
	glVertex3dv(F1);
	glVertex3dv(G1);
	glVertex3dv(H1);

	glEnd();

	glDisable(GL_BLEND);

	//конец рисования квадратика станкина


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	//(все матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //включаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);


	std::stringstream ss;
	ss << "T - вкл/выкл текстур" << std::endl;
	ss << "L - вкл/выкл освещение" << std::endl;
	ss << "F - Свет из камеры" << std::endl;
	ss << "E - сменить текстуру" << std::endl;
	ss << "G - двигать свет по горизонтали" << std::endl;
	ss << "G+ЛКМ двигать свет по вертекали" << std::endl;
	ss << "Коорд. света: (" << light.pos.X() << ", " << light.pos.Y() << ", " << light.pos.Z() << ")" << std::endl;
	ss << "Коорд. камеры: (" << camera.pos.X() << ", " << camera.pos.Y() << ", " << camera.pos.Z() << ")" << std::endl;
	ss << "Параметры камеры: R=" << camera.camDist << ", fi1=" << camera.fi1 << ", fi2=" << camera.fi2 << std::endl;

	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}