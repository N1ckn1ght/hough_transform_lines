#define _CRT_SECURE_NO_WARNINGS

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using json = nlohmann::json;

struct Point
{
	int x;
	int y;
};

void print(vector <vector <int>>& ar)
{
	for (int i = 0; i < ar.size(); i++)
	{
		for (int j = 0; j < ar[i].size(); j++)
		{
			cout << ar[i][j] << "\t";
		}
		cout << "\n";
	}
}

int main()
{
	json image;
	ifstream input_image("image_hough.json", ifstream::binary);
	input_image >> image;

	const int XMAX = 1024;
	const int YMAX = 1024;
	vector <vector <int>> hou(XMAX, vector <int>(YMAX, 0));
	vector <Point> pts;

	for (int i = 0; i < image.size(); i++)
	{
		for (int j = 0; j < image.size(); j++)
		{
			if (image[i][j] == 1)
			{
				pts.push_back({ i, j });
			}
		}
	}

	const int LIM = pts.size();
	cout << "There are " << LIM << " points.\n";

	double a, b;
	int itmp;
	for (int i = 0; i < LIM; i++)
	{
		for (int j = i + 1; j < LIM; j++)
		{
			// straight vertical and horizontal lines are NOT included
			if (pts[i].x != pts[j].x && pts[i].y != pts[j].y)
			{
				a = (pts[i].y - pts[j].y) / (pts[i].x - pts[j].x);
				b = (pts[j].y * pts[i].x - pts[i].y * pts[j].x) / (pts[i].x - pts[j].x);
				for (int k = 0; k < XMAX; k++)
				{
					itmp = (int)(a * k + b);
					if (itmp >= 0 && itmp < YMAX)
					{
						hou[k][itmp]++;
					}
				}
			}
		}
	}

	cout << "Hough has been calculated, now print..." << "\n";
	// print(hou);
	int max_bright = 0;
	double k;

	for (int i = 0; i < XMAX; i++)
	{
		for (int j = 0; j < YMAX; j++)
		{
			if (hou[i][j] > max_bright)
			{
				max_bright = hou[i][j];
			}
		}
	}

	k = (double)255 / max_bright;
	cout << "Max bright is " << max_bright << ", color k = " << k << ", now converting...\n";

	for (int i = 0; i < XMAX; i++)
	{
		for (int j = 0; j < YMAX; j++)
		{
			hou[i][j] *= k;
		}
	}

	// print(hou);
	const char* path = "result.bmp";
	string s = path;
	cout << "Finally, the .bmp file will be saved to .\\" << s << "\n";

	FILE* f;
	unsigned char* paint = NULL;
	int filesize = 54 + 3 * XMAX * YMAX;

	paint = (unsigned char*)malloc(3 * XMAX * YMAX);
	memset(paint, 0, 3 * XMAX * YMAX);

	int x, y, tmp;
	for (int i = 0; i < XMAX; i++)
	{
		for (int j = 0; j < YMAX; j++)
		{
			x = i;
			y = (YMAX - 1) - j;

			// just in case?
			tmp = hou[i][j];
			if (tmp > 255)
			{
				tmp = 255;
			}

			paint[(x + y * XMAX) * 3 + 2] = (unsigned char)tmp;
			paint[(x + y * XMAX) * 3 + 1] = (unsigned char)tmp;
			paint[(x + y * XMAX) * 3 + 0] = (unsigned char)tmp;
		}
	}

	unsigned char bmpfileheader[14] = { 'B', 'M', 0,0,0,0, 0,0, 0,0, 54,0,0,0 };
	unsigned char bmpinfoheader[40] = { 40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0 };
	unsigned char bmppad[3] = { 0,0,0 };

	bmpinfoheader[4] = (unsigned char)(XMAX);
	bmpinfoheader[5] = (unsigned char)(XMAX >> 8);
	bmpinfoheader[6] = (unsigned char)(XMAX >> 16);
	bmpinfoheader[7] = (unsigned char)(XMAX >> 24);
	bmpinfoheader[8] = (unsigned char)(YMAX);
	bmpinfoheader[9] = (unsigned char)(YMAX >> 8);
	bmpinfoheader[10] = (unsigned char)(YMAX >> 16);
	bmpinfoheader[11] = (unsigned char)(YMAX >> 24);

	f = fopen(path, "wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);

	// debug: очередное костыльное, изображение было перевёрнуто...
	for (int i = YMAX - 1; i >= 0; i--)
	{
		fwrite(paint + (XMAX * (YMAX - i - 1) * 3), 3, XMAX, f);
		fwrite(bmppad, 1, (4 - (XMAX * 3) % 4) % 4, f);
	}

	cout << "Almost done...\n";
	free(paint);
	fclose(f);

	cout << "Success!";
}
