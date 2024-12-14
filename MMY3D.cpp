#include "stdafx.h"
#include "AviFile.h"
#include "MMY3D.h"

#define MAX_LOADSTRING 100

HINSTANCE hInst;								// instance actuelle
TCHAR szTitle[MAX_LOADSTRING];					// Le texte de la barre de titre
TCHAR szWindowClass[MAX_LOADSTRING];			// le nom de la classe de fenêtre principale

ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ChPrev(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	KFYProperties(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	SpeedSetup(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ColorSetup(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	PreviewProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	VideoProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	AccelerateProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	PaletteProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	ReliefProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	f3DProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	OneFrameProc(HWND, UINT, WPARAM, LPARAM);
void MakeFinalMovie(void);

HWND hwndMain;

#define WM_TITRE WM_USER
#define WM_CHPREV WM_USER+1
#define WM_FINPREV WM_USER+2
#define PI   3.141592653589793238
#define PIs2 1.570796326794896619
#define MAXSH 8

TCHAR g_szFolder[MAX_PATH] = { 0 };
int NbFolder = 0;
char *lFolder[10];

struct RGB
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
}  *trgb = NULL,*trgbPal = NULL;
int VideoFPS = 30;
int VideoFPM = 1000;
char VideoBrowser[MAX_PATH] = { 0 };
BOOL VideoZoomText = FALSE;
BOOL bTextZoomShadow = FALSE;
char VideoZoomFont[LF_FACESIZE] = { 0 };
RGB VideoZoomColor = { 255, 255, 0 };
int VideoZoomSize = 8;
int VideoWidth = 1280;
int VideoHeight = 720;
int VideoHeightPlane = 720;
int VideoWidthC;
int VideoHeightC;
int VideoHeightPlaneC;
BOOL bAntiAlias;
BOOL VideoAbort = FALSE;
double ColSensitivity = 1.00;
BOOL bDE = FALSE;
BOOL bDEtronc = FALSE;
BOOL bSlope = FALSE;
BOOL bSQRT = FALSE;
BOOL bSQRT2 = FALSE;
double SlopeRatio = 0.35;
double SlopePower = 100.0;
double SlopeAngle = -45, SlopeAngle2, SlopeX, SlopeY, FrZoom, lightAngleRadians, lightPhongElev, lightPhongElev2, rShAngle;
BOOL bPhong = TRUE;
double PhongSize = 6.0;
double PhongRatio = 0.1;
double PhongElev = 25.0;
BOOL bPhong2 = TRUE;
double PhongSize2 = 3.0;
double PhongRatio2 = 0.15;
double PhongElev2 = 10.0;
double ShadowCoef = 0.8;
double ShadowAngle = 45.0;
BOOL SubIM = FALSE;
int nbminmoy = 30;
BOOL b3D = FALSE;
BOOL bSqrt3D = FALSE;
BOOL bLog3D = FALSE;
BOOL bAtan3D = TRUE;
BOOL bRotation = FALSE;
BOOL bPostRotation = FALSE;
BOOL bCrop3D=FALSE;
double RotationAngle = 0.0, sRotation, cRotation, RotationStart = 0.0;
double alpha3D = 0.87266462599716;
double Salpha3D = sin(alpha3D);
double Calpha3D = cos(alpha3D);
double echZ3D=1.00;
double maxniter3D=1000;
int decH3D = 10;
double nbpLiss3D = 8;
int CropX3D=0;
int CropY3D=0;
double perspectiveAmount = 0.0;
double perspectiveScale = 0.71;
BOOL bEvEchZ3D=FALSE;
BOOL bEvMaxIter3D = FALSE;
BOOL bEvVertDec3D = FALSE;
BOOL bEvRot = FALSE;
BOOL bReverseX = FALSE;
BOOL bReverseY = FALSE;
BOOL bKeyFrame = FALSE;
BOOL bMedian = FALSE;

int Width, Height, NbCol, HeightBmp, WidthBmp;
int **buffer = NULL;
float **fbuffer = NULL;
double **dbuffer = NULL;
double **dbufferprev = NULL;
HBITMAP hBmp = NULL;
HBITMAP hBmpSmall = NULL;
CRITICAL_SECTION cs = { NULL };
int WidthPrev = 0, HeightPrev, PrevRatio;

void Trace(char *s)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, "Trace.txt");
	fopen_s(&f, fname, "a");
	fputs(s, f);
	fputs("\n", f);
	fclose(f);
}

void SaveVideoBrowser(void)
{
	FILE *f;
	fopen_s(&f, "VBr.kfs", "w");
	fputs(VideoBrowser, f);
	fclose(f);
}

void LoadVideoBrowser(void)
{
	FILE *f;
	fopen_s(&f, "VBr.kfs", "r");
	if (!f)return;
	fgets(VideoBrowser, MAX_PATH, f);
	fclose(f);
}

void SaveFontSize(void)
{
	char txt[LF_FACESIZE];
	FILE *f;
	fopen_s(&f, "VFS.kfs", "w");
	sprintf_s(txt, "%d\n", VideoWidth);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoHeight);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoFPS);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoFPM);
	fputs(txt, f);
	int vzt=0;
	vzt = VideoZoomText;
	if (vzt && bTextZoomShadow)vzt = 2;
	if (bAntiAlias)vzt += 8;
	sprintf_s(txt, "%d\n", vzt);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoZoomSize);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoZoomColor.r);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoZoomColor.g);
	fputs(txt, f);
	sprintf_s(txt, "%d\n", VideoZoomColor.b);
	fputs(txt, f);
	sprintf_s(txt, "%s\n", VideoZoomFont);
	fputs(txt, f);
	fclose(f);
}

void LoadFontSize(void)
{
	char txt[LF_FACESIZE];
	FILE *f = NULL;
	fopen_s(&f, "VFS.kfs", "r");
	if (!f)return;
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoWidth);
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoHeight);
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoFPS);
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoFPM);
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoZoomText);
	if (VideoZoomText >= 8){ bAntiAlias = TRUE; VideoZoomText -= 8; }
	else bAntiAlias = FALSE;
	bTextZoomShadow = FALSE;
	if (VideoZoomText == 2){ VideoZoomText = 1; bTextZoomShadow = TRUE; }
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &VideoZoomSize);
	fgets(txt, LF_FACESIZE, f);
	int v;
	sscanf_s(txt, "%d", &v);
	VideoZoomColor.r = (char)v;
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &v);
	VideoZoomColor.g = (char)v;
	fgets(txt, LF_FACESIZE, f);
	sscanf_s(txt, "%d", &v);
	VideoZoomColor.b = (char)v;
	fgets(VideoZoomFont, LF_FACESIZE, f);
	for (int i = 0; i < strlen(VideoZoomFont); i++)if (VideoZoomFont[i] == 10 || VideoZoomFont[i] == 13)VideoZoomFont[i] = 0;
	fclose(f);
}
//ListKFY lKFYmen
struct KFYmem
{
	double min;
	double max;
	double moy;
	float zoomM;
	int   zoomE;
	HBITMAP hbm;
	int ndiv;
} *lKFYmem[16380];
BOOL KFYmemLu = FALSE;
int NbKFY = 0;
char *lKFY[16380];
int NoProp;
int  NoSelectDeb = -1, NoSelectFin = -1;

HBITMAP byte2hbitmap(BYTE *bytes, int cx, int cy)
{
	HDC hdc,hdcWnd = GetWindowDC(hwndMain);
	HBITMAP hbmp;
	BITMAPINFOHEADER bmi = { sizeof(BITMAPINFOHEADER) };
	int row;

	hdc = CreateCompatibleDC(hdcWnd);
	hbmp = CreateCompatibleBitmap(hdcWnd, cx, cy);
	GetDIBits(hdcWnd, hbmp, 0, 0, NULL, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	bmi.biCompression = bmi.biClrUsed = bmi.biClrImportant = 0;
	bmi.biBitCount = 24;
	row = ((((bmi.biWidth*(DWORD)bmi.biBitCount) + 31)&~31) >> 3);
	bmi.biSizeImage = row*bmi.biHeight;
	ReleaseDC(hwndMain, hdcWnd);
	SetDIBits(hdc, hbmp, 0, bmi.biHeight, bytes, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	DeleteDC(hdc);
	return(hbmp);
}
void LirKFYmem()
{
	char folder[MAX_PATH];
	BYTE *bmpBitsSmall = (BYTE *)malloc(27648); //3*128*72
	FILE *f;
	char src[MAX_PATH];
	HCURSOR hco = SetCursor(LoadCursor(NULL, IDC_WAIT));

	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
	for (int i = 0; i < NbKFY; i++)
	{
		strcpy_s(src, folder);
		strcat_s(src, "\\");
		strcat_s(src, lKFY[i]);
		fopen_s(&f, src, "rb");
		if (i == 0){ fread(&Width, sizeof(int), 1, f); fread(&Height, sizeof(int), 1, f); }	else fseek(f, 8L, SEEK_SET);
		lKFYmem[i] = (struct KFYmem *)malloc(sizeof(struct KFYmem));
		fread(&(lKFYmem[i]->min), sizeof(double), 1, f);
		fread(&(lKFYmem[i]->max), sizeof(double), 1, f);
		fread(&(lKFYmem[i]->moy), sizeof(double), 1, f);
		fread(&(lKFYmem[i]->zoomM), sizeof(float), 1, f);
		fread(&(lKFYmem[i]->zoomE), sizeof(int), 1, f);
		fread(bmpBitsSmall, 27648, 1, f); //3*128*72
		lKFYmem[i]->hbm = byte2hbitmap(bmpBitsSmall,128,72);
		fread(&(lKFYmem[i]->ndiv), sizeof(int), 1, f);
		fclose(f);
	}
	SetCursor(hco);
	free(bmpBitsSmall);
	KFYmemLu = TRUE;
	PostMessage(hwndMain, WM_SIZE, 0, 0L);
	InvalidateRect(hwndMain, NULL, TRUE);
}

void freelFKY(void)
{
	if (NbKFY)
	{
		for (int i = 0; i < NbKFY; i++)
		{
			free(lKFY[i]);
			if (KFYmemLu){ DeleteObject(lKFYmem[i]->hbm); free(lKFYmem[i]); }
		}
		NbKFY = 0;
		KFYmemLu = FALSE;
		Height = 0; Width = 0;
		NoSelectDeb = -1; NoSelectFin = -1;
		InvalidateRect(hwndMain, NULL, TRUE);
	}
}

void ListKFY(void)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char FileName[MAX_PATH];
	TCHAR rep[MAX_PATH];
	
	for (int i = 0; i < NbKFY; i++)free(lKFY[i]);
	NbKFY = 0;
	wcscpy_s(rep, g_szFolder);
	wcscat_s(rep, L"\\*.kfy");
	hFind = FindFirstFile(rep, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)return;

	while (true)
	{
		WideCharToMultiByte(CP_ACP, 0, FindFileData.cFileName, MAX_PATH, FileName, MAX_PATH, NULL, NULL);
		int ln = strlen(FileName) + 1;
		lKFY[NbKFY] = (char *)malloc(ln);
		strcpy_s(lKFY[NbKFY], ln, FileName);
		NbKFY++;
		if (!FindNextFile(hFind, &FindFileData))break;
	}
	FindClose(hFind);
	if (NbKFY)LirKFYmem();
}

//ListKFB
int NbKFB = 0;
char *lKFB[16380];
void ListKFB(void)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char FileName[MAX_PATH];
	TCHAR rep[MAX_PATH];

	if (NbKFB){for (int i = 0; i < NbKFB; i++)free(lKFB[i]);NbKFB = 0;}
	wcscpy_s(rep, g_szFolder);
	wcscat_s(rep, L"\\*.kfb");
	hFind = FindFirstFile(rep, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)return;

	while (true)
	{
		WideCharToMultiByte(CP_ACP, 0, FindFileData.cFileName, MAX_PATH, FileName, MAX_PATH, NULL, NULL);
		int ln = strlen(FileName) + 1;
		lKFB[NbKFB] = (char *)malloc(ln);
		strcpy_s(lKFB[NbKFB],ln, FileName);
		NbKFB++;
		if (!FindNextFile(hFind, &FindFileData))break;
	}
	FindClose(hFind);

}
//ConvertKFB2KFY
void  ThConvertKFB2KFY(void)
{
	int abort = 0;
	char folder[MAX_PATH];
	FILE *f;
	char buf[4];
	int width, height, ndiv, nbcol, maxiter;
	HDC hdc = NULL;
	HBITMAP hbmp = NULL;
	BITMAPINFOHEADER bmi = { sizeof(BITMAPINFOHEADER) };
	BYTE *bmpBits=0L;
	int row;
	HBITMAP hbmpSmall = NULL;
	BITMAPINFOHEADER bmiSmall = { sizeof(BITMAPINFOHEADER) };
	BYTE *bmpBitsSmall = 0L;
	int rowSmall;
	float zoomM;
	int   zoomE;

	f = 0L;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
	for (int i = 0; i < NbKFB; i++)
	{
		char src[MAX_PATH], dest[MAX_PATH];
		strcpy_s(src, folder);
		strcat_s(src, "\\");
		strcat_s(src, lKFB[i]);
		strcpy_s(dest, folder);
		strcat_s(dest, "\\");
		strcat_s(dest, lKFY[i]);

		PostMessage(hwndMain, WM_TITRE, i, 0L);

		fopen_s(&f, src, "rb");
		fread(buf, 1, 3, f);
		if (buf[0] != 'K' || buf[1] != 'F' || buf[2] != 'B' ){ abort = 1; break; }
		fread(&width, 4, 1, f);
		fread(&height, 4, 1, f);
		if (i == 0)
		{
			char tz[16];
			strcpy_s(tz, lKFB[0] + 6);
			int pe = -1, pp = -1;
			for (unsigned int i = 0; i < strlen(tz); i++)if (tz[i] == 'e'){ pe = i; break; };
			tz[pe] = 0;
			sscanf_s(tz, "%f", &zoomM);
			for (unsigned int i = 0; i < strlen(tz + pe + 1); i++)if (tz[i + pe + 1] == '.'){ pp = i + pe + 1; break; };
			tz[pp] = 0;
			sscanf_s(tz + pe + 1, "%d", &zoomE);
			Width = width;
			Height = height;
			buffer = (int **)malloc(width*sizeof(int *));
			for (int j = 0; j<width; j++)buffer[j] = (int *)malloc(sizeof(int)*height);
		}
		if (width != Width || Height != height){ abort = 2; break; }
		for (int j = 0; j<width; j++)
		{
			int n = fread(buffer[j], sizeof(int), height, f);
			if (n != height){ abort = 3; break; }
		}
		if (abort)break;
		fread(&ndiv, 4, 1, f);
		if (ndiv == 0)ndiv = 1;
		fread(&nbcol, 4, 1, f);
		if (i == 0)NbCol = nbcol;
		if (nbcol != NbCol){ abort = 4; break; }
		if (i==0)trgb = (struct RGB *)malloc(sizeof(RGB)*nbcol);
		fread(trgb, sizeof(RGB), nbcol, f);
	//------Noir->Gris-------
	for(int j=0;j<NbCol;j++)if (trgb[j].r == 0 && trgb[j].g == 0 && trgb[j].b == 0){ trgb[j].r = 60; trgb[j].g = 60; trgb[j].b = 60; }
	//---
		fread(&maxiter, 4, 1, f);
		if (i == 0)
		{
			fbuffer = (float **)malloc(width*sizeof(float *));
			for (int j = 0; j<width; j++)fbuffer[j] = (float *)malloc(sizeof(float)*height);
		}
		for (int j = 0; j<width; j++)
		{
			int n = fread(fbuffer[j], sizeof(float)*height, 1, f);
			if (n != 1){ abort = 5; break; }
		}
		if (abort)break;
		fclose(f);
		if (i == 0)
		{
			WidthPrev = 0;
			HeightPrev = 0;
			PostMessage(hwndMain, WM_CHPREV, i, 0L);
			while (WidthPrev == 0)Sleep(30);

			dbuffer = (double **)malloc(Height*sizeof(double *));
			for (int j = 0; j<Height; j++)dbuffer[j] = (double *)malloc(sizeof(double)*Width);
			dbufferprev = (double **)malloc(HeightPrev*sizeof(double *));
			for (int j = 0; j<HeightPrev; j++)dbufferprev[j] = (double *)malloc(sizeof(double)*WidthPrev);
		}
		for (int j = 0; j < Height; j++)for (int k = 0; k < Width; k++)
		{
			if (buffer[k][j] == maxiter)dbuffer[Height - j - 1][k] = 0.0;
			else dbuffer[Height-j-1][k] = (double)buffer[k][j] - fbuffer[k][j];
		}

		if (i == 0)
		{
			HDC hdcWnd = GetWindowDC(hwndMain);
			hdc = CreateCompatibleDC(hdcWnd);
			hbmp = CreateCompatibleBitmap(hdcWnd, Width, Height);
			hBmp = hbmp; HeightBmp = Height; WidthBmp = Width;
			GetDIBits(hdcWnd, hbmp, 0, 0, NULL, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
			bmi.biCompression = bmi.biClrUsed = bmi.biClrImportant = 0;
			bmi.biBitCount = 24;
			row = ((((bmi.biWidth*(DWORD)bmi.biBitCount) + 31)&~31) >> 3);
			bmi.biSizeImage = row*bmi.biHeight;
			bmpBits =(BYTE *) malloc(bmi.biSizeImage);

			hbmpSmall = CreateCompatibleBitmap(hdcWnd, 128, 72);
			hBmpSmall = hbmpSmall;
			GetDIBits(hdcWnd, hbmpSmall, 0, 0, NULL, (LPBITMAPINFO)&bmiSmall, DIB_RGB_COLORS);
			bmiSmall.biCompression = bmiSmall.biClrUsed = bmiSmall.biClrImportant = 0;
			bmiSmall.biBitCount = 24;
			rowSmall = ((((bmiSmall.biWidth*(DWORD)bmiSmall.biBitCount) + 31)&~31) >> 3);
			bmiSmall.biSizeImage = rowSmall*bmiSmall.biHeight;
			bmpBitsSmall = (BYTE *)malloc(bmiSmall.biSizeImage);
			ReleaseDC(hwndMain, hdcWnd);
		}
		RGB  *prgb;
		double min=1.0e100, max=0.0, moy=0.0;
		int nbv=0;
		int div = ndiv * 1024 / nbcol;
		for (int j = 0; j < Height; j++)
		{
			prgb = (RGB*)(bmpBits + row*j);
			for (int k = 0; k < Width; k++)
			{
				double v = dbuffer[j][k];
				if (v == 0.0){ prgb->r = 0; prgb->g = 0; prgb->b = 0; }
				else
				{
					if (v>max)max = v;
					if (v < min)min = v;
					moy += v;
					nbv++;
					double col = v / div;
					double r = col - (int)col;
					int ncol1 = ((int)col) % nbcol;
					int ncol2 = ncol1 + 1;
					if (ncol2 == nbcol)ncol2 = 0;

					double r1 = 1.0 - r;
					prgb->r = (unsigned char)(r1*(int)(trgb[ncol1].r) + r*(int)(trgb[ncol2].r));
					prgb->g = (unsigned char)(r1*(int)(trgb[ncol1].g) + r*(int)(trgb[ncol2].g));
					prgb->b = (unsigned char)(r1*(int)(trgb[ncol1].b) + r*(int)(trgb[ncol2].b));
				}
				prgb++;
			}
		}
		if (nbv)moy = moy / nbv;
		
		for (int i = 0; i < HeightPrev; i++)
		{
			for (int j = 0; j < WidthPrev; j++)
			{
				double vp = 0.0;
				int nb = 0;
				for (int n = 0; n < PrevRatio; n++)
				{
					for (int m = 0; m < PrevRatio; m++)
					{
					int ni, mj;

					ni = i*PrevRatio;
					mj = j*PrevRatio;
					if (ni>Height || mj>Width)continue;
					vp += dbuffer[ni][mj];
					nb++;
					}
				}
				dbufferprev[i][j] = vp/nb;
			}
		}

		EnterCriticalSection(&cs);
		HGDIOBJ ho=SelectObject(hdc, hbmp);
		SetDIBits(hdc, hbmp, 0, bmi.biHeight, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);

		SelectObject(hdc, hbmpSmall);
		SetStretchBltMode(hdc, HALFTONE);
		StretchDIBits(hdc, 0, 0, 128, 72, 0, 0, Width, Height, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, SRCCOPY);
		GetDIBits(hdc, hbmpSmall, 0, 72, bmpBitsSmall, (LPBITMAPINFO)&bmiSmall, DIB_RGB_COLORS);
		ho=SelectObject(hdc, ho);
		LeaveCriticalSection(&cs);
		InvalidateRect(hwndMain, NULL, FALSE);

		fopen_s(&f, dest, "wb");
		fwrite(&Width, sizeof(int), 1, f);
		fwrite(&Height, sizeof(int), 1, f);
		fwrite(&min, sizeof(double), 1, f);
		fwrite(&max, sizeof(double), 1, f);
		fwrite(&moy, sizeof(double), 1, f);
		fwrite(&zoomM, sizeof(float), 1, f);
		fwrite(&zoomE, sizeof(int), 1, f);
		fwrite(bmpBitsSmall, 27648, 1, f); //3*128*72
		fwrite(&ndiv, sizeof(int), 1, f);
		fwrite(&WidthPrev, sizeof(int), 1, f);
		fwrite(&HeightPrev, sizeof(int), 1, f);
		fwrite(&nbcol, sizeof(int), 1, f);
		//for (int i = 0; i < nbcol; i++){ unsigned char m; m = trgb[i].r; trgb[i].r = trgb[i].b; trgb[i].b = m; } //COLOR14 => RGB
		fwrite(trgb, sizeof(RGB), nbcol, f);
		for (int j = 0; j<HeightPrev; j++)fwrite(dbufferprev[j], sizeof(double), WidthPrev, f);
		for (int j = 0; j<Height; j++)fwrite(dbuffer[j], sizeof(double), Width, f);
		fclose(f);
		
		zoomM /= 2.0;
		if (zoomM < 1.0){ zoomM *= 10.0; zoomE -= 1; }
	}
	if (bmpBits)free(bmpBits); bmpBits = NULL;
	if (bmpBitsSmall)free(bmpBitsSmall); bmpBitsSmall = NULL;
	if (hbmp)DeleteObject(hbmp); hbmp = NULL;
	if (hbmpSmall)DeleteObject(hbmpSmall); hbmpSmall = NULL;
	ReleaseDC(hwndMain, hdc); DeleteDC(hdc);
	hBmp = NULL;
	hBmpSmall = NULL;
	if (buffer)
	{
		for (int i = 0; i < Width; i++)free(buffer[i]);
		free(buffer);
		buffer = NULL;
	}
	if (trgb != NULL){ free(trgb); trgb = NULL; }
	if (fbuffer != NULL)
	{
		for (int i = 0; i < Width; i++)free(fbuffer[i]);
		free(fbuffer);
		fbuffer = NULL;
	}
	if (dbuffer)
	{
		for (int i = 0; i < Height; i++)free(dbuffer[i]);
		free(dbuffer);
		dbuffer = NULL;
		for (int i = 0; i < HeightPrev; i++)free(dbufferprev[i]);
		free(dbufferprev);
		dbufferprev = NULL;
	}
	if (abort)
	{
		fclose(f);
		if (abort == 1)MessageBox(hwndMain, L"Invalid KFB file!", L"Movie Maker", MB_ICONERROR | MB_OK);
		if (abort == 2)MessageBox(hwndMain, L"All KFB files have not same size!", L"Movie Maker", MB_ICONERROR | MB_OK);
		if (abort == 3)MessageBox(hwndMain, L"Error reading KFB file (int table)!", L"Movie Maker", MB_ICONERROR | MB_OK);
		if (abort == 4)MessageBox(hwndMain, L"All KFB files have not same number of colors!", L"Movie Maker", MB_ICONERROR | MB_OK);
		if (abort == 5)MessageBox(hwndMain, L"Error reading KFB file  (float table)!", L"Movie Maker", MB_ICONERROR | MB_OK);
	}
	else
	{
		ListKFY();
		TCHAR Ttitre[300];
		wsprintf(Ttitre, L"Movie Maker - %s - %d x [%d / %d]", g_szFolder, NbKFY, Width, Height);
		SetWindowText(hwndMain, (LPWSTR)Ttitre);
		HMENU hm = GetMenu(hwndMain);
		EnableMenuItem(hm, 0, MF_ENABLED | MF_BYPOSITION);
		EnableMenuItem(hm, 1, MF_ENABLED | MF_BYPOSITION);
		EnableMenuItem(hm, 2, MF_ENABLED | MF_BYPOSITION);
		InvalidateRect(hwndMain, NULL, TRUE);
	}
}

void ConvertKFB2KFY(void)
{
	for (int i = 0; i < NbKFB; i++)
	{
		char buf[6];
		lKFY[i] = (char *)malloc(10);		
		strncpy_s(buf,6, lKFB[i],5);
		int no = atoi(buf);
		sprintf_s<6>(buf, "%05u", NbKFB-no);
		strncpy_s(lKFY[i], 10, buf, 5);
		strcat_s(lKFY[i], 10, ".kfy");
		NbKFY++;
	}
	HMENU hm = GetMenu(hwndMain);
	EnableMenuItem(hm, 0, MF_DISABLED | MF_BYPOSITION);
	EnableMenuItem(hm, 1, MF_DISABLED | MF_BYPOSITION);
	EnableMenuItem(hm, 2, MF_DISABLED | MF_BYPOSITION);
	_beginthread((void(_cdecl*)(void *))ThConvertKFB2KFY, 8 * 1024, NULL);
}

//BRowse
int WINAPI lpfnCallBack(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg==BFFM_SELCHANGED)SHGetPathFromIDList((LPITEMIDLIST)wParam,(LPWSTR)g_szFolder);
	return 0;
}

int Browse(HWND hWnd, TCHAR *szFolder)
{
	BROWSEINFO bi = { 0 };
	char szDisplayName[MAX_PATH];
	char szPathName[MAX_PATH];

	if(*szFolder)wcscpy_s(g_szFolder, szFolder);
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = (LPWSTR)szDisplayName;
	bi.lpszTitle = (LPWSTR)L"Dossier contenant les fichiers KFB ou KFY";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = (BFFCALLBACK)lpfnCallBack;
	bi.lParam = (LPARAM)szPathName;
	int nRet=(int)SHBrowseForFolder(&bi);
	if (!nRet){	wcscpy_s(g_szFolder, L""); return 0; }
	return nRet;
}

//List Color Speed
struct FPOINT
{
	float x;
	float y;
};
struct LISTFPOINTS
{
	int NbPoints;
	FPOINT *fPoint;
};
LISTFPOINTS lEvEchZ3D = { 0 };
LISTFPOINTS lEvMaxIter3D = { 0 };
LISTFPOINTS lEvVertDec3D = { 0 };
LISTFPOINTS lEvRot3D = { 0 };
LISTFPOINTS lSpeed = { 0 };
LISTFPOINTS lColorDiv = { 0 };
LISTFPOINTS lColorAdd = { 0 };
LISTFPOINTS lColorRot = { 0 };
float TRotCur = 0;

void SetFPoint(LISTFPOINTS *list, int n, float x, float y)
{
	if (n<list->NbPoints - 1 && n>0)
	{
		if (x > list->fPoint[n + 1].x)x = list->fPoint[n + 1].x - (float)0.1;
		if (x < list->fPoint[n - 1].x)x = list->fPoint[n - 1].x + (float)0.1;
	}
	list->fPoint[n].x = x;
	list->fPoint[n].y = y;
}

BOOL GetFPoint(LISTFPOINTS *list, int n, float *x, float *y)
{
	if (n<0 || n>list->NbPoints)return FALSE;
	*x=list->fPoint[n].x;
	*y=list->fPoint[n].y;
	return TRUE;
}

int NearestFPoint(LISTFPOINTS *list, float x, float *xn, float *yn)
{
	int n,pos = 0;
	n = list->NbPoints - 1;
	if (x > list->fPoint[n].x)pos = n;
	else for (int i = 0; i < list->NbPoints-1 ; i++)
	{
		if (x>=list->fPoint[i].x && x <= list->fPoint[i + 1].x){ pos = i; break; }
	}
	*xn = list->fPoint[pos].x;
	*yn = list->fPoint[pos].y;
	return(pos);
}

void InsertFPoint(LISTFPOINTS *list, float x, float y)
{
	int pos = list->NbPoints;
	if (list->NbPoints>0 && x < list->fPoint[0].x)pos = 0;
	else for (int i = 0; i < list->NbPoints-1; i++)
	{
		if (x>list->fPoint[i].x && x < list->fPoint[i + 1].x){ pos = i + 1; break; }
	}
	if (list->NbPoints == 0)
	{
		list->fPoint = (FPOINT *)malloc(2 * sizeof(float));
		SetFPoint(list, 0, x, y);
	}
	else
	{
		FPOINT *l = (FPOINT *)malloc((1+list->NbPoints) * 2 * sizeof(float));
		memcpy(l, list->fPoint, list->NbPoints * 2 * sizeof(float));
		free(list->fPoint);
		for (int i = list->NbPoints - 1; i >= pos; i--)l[i + 1] = l[i];
		list->fPoint = l;
		SetFPoint(list, pos , x, y);
	}
	list->NbPoints++;
}

void DeleteFPoint(LISTFPOINTS *list, int n)
{
	if (n < list->NbPoints)
	{
		FPOINT *l = (FPOINT *)malloc((list->NbPoints - 1) * 2 * sizeof(float));
		memcpy(l, list->fPoint, n * 2 * sizeof(float));
		for (int i = n; i < list->NbPoints - 1; i++)l[i] = list->fPoint[i + 1];
		free(list->fPoint);
		list->fPoint = l;
	}
	list->NbPoints--;
}

void DestroyFPoint(LISTFPOINTS *list)
{
	free(list->fPoint);
	list->NbPoints = 0;
}

void SaveFPoint(char *name, LISTFPOINTS *list)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "wb");
	fwrite(&list->NbPoints, 4, 1, f);
	for (int i = 0; i < list->NbPoints; i++)
	{
		fwrite(&list->fPoint[i].x, 4, 1, f);
		fwrite(&list->fPoint[i].y, 4, 1, f);
	}
	fclose(f);
}

void LoadFPoints(char *name, LISTFPOINTS *list)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "rb");
	if (f)
	{
		fread(&list->NbPoints, 4, 1, f);
		list->fPoint = (FPOINT *)malloc(list->NbPoints * 2 * sizeof(float));
		fread(list->fPoint, 2 * sizeof(float), list->NbPoints, f);
		fclose(f);
	}
}

void SaveSubMin(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "wb");
	fwrite(&SubIM, 4, 1, f);
	fwrite(&nbminmoy, 4, 1, f);
	fclose(f);
}

void LoadSubMin(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "rb");
	if (f)
	{
		fread(&SubIM, 4, 1, f);
		fread(&nbminmoy, 4, 1, f);
		fclose(f);
	}
}

void SaveRelief(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "wb");
	fwrite(&bSlope, 4, 1, f);
	fwrite(&SlopePower, 8, 1, f);
	fwrite(&SlopeRatio, 8, 1, f);
	fwrite(&SlopeAngle, 8, 1, f);
	fwrite(&bDE, 4, 1, f);
	fwrite(&bDEtronc, 4, 1, f);
	fwrite(&bSQRT, 4, 1, f);
	fwrite(&bSQRT2, 4, 1, f);
	fwrite(&bPhong, 4, 1, f);
	fwrite(&PhongSize, 8, 1, f);
	fwrite(&PhongRatio, 8, 1, f);
	fwrite(&PhongElev, 8, 1, f);	
	fwrite(&bPhong2, 4, 1, f);
	fwrite(&PhongSize2, 8, 1, f);
	fwrite(&PhongRatio2, 8, 1, f);
	fwrite(&PhongElev2, 8, 1, f);
	fwrite(&bMedian, 4, 1, f);
	fclose(f);
}

void LoadRelief(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "rb");
	if (f)
	{
		fread(&bSlope, 4, 1, f);
		fread(&SlopePower, 8, 1, f);
		fread(&SlopeRatio, 8, 1, f);
		fread(&SlopeAngle, 8, 1, f);
		fread(&bDE, 4, 1, f);
		fread(&bDEtronc, 4, 1, f);
		fread(&bSQRT, 4, 1, f);
		fread(&bSQRT2, 4, 1, f);
		fread(&bPhong, 4, 1, f);
		fread(&PhongSize, 8, 1, f);
		fread(&PhongRatio, 8, 1, f);
		fread(&PhongElev, 8, 1, f);
		fread(&bPhong2, 4, 1, f);
		fread(&PhongSize2, 8, 1, f);
		fread(&PhongRatio2, 8, 1, f);
		fread(&PhongElev2, 8, 1, f);
		fread(&bMedian, 4, 1, f);
		fclose(f);
	}
}

void Save3D(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "wb");
	fwrite(&b3D, 4, 1, f);
	fwrite(&bSqrt3D, 4, 1, f);
	fwrite(&bLog3D, 4, 1, f);
	fwrite(&alpha3D, 8, 1, f);
	fwrite(&echZ3D, 8, 1, f);
	fwrite(&maxniter3D, 8, 1, f);
	fwrite(&decH3D, 4, 1, f);
	fwrite(&nbpLiss3D, 8, 1, f);
	fwrite(&bAtan3D, 4, 1, f);
	fwrite(&ShadowCoef, 8, 1, f);
	fwrite(&ShadowAngle, 8, 1, f);
	fwrite(&bRotation, 4, 1, f);
	fwrite(&bPostRotation, 4, 1, f);
	fwrite(&bCrop3D, 4, 1, f);
	fwrite(&CropX3D, 4, 1, f);
	fwrite(&CropY3D, 4, 1, f);
	fwrite(&perspectiveAmount, 8, 1, f);
	fwrite(&perspectiveScale, 8, 1, f);
	fwrite(&RotationStart, 8, 1, f);
	fclose(f);
}

void Load3D(char *name)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "rb");
	if (f)
	{
		fread(&b3D, 4, 1, f);
		fread(&bSqrt3D, 4, 1, f);
		fread(&bLog3D, 4, 1, f);
		fread(&alpha3D, 8, 1, f);
		fread(&echZ3D, 8, 1, f);
		fread(&maxniter3D, 8, 1, f);
		fread(&decH3D, 4, 1, f);
		fread(&nbpLiss3D, 8, 1, f);
		fread(&bAtan3D, 4, 1, f);
		fread(&ShadowCoef, 8, 1, f);
		fread(&ShadowAngle, 8, 1, f);
		fread(&bRotation, 4, 1, f);
		fread(&bPostRotation, 4, 1, f);
		fread(&bCrop3D, 4, 1, f);
		fread(&CropX3D, 4, 1, f);
		fread(&CropY3D, 4, 1, f);
		fread(&perspectiveAmount, 8, 1, f);
		fread(&perspectiveScale, 8, 1, f);
		fread(&RotationStart, 8, 1, f);
		fclose(f);
		Salpha3D = sin(alpha3D);
		Calpha3D = cos(alpha3D);
	}
}

float RetrieveFPoint(float x, LISTFPOINTS *list, BOOL lg)
{
	float x0, y0, x1, y1, y;
	int n = NearestFPoint(list, x, &x0, &y0);
	if (n == list->NbPoints - 1)return(y0);
	else
	{
		x1 = list->fPoint[n + 1].x;
		y1 = list->fPoint[n + 1].y;
		if (lg)y = exp(log(y0) + (log(y1) - log(y0))*(x - x0) / (x1 - x0));
		else y = y0 + (y1 - y0)*(x - x0) / (x1 - x0);
		return(y);
	}
}

struct RGBPOS
{
	int pos;
	RGB rgb;
};
struct LRGBPOS
{
	int NbPoints;
	RGBPOS *lPoint;
}lRGBPos = { 0 };
BOOL RoundedPalette = FALSE;
BOOL UsePalette = FALSE;

void SetRGBPos(LRGBPOS *list, int n, int pos, RGB rgb)
{
	if (n > 0 && n<list->NbPoints - 1 && pos>list->lPoint[n - 1].pos && pos < list->lPoint[n + 1].pos)
	{
		list->lPoint[n].pos = pos;
		list->lPoint[n].rgb = rgb;
	}
}

void SetRGB(LRGBPOS *list, int n, RGB rgb)
{
		list->lPoint[n].rgb = rgb;
}

BOOL GetRGBPos(LRGBPOS *list, int n, int *pos, RGB *prgb)
{
	if (n<0 || n>list->NbPoints)return FALSE;
	*pos = list->lPoint[n].pos;
	*prgb = list->lPoint[n].rgb;
	return TRUE;
}

int NearestRGBPos(LRGBPOS *list, int posit, int *pn, RGB *prgb)
{
	int n, pos = 0;
	n = list->NbPoints - 1;
	if (posit > list->lPoint[n].pos)pos = n;
	else for (int i = 0; i < list->NbPoints - 1; i++)
	{
		if (posit >= list->lPoint[i].pos && posit < list->lPoint[i + 1].pos){ pos = i; break; }
	}
	*pn = list->lPoint[pos].pos;
	*prgb = list->lPoint[pos].rgb;
	return(pos);
}

void InsertRGBPos(LRGBPOS *list, int posit, RGB rgb)
{
	int pos = list->NbPoints;
	if (list->NbPoints>0 && posit < list->lPoint[0].pos)pos = 0;
	else for (int i = 0; i < list->NbPoints - 1; i++)
	{
		if (posit>list->lPoint[i].pos && posit < list->lPoint[i + 1].pos){ pos = i + 1; break; }
	}
	if (list->NbPoints == 0)
	{
		list->lPoint = (RGBPOS *)malloc(sizeof(RGBPOS));
		SetRGBPos(list, 0, posit, rgb);
		list->NbPoints++;
	}
	else
	{
		RGBPOS *l = (RGBPOS *)malloc((1 + list->NbPoints) * sizeof(RGBPOS));
		memcpy(l, list->lPoint, list->NbPoints * sizeof(RGBPOS));
		free(list->lPoint);
		for (int i = list->NbPoints - 1; i >= pos; i--)l[i + 1] = l[i];
		list->lPoint = l;
		list->NbPoints++;
		SetRGBPos(list, pos, posit, rgb);
	}
}

void DeleteRGBPos(LRGBPOS *list, int n)
{
	if (n < list->NbPoints)
	{
		RGBPOS *l = (RGBPOS *)malloc((list->NbPoints - 1) * sizeof(RGBPOS));
		memcpy(l, list->lPoint, n *  sizeof(RGBPOS));
		for (int i = n; i < list->NbPoints - 1; i++)l[i] = list->lPoint[i + 1];
		free(list->lPoint);
		list->lPoint = l;
	}
	list->NbPoints--;
}

void DestroyRGBPOS(LRGBPOS *list)
{
	free(list->lPoint);
	list->NbPoints = 0;
}

void SaveRGBPos(char *name, LRGBPOS *list)
{
	char fname[MAX_PATH];
	FILE *f;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "wb");
	fwrite(&UsePalette, 4, 1, f);
	fwrite(&RoundedPalette, 4, 1, f);
	fwrite(&ColSensitivity, 8, 1, f);
	fwrite(&list->NbPoints, 4, 1, f);
	for (int i = 0; i < list->NbPoints; i++)fwrite(&list->lPoint[i], sizeof(RGBPOS), 1, f);
	fclose(f);
}

void LoadRGBPos(char *name, LRGBPOS *list)
{
	char fname[MAX_PATH];
	FILE *f;
	UsePalette = FALSE;
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, fname, MAX_PATH, NULL, NULL);
	strcat_s(fname, "\\");
	strcat_s(fname, name);
	fopen_s(&f, fname, "rb");
	if (f)
	{
		fread(&UsePalette, 4, 1, f);
		fread(&RoundedPalette, 4, 1, f);
		fread(&ColSensitivity, 8, 1, f);
		fread(&list->NbPoints, 4, 1, f);
		list->lPoint = (RGBPOS *)malloc(list->NbPoints * sizeof(RGBPOS));
		fread(list->lPoint, 2 * sizeof(float), list->NbPoints, f);
		fclose(f);
	}
	else
	{
		UsePalette = FALSE; RoundedPalette = FALSE;
		if (list->NbPoints)DestroyRGBPOS(list);
	}
}

void MakeTrgbPal(BOOL Rounded)
{
	if (trgbPal == NULL)trgbPal = (RGB*)malloc(512 * sizeof(RGB));
	for (int i = 0; i < lRGBPos.NbPoints-1; i++)
	{
		RGB rgb1 = lRGBPos.lPoint[i].rgb;
		RGB rgb2 = lRGBPos.lPoint[i+1].rgb;
		int pos1 = lRGBPos.lPoint[i].pos;
		int pos2 = lRGBPos.lPoint[i+1].pos;
		for (int j = pos1; j <= pos2; j++)
		{
			if (Rounded)
			{
				double a = (1 + cos(PI*(pos2 - j) / (pos2 - pos1))) / 2;
				trgbPal[j].r = rgb1.r + (rgb2.r - rgb1.r)*a;
				trgbPal[j].g = rgb1.g + (rgb2.g - rgb1.g)*a;
				trgbPal[j].b = rgb1.b + (rgb2.b - rgb1.b)*a;
			}
			else
			{
				trgbPal[j].r = rgb1.r + ((rgb2.r - rgb1.r)*(j - pos1)) / (pos2 - pos1);
				trgbPal[j].g = rgb1.g + ((rgb2.g - rgb1.g)*(j - pos1)) / (pos2 - pos1);
				trgbPal[j].b = rgb1.b + ((rgb2.b - rgb1.b)*(j - pos1)) / (pos2 - pos1);
			}
		}
	}
}

void DestroyTrgbPal(void)
{
	free(trgbPal);
	trgbPal = NULL;
}

////////////////////////////////////////////////////////_tWinMain////////////////////////////////////////////
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
 	MSG msg;
	HACCEL hAccelTable;
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MMY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	if (!InitInstance (hInstance, nCmdShow))return FALSE;
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MMY));
	InitializeCriticalSection(&cs);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int) msg.wParam;
}

//  FONCTION : MyRegisterClass()
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MMY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MMY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassEx(&wcex);
}

//   FONCTION : InitInstance(HINSTANCE, int)
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;
   hInst = hInstance; // Stocke le handle d'instance dans la variable globale
   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_VSCROLL, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);
   if (!hWnd)return FALSE;
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   hwndMain = hWnd;
   FILE *f;
   errno_t r = fopen_s(&f,"mmy.fld","r");
   if (r==0)
   {
	   char line[260];
	   while (fgets(line, 260, f))
	   {
		   int ln = strlen(line)+1;
		   lFolder[NbFolder] = (char *)malloc(ln);
		   strncpy_s(lFolder[NbFolder++], ln, line, ln - 2);
	   }
	   fclose(f);
   }
   HMENU hm;
   hm = GetMenu(hWnd);
   hm = GetSubMenu(hm, 0);
   for (int i = 0; i < NbFolder; i++)
   {
	   TCHAR fold[MAX_PATH];
	   MultiByteToWideChar(CP_ACP, 0, lFolder[i], MAX_PATH,fold, MAX_PATH);
	   AppendMenu(hm, MF_STRING, 2000 + i, (LPCWSTR)fold);
   }
   return TRUE;
}
//AddlFolder
void AddlFolder(void)
{
	int trouve = 0;
	char folder[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
	for (int i = 0; i < NbFolder; i++)if (strcmp(folder, lFolder[i]) == 0)trouve = 1;
	if (trouve == 0)
	{
		if (NbFolder == 10)
		{
			free(lFolder[0]);
			for (int i = 0; i < 9; i++)lFolder[i] = lFolder[i + 1];
			NbFolder--;
		}
		int ln = strlen(folder);
		lFolder[NbFolder] = (char *)malloc(ln + 1);
		strncpy_s(lFolder[NbFolder++], ln + 1, folder, ln);

		FILE *f;
		errno_t r = fopen_s(&f, "mmy.fld", "w");
		if (r == 0)
		{
			for (int i = 0; i < NbFolder; i++){ fputs(lFolder[i], f); fputs("\n", f); }
			fclose(f);
		}
		else MessageBox(hwndMain, L"Error opening mmy.fld", L"MMY", MB_OK);
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int CurPos = 0, MaxPos, nby, nbx;
	static HDC hdcMemory = NULL;
	static HPEN hp1 = NULL;
	static HFONT hf = NULL;
	int wmId, wmEvent, shift, No;
	PAINTSTRUCT ps;
	HDC hdc;
	BOOL nob = false;
	POINTS PosSouris;

	switch (message)
	{
	case WM_TITRE:
		TCHAR txt[300];
		wsprintf(txt, L"Files Conversion KFB -> KFY %d/%d", wParam,NbKFY);
		SetWindowText(hwndMain, txt);
		break;
	case WM_CHPREV:
		DialogBox(hInst, MAKEINTRESOURCE(IDD_CHPREV), hWnd, ChPrev);
		break;
	case WM_CLOSE:
		if (NbKFY)if (MessageBox(hWnd, L"Do you want to quit?", L"Movie Maker", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)return(NULL);
		return DefWindowProc(hWnd, message, wParam, lParam);
	case WM_SIZE:
		if (wParam != SIZE_MINIMIZED)
		{
			RECT rect;
			GetClientRect(hWnd, &rect);
			int cx = rect.right - rect.left;
			int cy = rect.bottom - rect.top;
			nbx = (cx - 2) / 130;
			nby = (cy - 2) / 74;
			int nblt =1+ NbKFY / nbx;
			MaxPos = nblt - nby;
			if (MaxPos < 0)MaxPos = 0;
			SetScrollRange(hWnd, SB_VERT, 0, MaxPos, TRUE);
		}
		break;
	case WM_LBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		PosSouris = MAKEPOINTS(lParam);
		No = -1;
		if (PosSouris.x < 130 * nbx && PosSouris.y < 74 * (nby + 1))No = CurPos*nbx + PosSouris.x / 130 + (PosSouris.y / 74)*nbx;
		if (No >= NbKFY)No = -1;
		if (No>-1)
		{
			if (shift)
			{
				if (NoSelectDeb>-1)NoSelectFin = No; else NoSelectDeb = No;
				if (NoSelectDeb > NoSelectFin){ int m = NoSelectFin; NoSelectFin = NoSelectDeb; NoSelectDeb = m; }
				if (NoSelectDeb == NoSelectFin)NoSelectFin = -1;
				InvalidateRect(hWnd, NULL, TRUE);
			}
			else
			{
				NoSelectDeb = No; 
				NoSelectFin = -1;
				InvalidateRect(hWnd, NULL, TRUE);
			}
		}
		break;
	case WM_RBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		PosSouris = MAKEPOINTS(lParam);
		No = -1;
		if (PosSouris.x < 130 * nbx && PosSouris.y < 74 * (nby + 1))No = CurPos*nbx + PosSouris.x / 130 + (PosSouris.y / 74)*nbx;
		if (No >= NbKFY)No = -1;
		if (No>-1)
		{
			RECT rect;

			GetWindowRect(hWnd, &rect);
			HMENU hMen = CreatePopupMenu();
			AppendMenu(hMen, MF_STRING, 2, L"Select All");
			AppendMenu(hMen, MF_STRING, 1, L"Properties");
			AppendMenu(hMen, MF_STRING, 0, L"Cancel");
			int rc = TrackPopupMenu(hMen, TPM_LEFTALIGN | TPM_RETURNCMD, rect.left+PosSouris.x, rect.top+PosSouris.y, 0, hWnd, NULL);
			DestroyMenu(hMen);
			if (rc == 1){ NoProp = No; DialogBox(hInst, MAKEINTRESOURCE(IDD_KFYPROP), hWnd, KFYProperties); }
			if (rc == 2){ NoSelectDeb = 0; NoSelectFin = NbKFY - 1; InvalidateRect(hWnd, NULL, TRUE); }
		}

		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		if (wmId >= 2000 && wmId<2012)
		{
			int no = wmId - 2000;
			MultiByteToWideChar(CP_ACP, 0, lFolder[no], MAX_PATH, g_szFolder, MAX_PATH);
			nob = TRUE;
			wmId =IDM_OUVRIRDOSSIER;

		}
		switch (wmId)
		{
		case IDM_MAKEVIDEO:
			if (NoSelectFin == -1){ MessageBox(hwndMain, L"No Key Frames selected", L"Movie Maker", MB_OK | MB_ICONEXCLAMATION); break; }
			wsprintf(txt, L"Confirm you want to make final movie from %d to %d KeyFrames?\nFPS: %d   FPM: %d   Width: %d", NoSelectDeb, NoSelectFin, VideoFPS, VideoFPM, VideoWidth);
			if (MessageBox(hwndMain, txt, L"Movie Maker", MB_OKCANCEL | MB_ICONQUESTION)==IDOK)MakeFinalMovie();
				break;
		case IDM_ABORTVIDEO:
			VideoAbort = TRUE;
			break;
		case IDM_VIDEOPARAM:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_VIDEO), hWnd, VideoProc);
			break;
		case IDM_FLIPHORZ:
		    if(bReverseX)
			{
			HMENU hmenu = GetMenu(hwndMain);
			CheckMenuItem(hmenu, IDM_FLIPHORZ, MF_UNCHECKED | MF_BYCOMMAND);
			bReverseX=FALSE;
			}
			else
			{
			HMENU hmenu = GetMenu(hwndMain);
			CheckMenuItem(hmenu, IDM_FLIPHORZ, MF_CHECKED | MF_BYCOMMAND);
			bReverseX=TRUE;
			}
			break;
		case IDM_FLIPVERT:
			if (bReverseY)
			{
				HMENU hmenu = GetMenu(hwndMain);
				CheckMenuItem(hmenu, IDM_FLIPVERT, MF_UNCHECKED | MF_BYCOMMAND);
				bReverseY = FALSE;
			}
			else
			{
				HMENU hmenu = GetMenu(hwndMain);
				CheckMenuItem(hmenu, IDM_FLIPVERT, MF_CHECKED | MF_BYCOMMAND);
				bReverseY = TRUE;
			}
			break;
		case IDM_KEYFRAME:
			if (bKeyFrame)
			{
				HMENU hmenu = GetMenu(hwndMain);
				CheckMenuItem(hmenu, IDM_KEYFRAME, MF_UNCHECKED | MF_BYCOMMAND);
				bKeyFrame = FALSE;
			}
			else
			{
				HMENU hmenu = GetMenu(hwndMain);
				CheckMenuItem(hmenu, IDM_KEYFRAME, MF_CHECKED | MF_BYCOMMAND);
				bKeyFrame = TRUE;
			}
			break;

		case IDM_ACCELERATE:
			if (NoSelectFin == -1)MessageBox(hWnd, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else DialogBox(hInst, MAKEINTRESOURCE(IDD_ACCELERATE), hWnd, AccelerateProc);
			break;
		case IDM_ONEFRAME:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ONEFRAME), hWnd, OneFrameProc);
			break;
		case IDM_SPEEDSETUP:
			if (NoSelectFin == -1)MessageBox(hWnd, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker",MB_ICONEXCLAMATION | MB_OK );
			else DialogBox(hInst, MAKEINTRESOURCE(IDD_SPEEDSETUP), hWnd, SpeedSetup);
			break;
		case IDM_COLORSETUP:
			if (NoSelectFin == -1)MessageBox(hWnd, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else DialogBox(hInst, MAKEINTRESOURCE(IDD_COLORSETUP), hWnd, ColorSetup);
			break;
		case IDM_OUVRIRDOSSIER:
			if (nob || Browse(hWnd, g_szFolder))
			{
				LoadVideoBrowser();
				LoadFontSize();
				freelFKY();
				TCHAR Ttitre[300];
				wcscpy_s(Ttitre, L"Movie Maker - ");
				wcscat_s(Ttitre, g_szFolder);
				SetWindowText(hWnd, Ttitre);
				ListKFY();
				if (NbKFY)
				{
					SubIM = FALSE; CurPos = 0; MaxPos = 0; SetScrollPos(hWnd, SB_VERT, CurPos, TRUE);
					AddlFolder();
					TCHAR Ttitre[300];
					wsprintf(Ttitre, L"Movie Maker - %s - %d x [%d / %d]", g_szFolder, NbKFY, Width, Height);
					SetWindowText(hwndMain, (LPWSTR)Ttitre);
					HMENU hmenu = GetMenu(hwndMain);
					EnableMenuItem(hmenu, IDM_OUVRIRDOSSIER, MF_DISABLED);
					EnableMenuItem(hmenu, IDM_FERMERDOSSIER, MF_ENABLED);
					EnableMenuItem(hmenu, IDM_PALETTE, MF_ENABLED);
					EnableMenuItem(hmenu, IDM_ACCELERATE, MF_ENABLED);
				}
				else
				{
					MessageBox(hWnd, L"This folder do not contain any KFY files!", L"Movie Maker", MB_OK);
					ListKFB();
					if (NbKFB)
					{
						TCHAR txt[128];
						wsprintf(txt, L"This folder contains %d KFB files\nDo you want to convert them to KFY files?", NbKFB);
						if (MessageBox(hWnd, txt, L"Movie Maker", MB_OKCANCEL) == IDOK)
						{
							ConvertKFB2KFY();
							if (NbKFY)
							{
								SubIM = FALSE; CurPos = 0; MaxPos = 0;	SetScrollPos(hWnd, SB_VERT, CurPos, TRUE);
								AddlFolder();
								HMENU hmenu = GetMenu(hwndMain);
								EnableMenuItem(hmenu, IDM_OUVRIRDOSSIER, MF_DISABLED);
								EnableMenuItem(hmenu, IDM_FERMERDOSSIER, MF_ENABLED);
								EnableMenuItem(hmenu, IDM_PALETTE, MF_ENABLED);
								EnableMenuItem(hmenu, IDM_ACCELERATE, MF_ENABLED);
							}
						}
					}
					else
					{
						MessageBox(hWnd, L"This folder do not contain any KFB files!", L"Movie Maker", MB_OK);
						HMENU hmenu = GetMenu(hwndMain);
						EnableMenuItem(hmenu, IDM_OUVRIRDOSSIER, MF_ENABLED);
						EnableMenuItem(hmenu, IDM_FERMERDOSSIER, MF_DISABLED);
						EnableMenuItem(hmenu, IDM_PALETTE, MF_DISABLED);
						EnableMenuItem(hmenu, IDM_ACCELERATE, MF_DISABLED);
						SetWindowText(hwndMain, L"Movie Maker");
					}
				}
			}
			break;
		case IDM_FERMERDOSSIER:
			if (MessageBox(hWnd, L"Do you want to leave this folder?", L"Movie Maker", MB_ICONQUESTION | MB_OKCANCEL) == IDOK)
			{
				freelFKY(); InvalidateRect(hWnd, NULL, TRUE);
				HMENU hmenu = GetMenu(hwndMain);
				EnableMenuItem(hmenu, IDM_OUVRIRDOSSIER, MF_ENABLED);
				EnableMenuItem(hmenu, IDM_FERMERDOSSIER, MF_DISABLED);
				EnableMenuItem(hmenu, IDM_PALETTE, MF_ENABLED);
				SetWindowText(hwndMain, L"Movie Maker");
			}
			break;
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_RELIEF:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_RELIEF), hWnd, ReliefProc);
			break;
		case IDM_3D:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_3D), hWnd, f3DProc);
			break;
		case IDM_PALETTE:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_PALETTE), hWnd, PaletteProc);
			break;
		case IDM_EXIT:
			if (NbKFY)if (MessageBox(hWnd, L"Do you want to quit?", L"Movie Maker", MB_ICONQUESTION | MB_OKCANCEL) != IDOK)break;
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		if (hdcMemory == NULL)
		{
			hdcMemory = CreateCompatibleDC(GetDC(0));
		}
		if (hBmp)
		{
			EnterCriticalSection(&cs);
			HGDIOBJ ho=SelectObject(hdcMemory, hBmp);
			int cx, cy;
			RECT rect;
			POINT pt;
			GetClientRect(hWnd,&rect);
			cx = rect.right - rect.left;
			cy = rect.bottom - rect.top;
			SetStretchBltMode(hdc, HALFTONE);
			StretchBlt(hdc, 0, 0, cx, cy, hdcMemory, 0, 0, WidthBmp, HeightBmp, SRCCOPY);
			SetBrushOrgEx(hdc, 0, 0, &pt);
			SelectObject(hdcMemory, ho);
			LeaveCriticalSection(&cs);
		}
		if (hBmpSmall)
		{
			EnterCriticalSection(&cs);
			HGDIOBJ ho = SelectObject(hdcMemory, hBmpSmall);
			BitBlt(hdc, 0, 0, 128, 72, hdcMemory, 0, 0, SRCCOPY);
			SelectObject(hdcMemory, ho);
			LeaveCriticalSection(&cs);
		}
		if (KFYmemLu && !hBmp)
		{
			RECT rect;

			GetClientRect(hWnd, &rect);
			int cx = rect.right - rect.left;
			int cy = rect.bottom - rect.top ;
			int nbx = (cx - 2) / 130;
			int nby = (cy - 2) / 74;
			
			HGDIOBJ ho = NULL;
			int n = CurPos*nbx;
			int l = 0, c = 0;
			while (true)
			{
				if (!ho)ho = SelectObject(hdcMemory, lKFYmem[n]->hbm); else SelectObject(hdcMemory, lKFYmem[n]->hbm);
				BitBlt(hdc, 2 + c * 130, 2 + l * 74, 128, 72, hdcMemory, 0, 0, SRCCOPY);
				if (hf == NULL)hf = CreateFont(12, 5, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, (LPCTSTR)"Arial");
				HGDIOBJ hs = SelectObject(hdc, hf);
				SetTextColor(hdc, RGB(255, 220, 160));
				SetBkColor(hdc, RGB(80, 120, 140));
				TCHAR txt[8];
				wsprintf(txt, L"%d", n);
				TextOut(hdc, 2 + c * 130, 2 + l * 74, (LPWSTR)txt, _tcslen(txt));
				SelectObject(hdc, hs);
				if (n == NoSelectDeb || (NoSelectFin != -1 && n <= NoSelectFin && n>NoSelectDeb))
				{
					if (hp1 == NULL)hp1 = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
					HGDIOBJ hs = SelectObject(hdc, hp1);
					POINT Point;
					MoveToEx(hdc, 1 + c * 130, 1 + l * 74, &Point);
					LineTo(hdc, 130 + c * 130, 1 + l * 74);
					LineTo(hdc, 130 + c * 130, 74 + l * 74);
					LineTo(hdc, 1 + c * 130, 74 + l * 74);
					LineTo(hdc, 1 + c * 130, 1 + l * 74);
					SelectObject(hdc, hs);
				}
				if (++n == NbKFY)break;
				if (++c == nbx){ l++; c = 0; }
				if (l > nby)break;
			}
			SelectObject(hdcMemory, ho);
		}
		EndPaint(hWnd, &ps);
		break;
	case WM_VSCROLL:
		CurPos = GetScrollPos(hWnd, SB_VERT);
		switch (LOWORD(wParam))
		{
		case SB_TOP:
			CurPos = 0;
			break;
		case SB_PAGEUP:
			CurPos -= nby;
			if (CurPos<0)CurPos = 0;
			break;
		case SB_PAGEDOWN:
			CurPos += nby;
			if (CurPos>MaxPos)CurPos = MaxPos;
			break;
		case SB_LINEUP:
			if (CurPos > 0)CurPos--;
			break;
		case SB_THUMBPOSITION:
			CurPos = HIWORD(wParam);
			break;
		case SB_THUMBTRACK:
			CurPos = HIWORD(wParam);
			break;
		case SB_LINEDOWN:
			if (CurPos < MaxPos)CurPos++;
			break;
		case SB_BOTTOM:
			CurPos = MaxPos;
			break;
		case SB_ENDSCROLL:
			break;
		}
		SetScrollPos(hWnd, SB_VERT, CurPos, TRUE);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PaletteProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static BOOL Drag=FALSE;
	static HPEN hp1= NULL;
	static HBRUSH hb1 = NULL;
	static int npDrag = -1;
	HWND hwndGraph;
	PAINTSTRUCT ps;
	HDC hdc;
	POINTS PosSouris;
	int cxb, cyb, l, h;
	int shift, ctrl;
	TCHAR txt[50];
	char ctxt[50];
	static RECT rectGraph;

	switch (message)
	{
	case WM_PAINT:
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		hdc = BeginPaint(hwndGraph, &ps);
		h = rectGraph.bottom - rectGraph.top - 4;
		l = rectGraph.right - rectGraph.left - 4;
		if (hp1 == NULL)hp1 = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
		HGDIOBJ ho;
		ho = SelectObject(hdc, hp1);
		POINT Point;
		for (int i = 0; i < l; i++)
		{
			HPEN hp;
			int n;
			n = (512 * i) / l;
			hp = CreatePen(PS_SOLID, 1, RGB(trgbPal[n].r, trgbPal[n].g, trgbPal[n].b));
			SelectObject(hdc, hp);
			MoveToEx(hdc, i, 0, &Point);
			LineTo(hdc, i, h - 1);
			DeleteObject(hp);
		}
		SelectObject(hdc, hp1);
		MoveToEx(hdc, 0, h / 2, &Point);
		LineTo(hdc, l , h / 2);
		RECT rect;
		rect.top = h / 2 - 3; rect.left = 0; rect.bottom = h / 2 + 3; rect.right = 3;
		if (hb1 == NULL)hb1 = CreateSolidBrush(RGB(120, 120, 120));
		SelectObject(hdc, hb1);
		FillRect(hdc, &rect, hb1);
		rect.top = h / 2 - 3; rect.left = l-3; rect.bottom = h / 2 + 3; rect.right = l;
		FillRect(hdc, &rect, hb1);
		for (int i = 1; i < lRGBPos.NbPoints - 1; i++)
		{
			RGB rgb;
			int n,pos;
			GetRGBPos(&lRGBPos, i, &pos,&rgb);
			n = (l * pos) / 512;
			rect.top = h / 2 - 3; rect.left = n - 3; rect.bottom = h / 2 + 3; rect.right = n + 3;
			FillRect(hdc, &rect, hb1);
		}
		SelectObject(hdc, ho);
		EndPaint(hwndGraph, &ps);
		break;
	case WM_INITDIALOG:
		sprintf_s(ctxt, "%lf", ColSensitivity);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_COLSENS, txt);
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		RECT rw, rc;
		GetClientRect(hDlg, &rc);
		GetWindowRect(hDlg, &rw);
		cxb = (rw.right - rw.left - rc.right) / 2;  //GetSystemMetrics(SM_CXPADDEDBORDER );
		cyb = rw.bottom - rw.top - rc.bottom - cxb; //GetSystemMetrics(SM_CYCAPTION);
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		GetWindowRect(hwndGraph, &rectGraph);
		rectGraph.top -= rw.top + cyb;
		rectGraph.bottom -= rw.top + cyb;
		rectGraph.left -= rw.left + cxb;
		rectGraph.right -= rw.left + cxb;
		LoadRGBPos("palette.kfs", &lRGBPos);
		SendDlgItemMessage(hDlg, IDC_USEPALETTE, BM_SETCHECK, (UsePalette) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_LINEAR, BM_SETCHECK, (RoundedPalette) ? BST_CHECKED : BST_UNCHECKED, 0);
		if (lRGBPos.NbPoints < 2)
		{
			RGB rgb;
			lRGBPos.NbPoints = 3;
			lRGBPos.lPoint = (RGBPOS*)malloc(3 * sizeof(RGBPOS));
			rgb.r = 0; rgb.g = 0;	rgb.b = 0;
			lRGBPos.lPoint[0].pos = 0;
			lRGBPos.lPoint[0].rgb = rgb;
			lRGBPos.lPoint[2].pos = 511;
			lRGBPos.lPoint[2].rgb = rgb;
			rgb.r = 0; rgb.g = 0;	rgb.b = 255;
			lRGBPos.lPoint[1].pos = 255;
			lRGBPos.lPoint[1].rgb = rgb;
		}
		MakeTrgbPal(RoundedPalette);
		return (INT_PTR)TRUE;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MOUSEMOVE:
		shift = wParam & MK_SHIFT;
		ctrl = wParam & MK_CONTROL;
		int n,nx,pos;
		RGB rgb;
		PosSouris = MAKEPOINTS(lParam);
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			l = rectGraph.right - rectGraph.left - 4;
			nx = (512*(PosSouris.x - rectGraph.left - 2)) / l;
			if (message == WM_LBUTTONDOWN)
			{
				if (!shift && ctrl)
				{
					int np = NearestRGBPos(&lRGBPos, nx + 3, &pos, &rgb);
					if (abs(pos - nx) <= 6)
					{
						DeleteRGBPos(&lRGBPos, np);
						MakeTrgbPal(RoundedPalette);
						InvalidateRect(hDlg, NULL, FALSE);
					}
				}
				if (shift && !ctrl)
				{
					rgb.r = trgbPal[nx].r;
					rgb.g = trgbPal[nx].g;
					rgb.b = trgbPal[nx].b;
					InsertRGBPos(&lRGBPos, nx, rgb);
					InvalidateRect(hDlg, NULL, FALSE);
				}
				if (!shift && !ctrl)
				{
					int np = NearestRGBPos(&lRGBPos, nx + 3, &pos, &rgb);
					if (abs(pos - nx) <= 6)
					{
						Drag = true;
						npDrag = np;
					}
				}
			}
			if (message == WM_RBUTTONDOWN)
			{
				int np = NearestRGBPos(&lRGBPos, nx + 3, &pos, &rgb);
				if (abs(pos - nx) <= 6)
				{
					CHOOSECOLOR cc;                 // common dialog box structure 
					static COLORREF acrCustClr[16]; // array of custom colors 
					ZeroMemory(&cc, sizeof(cc));
					cc.lStructSize = sizeof(cc);
					cc.hwndOwner = hDlg;
					cc.lpCustColors = (LPDWORD)acrCustClr;
					cc.rgbResult = RGB(rgb.r, rgb.g, rgb.b);
					cc.Flags = CC_FULLOPEN | CC_RGBINIT;
					if (ChooseColor(&cc) == TRUE)
					{
						rgb.r = GetRValue(cc.rgbResult);
						rgb.g = GetGValue(cc.rgbResult);
						rgb.b = GetBValue(cc.rgbResult);
						SetRGB(&lRGBPos, np, rgb);
						MakeTrgbPal(RoundedPalette);
						InvalidateRect(hDlg, NULL, FALSE);
					}
				}
			}
			if (message == WM_MOUSEMOVE && Drag)
			{
				GetRGBPos(&lRGBPos, npDrag, &n, &rgb);
				SetRGBPos(&lRGBPos, npDrag, nx, rgb);
				MakeTrgbPal(RoundedPalette);
				InvalidateRect(hDlg, NULL, FALSE);
			}
		}
		break;
	case WM_LBUTTONUP:
		Drag = FALSE;
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_LINEAR)
		{
			if (IsDlgButtonChecked(hDlg, IDC_LINEAR) == BST_CHECKED)RoundedPalette = TRUE; else RoundedPalette = FALSE;
			MakeTrgbPal(RoundedPalette);
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_USEPALETTE)
		{
			if (IsDlgButtonChecked(hDlg, IDC_USEPALETTE) == BST_CHECKED)UsePalette = TRUE; else UsePalette = FALSE;
		}
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_COLSENS, txt, 50);
			swscanf(txt, L"%lf", &ColSensitivity);
			SaveRGBPos("palette.kfs", &lRGBPos);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK PreviewProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

void bmpImgKF(HWND hw1,HWND hw2,int n1,int n2)
{
	static HDC hdcMemory = NULL;
	HDC hdc = GetDC(hw1);
	if (hdcMemory == NULL)hdcMemory = CreateCompatibleDC(hdc);
	SelectObject(hdcMemory, lKFYmem[n1]->hbm);
	BOOL r=BitBlt(hdc, 0, 0, 128, 72, hdcMemory, 0, 0, SRCCOPY);
	ReleaseDC(hw1, hdc);
	hdc = GetDC(hw2);
	SelectObject(hdcMemory, lKFYmem[n2]->hbm);
	r = BitBlt(hdc, 0, 0, 128, 72, hdcMemory, 0, 0, SRCCOPY);
	ReleaseDC(hw2, hdc);
}

HWND hdlgPreview = NULL, hdlgOrg;
BOOL PreviewRun = FALSE, PreviewAbort = FALSE;
int cxPreview,cyPreview,mNoFrame=-1;
double **pbuffer1 = NULL, **pbuffer2 = NULL, **pbuffer3 = NULL, **pbuffer4 = NULL, **pbuffer = NULL, **pbufferR1 = NULL, **pbufferR2 = NULL, **pbufferR3 = NULL, **pbufferR4 = NULL;
CAviFile *pAvi;

void CreatePreview(HWND hdlg)
{
	if (hdlgPreview)return;
	hdlgPreview = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_PREVIEW), hwndMain, PreviewProc);
	RECT r, pr;
	GetWindowRect(hdlg, &r);
	GetWindowRect(hdlgPreview, &pr);
	pr.right -= pr.left;
	pr.bottom -= pr.top;
	MoveWindow(hdlgPreview, r.right, r.top, pr.right, pr.bottom, TRUE);
	ShowWindow(hdlgPreview, SW_SHOWNA);
}

void ShowPreviewBmp(HBITMAP hbmp)
{
	static HDC hdcMemory = NULL;
	HDC hdc = GetDC(hdlgPreview);
	if (hdcMemory == NULL)hdcMemory = CreateCompatibleDC(hdc);
	SelectObject(hdcMemory,hbmp);
	RECT rect;
	GetClientRect(hdlgPreview,&rect);
	BOOL r = StretchBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMemory, 0, 0, cxPreview, cyPreview, SRCCOPY);
	ReleaseDC(hdlgPreview, hdc);
}

HBITMAP CreateBitmap(int cx, int cy, RGB **pline, HDC *phdc, BITMAPINFOHEADER *pbmi)
{
	HDC hdcWnd = GetWindowDC(hwndMain);
	HBITMAP hbmp;
	pbmi->biSize = sizeof(BITMAPINFOHEADER);
	int row;

	*phdc = CreateCompatibleDC(hdcWnd);
	hbmp = CreateCompatibleBitmap(hdcWnd, cx, cy);
	GetDIBits(hdcWnd, hbmp, 0, 0, NULL, (LPBITMAPINFO)pbmi, DIB_RGB_COLORS);
	pbmi->biCompression = pbmi->biClrUsed = pbmi->biClrImportant = 0;
	pbmi->biBitCount = 24;
	row = ((((pbmi->biWidth*(DWORD)pbmi->biBitCount) + 31)&~31) >> 3);
	pbmi->biSizeImage = row*pbmi->biHeight;
	*pline = (RGB *)malloc(row);
	ReleaseDC(hwndMain, hdcWnd);
	return(hbmp);
}

void MakePreviewFrame(float x, float cdiv, float addcur, HDC *phdc, HBITMAP *phbmp, LPBITMAPINFO pbmi, RGB **pline, BOOL OneFrame)
{
	static int nbcol;
	double div;
	char src[MAX_PATH], folder[MAX_PATH];
	int ndiv;
	long pos;
	int n = (int)x;
	float alfa = x - n;
	FILE *f;
	if (mNoFrame == -1)
	{
		WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
		strcpy_s(src, folder);
		strcat_s(src, "\\");
		if (n > NbKFY - 1)n = NbKFY - 1;
		strcat_s(src, lKFY[n]);
		fopen_s(&f, src, "rb");

		pos = 27692L;
		fseek(f, pos, SEEK_SET);
		fread(&cxPreview, sizeof(int), 1, f);
		fread(&cyPreview, sizeof(int), 1, f);
		fread(&nbcol, sizeof(int), 1, f);
		SetWindowPos(hdlgPreview, HWND_TOP, 0, 0, cxPreview + 2*GetSystemMetrics(SM_CXDLGFRAME), cyPreview + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYDLGFRAME), SWP_NOMOVE);
		ndiv = lKFYmem[0]->ndiv;
		trgb = (RGB*)malloc(nbcol*sizeof(RGB));
		fread(trgb, sizeof(RGB), nbcol, f);
		pbuffer = (double **)malloc(cyPreview*sizeof(double *)); 
		pbuffer1 = (double **)malloc(cyPreview*sizeof(double *));
		pbuffer2 = (double **)malloc(cyPreview*sizeof(double *));
		for (int i = 0; i < cyPreview; i++)
		{
			pbuffer[i] = (double *)malloc(sizeof(double)* cxPreview); 
			pbuffer1[i] = (double *)malloc(sizeof(double)* cxPreview);
			pbuffer2[i] = (double *)malloc(sizeof(double)* cxPreview);
			fread(pbuffer2[i], sizeof(double), cxPreview, f);
		}
		fclose(f);
		*phbmp = CreateBitmap(cxPreview, cyPreview, pline, phdc, (BITMAPINFOHEADER *)pbmi);
		if (!OneFrame)
		{
			pAvi = new CAviFile("preview.avi", 24, VideoFPS, 1, 100);
			pAvi->ChooseCodec(hwndMain, cxPreview, cyPreview);
		}
	}
	if (n != mNoFrame || mNoFrame == -1)
	{
		if (n + 1 >= NbKFY)n = NbKFY - 2;
		WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
		strcpy_s(src, folder);
		strcat_s(src, "\\");
		strcat_s(src, lKFY[n+1]);
		fopen_s(&f, src, "rb");
		pos = 27696L + 3 * nbcol;
		fseek(f, pos, SEEK_SET);
		for (int i = 0; i < cyPreview; i++)
		{
			memcpy(pbuffer1[i], pbuffer2[i], sizeof(double)* cxPreview);
			fread(pbuffer2[i], sizeof(double), cxPreview, f);
		}
		fclose(f);
		mNoFrame = n;
	}
	double ratio = exp(-alfa * 0.693147180559);
	int cx2 = cxPreview / 2;
	int cy2 = cyPreview / 2;
	int cx4 = cx2 / 2;
	int cx34 = cx2 + cx4;
	int cy4 = cy2 / 2;
	int cy34 = cy2 + cy4;
	double min = 1.0E100;
	for (int i = 0; i < cyPreview; i++) 
	{
		for (int j = 0; j < cxPreview; j++) 
		{
			double x, y;
			int nx, ny;
			x = (j - cx2) * ratio + cx2;
			nx = (int)x;
			y = (i - cy2) * ratio + cy2;
			ny = (int)y;
			if (nx <= cx4 || nx >= cx34 || ny <= cy4 || ny >= cy34)
			{
				if (nx >= 0 && nx < cxPreview && ny >= 0 && ny < cyPreview)
				{
					double v = pbuffer1[ny][nx];
					if (v != 0.0 && v<min)min = v;
					pbuffer[i][j] = v;
				}
			}
			else 
			{
				int nx2 = cx2 + (nx - cx2) * 2;
				int ny2 = cy2 + (ny - cy2) * 2;
				pbuffer[i][j] = pbuffer2[ny2][nx2];
			}
		}
	}
	int nbc = (UsePalette) ? 512 : nbcol;
	div = (float)(cdiv * 1024 / nbc);
	double addcur2;
	addcur2 = addcur;
	addcur2 *= nbc;
	if (SubIM)addcur2 -= min / div;
	while (addcur2 < 0)addcur2 += nbc;
	RGB *t = (UsePalette) ? trgbPal : trgb;
	for (int i = 0; i < cyPreview; i++)  
	{
		RGB* prgb = *pline;
		for (int j = 0; j < cxPreview; j++) 
		{
			double v = pbuffer[i][j];
			if (v == 0.0){ prgb->r = 0; prgb->g = 0; prgb->b = 0; }
			else
			{
				double col = v / div + addcur2;
				double r = col - (int)col;
				int ncol1 = ((int)col) % nbc;
				int ncol2 = ncol1 + 1;
				if (ncol2 == nbc)ncol2 = 0;

				double r1 = 1.0 - r;
				prgb->r = (unsigned char)(r1*(int)(t[ncol1].r) + r*(int)(t[ncol2].r));
				prgb->g = (unsigned char)(r1*(int)(t[ncol1].g) + r*(int)(t[ncol2].g));
				prgb->b = (unsigned char)(r1*(int)(t[ncol1].b) + r*(int)(t[ncol2].b));
			}
			prgb++;
		}
		SetDIBits(*phdc, *phbmp, i, 1, *pline, pbmi, DIB_RGB_COLORS);
	}
	ShowPreviewBmp(*phbmp);
	if (!OneFrame)pAvi->AppendNewFrame(*phbmp);
	TCHAR wtxt[24];
	char txt[24];
	sprintf_s(txt, "Preview %4.2f", x);
	MultiByteToWideChar(CP_ACP, 0, txt, 24, wtxt, 24);
	SetWindowText(hdlgPreview, wtxt);
}

#define SPEED 1
#define COLORDIV 2
#define COLORADD 3
#define COLORROT 4

float xOneFrame=-1;
int npDragOneFrame = -1;
int orgOneFrame = 0;
void ThPreviewOneFrame(void)
{
	int mNo=-1,mode;
	HDC hdc = NULL;
	BITMAPINFOHEADER bmi;
	HBITMAP hbmp = NULL;
	RGB *line = NULL;
	float xcur;
	mNoFrame = -1;
	while (true)
	{
		mode = 0;
		if (xOneFrame >= 0)
		{
			npDragOneFrame = (int)xOneFrame;
			xcur = xOneFrame;
			xOneFrame = -1;
			mode = 1;
		}
		if (npDragOneFrame >= 0)
		{
			if (mode == 0)
			{
				if (orgOneFrame == SPEED)xcur = lSpeed.fPoint[npDragOneFrame].x;
				if (orgOneFrame == COLORDIV)xcur = lColorDiv.fPoint[npDragOneFrame].x;
				if (orgOneFrame == COLORADD)xcur = lColorAdd.fPoint[npDragOneFrame].x;
				if (orgOneFrame == COLORROT)xcur = lColorRot.fPoint[npDragOneFrame].x;
			}
			if ((int)xcur != mNo && mNo != -1)
			{
				if (pbuffer){ for (int i = 0; i < cyPreview; i++)free(pbuffer[i]); free(pbuffer); pbuffer = NULL; }
				if (pbuffer1){ for (int i = 0; i < cyPreview; i++)free(pbuffer1[i]); free(pbuffer1); pbuffer1 = NULL; }
				if (pbuffer2){ for (int i = 0; i < cyPreview; i++)free(pbuffer2[i]); free(pbuffer2); pbuffer2 = NULL; }
				if (trgb){ free(trgb); trgb = NULL; }
				DeleteDC(hdc);
				free(line);
				DeleteObject(hbmp);
				mNoFrame = -1;
			}
			float divcur, addcur, rotcur;
			TRotCur = 0;
			BOOL fini = FALSE;
			float x = lSpeed.fPoint[0].x;
			float y = lSpeed.fPoint[0].y;
			for (int i = 0; i < lSpeed.NbPoints; i++)
			{
				float xdeb = lSpeed.fPoint[i].x;
				float ydeb = lSpeed.fPoint[i].y;
				float xfin, yfin;
				if (i == lSpeed.NbPoints - 1)
				{
					xfin = (float)NoSelectFin;
					yfin = lSpeed.fPoint[i].y;
				}
				else
				{
					xfin = lSpeed.fPoint[i + 1].x;
					yfin = lSpeed.fPoint[i + 1].y;
				}
				while (x < xfin)
				{
					x += 1 / y;
					if (x >= xcur){ fini = TRUE; break; }
					y = exp(log(ydeb) + (log(yfin) - log(ydeb))*(x - xdeb) / (xfin - xdeb));
					rotcur = RetrieveFPoint(x, &lColorRot, FALSE);
					TRotCur += rotcur / y;
				}
				if (fini)break;
			}
			divcur = RetrieveFPoint(xcur, &lColorDiv, TRUE);
			addcur = RetrieveFPoint(xcur, &lColorAdd, FALSE);
			MakePreviewFrame(xcur, divcur*ColSensitivity, (addcur + TRotCur/2)/ColSensitivity, &hdc, &hbmp, (LPBITMAPINFO)&bmi, &line, TRUE);
			mNo = (int)xcur;
			npDragOneFrame = -1;
		}
		if (npDragOneFrame == -2)
		{
			if (pbuffer){ for (int i = 0; i < cyPreview; i++)free(pbuffer[i]); free(pbuffer); pbuffer = NULL; }
			if (pbuffer1){ for (int i = 0; i < cyPreview; i++)free(pbuffer1[i]); free(pbuffer1); pbuffer1 = NULL; }
			if (pbuffer2){ for (int i = 0; i < cyPreview; i++)free(pbuffer2[i]); free(pbuffer2); pbuffer2 = NULL; }
			if (trgb){ free(trgb); trgb = NULL; }
			DeleteDC(hdc);
			free(line);
			DeleteObject(hbmp);
			mNoFrame = -1;
			npDragOneFrame = -1;
			break;
		}
		Sleep(0);
	}
}

void ThPreview(void)
{
	HDC hdc=NULL;
	BITMAPINFOHEADER bmi;
	HBITMAP hbmp=NULL;
	RGB *line=NULL;
	
	float xdeb, xfin, xcur, ydeb, yfin, ycur, divcur, addcur, rotcur;
	int fini = 0;
	TRotCur = 0;
	PreviewRun = TRUE;
	xcur = lSpeed.fPoint[0].x;
	ycur = lSpeed.fPoint[0].y;
	for (int i = 0; i < lSpeed.NbPoints; i++)
	{
		xdeb = lSpeed.fPoint[i].x;
		ydeb = lSpeed.fPoint[i].y;
		if (i == lSpeed.NbPoints - 1)
		{
			xfin = (float)NoSelectFin;
			yfin = lSpeed.fPoint[i].y;
		}
		else
		{
			xfin = lSpeed.fPoint[i + 1].x;
			yfin = lSpeed.fPoint[i + 1].y;
		}
		while (xcur < xfin)
		{
			xcur += 1 / ycur;
			ycur = exp(log(ydeb) + (log(yfin) - log(ydeb))*(xcur - xdeb) / (xfin - xdeb));
			if (xcur > NoSelectFin || xcur >= NbKFY - 1 || PreviewAbort){ fini = 1; break; }
			if (xcur > NoSelectDeb)
			{ 
				divcur = RetrieveFPoint(xcur, &lColorDiv, TRUE);
				addcur = RetrieveFPoint(xcur, &lColorAdd, FALSE);
				rotcur = RetrieveFPoint(xcur, &lColorRot, FALSE);
				TRotCur += rotcur/ycur;
				MakePreviewFrame(xcur, divcur*ColSensitivity, (addcur + TRotCur/2)/ColSensitivity, &hdc, &hbmp, (LPBITMAPINFO)&bmi, &line, FALSE);
			}
		}
		if (fini)break;
	}
	PreviewRun = FALSE; PreviewAbort = FALSE;
	if (pbuffer){ for (int i = 0; i < cyPreview; i++)free(pbuffer[i]); free(pbuffer); pbuffer = NULL; }
	if (pbuffer1){ for (int i = 0; i < cyPreview; i++)free(pbuffer1[i]); free(pbuffer1); pbuffer1 = NULL; }
	if (pbuffer2){ for (int i = 0; i < cyPreview; i++)free(pbuffer2[i]); free(pbuffer2); pbuffer2 = NULL; }
	if (trgb){ free(trgb); trgb = NULL; }
	DeleteDC(hdc);
	free(line);
	DeleteObject(hbmp);
	delete pAvi;
	mNoFrame = -1;
	PostMessage(hdlgOrg,WM_FINPREV,0,0L);
	char cmd[MAX_PATH + 20];
	strcpy(cmd, VideoBrowser);
	strcat(cmd, " preview.avi");
	WinExec(cmd, SW_SHOW);
}

void Preview(HWND hdlg)
{
	hdlgOrg = hdlg;
	SetDlgItemText(hdlg, IDC_PREVIEW, L"Abort");
	EnableWindow(GetDlgItem(hdlg, IDOK), FALSE);
	EnableWindow(GetDlgItem(hdlg, IDCANCEL), FALSE);
	CreatePreview(hdlg);

	_beginthread((void(_cdecl*)(void *))ThPreview, 8 * 1024, NULL);

}

void AlphaBlt(HBITMAP bmDst, int x, int y, int w, int h, HBITMAP bmSrc, int sx, int sy, int nPercentage)
{
	int row, rowSrc;
	BYTE *lpBits, *lpBitsSrc;
	HDC dc = GetDC(NULL);
	BITMAPINFOHEADER bmi = { sizeof(BITMAPINFOHEADER) };
	BITMAPINFOHEADER bmiSrc = { sizeof(BITMAPINFOHEADER) };
	GetDIBits(dc, bmSrc, 0, 0, NULL, (LPBITMAPINFO)&bmiSrc, DIB_RGB_COLORS);
	bmiSrc.biCompression = bmiSrc.biClrUsed = bmiSrc.biClrImportant = 0;
	bmiSrc.biBitCount = 24;
	rowSrc = ((((bmiSrc.biWidth*(DWORD)bmiSrc.biBitCount) + 31)&~31) >> 3);
	bmiSrc.biSizeImage = rowSrc*bmiSrc.biHeight;
	lpBitsSrc = new BYTE[bmiSrc.biSizeImage];
	GetDIBits(dc, bmSrc, 0, bmiSrc.biHeight, lpBitsSrc, (LPBITMAPINFO)&bmiSrc, DIB_RGB_COLORS);
	GetDIBits(dc, bmDst, 0, 0, NULL, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	bmi.biCompression = bmi.biClrUsed = bmi.biClrImportant = 0;
	bmi.biBitCount = 24;
	row = ((((bmi.biWidth*(DWORD)bmi.biBitCount) + 31)&~31) >> 3);
	bmi.biSizeImage = row*bmi.biHeight;
	lpBits = new BYTE[bmi.biSizeImage];
	GetDIBits(dc, bmDst, 0, bmi.biHeight, lpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	int nXStart, nYStart;
	for (nXStart = 0; nXStart<w; nXStart++){
		for (nYStart = 0; nYStart<h; nYStart++){
			if ((nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row + 2 < (int)bmi.biSizeImage && (nXStart + sx) * 3 + (bmiSrc.biHeight - (nYStart + sy) - 1)*rowSrc + 2 < (int)bmiSrc.biSizeImage){
				lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row] = (nPercentage*lpBitsSrc[(nXStart + sx) * 3 + (bmiSrc.biHeight - (nYStart + sy) - 1)*rowSrc] + (100 - nPercentage)*lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row]) / 100;
				lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row + 1] = (nPercentage*lpBitsSrc[(nXStart + sx) * 3 + (bmiSrc.biHeight - (nYStart + sy) - 1)*rowSrc + 1] + (100 - nPercentage)*lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row + 1]) / 100;
				lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row + 2] = (nPercentage*lpBitsSrc[(nXStart + sx) * 3 + (bmiSrc.biHeight - (nYStart + sy) - 1)*rowSrc + 2] + (100 - nPercentage)*lpBits[(nXStart + x) * 3 + (bmi.biHeight - (nYStart + y) - 1)*row + 2]) / 100;
			}
		}
	}
	SetDIBits(dc, bmDst, 0, bmi.biHeight, lpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS);
	delete[] lpBits;
	delete[] lpBitsSrc;
	ReleaseDC(NULL, dc);
}

void SetZoomText(HBITMAP bmBmp, LPCWSTR tzoom)
{
	HDC dc = GetDC(NULL);
	SIZE sc;
	TCHAR font[LF_FACESIZE];
	HBITMAP bmText=NULL;
	MultiByteToWideChar(CP_ACP, 0, VideoZoomFont, LF_FACESIZE, font, LF_FACESIZE);
	HDC dcText = CreateCompatibleDC(dc);
	HFONT hFont = CreateFont(-VideoZoomSize, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, font);
	SelectObject(dcText, hFont);
	if(bTextZoomShadow)SelectObject(dcText, bmBmp);
	else	
		{
		GetTextExtentPoint32(dcText, tzoom, _tcslen(tzoom), &sc);
		sc.cx += 6;
		sc.cy += 3;
		bmText = CreateCompatibleBitmap(dc, sc.cx, sc.cy);
		SelectObject(dcText, bmText);
		RECT r = { 0, 0, sc.cx, sc.cy };
		FillRect(dcText, &r, (HBRUSH)GetStockObject(BLACK_BRUSH));
		}
	SelectObject(dcText, hFont);
	SetBkMode(dcText, TRANSPARENT);
	SetTextAlign(dcText, TA_TOP | TA_LEFT);
	if(bTextZoomShadow)
		{
		SetTextColor(dcText, RGB(0, 0, 0));
		TextOut(dcText, 10, 7, tzoom, _tcslen(tzoom));
		SetTextColor(dcText, RGB(VideoZoomColor.r, VideoZoomColor.g, VideoZoomColor.b));
		TextOut(dcText, 8, 5, tzoom, _tcslen(tzoom));
		}
	else
		{
		SetTextColor(dcText, RGB(VideoZoomColor.r, VideoZoomColor.g, VideoZoomColor.b));
		TextOut(dcText, 2, 2, tzoom, _tcslen(tzoom));
		}
	DeleteObject(hFont);
	if(!bTextZoomShadow)
		{
	BITMAP bm;
		GetObject(bmBmp, sizeof(BITMAP), &bm);
		AlphaBlt(bmBmp, 5, 5, sc.cx, sc.cy, bmText, 0, 0, 50);
		DeleteObject(bmText);
		}
	DeleteDC(dcText);
	ReleaseDC(NULL, dc);
}

#define MAXTHREAD 32
struct ThColorbmpBitsParam
{
	BYTE *bmpBits;
	double **pb;
	int row;
	double div;
	double addcur;
	int nbcol;
} TCBP;
HANDLE ColorbmpBitsEvents[MAXTHREAD];
void ThColorbmpBits(void *arg)
{
	int na = (int)arg;
	int np = na>>16;
	int no = na & 0xFF;
	BYTE *bmpBits = TCBP.bmpBits;
	double **pb = TCBP.pb;
	int row = TCBP.row;
	double div = TCBP.div;
	double addcur = TCBP.addcur;
	int nbcol = TCBP.nbcol;
	int nbc = (UsePalette) ? 512 : nbcol;
	RGB *t = (UsePalette) ? trgbPal : trgb;
	BOOL Neg = (echZ3D < 0 && b3D);
	for (int i = no; i < Height; i += np)
	{
		RGB* prgb = (RGB*)(bmpBits + row*i);
		for (int j = 0; j < Width; j++)
		{
			double v = pb[i][j];
			if (v == 0.0){ prgb->r = 0; prgb->g = 0; prgb->b = 0; }
			else
			{
				if (!bDE)
				{
					double col = v / div + addcur;
					double r = col - (int)col;
					int ncol1 = ((int)col) % nbc;
					int ncol2 = ncol1 + 1;
					if (ncol2 == nbc)ncol2 = 0;

					double r1 = 1.0 - r;
					prgb->r = (unsigned char)(r1*(int)(t[ncol1].r) + r*(int)(t[ncol2].r));
					prgb->g = (unsigned char)(r1*(int)(t[ncol1].g) + r*(int)(t[ncol2].g));
					prgb->b = (unsigned char)(r1*(int)(t[ncol1].b) + r*(int)(t[ncol2].b));
				}
			}

			if (bSlope)
			{
				double p1, p2;
				double diffx,diffy;
				if (i)
				{
					p1 = pb[i-1][j];
					p2 = pb[i][j];
				}
				else
				{
					p1 = pb[i][j];
					p2 = pb[i+1][j];
				}
				diffx = p1 - p2;
				if (Neg)diffx = -diffx;
				if (j)
				{
					p1 = pb[i][j-1];
					p2 = pb[i][j];
				}
				else
				{
					p1 = pb[i][j];
					p2 = pb[i][j+1];
				}
				diffy = p1 - p2;
				if (Neg)diffy = -diffy;
				double diff = sqrt(diffx*diffx + diffy*diffy);
				if (bSQRT || bSQRT2)diff = sqrt(diff);
				if (bSQRT2)diff = sqrt(diff);
				if (bDE && v != 0.0)
				{
					double col = diff*SlopePower*FrZoom / div;
					if (bSQRT || bSQRT2)col = col/sqrt(FrZoom);
					if (bSQRT2)col = col/sqrt(sqrt(FrZoom));
					if (col >= nbc - 1)col = bDEtronc ? 0 : nbc - 1;
					double r = col - (int)col;
					int ncol1 = ((int)col) % nbc;
					int ncol2 = ncol1 + 1;
					if (ncol2 == nbc)ncol2 = 0;

					double r1 = 1.0 - r;
					prgb->r = (unsigned char)(r1*(int)(t[ncol1].r) + r*(int)(t[ncol2].r));
					prgb->g = (unsigned char)(r1*(int)(t[ncol1].g) + r*(int)(t[ncol2].g));
					prgb->b = (unsigned char)(r1*(int)(t[ncol1].b) + r*(int)(t[ncol2].b));
				}
				else
				{
					double ad = atan(diff*SlopePower*FrZoom);
					double phi;
					if (diffx == 0.0)phi = 0;
					else phi = atan(diffy / diffx);
					if (diffx < 0.0)phi += PI;

					double dlum =sin(phi)*SlopeX + cos(phi)*SlopeY;
					dlum *= SlopeRatio;					
					if (dlum>0)
					{
						dlum = dlum*sin(ad);
						if(bPhong)
						{
							double ad2 = 2 * ad - PI + lightPhongElev;
							double ad2ps = ad2*PhongSize;
							if(abs(ad2ps)<PIs2)
							{
								double dphi=abs(lightAngleRadians-phi);
								if (dphi>PI)dphi -= 2 * PI;
								double dphips=dphi*PhongSize;
								if(abs(dphips)<PIs2)
								{
								dlum+=cos(dphips)*cos(ad2ps)*PhongRatio;
								if(dlum>1.0)dlum=1.0;
								}
							}
						}
						if (bPhong2)
						{
							double ad2 = 2 * ad - PI + lightPhongElev2;
							double ad2ps = ad2*PhongSize2;
							if (abs(ad2ps)<PIs2)
							{
								double dphi = abs(lightAngleRadians - phi);
								if (dphi>PI)dphi -= 2 * PI;
								double dphips = dphi*PhongSize2;
								if (abs(dphips)<PIs2)
								{
									dlum += cos(dphips)*cos(ad2ps)*PhongRatio2;
									if (dlum>1.0)dlum = 1.0;
								}
							}
						}
						prgb->r = (1 - dlum)*prgb->r + dlum * 255;
						prgb->g = (1 - dlum)*prgb->g + dlum * 255;
						prgb->b = (1 - dlum)*prgb->b + dlum * 255;
					}
					else
					{
						dlum = -dlum*sin(ad);
						prgb->r = (1 - dlum)*prgb->r;
						prgb->g = (1 - dlum)*prgb->g;
						prgb->b = (1 - dlum)*prgb->b;
					}
				}
			}
			prgb++;
		}
	}
	SetEvent(ColorbmpBitsEvents[no]);
}

void ColorbmpBits(BYTE *bmpBits, double **pb, int row, double div, double addcur, int nbcol)
{
	TCBP.bmpBits = bmpBits;
	TCBP.pb = pb;
	TCBP.row = row;
	TCBP.div = div;
	TCBP.addcur = addcur;
	TCBP.nbcol = nbcol;

	if (b3D)rShAngle = PI*(90-ShadowAngle) / 180;
	if (bSlope)
	{
		lightPhongElev = PI*PhongElev / 180;
		lightPhongElev2 = PI*PhongElev2 / 180;
		lightAngleRadians = PI*SlopeAngle2 / 180;
		SlopeX = sin(lightAngleRadians);
		SlopeY = cos(lightAngleRadians);
	}
	SYSTEM_INFO si;
	GetSystemInfo(&si);
	int np = si.dwNumberOfProcessors;
	if (np > 32)np = 32;
	for (int i = 0; i < np; i++)
	{
		ColorbmpBitsEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
		HANDLE hThread = (HANDLE)_beginthread((void(_cdecl*)(void *))ThColorbmpBits, 8 * 1024, (void *)(i + (np << 16)));
		SetThreadAffinityMask(hThread, (i == 0)?0:(1<<(i-1)));
		SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
	}
	WaitForMultipleObjects(np,ColorbmpBitsEvents,TRUE,INFINITE);
	for (int i = 0; i < np; i++)CloseHandle(ColorbmpBitsEvents[i]);
}

double MinIter2(int px, int py, int cx, int cy, double **pbuffer)
{
	double v, min = 1.0E100;
	for (int j = py; j < py + cy; j++)
	{
		for (int i = px; i < px + cx; i++)
		{
			v = pbuffer[j][i];
			if (v != 0.0 && v < min)min = v;
		}
	}
	return(min-0.5);
}

double MinIter(int px, int py, int cx, int cy, double **pbuffer)
{
	double v, min = 1.0E100;
	for (int i = px; i < px + cx; i++)
	{
		v = pbuffer[py][i];
		if (v != 0.0 && v<min)min = v;
	}
	int py2 = py + cy - 1;
	for (int i = px; i < px + cx; i++)
	{
		v = pbuffer[py2][i];
		if (v != 0.0 && v<min)min = v;
	}
	for (int i = py; i < py + cy; i++)
	{
		v = pbuffer[i][px];
		if (v != 0.0 && v<min)min = v;
	}
	int px2 = px + cx - 1;
	for (int i = py; i < py + cy; i++)
	{
		v = pbuffer[i][px2];
		if (v != 0.0 && v<min)min = v;
	}
	return(min-0.5);
}

void ReverseY(double **pb)
{
	for (int i = 0; i<Height / 2; i++)for (int j = 0; j<Width; j++)
	{
		double v;
		v = pb[i][j];
		pb[i][j] = pb[Height - i - 1][j];
		pb[Height -i - 1][j] = v;
	}
}

void ReverseX(double **pb)
{
	for(int i=0;i<Height;i++)for(int j=0;j<Width/2;j++)
	{
	double v;
	v=pb[i][j];
	pb[i][j]=pb[i][Width-j-1];
	pb[i][Width-j-1]=v;
	}
}

float ThisFrame = -1.0;
void MakeThisFrame(float x)
{
	ThisFrame = x;
	MakeFinalMovie();
}

void Rotate(double **pbi, double **pbo)
{
	int H2=Height / 2;
	int W2=Width / 2;
	for (int i = 0; i < Height; i++)
	{
		for (int j = 0; j < Width; j++)
		{
			int i2, j2;
			double di2, dj2;
			di2 = H2 + ((i - H2)*cRotation + (j - W2)*sRotation) / 1.5;
			i2 = (int)di2;
			if (i2 < 0)continue;
			if (i2 >= Height)continue;
			dj2 = W2 + ((j - W2)*cRotation - (i - H2)*sRotation) / 1.5;
			j2 = (int)dj2;
			if (j2 < 0)continue;
			if (j2 >= Width)continue;
			if (i2 < Height - 1 && j2 < Width - 1)
			{
				double Vij, Vi1j, Vij1;
				double Ri, Rj;
				Ri = di2 - i2;
				Rj = dj2 - j2;
				Vij = pbi[i2][j2];
				if (Vij == 0.0){ pbo[i][j] = 0.0; continue; }
				Vi1j = pbi[i2 + 1][j2];
				if (Vi1j == 0.0){ pbo[i][j] = Vij; continue; }
				Vij1 = pbi[i2][j2 + 1];
				if (Vij1 == 0.0){ pbo[i][j] = Vij; continue; }
				pbo[i][j] = (Vij*(1 - Ri) + Vi1j*Ri + Vij*(1 - Rj) + Vij1*Rj) / 2;
			}
			else pbo[i][j] = pbi[i2][j2];
		}
	}
}

void InitPerspective(int x0, int y0, int x1, int y1, int x2, int y2, int x3, int y3, double &hA, double &hB, double &hC, double &hD, double &hE, double &hF, double &hG, double &hH, double &hI)
{
	double dx1, dy1, dx2, dy2, dx3, dy3;
	dx1 = x1 - x2;
	dy1 = y1 - y2;
	dx2 = x3 - x2;
	dy2 = y3 - y2;
	dx3 = x0 - x1 + x2 - x3;
	dy3 = y0 - y1 + y2 - y3;

	double h11, h21, h31, h12, h22, h32, h13, h23, h33;
	double chkDenom;
	chkDenom = (dx1 * dy2 - dy1 * dx2);
	if (chkDenom == 0)  chkDenom = 0.000000001;

	h13 = (dx3 * dy2 - dx2 * dy3) / chkDenom;
	h23 = (dx1 * dy3 - dy1 * dx3) / chkDenom;
	h11 = x1 - x0 + h13 * x1;
	h21 = x3 - x0 + h23 * x3;
	h31 = x0;
	h12 = y1 - y0 + h13 * y1;
	h22 = y3 - y0 + h23 * y3;
	h32 = y0;
	h33 = 1;

	hA = h22 * h33 - h32 * h23;
	hB = h31 * h23 - h21 * h33;
	hC = h21 * h32 - h31 * h22;
	hD = h32 * h13 - h12 * h33;
	hE = h11 * h33 - h31 * h13;
	hF = h31 * h12 - h11 * h32;
	hG = h12 * h23 - h22 * h13;
	hH = h21 * h13 - h11 * h23;
	hI = h11 * h22 - h21 * h12;
}

HBITMAP ShrinkBitmap(HBITMAP bmSrc, int nNewWidth, int nNewHeight)
{
	EnterCriticalSection(&cs);
	HDC hDC = GetDC(NULL);
	BITMAP bm;
	GetObject(bmSrc, sizeof(BITMAP), &bm);
	HDC dcSrc = CreateCompatibleDC(hDC);
	HBITMAP bmOldSrc = (HBITMAP)SelectObject(dcSrc, bmSrc);
	HDC dcDst = CreateCompatibleDC(hDC);
	HBITMAP bmDst = CreateCompatibleBitmap(hDC, nNewWidth, nNewHeight);
	HBITMAP bmOldDst = (HBITMAP)SelectObject(dcDst, bmDst);
	SetStretchBltMode(dcDst, HALFTONE);
	StretchBlt(dcDst, 0, 0, nNewWidth, nNewHeight, dcSrc, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);
	SelectObject(dcDst, bmOldDst);
	SelectObject(dcSrc, bmOldSrc);
	DeleteDC(dcDst);
	DeleteDC(dcSrc);
	ReleaseDC(NULL, hDC);
	LeaveCriticalSection(&cs);
	return bmDst;
}

void AppendFrame(CAviFile *pAvi, HBITMAP bmBmp)
{
	BITMAP bm;
	GetObject(bmBmp, sizeof(BITMAP), &bm);
	if (!bAntiAlias)
	{
		pAvi->AppendNewFrame(bmBmp);
		return;
	}
	HBITMAP bmAppend = ShrinkBitmap(bmBmp, VideoWidth / 2, VideoHeight / 2);
	pAvi->AppendNewFrame(bmAppend);
	DeleteObject(bmAppend);
}

BYTE *bmpBits = NULL;
BYTE *bmpBitsPlane = NULL;
BYTE *bmpBits3D = NULL;
BYTE *bmpBits3DC = NULL;
double **bufferZ = NULL;
double **bufferZ2;
double **MG = NULL;
int inbp2;
int **bufferSh = NULL;
int *pid = NULL;

void ThGaussianSmoothingML(void *arg)
{
	int na = (int)arg;
	int np = na >> 16;
	int no = na & 0xFF;
	int i, j;
	for (i = no; i < VideoHeightPlaneC; i+=np)
	{
		for (j = 0; j < VideoWidthC; j++)
		{
			double sz = 0.0;
			double sc = 0.0;
			if (bufferZ2[i][j] == 1.0E100)continue;
			for (int k = -inbp2; k < 1 + inbp2; k++)
			{
				for (int l = -inbp2; l < 1 + inbp2; l++)
				{
					int ik = i + k;
					int jl = j + l;

					if (ik >= 0 && ik < VideoHeightPlaneC && jl >= 0 && jl < VideoWidthC)
					{
						double v = bufferZ2[ik][jl];
						if (v == 1.0E100)continue;
						double c = MG[k + inbp2][l + inbp2];
						sz += v * c;
						sc += c;
					}
				}
			}
			if (sc != 0.0)bufferZ[i][j] = sz / sc;
		}
	}
	SetEvent(ColorbmpBitsEvents[no]);
}
void GaussianSmoothing(double zmax)
{
	int i,j;
	double nbpLiss3Dn = nbpLiss3D*VideoWidth / 1920;
	double kg = -log(0.02) / (nbpLiss3Dn*nbpLiss3Dn);
	inbp2 = (int)(nbpLiss3Dn + 0.5);
	int nbp = 2 * inbp2 + 1;
	static double mnbpLiss3D = 0.0;
	if (nbp > 2)
	{
		if (mnbpLiss3D != nbpLiss3Dn)
		{
			if (MG)
			{
				for (i = 0; i < 2 * (int)(mnbpLiss3D + 0.5) + 1; i++)free(MG[i]);
				free(MG);
			}
			MG = NULL;
			mnbpLiss3D = nbpLiss3Dn;
		}
		if (MG == NULL)
		{
			MG = (double **)malloc(nbp*sizeof(double *));
			for (i = 0; i < nbp; i++)
			{
				MG[i] = (double *)malloc(nbp*sizeof(double));
				for (j = 0; j < nbp; j++)
				{
					MG[i][j] = exp(-kg*((i - inbp2)*(i - inbp2) + (j - inbp2)*(j - inbp2)));
				}
			}
		}
		bufferZ2 = (double **)malloc(VideoHeightPlaneC*sizeof(double *));
		for (i = 0; i < VideoHeightPlaneC; i++)
		{
			bufferZ2[i] = (double *)malloc(VideoWidthC*sizeof(double));
			memcpy(bufferZ2[i], bufferZ[i], VideoWidthC*sizeof(double));
		}
		SYSTEM_INFO si;
		GetSystemInfo(&si);
		int np = si.dwNumberOfProcessors;
		if (np > 32)np = 32;
		for (int i = 0; i < np; i++)
		{
			ColorbmpBitsEvents[i] = CreateEvent(NULL, FALSE, FALSE, NULL); // reuse EVENT
			HANDLE hThread = (HANDLE)_beginthread((void(_cdecl*)(void *))ThGaussianSmoothingML, 8 * 1024, (void *)(i + (np << 16)));
			SetThreadAffinityMask(hThread, (i == 0) ? 0 : (1 << (i - 1)));
			SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
		}
		WaitForMultipleObjects(np, ColorbmpBitsEvents, TRUE, INFINITE);
		for (int i = 0; i < np; i++)CloseHandle(ColorbmpBitsEvents[i]);

		for (i = 0; i < VideoHeightPlaneC; i++)
		{
			free(bufferZ2[i]);
			for (j = 0; j < VideoWidthC; j++)
			if (bufferZ[i][j] == 1.0E100)bufferZ[i][j] = zmax;
		}
		free(bufferZ2);
	}
}

void MedianFilter(double **pb)
{
	double v[9],**pb2;
	pb2 = (double **)malloc(sizeof(double **)*Height);
	for (int i = 0; i < Height; i++)
	{
		pb2[i] = (double *)malloc(sizeof(double)* Width);
		memcpy(pb2[i], pb[i], sizeof(double)* Width);
	}
	for (int i = 0; i < Height; i++)for (int j = 0; j < Width; j++)
	{
		int n = 0;
		for (int i2 = i - 1; i2 <= i + 1; i2++)for (int j2 = j - 1; j2 <= j + 1; j2++)
		if (i2 >= 0 && i2 < Height && j2 >= 0 && j2 < Width)
		{
			double t = pb2[i2][j2];
			if (t == 0.0)t = 1.0E100;
			v[n++] = t;
		}
		for (int k = 0; k < n/2+1; k++)
		{
			int min = k;
			for (int l = k + 1; l < n; l++)if (v[l] < v[min])min = l;
			if (min != k){ double t = v[k]; v[k] = v[min]; v[min] = t; }
		}
		if (n == 4)pb[i][j] = v[1];
		else if (n == 6)pb[i][j] = v[2];
		else pb[i][j] = v[4];
		if (pb[i][j] == 1.0E100)pb[i][j] = 0.0;
	}
	for (int i = 0; i < Height; i++)free(pb2[i]);
	free(pb2);
}

double EvEchZ3Dcur;
double EvMaxIter3Dcur;
double EvVertDec3Dcur;
double EvRot3Dcur;
double minmoy, minmoy3D, min3D;
int i1min3D, i2min3D;
FILE *fz = NULL;
void MakeFrame(float x, float cdiv, float addcur, HDC *phdc, HBITMAP *phbmp, LPBITMAPINFO pbmi, RGB **pline, HBITMAP *phbmp3D, LPBITMAPINFO pbmi3D, RGB **pline3D, HBITMAP *phbmp3DC, LPBITMAPINFO pbmi3DC, RGB **pline3DC)
{
	static HDC hdc=NULL;
	static BITMAPINFOHEADER bmi = { 0 };
	static HBITMAP hbmp = NULL;
	static RGB *line;
	static int row, rowPlane, row3D, row3DC, nbcol, nofilm, posbuffer, nbframe;
	static char  folder[MAX_PATH];
	clock_t t0, t1, t2, t3, t4, t5;
	double div;
	double min = 0;
	char src[MAX_PATH], film[MAX_PATH], txt[96];
	int ndiv;
	long pos;
	int n = (int)x;
	float alfa = x - n;
	FILE *f = NULL;
	double znitermax = 0.0;
	if (mNoFrame == -1)
	{
		nofilm = 1; nbframe = 0; minmoy3D = 0.0; i1min3D = 0; i2min3D = VideoHeightPlaneC;
		WideCharToMultiByte(CP_ACP, 0, g_szFolder, MAX_PATH, folder, MAX_PATH, NULL, NULL);
		strcat_s(folder, "\\");
		strcpy_s(src, folder);
		strcat_s(src, lKFY[n]);
		fopen_s(&f, src, "rb");
		fread(&Width, sizeof(int), 1, f);
		fread(&Height, sizeof(int), 1, f);
		pos = 27692L;
		fseek(f, pos, SEEK_SET);
		fread(&cxPreview, sizeof(int), 1, f);
		fread(&cyPreview, sizeof(int), 1, f);
		fread(&nbcol, sizeof(int), 1, f);
		ndiv = lKFYmem[0]->ndiv;
		trgb = (RGB*)malloc(nbcol*sizeof(RGB));
		posbuffer = pos + 12 + 3 * nbcol + cxPreview * cyPreview * 8;
		fread(trgb, sizeof(RGB), nbcol, f);
		fseek(f, posbuffer, SEEK_SET);
		pbuffer1 = (double **)malloc(Height*sizeof(double *));
		pbuffer2 = (double **)malloc(Height*sizeof(double *));
		pbuffer3 = (double **)malloc(Height*sizeof(double *));
		pbuffer4 = (double **)malloc(Height*sizeof(double *));
		for (int i = 0; i < Height; i++)
		{
			pbuffer1[i] = (double *)malloc(sizeof(double)* Width);
			pbuffer2[i] = (double *)malloc(sizeof(double)* Width);
			fread(pbuffer2[i], sizeof(double), Width, f);
			pbuffer3[i] = (double *)malloc(sizeof(double)* Width);
			pbuffer4[i] = (double *)malloc(sizeof(double)* Width);
		}
		fclose(f);
		if (bMedian)MedianFilter(pbuffer2);
		if(bReverseX)ReverseX(pbuffer2);
		if (bReverseY)ReverseY(pbuffer2);
		strcpy_s(src, folder);
		strcat_s(src, lKFY[n + 1]); // Pb sur derniers KeyFrames
		fopen_s(&f, src, "rb");
		fseek(f, posbuffer, SEEK_SET);
		for (int i = 0; i < Height; i++)fread(pbuffer3[i], sizeof(double), Width, f);
		fclose(f);
		if (bMedian)MedianFilter(pbuffer3);
		if (bReverseX)ReverseX(pbuffer3);
		if (bReverseY)ReverseY(pbuffer3);
		strcpy_s(src, folder);
		strcat_s(src, lKFY[n + 2]); // Pb sur derniers KeyFrames
		fopen_s(&f, src, "rb");
		fseek(f, posbuffer, SEEK_SET);
		for (int i = 0; i < Height; i++)fread(pbuffer4[i], sizeof(double), Width, f);
		fclose(f);
		if (bMedian)MedianFilter(pbuffer4);
		if (bReverseX)ReverseX(pbuffer4);
		if (bReverseY)ReverseY(pbuffer4);
		if (b3D)*phbmp = CreateBitmap(VideoWidthC, VideoHeightPlaneC, pline, phdc, (BITMAPINFOHEADER *)pbmi);
		else *phbmp = CreateBitmap(VideoWidth, VideoHeightPlane, pline, phdc, (BITMAPINFOHEADER *)pbmi);		
		if (b3D)
		{
			*phbmp3D = CreateBitmap(VideoWidth, VideoHeight, pline3D, phdc, (BITMAPINFOHEADER *)pbmi3D); 
			*phbmp3DC = CreateBitmap(VideoWidthC, VideoHeightC, pline3DC, phdc, (BITMAPINFOHEADER *)pbmi3DC);
		}
		SetStretchBltMode(*phdc, HALFTONE);
		if (hbmp == NULL || bmi.biHeight != Height || bmi.biWidth != Width)
		{
			hbmp = CreateBitmap(Width, Height, &line, &hdc, (BITMAPINFOHEADER *)&bmi);
			free(line);
			ReleaseDC(NULL, hdc); DeleteDC(hdc);
			DeleteObject(hbmp);
			if (bmpBits)free(bmpBits);
			bmpBits = (BYTE *)malloc(bmi.biSizeImage);
			row = bmi.biSizeImage / Height;
		}

		if (b3D){ hBmp = *phbmp3D; HeightBmp = VideoHeight; WidthBmp = VideoWidth; }
		else { hBmp = *phbmp; HeightBmp = VideoHeightPlane; WidthBmp = VideoWidth; }
		strcpy_s(film, folder);
		sprintf_s(txt, "\\film(%d-%d)%03d.avi", NoSelectDeb, NoSelectFin, nofilm);
		strcat_s(film, txt);
		if (ThisFrame == -1.0)
		{
			pAvi = new CAviFile(film, 24, VideoFPS, 1, 100);
			pAvi->ChooseCodec(hwndMain, bAntiAlias ? VideoWidth / 2 : VideoWidth, bAntiAlias ? VideoHeight / 2 : VideoHeight);
		}
	}
	if (n != mNoFrame || mNoFrame == -1)
	{
		if (n + 3 < NbKFY)
		{
			strcpy_s(src, folder);
			strcat_s(src, lKFY[n + 3]);
			fopen_s(&f, src, "rb");
			fseek(f, posbuffer, SEEK_SET);
			for (int i = 0; i < Height; i++)
			{
				memcpy(pbuffer1[i], pbuffer2[i], sizeof(double)* Width);
				memcpy(pbuffer2[i], pbuffer3[i], sizeof(double)* Width);
				memcpy(pbuffer3[i], pbuffer4[i], sizeof(double)* Width);
				fread(pbuffer4[i], sizeof(double), Width, f);
			}
			fclose(f);
			if (bMedian)MedianFilter(pbuffer4);
			if(bReverseX)ReverseX(pbuffer4);
			if (bReverseY)ReverseY(pbuffer4);
		}
		else
		{
			for (int i = 0; i < Height; i++)
			{
				memcpy(pbuffer1[i], pbuffer2[i], sizeof(double)* Width);
				memcpy(pbuffer2[i], pbuffer3[i], sizeof(double)* Width);
				memcpy(pbuffer3[i], pbuffer4[i], sizeof(double)* Width);
			}
			for (int i = 0; i < Height; i += 2)
			{
				for (int j = 0; j < Width; j += 2)
				{
					double v = pbuffer3[Height / 4 + i / 2][Width / 4 + j / 2];
					pbuffer4[i][j] = pbuffer4[i][j + 1] = pbuffer4[i + 1][j] = pbuffer4[i + 1][j + 1] = v;
				}
			}
		}
		mNoFrame = n;
	}
	t0 = clock();
	double ratio = exp(-alfa * 0.693147180559);
//----Rotation----
	if (bRotation)
	{
		sRotation = sin(RotationAngle*PI / 180);
		cRotation = cos(RotationAngle*PI / 180);
		if (!bPostRotation)
		{
			if (pbufferR1 == NULL)
			{
				pbufferR1 = (double **)malloc(Height*sizeof(double *));
				pbufferR2 = (double **)malloc(Height*sizeof(double *));
				pbufferR3 = (double **)malloc(Height*sizeof(double *));
				pbufferR4 = (double **)malloc(Height*sizeof(double *));
				for (int i = 0; i < Height; i++)
				{
					pbufferR1[i] = (double *)malloc(sizeof(double)* Width);
					memcpy(pbufferR1[i], pbuffer1[i], sizeof(double)* Width);
					pbufferR2[i] = (double *)malloc(sizeof(double)* Width);
					memcpy(pbufferR2[i], pbuffer2[i], sizeof(double)* Width);
					pbufferR3[i] = (double *)malloc(sizeof(double)* Width);
					memcpy(pbufferR3[i], pbuffer3[i], sizeof(double)* Width);
					pbufferR4[i] = (double *)malloc(sizeof(double)* Width);
					memcpy(pbufferR4[i], pbuffer4[i], sizeof(double)* Width);
				}
			}
			Rotate(pbuffer1, pbufferR1);
			Rotate(pbuffer2, pbufferR2);
			Rotate(pbuffer3, pbufferR3);
			Rotate(pbuffer4, pbufferR4);
		}
		else sRotation = -sRotation;
	}
	int cx = (int)Width*ratio;
	int cy = (int)Height*ratio;
	int px = Width*(1-ratio)/2;
	int py = Height*(1-ratio)/2;
	
	int nbc = (UsePalette) ? 512 : nbcol;
	div = (float)(cdiv * 1024 / nbc);
	double addcur2 = addcur;
	addcur2 *= nbc;
	if (b3D)
	{
		min3D = MinIter2(px, py + ratio*(i1min3D*Height) / VideoHeightPlaneC, cx, ratio*((i2min3D - i1min3D)*Height) / VideoHeightPlaneC, (bRotation && !bPostRotation)? pbufferR1 : pbuffer1);
		if (minmoy3D == 0.0)minmoy3D = min3D; else minmoy3D = (minmoy3D*(nbminmoy - 1) + min3D) / nbminmoy;
		if (SubIM)addcur2 -= minmoy3D / div;
	}
	else
	{
		if (SubIM)
		{
			min = MinIter(px, py, cx, cy, (bRotation && !bPostRotation)? pbufferR1 : pbuffer1);
			if (min > 1e90)min = minmoy;
			if (minmoy == 0.00)minmoy = min; else minmoy = (minmoy*(nbminmoy - 1) + min) / nbminmoy;
			addcur2 -= minmoy / div;
		}
	}
	while (addcur2 < 0)addcur2 += nbc;
	float zm = lKFYmem[n]->zoomM;
	int ze = lKFYmem[n]->zoomE;
	FrZoom = ratio;
	ColorbmpBits(bmpBits, (bRotation && !bPostRotation)? pbufferR1 : pbuffer1, row, div, addcur2, nbc);
	EnterCriticalSection(&cs);
	HGDIOBJ ho= SelectObject(*phdc, *phbmp);
	StretchDIBits(*phdc, 0, 0, VideoWidthC, VideoHeightPlaneC, px, py, cx, cy, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, SRCCOPY);
	SelectObject(*phdc, ho);
	LeaveCriticalSection(&cs);
	if (!bKeyFrame)
	{
		FrZoom *= 2;
		ColorbmpBits(bmpBits, (bRotation && !bPostRotation)? pbufferR2 : pbuffer2, row, div, addcur2, nbc);
		double ratio2 = 0.5 / ratio;
		EnterCriticalSection(&cs);
		ho = SelectObject(*phdc, *phbmp);
		double dpx, dpy;
		dpx = (VideoWidthC / 2)*(1 - ratio2);
		px = (int)ceil(dpx);
		dpy = (VideoHeightPlaneC / 2)*(1 - ratio2);
		py = (int)ceil(dpy);
		cx = VideoWidthC - 2 * px;
		cy = VideoHeightPlaneC - 2 * py;
		int pxb = (int)((px - dpx)*Width / (double)cx);
		int cxb = Width - 2 * pxb;
		int pyb = (int)((py - dpy)*Height / (double)cy);
		int cyb = Height - 2 * pyb;
		StretchDIBits(*phdc, px, py, cx, cy, pxb, pyb, cxb, cyb, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(*phdc, ho);
		LeaveCriticalSection(&cs);
		FrZoom *= 2;
		ColorbmpBits(bmpBits, (bRotation && !bPostRotation)? pbufferR3 : pbuffer3, row, div, addcur2, nbc);
		ratio2 = 0.25 / ratio;
		EnterCriticalSection(&cs);
		ho = SelectObject(*phdc, *phbmp);
		dpx = (VideoWidthC / 2)*(1 - ratio2);
		px = (int)ceil(dpx);
		dpy = (VideoHeightPlaneC / 2)*(1 - ratio2);
		py = (int)ceil(dpy);
		cx = VideoWidthC - 2 * px;
		cy = VideoHeightPlaneC - 2 * py;
		pxb = (int)((px - dpx)*Width / (double)cx);
		cxb = Width - 2 * pxb;
		pyb = (int)((py - dpy)*Height / (double)cy);
		cyb = Height - 2 * pyb;
		StretchDIBits(*phdc, px, py, cx, cy, pxb, pyb, cxb, cyb, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(*phdc, ho);
		LeaveCriticalSection(&cs);
		FrZoom *= 2;
		ColorbmpBits(bmpBits, (bRotation && !bPostRotation)? pbufferR4 : pbuffer4, row, div, addcur2, nbc);
		ratio2 = 0.125 / ratio;
		EnterCriticalSection(&cs);
		ho = SelectObject(*phdc, *phbmp);
		dpx = (VideoWidthC / 2)*(1 - ratio2);
		px = (int)ceil(dpx);
		dpy = (VideoHeightPlaneC / 2)*(1 - ratio2);
		py = (int)ceil(dpy);
		cx = VideoWidthC - 2 * px;
		cy = VideoHeightPlaneC - 2 * py;
		pxb = (int)((px - dpx)*Width / (double)cx);
		cxb = Width - 2 * pxb;
		pyb = (int)((py - dpy)*Height / (double)cy);
		cyb = Height - 2 * pyb;
		StretchDIBits(*phdc, px, py, cx, cy, pxb, pyb, cxb, cyb, bmpBits, (LPBITMAPINFO)&bmi, DIB_RGB_COLORS, SRCCOPY);
		SelectObject(*phdc, ho);
		LeaveCriticalSection(&cs);
	}
//---------------3D---------------
	t1 = clock();
	if (b3D)
	{
		if (!bmpBitsPlane)
		{
			bmpBitsPlane = (BYTE *)malloc(pbmi->bmiHeader.biSizeImage);
			rowPlane = pbmi->bmiHeader.biSizeImage / VideoHeightPlaneC;
		}
		ho = SelectObject(*phdc, *phbmp);
		GetDIBits(*phdc, *phbmp, 0, VideoHeightPlaneC, bmpBitsPlane, pbmi, DIB_RGB_COLORS);
		SelectObject(*phdc, ho);
		if (!bmpBits3DC)
		{
			bmpBits3DC = (BYTE *)malloc(pbmi3DC->bmiHeader.biSizeImage);
			row3DC = pbmi3DC->bmiHeader.biSizeImage / VideoHeightC;
		}
		if (!bufferZ)
		{
			bufferZ = (double **)malloc(VideoHeightPlaneC*sizeof(double *));
			for (int i = 0; i < VideoHeightPlaneC; i++)bufferZ[i] = (double *)malloc(VideoWidthC*sizeof(double));
		}

		{
			int i, j, i1, j1, i2, j2, i3, j3, i4, j4, idest, id2, idb2;
			double raa;
			double vhr = (VideoHeightPlaneC / ratio);
			double vwr = (VideoWidthC / ratio);
			double dest, z;
			BYTE *pbi, *pbo;

			raa = ((ratio * Width) / VideoWidth);
			double MaxIterCur = maxniter3D*EvMaxIter3Dcur;
			for (i = 0; i < VideoHeightPlaneC; i++)
			{
				i1 = (i - VideoHeightPlaneC / 2)*Height;
				i2 = i1 * 2;
				i3 = i2 * 2;
				i4 = i3 * 2;
				i1 = (int)(i1 / vhr);
				i2 = (int)(i2 / vhr);
				i3 = (int)(i3 / vhr);
				i4 = (int)(i4 / vhr);
				i1 += Height / 2;
				i2 += Height / 2;
				i3 += Height / 2;
				i4 += Height / 2;
				for (j = 0; j < VideoWidthC; j++)
				{
					j1 = (j - VideoWidthC / 2)*Width;
					j2 = j1 * 2;
					j3 = j2 * 2;
					j4 = j3 * 2;
					j1 = (int)(j1 / vwr);
					j2 = (int)(j2 / vwr);
					j3 = (int)(j3 / vwr);
					j4 = (int)(j4 / vwr);
					j1 += Width / 2;
					j2 += Width / 2;
					j3 += Width / 2;
					j4 += Width / 2;
					double ni, niter = 1.0E100;
					if (!bKeyFrame)
					{
						if (i4 >= 0 && i4 < Height && j4 >= 0 && j4 < Width)
						{
							for (int ip = 0; ip < raa * 8; ip++)for (int jp = 0; jp < raa * 8; jp++)
							if ((i4 + ip) < Height && (j4 + jp) < Width){ ni = (bRotation && !bPostRotation)? pbufferR4[i4 + ip][j4 + jp] : pbuffer4[i4 + ip][j4 + jp]; if (ni != 0.0 && ni < niter)niter = ni; }
						}
						else if (i3 >= 0 && i3 < Height && j3 >= 0 && j3 < Width)
						{
							for (int ip = 0; ip < raa * 4; ip++)for (int jp = 0; jp < raa * 4; jp++)
							if ((i3 + ip) < Height && (j3 + jp) < Width){ ni = (bRotation && !bPostRotation)? pbufferR3[i3 + ip][j3 + jp] : pbuffer3[i3 + ip][j3 + jp]; if (ni != 0.0 && ni < niter)niter = ni; }
						}
						else if (i2 >= 0 && i2 < Height && j2 >= 0 && j2 < Width)
						{
							for (int ip = 0; ip < raa * 2; ip++)for (int jp = 0; jp < raa * 2; jp++)
							if ((i2 + ip) < Height && (j2 + jp) < Width){ ni = (bRotation && !bPostRotation)? pbufferR2[i2 + ip][j2 + jp] : pbuffer2[i2 + ip][j2 + jp]; if (ni != 0.0 && ni < niter)niter = ni; }
						}
						else if (i1 >= 0 && i1 < Height && j1 >= 0 && j1 < Width)
						{
							for (int ip = 0; ip < raa; ip++)for (int jp = 0; jp < raa; jp++)
							if ((i1 + ip) < Height && (j1 + jp) < Width){ ni = (bRotation && !bPostRotation)? pbufferR1[i1 + ip][j1 + jp] : pbuffer1[i1 + ip][j1 + jp]; if (ni != 0.0 && ni < niter)niter = ni; }
						}
					}
					else if (i1 >= 0 && i1 < Height && j1 >= 0 && j1 < Width)
					{
						for (int ip = 0; ip < raa; ip++)for (int jp = 0; jp < raa; jp++)
						if ((i1 + ip) < Height && (j1 + jp) < Width){ ni = pbuffer1[i1 + ip][j1 + jp]; if (ni != 0.0 && ni < niter)niter = ni; }
					}
					if (niter == 1.0E100)bufferZ[i][j] = niter;
					else
					{
						if (niter<minmoy3D)niter = minmoy3D;
						if (niter - minmoy3D > MaxIterCur)niter = minmoy3D + MaxIterCur;
						if (bLog3D)
						{
							if (bSqrt3D)z = sqrt(log(2 + niter - minmoy3D));
							else z = log(1 + niter - minmoy3D);
						}
						else
						{
							z = niter - minmoy3D;
							if (bSqrt3D)z = sqrt(z + 1);
						}
						if (bAtan3D)bufferZ[i][j] = 0.63661977*atan(z / MaxIterCur)*MaxIterCur;
						else bufferZ[i][j] = z;
					}
				}
			}
			if (bLog3D)
			{
				if (bSqrt3D)znitermax = sqrt(log(2 + minmoy3D + MaxIterCur - minmoy3D));
				else znitermax = log(1 + minmoy3D + MaxIterCur - minmoy3D);
			}
			else
			{
				if (bSqrt3D)znitermax = sqrt(1 + minmoy3D + MaxIterCur - minmoy3D);
				else znitermax = MaxIterCur;
			}
			if (bAtan3D)znitermax = 0.63661977*atan(znitermax / MaxIterCur)*MaxIterCur;
			//------------- Lissage bufferZ -----------
			t2 = clock();
			GaussianSmoothing(znitermax);
			//-------------Shadow---------------------// lightAngleRadians, lightPhongElev
			t3 = clock();
			double CaeZ3D = Calpha3D * echZ3D * EvEchZ3Dcur * VideoWidth / 1920.0;
			if (ShadowCoef < 1.00)
			{
				if (!bufferSh)
				{
					bufferSh = (int **)malloc(VideoHeightPlaneC*sizeof(int *));
					for (int i = 0; i < VideoHeightPlaneC; i++)bufferSh[i] = (int *)malloc(VideoWidthC*sizeof(int));
				}
				for (i = 0; i < VideoHeightPlaneC; i++)for (j = 0; j < VideoWidthC; j++)bufferSh[i][j] = 0;
				double SAR = abs(sin(lightAngleRadians));
				double CAR = abs(cos(lightAngleRadians));
				double TSA = abs(tan(rShAngle));
				double TSACaeZ3D = TSA*CaeZ3D;
				double h, l;
				int ik, jk, mik, mjk;
				if (lightAngleRadians >= 0)
				{
					if (lightAngleRadians < PIs2) //+45°
					{
						for (i = VideoHeightPlaneC - 1; i >0; i--)
						{
							for (j = VideoWidthC - 1; j > 0; j--)
							{
								if (bufferSh[i][j] >= MAXSH)continue;
								h = bufferZ[i][j];
								l = h*TSACaeZ3D;
								mik = i;
								mjk = j;
								if (l > 0)
								{
									for (int k = 1; k <= l; k++)
									{
										ik = i - (int)(k*CAR);
										jk = j - (int)(k*SAR);
										if (ik < 0)break;
										if (jk < 0)break;
										if (ik != mik || jk != mjk)
										{
											if (bufferZ[ik][jk] > h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
								else
								{
									for (int k = 1; k <= -l; k++)
									{
										ik = i - (int)(k*CAR);
										jk = j - (int)(k*SAR);
										if (ik < 0)break;
										if (jk < 0)break;
										if (ik != mik || jk != mjk)
										{
											if (-bufferZ[ik][jk] > -h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
							}
						}
					}
					else //+135°
					{
						for (i = 0; i <VideoHeightPlaneC; i++)
						{
							for (j = VideoWidthC - 1; j >0; j--)
							{
								if (bufferSh[i][j] >= MAXSH)continue;
								h = bufferZ[i][j];
								l = h*TSACaeZ3D;
								mik = i;
								mjk = j;
								if (l > 0)
								{
									for (int k = 1; k <= l; k++)
									{
										ik = i + (int)(k*CAR);
										jk = j - (int)(k*SAR);
										if (ik >= VideoHeightPlaneC)break;
										if (jk <0)break;
										if (ik != mik || jk != mjk)
										{
											if (bufferZ[ik][jk] > h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
								else
								{
									for (int k = 1; k <= -l; k++)
									{
										ik = i + (int)(k*CAR);
										jk = j - (int)(k*SAR);
										if (ik >= VideoHeightPlaneC)break;
										if (jk <0)break;
										if (ik != mik || jk != mjk)
										{
											if (-bufferZ[ik][jk] > -h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
							}
						}
					}
				}
				else
				{
					if (lightAngleRadians > -PIs2) //-45°
					{
						for (i = VideoHeightPlaneC - 1; i > 0; i--)
						{
							for (j = 0; j < VideoWidthC; j++)
							{
								if (bufferSh[i][j] >= MAXSH)continue;
								h = bufferZ[i][j];
								l = h*TSACaeZ3D;
								mik = i;
								mjk = j;
								if (l>0)
								{
									for (int k = 1; k <= l; k++)
									{
										ik = i - (int)(k*CAR);
										jk = j + (int)(k*SAR);
										if (ik < 0)break;
										if (jk >= VideoWidthC)break;
										if (ik != mik || jk != mjk)
										{
											if (bufferZ[ik][jk] > h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
								else
								{
									for (int k = 1; k <= -l; k++)
									{
										ik = i - (int)(k*CAR);
										jk = j + (int)(k*SAR);
										if (ik < 0)break;
										if (jk >= VideoWidthC)break;
										if (ik != mik || jk != mjk)
										{
											if (-bufferZ[ik][jk] > -h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
							}
						}
					}
					else //-135°
					{
						for (i = 0; i < VideoHeightPlaneC; i++)
						{
							for (j = 0; j < VideoWidthC; j++)
							{
								if (bufferSh[i][j] >= MAXSH)continue;
								h = bufferZ[i][j];
								l = h*TSACaeZ3D;
								mik = i;
								mjk = j;
								if (l > 0)
								{
									for (int k = 1; k <= l; k++)
									{
										ik = i + (int)(k*CAR);
										jk = j + (int)(k*SAR);
										if (ik >= VideoHeightPlaneC)break;
										if (jk >= VideoWidthC)break;
										if (ik != mik || jk != mjk)
										{
											if (bufferZ[ik][jk] > h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
								else
								{
									for (int k = 1; k <= -l; k++)
									{
										ik = i + (int)(k*CAR);
										jk = j + (int)(k*SAR);
										if (ik >= VideoHeightPlaneC)break;
										if (jk >= VideoWidthC)break;
										if (ik != mik || jk != mjk)
										{
											if (-bufferZ[ik][jk] > -h*(1 - k / l))continue;
											mik = ik;
											mjk = jk;
											bufferSh[ik][jk]++;
										}
									}
								}
							}
						}
					}

				}
			}
			//------------- Affichage -----------------
			t4 = clock();
			memset(bmpBits3DC,0,pbmi3DC->bmiHeader.biSizeImage); //Raz Bitmap
			int VHm1 = VideoHeightC - 1;
			i1min3D = VideoHeightPlaneC;
			i2min3D = 0;
			int W2 = VideoWidthC / 2;
			int H2 = VideoHeightPlaneC / 2;
			double coefSh[MAXSH];
			for (i = 0; i < MAXSH; i++)coefSh[i] = exp(((i + 1)*log(ShadowCoef)) / MAXSH);
			if (!pid) pid = (int *)malloc(VideoWidthC*sizeof(int));
			int x0 = 0, y0 = 0,
				x1 = VideoWidthC, y1 = 0,
				x2 = VideoWidthC + VideoWidthC*perspectiveAmount, y2 = VideoHeightPlaneC,
				x3 = -VideoWidthC*perspectiveAmount, y3 = VideoHeightPlaneC;
			double hA, hB, hC, hD, hE, hF, hG, hH, hI;
			InitPerspective(x0, y0, x1, y1, x2, y2, x3, y3, hA, hB, hC, hD, hE, hF, hG, hH, hI);
/*
char trace[256];
sprintf_s(trace, "PA:%4.2lf => hA=%4.2lf hB=%4.2lf hC=%4.2lf hD=%4.2lf hE=%4.2lf hF=%4.2lf hG=%4.2lf hH=%4.2lf hI=%4.2lf", perspectiveAmount, hA, hB, hC, hD, hE, hF, hG, hH, hI);
Trace(trace);

 Video 480x270
PA:0.50 => hA=240.00 hB=120.00 hC=-0.00 hD=-0.00 hE=480.00 hF=0.00 hG=0.00 hH=240.00 hI=115200.00
PA:1.00 => hA=160.00 hB=160.00 hC=-0.00 hD=-0.00 hE=480.00 hF=0.00 hG=0.00 hH=320.00 hI=76800.00
PA:0.00 => hA=480.00 hB=-0.00 hC=0.00 hD=-0.00 hE=480.00 hF=0.00 hG=0.00 hH=0.00 hI=230400.00
chkDenom = (hH * i + hI);
j2 = VideoWidthC * (hA * j + hB * i) / chkDenom;  // dj2/dj = VideoWidthC*hA/(hH * i + hI) dj2/di = VideoWidthC*hB/(hH * i + hI)  zm = z/(dj2/dj) = z * (hH * i + hI)/(hA*VideoWidthC)
i2 = VideoHeightPlaneC * ( hE * i) / chkDenom;    // di2/di = VideoHeightPlaneC*hE/(hH * i + hI) di2/dj = 0
*/					
			for (i = 0; i < VideoHeightPlaneC; i++)
			{
				double VHPiVH = (VideoHeightPlaneC / 2 - i) * Salpha3D + VideoHeightC / 2 - decH3D*EvVertDec3Dcur*VideoWidthC / 1920.0;
				for (j = 0; j < VideoWidthC; j++)
				{
					RGB rgb;
					double dx = 0, dy = 0;
					int rj;
					int ri;
					double chkDenom = (hG * j + hH * i + hI);
					double j2 = VideoWidthC * (hA * j + hB * i + hC) / chkDenom;
					double i2 = VideoHeightPlaneC * (hD * j + hE * i + hF) / chkDenom;
					BOOL bInterpol=FALSE;
					if (bPostRotation)
					{
						bInterpol=TRUE;
						double coef = 0.71;
						if (perspectiveAmount >= 0.001)coef = coef/perspectiveScale;
						dx = W2 + ((j2 - W2)*cRotation + (i2 - H2)*sRotation)*coef;
						dy = H2 + ((i2 - H2)*cRotation - (j2 - W2)*sRotation)*coef;
						rj = (int)dx;
						ri = (int)dy;
						if (rj<0 || rj >= VideoWidthC || ri<0 || ri >= VideoHeightPlaneC)continue;
					}
					else
					{
						if (perspectiveAmount < 0.001){ i2 = i; j2 = j; }
						else
						{ 
							bInterpol=TRUE;
							dx = W2 + (j2 - W2)*0.71/perspectiveScale;
							dy = H2 + (i2 - H2)*0.71/perspectiveScale;
							rj = (int)dx;
							ri = (int)dy;
							if (ri<0 || rj<0 || ri >= VideoWidthC || rj >= VideoHeightPlaneC)continue;
						}
					}
					double h;
					if (bPostRotation || perspectiveAmount >= 0.001)h = bufferZ[ri][rj] * CaeZ3D *((hH * i + hI) / (hA*VideoWidthC));
					else h = bufferZ[i][j] * CaeZ3D;
					dest = VHPiVH + h;
					idest = (int)dest;
					id2 = pid[j];
					idb2 = idest + 1;
					pid[j] = idest;
					if (i == 0)continue;
					if (id2<0)continue;
					if (idb2 < 0)idb2 = 0;
					if (idb2 > VHm1)continue;
					if (id2 > VHm1)id2 = VHm1;
					if(bInterpol)
					{
						int R, Rx, Ry, Rz, G, Gx, Gy, Gz, B, Bx, By, Bz;
						double X, Xn, Y, Yn, Z, Zn;
						Xn = dx - rj;
						X = 1 - Xn;
						Yn = dy - ri;
						Y = 1 - Yn;
						Zn = (Xn + Yn) / 2;
						Z = 1 - Zn;
						pbi = bmpBitsPlane + rowPlane*ri + rj * 3; // Rotate
						R = ((RGB *)pbi)->r;
						G = ((RGB *)pbi)->g;
						B = ((RGB *)pbi)->b;
						rj++;
						if (rj<0 || rj >= VideoWidthC || ri<0 || ri >= VideoHeightPlaneC)
						{
							Rx = R;
							Gx = G;
							Bx = B;
						}
						else
						{
							pbi = bmpBitsPlane + rowPlane*ri + rj * 3;
							Rx = ((RGB *)pbi)->r;
							Gx = ((RGB *)pbi)->g;
							Bx = ((RGB *)pbi)->b;
						}
						rj--;
						ri++;
						if (rj<0 || rj >= VideoWidthC || ri<0 || ri >= VideoHeightPlaneC)
						{
							Ry = R;
							Gy = G;
							By = B;
						}
						else
						{
							pbi = bmpBitsPlane + rowPlane*ri + rj * 3;
							Ry = ((RGB *)pbi)->r;
							Gy = ((RGB *)pbi)->g;
							By = ((RGB *)pbi)->b;
						}
						rj++;
						if (rj<0 || rj >= VideoWidthC || ri<0 || ri >= VideoHeightPlaneC)
						{
							Rz = R;
							Gz = G;
							Bz = B;
						}
						else
						{
							pbi = bmpBitsPlane + rowPlane*ri + rj * 3;
							Rz = ((RGB *)pbi)->r;
							Gz = ((RGB *)pbi)->g;
							Bz = ((RGB *)pbi)->b;
						}
						rgb.r = (int)(((double)R*X + (double)Rx*Xn + (double)R*Y + (double)Ry*Yn + (double)R*Z + (double)Rz*Zn) / 3);
						rgb.g = (int)(((double)G*X + (double)Gx*Xn + (double)G*Y + (double)Gy*Yn + (double)G*Z + (double)Gz*Zn) / 3);
						rgb.b = (int)(((double)B*X + (double)Bx*Xn + (double)B*Y + (double)By*Yn + (double)B*Z + (double)Bz*Zn) / 3);
						rj--;
						ri--;
						rgb = *((RGB *)pbi);
					}
					else
					{
						pbi = bmpBitsPlane + rowPlane*(int)i2 + (int)j2 * 3;
						rgb = *((RGB *)pbi);
					}
					if (bufferSh)
					{
						int iSh;
						if(bPostRotation)iSh= bufferSh[ri][rj];
						else iSh = bufferSh[(int)i2][(int)j2];
						if (iSh > MAXSH)iSh = MAXSH;
						if (iSh)
						{
							iSh--;
							rgb.r *= coefSh[iSh];
							rgb.g *= coefSh[iSh];
							rgb.b *= coefSh[iSh];
						}
					}
					pbo = bmpBits3DC + row3DC*idb2 + j * 3;
					if (i2 < i1min3D)i1min3D = i2;
					if (i2 > i2min3D)i2min3D = i2;
					for (int id = idb2; id <= id2; id++)
					{
						*((RGB *)pbo) = rgb;
						pbo += row3DC;
					}
				}
			}
		}
		// Crop 3DC -> 3D
		if (!bmpBits3D)
		{
			bmpBits3D = (BYTE *)malloc(pbmi3D->bmiHeader.biSizeImage);
			row3D = pbmi3D->bmiHeader.biSizeImage / VideoHeight;
		}
		BYTE *pbi, *pbo;
		pbi = bmpBits3DC;
		pbo = bmpBits3D;
		int a, b;
		if (bCrop3D && CropX3D >= 0)a = CropX3D; else a = 0;
		if (bCrop3D && CropY3D >= 0)b = CropY3D; else b = 0;
		for (int i = 0; i < VideoHeight; i++)
			for (int j = 0; j < VideoWidth; j++)
			{
				pbi = bmpBits3DC + (i + b)*row3DC + (j + a) * 3;
				pbo = bmpBits3D + i*row3D + j * 3;
				*((RGB *)pbo) = *((RGB *)pbi);
			}
	}
	t5 = clock();
	EnterCriticalSection(&cs);
	ho = SelectObject(*phdc, *phbmp3D);
	SetDIBits(*phdc, *phbmp3D, 0, VideoHeight, bmpBits3D, pbmi3D, DIB_RGB_COLORS);
	SelectObject(*phdc, ho);
	LeaveCriticalSection(&cs);
	//-----Zoom-------
	if (VideoZoomText)
	{
		static TCHAR tzoom[32];
		zm /= (float)ratio;
		if (zm > (float)10.0){ zm = zm / (float)10.0; ze++; }
		if (ze == -1){ zm /= 10; ze = 0; }
		if (ze == 0)sprintf_s(txt, "Zoom : %4.2f", (double)zm);
		else if (ze == 1)sprintf_s(txt, "Zoom : %4.1f", (double)zm*10);
		else sprintf_s(txt, "Zoom : %4.2fE%d", (double)zm, ze);
		MultiByteToWideChar(CP_ACP, 0, txt, 64, tzoom, 32);
		if (b3D)SetZoomText(*phbmp3D, (LPCWSTR)tzoom);
		else SetZoomText(*phbmp, (LPCWSTR)tzoom);
	}
//------Affichage, AVI --------------------
	InvalidateRect(hwndMain, NULL, FALSE);
	if (ThisFrame == -1.0)
	{
		AppendFrame(pAvi, (b3D) ? *phbmp3D : *phbmp);
		nbframe++;
		if (nbframe == VideoFPM)
		{
			CAviFile *pOld = pAvi;
			nofilm++; nbframe = 0;
			strcpy_s(film, folder);
			sprintf_s(txt, "film(%d-%d)%03d.avi",NoSelectDeb, NoSelectFin, nofilm);
			strcat_s(film, txt);
			pAvi = new CAviFile(film, 24, VideoFPS, 1, 100, pOld->GetCodec());
			delete pOld;
		}
	}
	TCHAR wtxt[96];
	sprintf_s(txt, " - Frame: %4.3f - film(%d-%d)%03d-%03d", x, NoSelectDeb, NoSelectFin, nofilm, nbframe);
	MultiByteToWideChar(CP_ACP, 0, txt, 96, wtxt, 96);
	TCHAR Ttitre[300];
	wcscpy_s(Ttitre, L"Movie Maker - ");
	wcscat_s(Ttitre, g_szFolder);
	wcscat_s(Ttitre, wtxt);
	sprintf_s(txt, " C(%4.2lfS) Z(%4.2lfS) G(%4.2lfS) S(%4.2lf) D(%4.2lfS) R:%4.1lf° min:%3.1lf/%3.1lf", (double)(t1 - t0) / CLOCKS_PER_SEC, (double)(t2 - t1) / CLOCKS_PER_SEC, (double)(t3 - t2) / CLOCKS_PER_SEC, (double)(t4 - t3) / CLOCKS_PER_SEC, (double)(t5 - t4) / CLOCKS_PER_SEC,RotationAngle,min3D,minmoy3D);
	MultiByteToWideChar(CP_ACP, 0, txt, 96, wtxt, 96);
	wcscat_s(Ttitre, wtxt);
	SetWindowText(hwndMain, (LPWSTR)Ttitre);
}

void ThFinalVideo(void)
{
	HDC hdc = NULL;
	BITMAPINFOHEADER bmi, bmi3D, bmi3DC;
	HBITMAP hbmp = NULL;
	HBITMAP hbmp3D = NULL, hbmp3DC = NULL;
	RGB *line = NULL;
	RGB *line3D = NULL, *line3DC = NULL;

	Load3D("3D.kfs");
	if (bKeyFrame)
	{
		InsertFPoint(&lSpeed, 0, 1);
	}
	else
	{
		LoadFPoints("speed.kfs", &lSpeed);
		if (lSpeed.NbPoints == 0)InsertFPoint(&lSpeed, 0, 20);
	}
	LoadFPoints("colordiv.kfs", &lColorDiv);
	int idiv=10;
	if (KFYmemLu)idiv = lKFYmem[0]->ndiv;
	if (lColorDiv.NbPoints == 0)InsertFPoint(&lColorDiv, 0.0, (float)idiv);
	LoadFPoints("coloradd.kfs", &lColorAdd);
	if (lColorAdd.NbPoints == 0)InsertFPoint(&lColorAdd, 0, 0);
	LoadFPoints("colorrot.kfs", &lColorRot);
	if (lColorRot.NbPoints == 0)InsertFPoint(&lColorRot, 0, 0);

	LoadFPoints("EvEchZ3D.kfs", &lEvEchZ3D);
	if (lEvEchZ3D.NbPoints == 0)InsertFPoint(&lEvEchZ3D, 0, 1.0);
	LoadFPoints("EvMaxIter3D.kfs", &lEvMaxIter3D);
	if (lEvMaxIter3D.NbPoints == 0)InsertFPoint(&lEvMaxIter3D, 0, 1.0);
	LoadFPoints("EvVertDec3D.kfs", &lEvVertDec3D);
	if (lEvVertDec3D.NbPoints == 0)InsertFPoint(&lEvVertDec3D, 0, 1.0);
	if (bRotation)
	{
		LoadFPoints("EvRot3D.kfs", &lEvRot3D);
		if (lEvRot3D.NbPoints == 0)InsertFPoint(&lEvRot3D, 0, 0.0);
	}
	VideoHeightPlane = (VideoWidth * Height) / Width;
	if (bCrop3D)
	{
		VideoWidthC = VideoWidth + abs(CropX3D);
		VideoHeightC = VideoHeight + abs(CropY3D);
		VideoHeightPlaneC = VideoHeightPlane + abs(CropY3D);
	}
	else
	{
		VideoWidthC = VideoWidth;
		VideoHeightC = VideoHeight;
		VideoHeightPlaneC = VideoHeightPlane;
	}
	LoadRGBPos("palette.kfs", &lRGBPos);
	if (UsePalette)MakeTrgbPal(RoundedPalette);
	LoadSubMin("SubIM.kfs");
	LoadRelief("Relief.kfs");
	Load3D("3D.kfs");
	float xdeb, xfin, xcur, ydeb, yfin, ycur, divcur, addcur, rotcur;
	int fini = 0;
	TRotCur = 0;
	minmoy = 0.00;
	RotationAngle = RotationStart;
	EvRot3Dcur = 0.0;
	SlopeAngle2 = SlopeAngle;
	xcur = lSpeed.fPoint[0].x;
	ycur = lSpeed.fPoint[0].y;
	double ratio = 1.0;
	for (int i = 0; i < lSpeed.NbPoints; i++)
	{
		xdeb = lSpeed.fPoint[i].x;
		ydeb = lSpeed.fPoint[i].y;
		if (i == lSpeed.NbPoints - 1)
		{
			xfin = (float)NoSelectFin;
			yfin = lSpeed.fPoint[i].y;
		}
		else
		{
			xfin = lSpeed.fPoint[i + 1].x;
			yfin = lSpeed.fPoint[i + 1].y;
		}
		while (xcur < xfin)
		{
			if (bKeyFrame)ycur = 1;
			else ycur = exp(log(ydeb) + (log(yfin) - log(ydeb))*(xcur - xdeb) / (xfin - xdeb));
			if (xcur> NoSelectFin || xcur >= NbKFY - 1 || VideoAbort){ fini = 1; break; }
			if (xcur > NoSelectDeb)
			{
				char txt[64];
				divcur = RetrieveFPoint(xcur, &lColorDiv, TRUE);
				addcur = RetrieveFPoint(xcur, &lColorAdd, FALSE);
				rotcur = RetrieveFPoint(xcur, &lColorRot, FALSE);
				if (b3D)
				{
					EvEchZ3Dcur = RetrieveFPoint(xcur, &lEvEchZ3D, TRUE);
					EvMaxIter3Dcur = RetrieveFPoint(xcur, &lEvMaxIter3D, TRUE);
					EvVertDec3Dcur = RetrieveFPoint(xcur, &lEvVertDec3D, TRUE);
					if(bRotation)EvRot3Dcur = RetrieveFPoint(xcur, &lEvRot3D, FALSE);
				}
				TRotCur += rotcur / ycur;
				if (ThisFrame != -1.0)
				{
					if (xcur > ThisFrame)
					{
						MakeFrame(xcur, divcur*ColSensitivity, (addcur + TRotCur/2) / ColSensitivity, &hdc, &hbmp, (LPBITMAPINFO)&bmi, &line, &hbmp3D, (LPBITMAPINFO)&bmi3D, &line3D, &hbmp3DC, (LPBITMAPINFO)&bmi3DC, &line3DC);
						fini = 1; break;
					}
				}
				else MakeFrame(xcur, divcur*ColSensitivity, (addcur + TRotCur/2) / ColSensitivity, &hdc, &hbmp, (LPBITMAPINFO)&bmi, &line, &hbmp3D, (LPBITMAPINFO)&bmi3D, &line3D, &hbmp3DC, (LPBITMAPINFO)&bmi3DC, &line3DC);
			}
			if (fini)break;
			xcur += 1 / ycur;
			if (bRotation)
			{
				RotationAngle += EvRot3Dcur;
				if (RotationAngle > 180)RotationAngle -= 360;
				if (RotationAngle < -180)RotationAngle += 360;
			}
			if (bRotation && bPostRotation)
			{
				SlopeAngle2 -= EvRot3Dcur;
				if (SlopeAngle2 > 180)SlopeAngle2 -= 360;
				if (SlopeAngle2 < -180)SlopeAngle2 += 360;
			}
		}
		if (fini)break;
	}
	DestroyFPoint(&lSpeed);
	DestroyFPoint(&lColorDiv);
	DestroyFPoint(&lColorAdd);
	DestroyFPoint(&lEvEchZ3D);
	DestroyFPoint(&lEvMaxIter3D);
	DestroyFPoint(&lEvVertDec3D);
	if (bRotation)DestroyFPoint(&lEvRot3D);
	DeleteDC(hdc);
	free(line);
	DeleteObject(hbmp);
	if (b3D)
	{
		free(line3D);
		DeleteObject(hbmp3D);
		free(bmpBitsPlane);
		bmpBitsPlane = NULL;
		free(bmpBits3D);
		bmpBits3D = NULL;
		free(line3DC);
		DeleteObject(hbmp3DC);
		free(bmpBits3DC);
		bmpBits3DC = NULL;
		for (int i = 0; i < VideoHeightPlaneC; i++)free(bufferZ[i]);
		bufferZ=NULL;
		if (bufferSh)
		  { for (int i = 0; i < VideoHeightPlaneC; i++)free(bufferSh[i]); bufferSh = NULL; }
		free(pid);
		pid = NULL;
	}
	if (ThisFrame == -1.0)delete pAvi;
	if (bRotation && !bPostRotation)
	{
		for (int i = 0; i < Height; i++)
		{
			free(pbufferR1[i]);
			free(pbufferR2[i]);
			free(pbufferR3[i]);
			free(pbufferR4[i]);
		}
		free(pbufferR1);
		free(pbufferR2);
		free(pbufferR3);
		free(pbufferR4);
		pbufferR1 = NULL; pbufferR2 = NULL; pbufferR3 = NULL; pbufferR4 = NULL;
	}
	for (int i = 0; i < Height; i++)
	{
		free(pbuffer1[i]);
		free(pbuffer2[i]);
		free(pbuffer3[i]);
		free(pbuffer4[i]);
	}
	free(pbuffer1);
	free(pbuffer2);
	free(pbuffer3);
	free(pbuffer4);
	mNoFrame = -1;
	hBmp = NULL;
	HMENU hm = GetMenu(hwndMain);
	EnableMenuItem(hm, 0, MF_ENABLED | MF_BYPOSITION);
	EnableMenuItem(hm, 1, MF_ENABLED | MF_BYPOSITION);
	EnableMenuItem(hm, IDM_MAKEVIDEO, MF_ENABLED);
	EnableMenuItem(hm, IDM_ABORTVIDEO, MF_DISABLED);
	if (ThisFrame != -1.0)ThisFrame = -1.0;
	else
	{
		InvalidateRect(hwndMain, NULL, TRUE);
		if (!VideoAbort)MessageBox(hwndMain, L"Movie is done!", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
	}
}

void MakeFinalMovie()
{
	HMENU hm = GetMenu(hwndMain);
	EnableMenuItem(hm, 0, MF_DISABLED | MF_BYPOSITION);
	EnableMenuItem(hm, 1, MF_DISABLED | MF_BYPOSITION);
	EnableMenuItem(hm, IDM_MAKEVIDEO, MF_DISABLED);
	EnableMenuItem(hm, IDM_ABORTVIDEO, MF_ENABLED);
	VideoAbort = FALSE;
	_beginthread((void(_cdecl*)(void *))ThFinalVideo, 8 * 1024, NULL);
}

INT_PTR CALLBACK SpeedSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int CurPos, MaxPos, zoom, cxPosit, cyPosit, mNo, npDrag;
	static RECT rectGraph;
	static HBRUSH hb1 = NULL, hb2 = NULL;
	static HPEN hp1 = NULL, hp2 = NULL;
	static HFONT hf1 = NULL;
	static BOOL dragCP, VertZoom=TRUE;
	static double EchBase = 40.0;
	static char *nomkfs;
	HWND hwndImg1, hwndImg2, hwndGraph, hwndPosit;
	PAINTSTRUCT ps;
	HDC hdc;
	POINTS PosSouris;
	int cxb, cyb, nb;
	int shift, ctrl;
	switch (message)
	{
	case WM_PAINT:
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		hdc = BeginPaint(hwndGraph, &ps);
		ExcludeClipRect(hdc, -30, 0, 0, rectGraph.bottom-rectGraph.top);
		ExcludeClipRect(hdc, rectGraph.right - rectGraph.left -4, 0, rectGraph.right - rectGraph.left + 30, rectGraph.bottom - rectGraph.top);
		ExcludeClipRect(hdc, -30, -rectGraph.top, rectGraph.right + 30, 0);
		ExcludeClipRect(hdc, -30, rectGraph.bottom - rectGraph.top, rectGraph.right + 30, rectGraph.bottom + 20);
		POINT Point;
		HGDIOBJ hs;
		int l, h;
		RECT rect;
		h = rectGraph.bottom - rectGraph.top - 4;
		l = rectGraph.right - rectGraph.left - 4;
		rect.top = 0; rect.left = 0; rect.bottom = h; rect.right = l;
		if (hb1 == NULL)hb1 = CreateSolidBrush(RGB(221, 239, 255)); //RGB(187, 223, 255)
		FillRect(hdc, &rect, hb1);
		if (hp1 == NULL)hp1 = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
		if (hf1 == NULL)hf1=CreateFont(12, 5, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, (LPCTSTR)"Arial");
		hs = SelectObject(hdc, hp1);
		SetBkColor(hdc, (bEvRot)?RGB(221, 239, 255):RGB(255, 221, 239));  
		nb = NoSelectFin + 1 - CurPos - NoSelectDeb;
		if (bEvRot)
		{
			MoveToEx(hdc, 2, h/2 , &Point);
			LineTo(hdc, l - 2, h/2);
		}
		else
		{
			MoveToEx(hdc, 2, h - 2, &Point);
			LineTo(hdc, l - 2, h - 2);
		}
		for (int i = 0; i < nb; i++)
		{
			int nbz = 1;
			int p = 2 + zoom * 4 * i;
			SelectObject(hdc, hp1);
			MoveToEx(hdc, p, 2, &Point);
			LineTo(hdc, p, h - 2);
			if (zoom < 6)nbz = 5;
			if (zoom < 3)nbz = 10;
			TCHAR txt[8];
			if ((NoSelectDeb+CurPos+i)%nbz == 0)
			{
				wsprintf(txt, L"%d", i + CurPos + NoSelectDeb);
				SelectObject(hdc, hf1);
				TextOut(hdc, p + 2, 12, (LPWSTR)txt, _tcslen(txt));
			}
			if (p>l)break;
		}
		if (hp2 == NULL)hp2 = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
		SelectObject(hdc, hp2);
		float x, y, xn, yn;
		int nf;
		int px0, py0, px, py;
		nf = NearestFPoint(&lSpeed, (float)NoSelectDeb + CurPos, &xn,&yn);
		GetFPoint(&lSpeed, nf, &x, &y);
		px0 = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
		if (bEvRot)
		{
			if (VertZoom)py0 = h/2 - (int)(h*y * 10 / EchBase);
			else py0 = h/2 - (int)(h*y / EchBase);
		}
		else
		{
			if (VertZoom)py0 = h - 2 - (int)(h*log(y * 10 / EchBase) / 5);
			else py0 = h - 2 - (int)(h*log(y / EchBase) / 5);
		}
		MoveToEx(hdc, px0, py0, &Point);
		rect.top = py0 - 3; rect.left = px0 - 3; rect.bottom = py0 + 3; rect.right = px0 + 3;
		if (hb2 == NULL)hb2 = CreateSolidBrush(RGB(120, 120, 120));
		FillRect(hdc, &rect, hb2);
		for (int i = nf + 1; i < lSpeed.NbPoints; i++)
		{
			GetFPoint(&lSpeed, i, &x, &y);
			px =(int)( (x - NoSelectDeb - CurPos)*zoom * 4) + 2;
			if (bEvRot)
			{
				if (VertZoom)py = h / 2 - (int)(h*y * 10 / EchBase);
				else py = h / 2 - (int)(h*y / EchBase);
			}
			else
			{
				if (VertZoom)py = h - 2 - (int)(h*log(y * 10 / EchBase) / 5);
				else py = h - 2 - (int)(h*log(y / EchBase) / 5);
			}
			LineTo(hdc, px, py);
			px0 = px;
			py0 = py;
			rect.top = py - 3; rect.left = px - 3; rect.bottom = py + 3; rect.right = px + 3;
			FillRect(hdc, &rect, hb2);
			if (px>l)break;
		}
		px = l - 2;
		py = py0;
		LineTo(hdc, px, py);
		SelectObject(hdc, hs);
		EndPaint(hwndGraph, &ps);
		break;
	case WM_RBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		ctrl = wParam & MK_CONTROL;
		PosSouris = MAKEPOINTS(lParam);
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			float x, y;
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (bEvRot)y = EchBase * (float)(rectGraph.bottom - 2 - PosSouris.y - h/2) / h;
			else y = EchBase * exp(5 * (float)(rectGraph.bottom - 2 - PosSouris.y) / h);
			if (VertZoom)y /= 10;
			if (shift && !ctrl)
			{
				int np = NearestFPoint(&lSpeed, x + (float)0.55 / (float)zoom, &xn, &yn);
				{ InsertFPoint(&lSpeed, x, yn); InvalidateRect(hDlg, NULL, FALSE); }
			}
		}
		break;
	case WM_LBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		ctrl = wParam & MK_CONTROL;
		PosSouris = MAKEPOINTS(lParam);
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			float x, y;
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (bEvRot)y = EchBase * (float)(rectGraph.bottom - 2 - PosSouris.y - h / 2) / h;
			else y = EchBase * exp(5 * (float)(rectGraph.bottom - 2 - PosSouris.y) / h);
			if (VertZoom)y /= 10;
			if (!shift && !ctrl)
			{
				float xn, yn;
				int np = NearestFPoint(&lSpeed, x+(float)0.55/(float)zoom, &xn, &yn);
				if (abs(x - xn) < 0.55 / zoom && ((!bEvRot)?abs(y / yn - 1):abs(y-yn)) < 0.20)
				{
					dragCP = true;
					npDrag = np;
				}
			}
			if (shift && !ctrl)
			{
				InsertFPoint(&lSpeed, x, y);
				InvalidateRect(hDlg, NULL, FALSE);
			}
			if (ctrl && !shift)
			{
				int np = NearestFPoint(&lSpeed, x + (float)0.55 / (float)zoom, &xn, &yn);
				if (abs(x - xn) < 0.55 / zoom && ((!bEvRot)?abs(y / yn - 1):abs(y-yn)) < 0.20){ DeleteFPoint(&lSpeed, np); InvalidateRect(hDlg, NULL, FALSE); }
			}
		}
		break;
	case WM_LBUTTONUP:
		dragCP = false;
		PosSouris = MAKEPOINTS(lParam);
		break;
	case WM_MOUSEMOVE:
		hwndPosit = GetDlgItem(hDlg, IDC_POSIT);
		PosSouris = MAKEPOINTS(lParam);
		int no;
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			TCHAR ctext[32];
			char text[32];
			float x, y;
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (bEvRot)y = EchBase * (float)(rectGraph.bottom - 2 - PosSouris.y - h / 2) / h;
			else y = EchBase * exp(5 * (float)(rectGraph.bottom - 2 - PosSouris.y) / h);
			if (VertZoom)y /= 10;
			sprintf_s(text, "%5.2f, %6.3f", x, y);
			MultiByteToWideChar(CP_ACP, 0, text, 32, ctext, 32);
			SetDlgItemText(hDlg, IDC_POSIT, ctext);
			SetWindowPos(hwndPosit, HWND_TOP, PosSouris.x - cxPosit / 2, PosSouris.y - cyPosit - 2, cxPosit, cyPosit, SWP_SHOWWINDOW);
			if (dragCP){ SetFPoint(&lSpeed, npDrag, x, y); orgOneFrame = SPEED; npDragOneFrame = npDrag; InvalidateRect(hDlg, NULL, FALSE); }
			else 
			{
				SetDlgItemText(hDlg, IDC_TIME, L"");
				SetDlgItemText(hDlg, IDC_TTIME, L"");
				if (bEvRot)
				{
					TCHAR ctext[32];
					char text[32];
					float anglec = 0.0, anglet = 0.0;
					double ev;
					SetDlgItemText(hDlg, IDC_TTIME, L"Angle Mouse / Sequence");
					for (int i = NoSelectDeb*VideoFPS; i < NoSelectFin*VideoFPS; i++)
					{
						float xc = (float)i / VideoFPS;
						ev = RetrieveFPoint(xc, &lSpeed, FALSE);
						if(xc<=x)anglec+=ev;
						anglet += ev;
					}
					sprintf_s(text, "%6.2f, %6.2f",anglec,anglet);
					MultiByteToWideChar(CP_ACP, 0, text, 32, ctext, 32);
					SetDlgItemText(hDlg, IDC_TIME, ctext);
				}
				if (bEvEchZ3D == FALSE && bEvMaxIter3D == FALSE && bEvVertDec3D == FALSE && bEvRot == FALSE)
				{
					int nbframes = 0, nbframes2 = 0;
					int fini;
					double time, time2, ft, ft2;
					int sec, sec2, minut, minut2, ift, ift2;
					TCHAR txt[64];


					float xdeb, xfin, xcur, ydeb, yfin, ycur;
					xcur = lSpeed.fPoint[0].x;
					ycur = lSpeed.fPoint[0].y;
					fini = 0; nbframes = 0;
					for (int i = 0; i < lSpeed.NbPoints; i++)
					{
						xdeb = lSpeed.fPoint[i].x;
						ydeb = lSpeed.fPoint[i].y;
						if (i < lSpeed.NbPoints - 1)
						{
							xfin = lSpeed.fPoint[i + 1].x;
							yfin = lSpeed.fPoint[i + 1].y;
						}
						else
						{
							xfin = (float)(NbKFY - 1);
							yfin = (float)lSpeed.fPoint[i].y;
						}
						while (xcur < xfin)
						{
							xcur += 1 / ycur;
							ycur = exp(log(ydeb) + (log(yfin) - log(ydeb))*(xcur - xdeb) / (xfin - xdeb));
							if (xcur >= NoSelectFin){ fini = 1; break; }
							if (xcur >= NoSelectDeb)
							{
								if (xcur <= x)nbframes2++;
								nbframes++;
							}
						}
					}
					time = (double)nbframes / VideoFPS;
					sec = (int)time;
					ft = time - sec;
					ift = (int)(ft * 100);
					minut = sec / 60;
					sec = sec % 60;
					time2 = (double)nbframes2 / VideoFPS;
					sec2 = (int)time2;
					ft2 = time2 - sec2;
					ift2 = (int)(ft2 * 100);
					minut2 = sec2 / 60;
					sec2 = sec2 % 60;
					wsprintf(txt, L"%02d:%02d.%02d / %02d:%02d.%02d", minut2, sec2, ift2, minut, sec, ift);
					SetDlgItemText(hDlg, IDC_TIME, txt);
				}
				orgOneFrame = SPEED; xOneFrame = x;
			}
			no = (int)x;
			if (no != mNo)
			{
				mNo = no;
				hwndImg1 = GetDlgItem(hDlg, IDC_IMGKF1);
				hwndImg2 = GetDlgItem(hDlg, IDC_IMGKF2);
				if (no >= NbKFY - 1){ no = NbKFY - 1; bmpImgKF(hwndImg1, hwndImg2, no, no); }
				else bmpImgKF(hwndImg1, hwndImg2, no, no + 1);
			}
		}
		else SetWindowPos(hwndPosit, HWND_TOP, 0, 0, cxPosit, cyPosit, SWP_HIDEWINDOW | SWP_NOMOVE);
		break;
	case WM_INITDIALOG:
		if (bEvEchZ3D)
		{
			SetWindowText(hDlg, L"Z Scale evolution");
			SetDlgItemText(hDlg, IDC_COMMENT, L"Z Scale relative to base");
			EchBase = 10.0;
			nomkfs = "EvEchZ3D.kfs";
			EnableWindow(GetDlgItem(hDlg, IDC_PREVIEW), FALSE);
		}
		else if (bEvMaxIter3D)
		{
			SetWindowText(hDlg, L"Maximum Iteration evolution");
			SetDlgItemText(hDlg, IDC_COMMENT, L"Max Iter. relative to base");
			EchBase = 10.0;
			nomkfs = "EvMaxIter3D.kfs";
			EnableWindow(GetDlgItem(hDlg, IDC_PREVIEW), FALSE);
		}
		else if (bEvVertDec3D)
		{
			SetWindowText(hDlg, L"Vertical Shift evolution");
			SetDlgItemText(hDlg, IDC_COMMENT, L"Vertical Shift relative to base");
			EchBase = 10.0;
			nomkfs = "EvVertDec3D.kfs";
			EnableWindow(GetDlgItem(hDlg, IDC_PREVIEW), FALSE);
		}
		else if (bEvRot)
		{
			SetWindowText(hDlg, L"Rotation evolution");
			SetDlgItemText(hDlg, IDC_COMMENT, L"Rotation in °/frame");
			EchBase = 10.0;
			nomkfs = "EvRot3D.kfs";
			EnableWindow(GetDlgItem(hDlg, IDC_PREVIEW), FALSE);
		}
		else
		{
			EchBase = 40.0;
			nomkfs = "speed.kfs";
		}
		LoadRGBPos("palette.kfs", &lRGBPos);
		if (UsePalette)MakeTrgbPal(RoundedPalette);
		CreatePreview(hDlg);
		_beginthread((void(_cdecl*)(void *))ThPreviewOneFrame, 8 * 1024, NULL);
		SendDlgItemMessage(hDlg, IDC_VERTZOOM, BM_SETCHECK, (VertZoom) ? BST_CHECKED : BST_UNCHECKED, 0);
		RECT rw, rc;
		GetClientRect(hDlg, &rc);
		GetWindowRect(hDlg, &rw);
		cxb = (rw.right - rw.left - rc.right) / 2;  //GetSystemMetrics(SM_CXPADDEDBORDER );
		cyb = rw.bottom - rw.top - rc.bottom - cxb; //GetSystemMetrics(SM_CYCAPTION);
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		GetWindowRect(hwndGraph, &rectGraph);
		rectGraph.top -= rw.top + cyb;
		rectGraph.bottom -= rw.top + cyb;
		rectGraph.left -= rw.left + cxb;
		rectGraph.right -= rw.left + cxb;
		l = rectGraph.right - rectGraph.left - 4;
		CurPos = 0; zoom = 10;
		MaxPos = NoSelectFin - NoSelectDeb - (l / (4 * zoom));
		if (MaxPos < 0)MaxPos = 0;
		SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_SETRANGE, TRUE, MAKELPARAM(1, 21));
		SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_SETPOS, TRUE, MAKELPARAM(10, 0));
		SetScrollRange(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, 0, MaxPos, TRUE);
		hwndPosit = GetDlgItem(hDlg, IDC_POSIT);
		GetWindowRect(hwndPosit, &rw);
		cxPosit = rw.right - rw.left; cyPosit = rw.bottom - rw.top;
		SetWindowPos(hwndPosit, HWND_TOP, 0, 0, cxPosit, cyPosit, SWP_HIDEWINDOW | SWP_NOMOVE);
		if (lSpeed.NbPoints == 0)
		{
			LoadFPoints(nomkfs, &lSpeed);
			if (lSpeed.NbPoints == 0)InsertFPoint(&lSpeed, 0.0, (bEvEchZ3D || bEvMaxIter3D || bEvVertDec3D || bEvRot)? (bEvRot)?0.0 : 1.0  : 25.0);
		}
		if (lColorAdd.NbPoints == 0)
		{
			LoadFPoints("coloradd.kfs", &lColorAdd);
			if (lColorDiv.NbPoints == 0)InsertFPoint(&lColorAdd, 0.0, 0.0);
		}
		if (lColorRot.NbPoints == 0)
		{
			LoadFPoints("colorrot.kfs", &lColorRot);
			if (lColorRot.NbPoints == 0)InsertFPoint(&lColorRot, 0.0, 0.0);
		}
		if (lColorDiv.NbPoints == 0)
		{
			int idiv;
			LoadFPoints("colordiv.kfs", &lColorDiv);
			if (KFYmemLu)idiv = lKFYmem[0]->ndiv;
			if (lColorDiv.NbPoints == 0)InsertFPoint(&lColorDiv, 0.0, (float)idiv);
		}
		LoadSubMin("SubIM.kfs");
		LoadRelief("Relief.kfs");
		Load3D("3D.kfs");
		return (INT_PTR)TRUE;
	case WM_FINPREV:
		SetDlgItemText(hDlg, IDC_PREVIEW, L"Preview");
		EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
		_beginthread((void(_cdecl*)(void *))ThPreviewOneFrame, 8 * 1024, NULL);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_VERTZOOM)
		{
			if (IsDlgButtonChecked(hDlg, IDC_VERTZOOM) == BST_CHECKED)VertZoom = TRUE;
			else VertZoom = FALSE;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_PREVIEW)
		{
			if (VideoBrowser[0] == 0)
			{
				MessageBox(hDlg, L"You must define a Video Browser\nUse menu Video->Video parameters...", L"Movie Maker", MB_ICONINFORMATION | MB_OK);
				break;
			}
			if (PreviewRun)PreviewAbort = TRUE;
			else
			{
				npDragOneFrame = -2;
				while (npDragOneFrame == -2)Sleep(30);
				Preview(hDlg);
			}
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			npDragOneFrame = -2;
			while (npDragOneFrame == -2)Sleep(30);
			DestroyFPoint(&lSpeed);
			DestroyFPoint(&lColorDiv);
			DestroyFPoint(&lColorAdd);
			DestroyFPoint(&lColorRot);
			CloseWindow(hdlgPreview);
			DestroyWindow(hdlgPreview);
			hdlgPreview = NULL;
			bEvEchZ3D = FALSE;
			bEvMaxIter3D = FALSE;
			bEvVertDec3D = FALSE;
			bEvRot = FALSE;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDOK)
		{
			npDragOneFrame = -2;
			while (npDragOneFrame == -2)Sleep(30);
			SaveFPoint(nomkfs, &lSpeed);
			DestroyFPoint(&lSpeed);
			DestroyFPoint(&lColorDiv);
			DestroyFPoint(&lColorAdd);
			DestroyFPoint(&lColorRot);
			CloseWindow(hdlgPreview);
			DestroyWindow(hdlgPreview);
			hdlgPreview = NULL;
			bEvEchZ3D = FALSE;
			bEvMaxIter3D = FALSE;
			bEvVertDec3D = FALSE;
			bEvRot = FALSE;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_HSCROLL:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_SCROLL))
		{
			CurPos = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL);
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				CurPos = 0;
				break;
			case SB_PAGEUP:
				CurPos -= 5;
				if (CurPos<0)CurPos = 0;
				break;
			case SB_PAGEDOWN:
				CurPos += 5;
				if (CurPos>MaxPos)CurPos = MaxPos;
				break;
			case SB_LINEUP:
				if (CurPos > 0)CurPos--;
				break;
			case SB_THUMBPOSITION:
				CurPos = HIWORD(wParam);
				break;
			case SB_THUMBTRACK:
				CurPos = HIWORD(wParam);
				break;
			case SB_LINEDOWN:
				if (CurPos < MaxPos)CurPos++;
				break;
			case SB_BOTTOM:
				CurPos = MaxPos;
				break;
			case SB_ENDSCROLL:
				break;
			}
			SetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, CurPos, TRUE);
		}
		else
		{
			zoom = SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_GETPOS, 0, 0L);
			int l;
			l = rectGraph.right - rectGraph.left - 4;
			MaxPos = NoSelectFin - NoSelectDeb - (l / (4 * zoom));
			if (MaxPos < 0)MaxPos = 0;
			if (CurPos > MaxPos)
			{
				CurPos = MaxPos;
				SetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, CurPos, TRUE);
			}
			SetScrollRange(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, 0, MaxPos, TRUE);
		}
		no = CurPos;
		hwndImg1 = GetDlgItem(hDlg, IDC_IMGKF1);
		hwndImg2 = GetDlgItem(hDlg, IDC_IMGKF2);
		if (no >= NbKFY - 2)no = NbKFY - 2;
		bmpImgKF(hwndImg1, hwndImg2, no, no + 1);
		InvalidateRect(hDlg, NULL, FALSE);
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ColorSetup(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int CurPos, MaxPos, VCurPos=0, VMaxPos, zoom, cxPosit, cyPosit, mNo, npDrag, DivAdd = 1;
	static RECT rectGraph;
	static HBRUSH hb1d = NULL, hb1a = NULL, hb1r = NULL, hb2 = NULL;
	static HPEN hp1 = NULL, hp2 = NULL;
	static HFONT hf1 = NULL;
	static BOOL dragCP, VertZoom;
	HWND hwndImg1, hwndImg2, hwndGraph, hwndPosit;
	PAINTSTRUCT ps;
	HDC hdc;
	POINTS PosSouris;
	int cxb, cyb, nb;
	int shift, ctrl;
	TCHAR txt[64];

	switch (message)
	{
	case WM_PAINT:
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		hdc = BeginPaint(hwndGraph, &ps);
		ExcludeClipRect(hdc, -30, 0, 0, rectGraph.bottom - rectGraph.top);
		ExcludeClipRect(hdc, rectGraph.right - rectGraph.left - 4, 0, rectGraph.right - rectGraph.left + 30, rectGraph.bottom - rectGraph.top);
		ExcludeClipRect(hdc, -30, -rectGraph.top, rectGraph.right + 30, 0);
		ExcludeClipRect(hdc, -30, rectGraph.bottom - rectGraph.top, rectGraph.right + 30, rectGraph.bottom + 20);
		POINT Point;
		HGDIOBJ hs;
		int l, h;
		RECT rect;
		h = rectGraph.bottom - rectGraph.top - 4;
		l = rectGraph.right - rectGraph.left - 4;
		rect.top = 0; rect.left = 0; rect.bottom = h; rect.right = l;
		if (hb1d == NULL)hb1d = CreateSolidBrush(RGB(221, 255, 239));
		if (hb1a == NULL)hb1a = CreateSolidBrush(RGB(255, 229, 248));
		if (hb1r == NULL)hb1r = CreateSolidBrush(RGB(229, 229, 248));
		FillRect(hdc, &rect, (DivAdd == 1) ? hb1d : ((DivAdd == 2) ? hb1r : hb1a));
		if (hp1 == NULL)hp1 = CreatePen(PS_SOLID, 1, RGB(180, 180, 180));
		if (hf1 == NULL)hf1 = CreateFont(12, 5, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH, (LPCTSTR)"Arial");
		hs = SelectObject(hdc, hp1);
		SetBkColor(hdc, (DivAdd == 1) ? RGB(221, 255, 239) : ((DivAdd == 0) ? RGB(255, 229, 248) : RGB(229, 229, 248)));
		nb = NoSelectFin + 1 - CurPos - NoSelectDeb;
		if (DivAdd==1)
		{
			MoveToEx(hdc, 2, h - 2, &Point);
			LineTo(hdc, l - 2, h - 2);
		}
		else
		{
			int y = h / 2;
			if (DivAdd == 0)if (VertZoom)y -= VCurPos*h; else y -= VCurPos*h/10;
			MoveToEx(hdc, 2, y, &Point);
			LineTo(hdc, l - 2, y);
		}

		for (int i = 0; i < nb; i++)
		{
			int nbz = 1;
			int p = 2 + zoom * 4 * i;
			SelectObject(hdc, hp1);
			MoveToEx(hdc, p, 2, &Point);
			LineTo(hdc, p, h - 2);
			if (zoom < 6)nbz = 5;
			if (zoom < 3)nbz = 10;
			TCHAR txt[8];
			if ((NoSelectDeb + CurPos + i) % nbz == 0)
			{
				wsprintf(txt, L"%d", i + CurPos + NoSelectDeb);
				SelectObject(hdc, hf1);
				TextOut(hdc, p + 2, 12, (LPWSTR)txt, _tcslen(txt));
			}
			if (p>l)break;
		}
		if (hp2 == NULL)hp2 = CreatePen(PS_SOLID, 1, RGB(120, 120, 120));
		SelectObject(hdc, hp2);
		float x, y, xn, yn;
		int nf;
		int px0, py0, px, py;
		if (DivAdd==1)
		{
			nf = NearestFPoint(&lColorDiv, (float)NoSelectDeb + CurPos, &xn, &yn);
			GetFPoint(&lColorDiv, nf, &x, &y);
			px0 = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
			if (VertZoom)py0 = h - 2 - (int)(h*log(y * 100) / 10);
			else py0 = h - 2 - (int)(h*log(y*10) / 10);
			rect.top = py0 - 3; rect.left = px0 - 3; rect.bottom = py0 + 3; rect.right = px0 + 3;
		}
		else if (DivAdd==0)
		{
			nf = NearestFPoint(&lColorAdd, (float)NoSelectDeb + CurPos, &xn, &yn);
			GetFPoint(&lColorAdd, nf, &x, &y);
			px0 = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
			if (VertZoom)py0 = (int)(h / 2 - (y + VCurPos) *h);
			else py0 = (int)(h / 2 - ((y + VCurPos) / 10)*h);
			rect.top = py0 - 3; rect.left = px0 - 3; rect.bottom = py0 + 3; rect.right = px0 + 3;
		}
		else
		{
			nf = NearestFPoint(&lColorRot, (float)NoSelectDeb + CurPos, &xn, &yn);
			GetFPoint(&lColorRot, nf, &x, &y);
			px0 = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
			if (VertZoom)py0 = (int)(h / 2 - y * 10 *h);
			else py0 = (int)(h / 2 - y *h);
			rect.top = py0 - 3; rect.left = px0 - 3; rect.bottom = py0 + 3; rect.right = px0 + 3;
		}
		MoveToEx(hdc, px0, py0, &Point);
		if (hb2 == NULL)hb2 = CreateSolidBrush(RGB(120, 120, 120));
		FillRect(hdc, &rect, hb2);
		nb = (DivAdd == 1) ? lColorDiv.NbPoints : ((DivAdd == 0) ? lColorAdd.NbPoints : lColorRot.NbPoints);
		for (int i = nf + 1; i < nb; i++)
		{
			if (DivAdd==1)
			{
				GetFPoint(&lColorDiv, i, &x, &y);
				px = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
				if (VertZoom)py = h - 2 - (int)(h*log(y * 100) / 10);
				else py = h - 2 - (int)(h*log(y*10) / 10);
			}
			else if (DivAdd == 0)
			{
				GetFPoint(&lColorAdd, i, &x, &y);
				px = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
				if (VertZoom)py = (int)(h / 2 - (y+VCurPos) *h);
				else py = (int)(h / 2 - ((y+VCurPos) / 10)*h);
			}
			else
			{
				GetFPoint(&lColorRot, i, &x, &y);
				px = (int)((x - NoSelectDeb - CurPos)*zoom * 4) + 2;
				if (VertZoom)py = (int)(h / 2 - y * 10 *h);
				else py = (int)(h / 2 - y *h);
			}
			LineTo(hdc, px, py);
			px0 = px;
			py0 = py;
			rect.top = py - 3; rect.left = px - 3; rect.bottom = py + 3; rect.right = px + 3;
			FillRect(hdc, &rect, hb2);
			if (px>l)break;
		}
		px = l - 2;
		py = py0;
		LineTo(hdc, px, py);
		SelectObject(hdc, hs);
		EndPaint(hwndGraph, &ps);
		break;
	case WM_RBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		ctrl = wParam & MK_CONTROL;
		PosSouris = MAKEPOINTS(lParam);
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			float x, y;
			LISTFPOINTS *lColor = (DivAdd == 1) ? &lColorDiv : ((DivAdd == 0) ? &lColorAdd : &lColorRot);
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (DivAdd == 1)
			{
				y = (float)(0.1*exp(10 * (float)(rectGraph.bottom - 2 - PosSouris.y) / h));
				if (VertZoom)y /= 10;
			}
			else if (DivAdd == 0)
			{
				y = (float)10.0 * ((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				y -= VCurPos;
				if (VertZoom)y /= 10.0;
			}
			else
			{
				y = (float)((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				if (VertZoom)y /= 10.0;
			}
			if (shift && !ctrl)
			{
				int np = NearestFPoint(lColor, x + (float)0.55 / (float)zoom, &xn, &yn);
				{ InsertFPoint(lColor, x, yn); InvalidateRect(hDlg, NULL, FALSE); }
			}
		}
		break;
	case WM_LBUTTONDOWN:
		shift = wParam & MK_SHIFT;
		ctrl = wParam & MK_CONTROL;
		PosSouris = MAKEPOINTS(lParam);
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			float x, y;
			LISTFPOINTS *lColor = (DivAdd == 1) ? &lColorDiv : ((DivAdd == 0) ? &lColorAdd : &lColorRot);
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (DivAdd==1)
			{
				y = (float)(0.1*exp(10 * (float)(rectGraph.bottom - 2 - PosSouris.y) / h));
				if (VertZoom)y /= 10;
			}
			else if (DivAdd == 0)
			{
				y = (float)10.0 * ((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				y -= VCurPos;
				if (VertZoom)y /= 10.0;
			}
			else
			{
				y = (float)((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				if (VertZoom)y /= 10.0;
			}
			if (!shift && !ctrl)
			{
				float xn, yn;
				int np = NearestFPoint(lColor, x + (float)0.55 / (float)zoom, &xn, &yn);
				if (abs(x - xn) < 0.55 / zoom && ((DivAdd==1)?abs(y / yn - 1):abs(y-yn)) < 0.20)
				{
					dragCP = true;
					npDrag = np;
				}
			}
			if (shift && !ctrl)
			{
				InsertFPoint(lColor, x, y);
				InvalidateRect(hDlg, NULL, FALSE);
			}
			if (ctrl && !shift)
			{
				int np = NearestFPoint(lColor, x + (float)0.55 / (float)zoom, &xn, &yn);
				if (abs(x - xn) < 0.55 / zoom && ((DivAdd==1) ? abs(y / yn - 1) : abs(y - yn)) < 0.20)
				{
					DeleteFPoint(lColor, np);
					InvalidateRect(hDlg, NULL, FALSE);
				}
			}
		}
		break;
	case WM_LBUTTONUP:
		dragCP = false;
		PosSouris = MAKEPOINTS(lParam);
		break;
	case WM_MOUSEMOVE:
		hwndPosit = GetDlgItem(hDlg, IDC_POSIT);
		PosSouris = MAKEPOINTS(lParam);
		int no;
		if (PosSouris.x > rectGraph.left && PosSouris.x<rectGraph.right && PosSouris.y>rectGraph.top && PosSouris.y < rectGraph.bottom)
		{
			TCHAR ctext[32];
			char text[32];
			float x, y;
			h = rectGraph.bottom - rectGraph.top - 4;
			x = NoSelectDeb + CurPos + (float)(PosSouris.x - rectGraph.left - 4) / (float)(4 * zoom);
			if (DivAdd==1)
			{
				y = (float)(0.1*exp((float)10.0 * (float)(rectGraph.bottom - 2 - PosSouris.y) / (float)h));
				if (VertZoom)y /= 10.0;
			}
			else if (DivAdd == 0)
			{
				y = (float)10.0 * ((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				y -= VCurPos;
				if (VertZoom)y /= 10.0;
			}
			else
			{
				y = (float)((rectGraph.bottom - 2 - PosSouris.y - h / 2) / (float)h);
				if (VertZoom)y /= 10.0;
			}
			sprintf_s(text, "%5.2f, %6.3f", x, y);
			MultiByteToWideChar(CP_ACP, 0, text, 32, ctext, 32);
			SetDlgItemText(hDlg, IDC_POSIT, ctext);
			SetWindowPos(hwndPosit, HWND_TOP, PosSouris.x - cxPosit / 2, PosSouris.y - cyPosit - 2, cxPosit, cyPosit, SWP_SHOWWINDOW);
			if (dragCP)
			{
				LISTFPOINTS *lColor;
				if (DivAdd == 1)
				{
					lColor = &lColorDiv;
					orgOneFrame = COLORDIV;
				}
				else if (DivAdd == 0)
				{
					lColor = &lColorAdd;
					orgOneFrame = COLORADD;
				}
				else
				{
					lColor = &lColorRot;
					orgOneFrame = COLORROT;
				}
				SetFPoint(lColor, npDrag, x, y); npDragOneFrame = npDrag; InvalidateRect(hDlg, NULL, FALSE);
			}
			else
			{
				if (DivAdd == 0)orgOneFrame = COLORDIV;
				else if (DivAdd == 1)orgOneFrame = COLORADD;
				else orgOneFrame = COLORROT;
				xOneFrame = x;
			}
			no = (int)x;
			if (no != mNo)
			{
				mNo = no;
				hwndImg1 = GetDlgItem(hDlg, IDC_IMGKF1);
				hwndImg2 = GetDlgItem(hDlg, IDC_IMGKF2);
				if (no >= NbKFY - 1){ no = NbKFY - 1; bmpImgKF(hwndImg1, hwndImg2, no, no); }
				else bmpImgKF(hwndImg1, hwndImg2, no, no + 1);
			}
		}
		else SetWindowPos(hwndPosit, HWND_TOP, 0, 0, cxPosit, cyPosit, SWP_HIDEWINDOW | SWP_NOMOVE);
		break;
	case WM_INITDIALOG:
		LoadRGBPos("palette.kfs", &lRGBPos);
		if (UsePalette)MakeTrgbPal(RoundedPalette);
		CreatePreview(hDlg);
		_beginthread((void(_cdecl*)(void *))ThPreviewOneFrame, 8 * 1024, NULL);
		SendDlgItemMessage(hDlg, IDC_VERTZOOM, BM_SETCHECK, (VertZoom) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, (DivAdd == 1) ? IDC_ITERDIV : ((DivAdd == 0) ? IDC_ADDCOLOR : IDC_ROTATE), BM_SETCHECK, BST_CHECKED, 0);
		RECT rw, rc;
		GetClientRect(hDlg, &rc);
		GetWindowRect(hDlg, &rw);
		cxb = (rw.right - rw.left - rc.right) / 2;  //GetSystemMetrics(SM_CXPADDEDBORDER );
		cyb = rw.bottom - rw.top - rc.bottom - cxb; //GetSystemMetrics(SM_CYCAPTION);
		hwndGraph = GetDlgItem(hDlg, IDC_GRAPH);
		GetWindowRect(hwndGraph, &rectGraph);
		rectGraph.top -= rw.top + cyb;
		rectGraph.bottom -= rw.top + cyb;
		rectGraph.left -= rw.left + cxb;
		rectGraph.right -= rw.left + cxb;
		l = rectGraph.right - rectGraph.left - 4;
		CurPos = 0; zoom = 10;
		MaxPos = NoSelectFin - NoSelectDeb - (l / (4 * zoom));
		if (MaxPos < 0)MaxPos = 0;
		SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_SETRANGE, TRUE, MAKELPARAM(1, 21));
		SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_SETPOS, TRUE, MAKELPARAM(10, 0));
		SetScrollRange(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, 0, MaxPos, TRUE);
		hwndPosit = GetDlgItem(hDlg, IDC_POSIT);
		GetWindowRect(hwndPosit, &rw);
		cxPosit = rw.right - rw.left; cyPosit = rw.bottom - rw.top;
		SetWindowPos(hwndPosit, HWND_TOP, 0, 0, cxPosit, cyPosit, SWP_HIDEWINDOW | SWP_NOMOVE);
		if (lColorRot.NbPoints == 0)
		{
			LoadFPoints("colorrot.kfs", &lColorRot);
			if (lColorRot.NbPoints == 0)InsertFPoint(&lColorRot, 0.0, 0.0);
		}
		if (lColorAdd.NbPoints == 0)
		{
			LoadFPoints("coloradd.kfs", &lColorAdd);
			if (lColorAdd.NbPoints == 0)InsertFPoint(&lColorAdd, 0.0, 0.0);
		}
		if (lColorDiv.NbPoints == 0)
		{
			int idiv;
			LoadFPoints("colordiv.kfs", &lColorDiv);
			if (KFYmemLu)idiv = lKFYmem[0]->ndiv;
			if (lColorDiv.NbPoints == 0)InsertFPoint(&lColorDiv, 0.0, (float)idiv);
		}
		if (lSpeed.NbPoints == 0)
		{
			LoadFPoints("speed.kfs", &lSpeed);
			if (lSpeed.NbPoints == 0)InsertFPoint(&lSpeed, 0.0, 25.0);
		}
		LoadSubMin("SubIM.kfs");
		LoadRelief("Relief.kfs");
		Load3D("3D.kfs");
		ShowWindow(GetDlgItem(hDlg, IDC_VSCROLL), (DivAdd == 0) ? SW_SHOWNOACTIVATE : SW_HIDE);
		SendDlgItemMessage(hDlg, IDC_SUBIM, BM_SETCHECK, (SubIM) ? BST_CHECKED : BST_UNCHECKED, 0);
		wsprintf(txt, L"%d", nbminmoy);
		SetDlgItemText(hDlg, IDC_AVERAGE, txt);
		VMaxPos = 100;
		SetScrollRange(GetDlgItem(hDlg, IDC_VSCROLL), SB_CTL, -VMaxPos, VMaxPos, TRUE);
		SetScrollPos(GetDlgItem(hDlg, IDC_VSCROLL), SB_CTL, VCurPos, TRUE);
		return (INT_PTR)TRUE;
	case WM_FINPREV:
		SetDlgItemText(hDlg, IDC_PREVIEW, L"Preview");
		EnableWindow(GetDlgItem(hDlg, IDOK), TRUE);
		EnableWindow(GetDlgItem(hDlg, IDCANCEL), TRUE);
		_beginthread((void(_cdecl*)(void *))ThPreviewOneFrame, 8 * 1024, NULL);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_SUBIM)
		{
			if (IsDlgButtonChecked(hDlg, IDC_SUBIM) == BST_CHECKED)SubIM = TRUE; else SubIM = FALSE;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_ROTATE)
		{
			if (IsDlgButtonChecked(hDlg, IDC_ROTATE) == BST_CHECKED)DivAdd = 2; else DivAdd = 0;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_ITERDIV)
		{
			if (IsDlgButtonChecked(hDlg, IDC_ITERDIV) == BST_CHECKED)DivAdd = 1; else DivAdd = 0;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_ADDCOLOR)
		{
			if (IsDlgButtonChecked(hDlg, IDC_ADDCOLOR) == BST_CHECKED)DivAdd = 0; else DivAdd = 1;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		ShowWindow(GetDlgItem(hDlg, IDC_VSCROLL), (DivAdd == 0) ? SW_SHOWNOACTIVATE : SW_HIDE);
		if (LOWORD(wParam) == IDC_VERTZOOM)
		{
			if (IsDlgButtonChecked(hDlg, IDC_VERTZOOM) == BST_CHECKED)VertZoom = TRUE;
			else VertZoom = FALSE;
			InvalidateRect(hDlg, NULL, FALSE);
		}
		if (LOWORD(wParam) == IDC_PREVIEW)
		{
			if (VideoBrowser[0] == 0)
			{
				MessageBox(hDlg, L"You must define a Video Browser\nUse menu Video->Video parameters...", L"Movie Maker", MB_ICONINFORMATION | MB_OK);
				break;
			}
			if (PreviewRun)PreviewAbort = TRUE;
			else
			{
				npDragOneFrame = -2;
				while (npDragOneFrame == -2)Sleep(30);
				Preview(hDlg);
			}
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			npDragOneFrame = -2;
			while (npDragOneFrame == -2)Sleep(30);
			DestroyFPoint(&lSpeed);
			DestroyFPoint(&lColorDiv);
			DestroyFPoint(&lColorAdd);
			DestroyFPoint(&lColorRot);
			CloseWindow(hdlgPreview);
			DestroyWindow(hdlgPreview);
			hdlgPreview = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_AVERAGE, txt, MAX_PATH);
			swscanf(txt, L"%d", &nbminmoy);
			npDragOneFrame = -2;
			while (npDragOneFrame == -2)Sleep(30);
			DestroyFPoint(&lSpeed);
			SaveFPoint("colordiv.kfs", &lColorDiv);
			DestroyFPoint(&lColorDiv);
			SaveFPoint("coloradd.kfs", &lColorAdd);
			DestroyFPoint(&lColorAdd);
			SaveFPoint("colorrot.kfs", &lColorRot);
			DestroyFPoint(&lColorRot);
			SaveSubMin("SubIM.kfs");
			CloseWindow(hdlgPreview);
			DestroyWindow(hdlgPreview);
			hdlgPreview = NULL;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	case WM_VSCROLL:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_VSCROLL))
		{
			VCurPos = GetScrollPos(GetDlgItem(hDlg, IDC_VSCROLL), SB_CTL);
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				VCurPos = -VMaxPos;
				break;
			case SB_PAGEUP:
				VCurPos -= 5;
				if (VCurPos<-VMaxPos)VCurPos = -VMaxPos;
				break;
			case SB_PAGEDOWN:
				VCurPos += 5;
				if (VCurPos>VMaxPos)VCurPos = VMaxPos;
				break;
			case SB_LINEUP:
				if (VCurPos > -VMaxPos)VCurPos--;
				break;
			case SB_THUMBPOSITION:
				VCurPos = (short)HIWORD(wParam);
				break;
			case SB_THUMBTRACK:
				VCurPos = HIWORD(wParam);
				break;
			case SB_LINEDOWN:
				if (VCurPos < VMaxPos)VCurPos++;
				break;
			case SB_BOTTOM:
				VCurPos = VMaxPos;
				break;
			case SB_ENDSCROLL:
				break;
			}
			SetScrollPos(GetDlgItem(hDlg, IDC_VSCROLL), SB_CTL, VCurPos, TRUE);
			InvalidateRect(hDlg, NULL, FALSE);
		}
		break;
	case WM_HSCROLL:
		if (lParam == (LPARAM)GetDlgItem(hDlg, IDC_SCROLL))
		{
			CurPos = GetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL);
			switch (LOWORD(wParam))
			{
			case SB_TOP:
				CurPos = 0;
				break;
			case SB_PAGEUP:
				CurPos -= 5;
				if (CurPos<0)CurPos = 0;
				break;
			case SB_PAGEDOWN:
				CurPos += 5;
				if (CurPos>MaxPos)CurPos = MaxPos;
				break;
			case SB_LINEUP:
				if (CurPos > 0)CurPos--;
				break;
			case SB_THUMBPOSITION:
				CurPos = HIWORD(wParam);
				break;
			case SB_THUMBTRACK:
				CurPos = HIWORD(wParam);
				break;
			case SB_LINEDOWN:
				if (CurPos < MaxPos)CurPos++;
				break;
			case SB_BOTTOM:
				CurPos = MaxPos;
				break;
			case SB_ENDSCROLL:
				break;
			}
			SetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, CurPos, TRUE);
		}
		else
		{
			zoom = SendDlgItemMessage(hDlg, IDC_ZOOMG, TBM_GETPOS, 0, 0L);
			int l;
			l = rectGraph.right - rectGraph.left - 4;
			MaxPos = NoSelectFin - NoSelectDeb - (l / (4 * zoom));
			if (MaxPos < 0)MaxPos = 0;
			if (CurPos > MaxPos)
			{
				CurPos = MaxPos;
				SetScrollPos(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, CurPos, TRUE);
			}
			SetScrollRange(GetDlgItem(hDlg, IDC_SCROLL), SB_CTL, 0, MaxPos, TRUE);
		}
		no = CurPos;
		hwndImg1 = GetDlgItem(hDlg, IDC_IMGKF1);
		hwndImg2 = GetDlgItem(hDlg, IDC_IMGKF2);
		if (no >= NbKFY - 2)no = NbKFY - 2;
		bmpImgKF(hwndImg1, hwndImg2, no, no + 1);
		InvalidateRect(hDlg, NULL, FALSE);
	}
	return (INT_PTR)FALSE;
}

void PaintZoomColor(HWND hDlg)
{
	HBRUSH hb;
	PAINTSTRUCT ps;
	RECT rect;
	HWND hwndCol = GetDlgItem(hDlg, IDC_COLOR);
	HDC hdc = BeginPaint(hwndCol, &ps);
	GetClientRect(hwndCol, &rect);
	hb=CreateSolidBrush(RGB(VideoZoomColor.r, VideoZoomColor.g, VideoZoomColor.b));
	FillRect(hdc, &rect, hb);
	DeleteObject(hb);
	EndPaint(hwndCol, &ps);
}

INT_PTR CALLBACK ReliefProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		TCHAR txt[50];
		char ctxt[50];

		LoadRelief("Relief.kfs");
		SendDlgItemMessage(hDlg, IDC_BPHONG, BM_SETCHECK, (bPhong) ? BST_CHECKED : BST_UNCHECKED, 0);
		sprintf_s(ctxt, "%5.2lf", PhongSize);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGSIZE, txt);
		sprintf_s(ctxt, "%5.3lf", PhongRatio);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGRATIO, txt);
		sprintf_s(ctxt, "%5.1lf", PhongElev);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGELEV, txt);		
		SendDlgItemMessage(hDlg, IDC_BPHONG2, BM_SETCHECK, (bPhong2) ? BST_CHECKED : BST_UNCHECKED, 0);
		sprintf_s(ctxt, "%5.2lf", PhongSize2);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGSIZE2, txt);
		sprintf_s(ctxt, "%5.3lf", PhongRatio2);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGRATIO2, txt);
		sprintf_s(ctxt, "%5.1lf", PhongElev2);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PHONGELEV2, txt);
		SendDlgItemMessage(hDlg, IDC_ACTIVATE, BM_SETCHECK, (bSlope) ? BST_CHECKED : BST_UNCHECKED, 0);
		sprintf_s(ctxt, "%5.2lf", SlopePower);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_POWER, txt);
		sprintf_s(ctxt, "%5.3lf", SlopeRatio);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_RATIO, txt);
		sprintf_s(ctxt, "%5.1lf", SlopeAngle);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_LD, txt);
		SendDlgItemMessage(hDlg, IDC_NONE, BM_SETCHECK, (bDE) ? BST_UNCHECKED : BST_CHECKED, 0);
		if (bDE)
		{
			if (bDEtronc)SendDlgItemMessage(hDlg, IDC_NBC0, BM_SETCHECK, BST_CHECKED,0);
			else        SendDlgItemMessage(hDlg, IDC_NBC1, BM_SETCHECK, BST_CHECKED,0);
		}
		SendDlgItemMessage(hDlg, IDC_SQRT, BM_SETCHECK, (bSQRT) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_SQRT2, BM_SETCHECK, (bSQRT2) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_MEDIANFILTER, BM_SETCHECK, (bMedian) ? BST_CHECKED : BST_UNCHECKED, 0);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			bSlope = (SendDlgItemMessage(hDlg, IDC_ACTIVATE, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			GetDlgItemText(hDlg, IDC_POWER, txt, 50);
			swscanf(txt, L"%lf", &SlopePower);
			GetDlgItemText(hDlg, IDC_RATIO, txt, 50);
			swscanf(txt, L"%lf", &SlopeRatio);
			GetDlgItemText(hDlg, IDC_LD, txt, 50);
			swscanf(txt, L"%lf", &SlopeAngle);
			bPhong = (SendDlgItemMessage(hDlg, IDC_BPHONG, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			GetDlgItemText(hDlg, IDC_PHONGSIZE, txt, 50);
			swscanf(txt, L"%lf", &PhongSize);
			GetDlgItemText(hDlg, IDC_PHONGRATIO, txt, 50);
			swscanf(txt, L"%lf", &PhongRatio);
			GetDlgItemText(hDlg, IDC_PHONGELEV, txt, 50);
			swscanf(txt, L"%lf", &PhongElev);
			bPhong2 = (SendDlgItemMessage(hDlg, IDC_BPHONG2, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			GetDlgItemText(hDlg, IDC_PHONGSIZE2, txt, 50);
			swscanf(txt, L"%lf", &PhongSize2);
			GetDlgItemText(hDlg, IDC_PHONGRATIO2, txt, 50);
			swscanf(txt, L"%lf", &PhongRatio2);
			GetDlgItemText(hDlg, IDC_PHONGELEV2, txt, 50);
			swscanf(txt, L"%lf", &PhongElev2);
			if (SendDlgItemMessage(hDlg, IDC_NONE, BM_GETCHECK, 0, 0L) == BST_CHECKED)bDE = FALSE;
			else
			{
				bDE = TRUE;
				if (SendDlgItemMessage(hDlg, IDC_NBC0, BM_GETCHECK, 0, 0L) == BST_CHECKED)bDEtronc = TRUE; else bDEtronc = FALSE;
			}
			bSQRT =(SendDlgItemMessage(hDlg, IDC_SQRT, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bSQRT2 = (SendDlgItemMessage(hDlg, IDC_SQRT2, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bMedian = (SendDlgItemMessage(hDlg, IDC_MEDIANFILTER, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			SaveRelief("Relief.kfs");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK f3DProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		TCHAR txt[50];
		char ctxt[50];

		Load3D("3D.kfs");
		SendDlgItemMessage(hDlg, IDC_3D, BM_SETCHECK, (b3D) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_SQRT3D, BM_SETCHECK, (bSqrt3D) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_LOG3D, BM_SETCHECK, (bLog3D) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_ATAN3D, BM_SETCHECK, (bAtan3D) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_POSTROT, BM_SETCHECK, (bPostRotation) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_ROTATION, BM_SETCHECK, (bRotation) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_BCROP3D, BM_SETCHECK, (bCrop3D) ? BST_CHECKED : BST_UNCHECKED, 0);
		sprintf_s(ctxt, "%d", CropX3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_CROPX3D, txt);
		sprintf_s(ctxt, "%d", CropY3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_CROPY3D, txt);
		sprintf_s(ctxt, "%5.3lf", 180 * alpha3D / PI);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_ALPHA3D, txt);
		sprintf_s(ctxt, "%5.5lf", echZ3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_ECHZ3D, txt);
		sprintf_s(ctxt, "%5.1lf", maxniter3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_MAXNITER3D, txt);
		sprintf_s(ctxt, "%d", decH3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_DECH3D, txt);
		sprintf_s(ctxt, "%5.2lf", nbpLiss3D);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_NBPLISS3D, txt);
		sprintf_s(ctxt, "%5.2lf", ShadowCoef);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_SHADOWCOEF, txt);
		sprintf_s(ctxt, "%5.2lf", ShadowAngle);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_SHADOWANGLE, txt);
		sprintf_s(ctxt, "%4.2lf", perspectiveAmount);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PERSPAMOUNT, txt);
		sprintf_s(ctxt, "%4.2lf", perspectiveScale);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_PERSPSCALE, txt);
		sprintf_s(ctxt, "%4.2lf", RotationStart);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_STARTROT, txt);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_ALPHA3D, txt, 50);
			swscanf(txt, L"%lf", &alpha3D);
			alpha3D = alpha3D*PI / 180;
			Salpha3D = sin(alpha3D);
			Calpha3D = cos(alpha3D);
			GetDlgItemText(hDlg, IDC_ECHZ3D, txt, 50);
			swscanf(txt, L"%lf", &echZ3D);
			GetDlgItemText(hDlg, IDC_MAXNITER3D, txt, 50);
			swscanf(txt, L"%lf", &maxniter3D);
			GetDlgItemText(hDlg, IDC_DECH3D, txt, 50);
			swscanf(txt, L"%d", &decH3D);
			GetDlgItemText(hDlg, IDC_NBPLISS3D, txt, 50);
			swscanf(txt, L"%lf", &nbpLiss3D);
			GetDlgItemText(hDlg, IDC_SHADOWCOEF, txt, 50);
			swscanf(txt, L"%lf", &ShadowCoef);
			GetDlgItemText(hDlg, IDC_SHADOWANGLE, txt, 50);
			swscanf(txt, L"%lf", &ShadowAngle);
			GetDlgItemText(hDlg, IDC_CROPX3D, txt, 50);
			swscanf(txt, L"%d", &CropX3D);
			GetDlgItemText(hDlg, IDC_CROPY3D, txt, 50);
			swscanf(txt, L"%d", &CropY3D);
			GetDlgItemText(hDlg, IDC_PERSPAMOUNT, txt, 50);
			swscanf(txt, L"%lf", &perspectiveAmount);
			GetDlgItemText(hDlg, IDC_PERSPSCALE, txt, 50);
			swscanf(txt, L"%lf", &perspectiveScale);
			GetDlgItemText(hDlg, IDC_STARTROT, txt, 50);
			swscanf(txt, L"%lf", &RotationStart);
			bCrop3D = (SendDlgItemMessage(hDlg, IDC_BCROP3D, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bRotation = (SendDlgItemMessage(hDlg, IDC_ROTATION, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bPostRotation = (SendDlgItemMessage(hDlg, IDC_POSTROT, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			if (bPostRotation && !bRotation)bPostRotation = FALSE;
			b3D = (SendDlgItemMessage(hDlg, IDC_3D, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bSqrt3D = (SendDlgItemMessage(hDlg, IDC_SQRT3D, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bLog3D = (SendDlgItemMessage(hDlg, IDC_LOG3D, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			bAtan3D = (SendDlgItemMessage(hDlg, IDC_ATAN3D, BM_GETCHECK, 0, 0L) == BST_CHECKED);
			Save3D("3D.kfs");
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDC_EVECHZ3D)
		{
			if (NoSelectFin == -1)MessageBox(hDlg, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else
			{
				bEvEchZ3D = TRUE;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SPEEDSETUP), hDlg, SpeedSetup);
			}
		}
		else if (LOWORD(wParam) == IDC_EVMAXITER3D)
		{
			if (NoSelectFin == -1)MessageBox(hDlg, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else
			{
				bEvMaxIter3D = TRUE;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SPEEDSETUP), hDlg, SpeedSetup);
			}
		}
		else if (LOWORD(wParam) == IDC_EVVDEC3D)
		{
			if (NoSelectFin == -1)MessageBox(hDlg, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else
			{
				bEvVertDec3D = TRUE;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SPEEDSETUP), hDlg, SpeedSetup);
			}
		}
		else if (LOWORD(wParam) == IDC_EVROT)
		{
			if (NoSelectFin == -1)MessageBox(hDlg, L"You have to select key frames!\nClick to Select First.\nShift+Click to Select Last", L"Movie Maker", MB_ICONEXCLAMATION | MB_OK);
			else
			{
				bEvRot = TRUE;
				DialogBox(hInst, MAKEINTRESOURCE(IDD_SPEEDSETUP), hDlg, SpeedSetup);
			}
		}
		else if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK VideoProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static RECT rectGraph;
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		TCHAR txt[MAX_PATH];
		wsprintf(txt, L"%d", VideoFPS);
		SetDlgItemText(hDlg, IDC_FPS , txt);
		wsprintf(txt, L"%d", VideoFPM);
		SetDlgItemText(hDlg, IDC_FPM, txt);
		wsprintf(txt, L"%d", VideoWidth);
		SetDlgItemText(hDlg, IDC_WIDTH, txt);
		wsprintf(txt, L"%d", VideoHeight);
		SetDlgItemText(hDlg, IDC_HEIGHT, txt);
		wsprintf(txt, L"%d", VideoZoomSize);
		SetDlgItemText(hDlg, IDC_FONTSIZE, txt);
		if (strlen(VideoZoomFont) == 0)strcpy(VideoZoomFont, "Arial");
		MultiByteToWideChar(CP_ACP, 0, VideoZoomFont, 64, txt, 64);
		SetDlgItemText(hDlg, IDC_FONTNAME, txt);
		MultiByteToWideChar(CP_ACP, 0, VideoBrowser, MAX_PATH, txt, MAX_PATH);
		SetDlgItemText(hDlg, IDC_VIDEOBROWSERPATH, txt);
		SendDlgItemMessage(hDlg, IDC_SHOWZOOM, BM_SETCHECK, (VideoZoomText)?BST_CHECKED:BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_SHADOWRECT, BM_SETCHECK, (bTextZoomShadow) ? BST_CHECKED : BST_UNCHECKED, 0);
		SendDlgItemMessage(hDlg, IDC_ANTIALIAS, BM_SETCHECK, (bAntiAlias) ? BST_CHECKED : BST_UNCHECKED, 0);
		return (INT_PTR)TRUE;

	case WM_PAINT:
		PaintZoomColor(hDlg);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_BCOLOR)
		{
			CHOOSECOLOR cc;                 // common dialog box structure 
			static COLORREF acrCustClr[16]; // array of custom colors 
			ZeroMemory(&cc, sizeof(cc));
			cc.lStructSize = sizeof(cc);
			cc.hwndOwner = hDlg;
			cc.lpCustColors = (LPDWORD)acrCustClr;
			cc.rgbResult = RGB(VideoZoomColor.r, VideoZoomColor.g, VideoZoomColor.b);
			cc.Flags = CC_FULLOPEN | CC_RGBINIT;

			if (ChooseColor(&cc) == TRUE)
			{
				VideoZoomColor.r = GetRValue(cc.rgbResult);
				VideoZoomColor.g = GetGValue(cc.rgbResult);
				VideoZoomColor.b = GetBValue(cc.rgbResult);
				InvalidateRect(hDlg, NULL, FALSE);
			}
		}
		if (LOWORD(wParam) == IDC_FONTCHOOSE)
		{
			LOGFONT lfFont = { 0 };
			CHOOSEFONT cf = { sizeof(CHOOSEFONT),hDlg };
			MultiByteToWideChar(CP_ACP, 0, VideoZoomFont, LF_FACESIZE, lfFont.lfFaceName, LF_FACESIZE);
			cf.lpLogFont = &lfFont;
			cf.iPointSize = 10 * VideoZoomSize;
			cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
			if (ChooseFont(&cf))
			{
				WideCharToMultiByte(CP_ACP, 0, lfFont.lfFaceName, MAX_PATH, VideoZoomFont, MAX_PATH, NULL, NULL);
				SetDlgItemText(hDlg, IDC_FONTNAME, lfFont.lfFaceName);
				VideoZoomSize=cf.iPointSize/10;
				wsprintf(txt, L"%d", VideoZoomSize);
				SetDlgItemText(hDlg, IDC_FONTSIZE, txt);
			}
		}
		if (LOWORD(wParam) == IDC_VIDEOBROWSER)
		{
			TCHAR dir[MAX_PATH];
			GetCurrentDirectory(MAX_PATH, dir);
			OPENFILENAME ofn;
			ZeroMemory(&ofn, sizeof(ofn)); 
			ofn.lStructSize = sizeof(OPENFILENAME);
			TCHAR File[MAX_PATH], FileName[MAX_PATH];
			MultiByteToWideChar(CP_ACP, 0, "", 1, File, MAX_PATH);
			MultiByteToWideChar(CP_ACP, 0, "", 1, FileName, MAX_PATH);
			ofn.hInstance = GetModuleHandle(NULL);
			ofn.lpstrFile = File;
			ofn.lpstrTitle = FileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = L"Executables\0*.exe\0\0";
			ofn.nFilterIndex = 1;
		    ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			if (GetOpenFileName(&ofn))
			{
				WideCharToMultiByte(CP_ACP, 0, File, MAX_PATH, VideoBrowser, MAX_PATH, NULL, NULL);
				SetDlgItemText(hDlg, IDC_VIDEOBROWSERPATH, File);
			}
			SetCurrentDirectory(dir);
		}

		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_FPS, txt, MAX_PATH);
			swscanf(txt, L"%d", &VideoFPS);
			GetDlgItemText(hDlg, IDC_FPM, txt, MAX_PATH);
			swscanf(txt, L"%d", &VideoFPM);
			GetDlgItemText(hDlg, IDC_WIDTH, txt, MAX_PATH);
			swscanf(txt, L"%d", &VideoWidth);
			GetDlgItemText(hDlg, IDC_HEIGHT, txt, MAX_PATH);
			swscanf(txt, L"%d", &VideoHeight);
			if (SendDlgItemMessage(hDlg, IDC_SHOWZOOM, BM_GETCHECK, 0, 0L) == BST_CHECKED)VideoZoomText = TRUE; else VideoZoomText = FALSE;
			if (SendDlgItemMessage(hDlg, IDC_SHADOWRECT, BM_GETCHECK, 0, 0L) == BST_CHECKED)bTextZoomShadow = TRUE; else bTextZoomShadow = FALSE;
			if (SendDlgItemMessage(hDlg, IDC_ANTIALIAS, BM_GETCHECK, 0, 0L) == BST_CHECKED)bAntiAlias = TRUE; else bAntiAlias = FALSE;
			GetDlgItemText(hDlg, IDC_VIDEOBROWSERPATH, txt, MAX_PATH);
			MultiByteToWideChar(CP_ACP, 0, VideoBrowser, MAX_PATH, txt, MAX_PATH);
			SaveVideoBrowser(); SaveFontSize();
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} else 
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK ChPrev(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		TCHAR txt[50];

		for (int i = 40; i >=2; i--)
		{
			int cx, cy;
			cx = Width / i;
			cx = cx - cx % 4;
			cy = (cx*Height) / Width;
			wsprintf(txt, L"%dx%d", cx,cy);
			SendDlgItemMessage(hDlg, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)txt);
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			int r = SendDlgItemMessage(hDlg, IDC_LIST, LB_GETCURSEL, 0, NULL);
			if (r != LB_ERR)
			{
				PrevRatio = 40 - r;
				int cx, cy;
				cx = Width / PrevRatio;
				cx = cx - cx % 4;
				cy = (cx*Height) / Width;

				WidthPrev = cx;
				HeightPrev = cy;
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			return (INT_PTR)FALSE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK KFYProperties(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		TCHAR txt[50];
		char ctxt[50],te[10];

		wsprintf(txt, L"%d", NoProp);
		SetDlgItemText(hDlg, IDC_KFN, txt);
		
		MultiByteToWideChar(CP_ACP, 0, lKFY[NoProp], 50, txt, 50);
		SetDlgItemText(hDlg, IDC_FN, txt);
		sprintf_s(ctxt, "%10.3lf", lKFYmem[NoProp]->min);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_MINI, txt);
		sprintf_s(ctxt, "%10.3lf", lKFYmem[NoProp]->moy);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_MOY, txt);
		sprintf_s(ctxt, "%10.3lf", lKFYmem[NoProp]->max);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_MAXI, txt);
		sprintf_s(ctxt, "%6.3f", lKFYmem[NoProp]->zoomM);
		if (strlen(ctxt) == 1)strcat_s(ctxt, ".");
		while (strlen(ctxt) <5)strcat_s(ctxt, "0");
		strcat_s(ctxt, "e");
		sprintf_s(te, "%03d", lKFYmem[NoProp]->zoomE);
		strcat_s(ctxt, te);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_ZOOM, txt);
		wsprintf(txt, L"%d", lKFYmem[NoProp]->ndiv);
		SetDlgItemText(hDlg, IDC_NDIV, txt);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AccelerateProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static int nbframes = 0;
	static double acc;
	int fini,nbf;
	double time,ft;
	int sec,minut,ift;
	TCHAR txt[64];
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemText(hDlg, IDC_ACCFACT, L"1.00");
		LoadFPoints("speed.kfs", &lSpeed);
		float xdeb, xfin, xcur, ydeb, yfin, ycur;
		xcur = lSpeed.fPoint[0].x;
		ycur = lSpeed.fPoint[0].y;
		fini = 0; nbframes = 0;
		for (int i = 0; i < lSpeed.NbPoints; i++)
		{
			xdeb = lSpeed.fPoint[i].x;
			ydeb = lSpeed.fPoint[i].y;
			if (i < lSpeed.NbPoints - 1)
			{
				xfin = lSpeed.fPoint[i + 1].x;
				yfin = lSpeed.fPoint[i + 1].y;
			}
			else
			{
				xfin = (float)(NbKFY-1);
				yfin = (float)lSpeed.fPoint[i].y;
			}
			while (xcur < xfin)
			{
				xcur += 1 / ycur;
				ycur = exp(log(ydeb) + (log(yfin) - log(ydeb))*(xcur - xdeb) / (xfin - xdeb));
				if (xcur >= NoSelectFin){ fini = 1; break; }
				if (xcur >= NoSelectDeb)nbframes++;
			}
		}
		time = (double)nbframes / VideoFPS;
		sec = (int)time;
		ft = time - sec;
		ift = (int)(ft * 100);
		minut = sec / 60;
		sec = sec % 60;
		wsprintf(txt, L"%02d:%02d.%02d", minut, sec, ift);
		SetDlgItemText(hDlg, IDC_DURATION, txt);
		SetDlgItemText(hDlg, IDC_NEWDURATION, txt);
		return (INT_PTR)TRUE;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_ACCFACT && HIWORD(wParam) == EN_CHANGE)
		{
			GetDlgItemText(hDlg, IDC_ACCFACT, txt, 64);
			swscanf(txt, L"%lf", &acc);
			nbf = (int)(nbframes / acc);
			time = (double)nbf / VideoFPS;
			sec = (int)time;
			ft = time - sec;
			ift = (int)(ft * 100);
			minut = sec / 60;
			sec = sec % 60;
			wsprintf(txt, L"%02d:%02d.%02d", minut, sec, ift);
			SetDlgItemText(hDlg, IDC_NEWDURATION, txt);
		}
		if (LOWORD(wParam) == IDOK)
		{
			for (int i = 0; i < lSpeed.NbPoints; i++)
			{
				if (lSpeed.fPoint[i].x >= NoSelectDeb - 1 && lSpeed.fPoint[i].x <= NoSelectFin + 1)lSpeed.fPoint[i].y /= (float)acc;
			}
			SaveFPoint("speed.kfs", &lSpeed);
			DestroyFPoint(&lSpeed);
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK OneFrameProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	static double mx = 1.00;
	TCHAR txt[50];
	char ctxt[50];

	switch (message)
	{
	case WM_INITDIALOG:
		sprintf_s(ctxt, "%6.2lf", mx);
		MultiByteToWideChar(CP_ACP, 0, ctxt, 50, txt, 50);
		SetDlgItemText(hDlg, IDC_FRAMENUMBER,txt);
		return (INT_PTR)TRUE;
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			GetDlgItemText(hDlg, IDC_FRAMENUMBER, txt, 50);
			swscanf(txt, L"%lf", &mx);
			if (mx >= NoSelectDeb && mx <= NoSelectFin)
			{
				MakeThisFrame((float)mx);
				EndDialog(hDlg, LOWORD(wParam));
			}
			else MessageBox(hwndMain, L"Frame must be in selection!", L"Movie Maker", MB_ICONERROR | MB_OK);
			return (INT_PTR)TRUE;
		}
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
