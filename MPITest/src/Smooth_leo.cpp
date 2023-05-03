#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//定義平滑運算的次數
#define NSmooth 1000

/*********************************************************/
/*變數宣告：                                              */
/*  bmpHeader    ： BMP檔的標頭                           */
/*  bmpInfo      ： BMP檔的資訊                           */
/*  **BMPSaveData： 儲存要被寫入的像素資料                 */
/*  **BMPData    ： 暫時儲存要被寫入的像素資料              */
/*********************************************************/
BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE **BMPData = NULL;
RGBTRIPLE **TMPData = NULL;

/*********************************************************/
/*函數宣告：                                              */
/*  readBMP    ： 讀取圖檔，並把像素資料儲存在BMPSaveData   */
/*  saveBMP    ： 寫入圖檔，並把像素資料BMPSaveData寫入     */
/*  swap       ： 交換二個指標                             */
/*  **alloc_memory： 動態分配一個Y * X矩陣                 */
/*********************************************************/
int readBMP(char *fileName);        //read file
int saveBMP(char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X);        //allocate memory

int main(int argc, char *argv[]) {
/*********************************************************/
/*變數宣告：                                              */
/*  *infileName  ： 讀取檔名                              */
/*  *outfileName ： 寫入檔名                              */
/*  startwtime   ： 記錄開始時間                          */
/*  endwtime     ： 記錄結束時間                          */
/*********************************************************/
	char *infileName = "../input.bmp";
	char *outfileName = "../output.bmp";
	double startwtime = 0.0, endwtime = 0;
	int id, size;
	int *scnt, *disp;
	int l, r;
	int width, height;
	MPI_Datatype rgb_triple;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &id);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Type_contiguous(3, MPI_BYTE, &rgb_triple);
	MPI_Type_commit(&rgb_triple);

	//讀取檔案
	if(id == 0){
		if (readBMP(infileName))
			cout << "Read file successfully!!" << endl;
		else
			cout << "Read file fails!!" << endl;

		width = bmpInfo.biWidth;
		height = bmpInfo.biHeight;
	}

	MPI_Bcast(&width, 1, MPI_LONG, 0, MPI_COMM_WORLD);
	MPI_Bcast(&height, 1, MPI_LONG, 0, MPI_COMM_WORLD);

	if(id)
		BMPSaveData = alloc_memory(height, width);

	scnt = (int*)malloc(sizeof(int) * size);
	disp = (int*)malloc(sizeof(int) * size);

	for(int i = 0; i < size; i++){
		l = height * i / size;       // left bound
		r = height * (i + 1) / size; // right bound
		scnt[i] = (r - l) * width;
		disp[i] = l * width;
	}

	l = height * id / size;
	r = height * (id + 1) / size;

	//動態分配記憶體給暫存空間
	BMPData = alloc_memory(r - l + 2, width);
	TMPData = alloc_memory(r - l + 2, width);

	MPI_Scatterv(BMPSaveData[0], scnt, disp, rgb_triple, BMPData[1], (r - l + 2) * width, rgb_triple, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);
	//記錄開始時間
	startwtime = MPI_Wtime();

	//進行多次的平滑運算
	for(int count = 0; count < NSmooth; count++){

		int last_proc = id > 0 ? id - 1 : size - 1;
		int next_proc = id < size - 1 ? id + 1 : 0;
		if(size != 1){
			MPI_Send(BMPData[1], width, rgb_triple, last_proc, 0, MPI_COMM_WORLD);
			MPI_Send(BMPData[r - l], width, rgb_triple, next_proc, 0, MPI_COMM_WORLD);
			MPI_Recv(BMPData[0], width, rgb_triple, last_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(BMPData[r - l + 1], width, rgb_triple, next_proc, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}

		//進行平滑運算
		for(int i = 1; i <= r - l; i++)
			for(int j = 0; j < width; j++){
				/*********************************************************/
				/*設定上下左右像素的位置                                   */
				/*********************************************************/
				int Top = i - 1;
				int Down = i + 1;
				int Left = j > 0 ? j - 1 : width - 1;
				int Right = j < width - 1 ? j + 1 : 0;
				/*********************************************************/
				/*與上下左右像素做平均，並四捨五入                          */
				/*********************************************************/
				TMPData[i][j].rgbBlue = double(BMPData[i][j].rgbBlue + BMPData[Top][j].rgbBlue + BMPData[Top][Left].rgbBlue + BMPData[Top][Right].rgbBlue + BMPData[Down][j].rgbBlue + BMPData[Down][Left].rgbBlue + BMPData[Down][Right].rgbBlue + BMPData[i][Left].rgbBlue + BMPData[i][Right].rgbBlue) / 9 + 0.5;
				TMPData[i][j].rgbGreen = double(BMPData[i][j].rgbGreen + BMPData[Top][j].rgbGreen + BMPData[Top][Left].rgbGreen + BMPData[Top][Right].rgbGreen + BMPData[Down][j].rgbGreen + BMPData[Down][Left].rgbGreen + BMPData[Down][Right].rgbGreen + BMPData[i][Left].rgbGreen + BMPData[i][Right].rgbGreen) / 9 + 0.5;
				TMPData[i][j].rgbRed = double(BMPData[i][j].rgbRed + BMPData[Top][j].rgbRed + BMPData[Top][Left].rgbRed + BMPData[Top][Right].rgbRed + BMPData[Down][j].rgbRed + BMPData[Down][Left].rgbRed + BMPData[Down][Right].rgbRed + BMPData[i][Left].rgbRed + BMPData[i][Right].rgbRed) / 9 + 0.5;
			}

		//把像素資料與暫存指標做交換
		swap(TMPData, BMPData);
	}

	MPI_Gatherv(BMPData[1], (r - l) * width, rgb_triple, BMPSaveData[0], scnt, disp, rgb_triple, 0, MPI_COMM_WORLD);

	MPI_Barrier(MPI_COMM_WORLD);

	//寫入檔案
	if(id == 0){
		//得到結束時間，並印出執行時間
		endwtime = MPI_Wtime();
		cout << "The execution time = " << endwtime - startwtime << endl;

		if(saveBMP(outfileName))
			cout << "Save file successfully!!" << endl;
		else
			cout << "Save file fails!!" << endl;

		free(BMPSaveData[0]);
	}

	MPI_Type_free(&rgb_triple);
	free(BMPData[0]);
	free(TMPData[0]);

	MPI_Finalize();

	return 0;
}

/*********************************************************/
/* 讀取圖檔                                              */
/*********************************************************/
int readBMP(char *fileName){
	//建立輸入檔案物件
	ifstream bmpFile(fileName, ios::in | ios::binary);

	//檔案無法開啟
	if(!bmpFile){
		cout << "It can't open file!!" << endl;
		return 0;
	}

	//讀取BMP圖檔的標頭資料
	bmpFile.read((char*)&bmpHeader, sizeof(BMPHEADER));

	//判決是否為BMP圖檔
	if(bmpHeader.bfType != 0x4d42){
		cout << "This file is not .BMP!!" << endl;
		return 0;
	}

	//讀取BMP的資訊
	bmpFile.read((char*)&bmpInfo, sizeof(BMPINFO));

	//判斷位元深度是否為24 bits
	if(bmpInfo.biBitCount != 24){
		cout << "The file is not 24 bits!!" << endl;
		return 0;
	}

	//修正圖片的寬度為4的倍數
	while(bmpInfo.biWidth % 4 != 0)
		bmpInfo.biWidth++;

	//動態分配記憶體
	BMPSaveData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

	//讀取像素資料
	//for(int i = 0; i < bmpInfo.biHeight; i++)
	//	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
	bmpFile.read((char*)BMPSaveData[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

	//關閉檔案
	bmpFile.close();

	return 1;
}
/*********************************************************/
/* 儲存圖檔                                               */
/*********************************************************/
int saveBMP(char *fileName){
	//判決是否為BMP圖檔
	if(bmpHeader.bfType != 0x4d42){
		cout << "This file is not .BMP!!" << endl;
		return 0;
	}

	//建立輸出檔案物件
	ofstream newFile(fileName, ios::out | ios::binary);

	//檔案無法建立
	if(!newFile){
		cout << "The File can't create!!" << endl;
		return 0;
	}

	//寫入BMP圖檔的標頭資料
	newFile.write((char*)&bmpHeader, sizeof(BMPHEADER));

	//寫入BMP的資訊
	newFile.write((char*)&bmpInfo, sizeof(BMPINFO));

	//寫入像素資料
	//for( int i = 0; i < bmpInfo.biHeight; i++ )
	//        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
	newFile.write((char*)BMPSaveData[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

	//寫入檔案
	newFile.close();

	return 1;

}


/*********************************************************/
/* 分配記憶體：回傳為Y*X的矩陣                             */
/*********************************************************/
RGBTRIPLE **alloc_memory(int Y, int X){
	//建立長度為Y的指標陣列
	RGBTRIPLE **temp = new RGBTRIPLE * [Y];
	RGBTRIPLE *temp2 = new RGBTRIPLE [Y * X];
	memset(temp, 0, sizeof(RGBTRIPLE) * Y);
	memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);

	//對每個指標陣列裡的指標宣告一個長度為X的陣列
	for( int i = 0; i < Y; i++)
		temp[i] = &temp2[i * X];

	return temp;

}
/*********************************************************/
/* 交換二個指標                                           */
/*********************************************************/
void swap(RGBTRIPLE *a, RGBTRIPLE *b){
	RGBTRIPLE *temp;
	temp = a;
	a = b;
	b = temp;
}