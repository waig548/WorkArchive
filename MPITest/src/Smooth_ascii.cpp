#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using namespace std;

//Intensity of smoothing
#define NSmooth 1000

/*********************************************************/
/*	Variables                                            */
/*  bmpHeader    : BMP Header                            */
/*  bmpInfo      : BMP Info                              */
/*  **BMPSaveData: Pixel data to be saved                */
/*  **BMPData    : Tmp. Pixel data to be saved           */
/*********************************************************/
BMPHEADER bmpHeader;
BMPINFO bmpInfo;
RGBTRIPLE **BMPSaveData = NULL;
RGBTRIPLE *BMPData = NULL;
RGBTRIPLE *TMPData = NULL;
RGBTRIPLE *RECData = NULL;
RGBTRIPLE *RETData = NULL;
MPI_Datatype rgb_triple;
LONG biWidth;

/*********************************************************/
/*  Functions                                            */
/*  readBMP    : Read the image file and  	 			 */
/*				 store pixel data in BMPSaveData  	 	 */
/*  saveBMP    : Write pixel data in BMPSaveData		 */
/*				 to an image file 	    				 */
/*  swap       : Swap 2 given pointers                   */
/*  **alloc_memory: Dynamically allocate a Y * X matrix  */
/*********************************************************/

int readBMP(char *fileName);        //read file
int saveBMP(char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X);
RGBTRIPLE *alloc_memory_linear(int Y, int X);        //allocate memory

int main(int argc, char *argv[]) {
/*********************************************************/
/*  Variables                                            */
/*  *infileName  : input filename                        */
/*  *outfileName : output filename                       */
/*  startwtime   : the beginning                         */
/*  endwtime     : the end                               */
/*********************************************************/
    char infileName[] = "../input.bmp";
    char outfileName[] = "../output.bmp";
    double startwtime = 0.0, endwtime = 0;
    int id, size, *scnt, *disp;
    int l, r;
    int offset, length;
    int *recvScnt, *recvDisp;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Type_contiguous(3, MPI_BYTE, &rgb_triple);
    MPI_Type_commit(&rgb_triple);


    //load file
    if(id == 0){
        //string nil;
        //cin >> nil;
        if (readBMP(infileName))
            cout << "Read file successfully!!" << endl;
        else
            cout << "Read file fails!!" << endl;
        
        biWidth = bmpInfo.biWidth;

        //allocate memory for tmp
        BMPData = alloc_memory_linear(bmpInfo.biHeight, bmpInfo.biWidth);
        TMPData = alloc_memory_linear(bmpInfo.biHeight + 2, bmpInfo.biWidth);

        scnt = (int*)malloc(sizeof(int) * size);
        disp = (int*)malloc(sizeof(int) * size);
        recvScnt = (int*)malloc(sizeof(int) * size);
        recvDisp = (int*)malloc(sizeof(int) * size);

        for(int i = 0; i < size; i++){
            /*if(i==0)
            {
                scnt[i]=0;
                disp[i]=0;
                recvScnt[i]=0;
                continue;
            }*/
            l = bmpInfo.biHeight * i / size + 1;       // left bound of enumeration
            r = (bmpInfo.biHeight + 1) * (i + 1) / size + 1; // right bound of enumeration
            scnt[i] = (r - l + 2)*biWidth;
            disp[i] = (l - 1)*biWidth;
            recvScnt[i] = (r - l)*biWidth;
            //recvDisp[i] = l*biWidth - 1;
        }

        for(int i = 0; i < bmpInfo.biHeight; i++)
            for(int j = 0; j < bmpInfo.biWidth; j++)
            {
                BMPData[i*bmpInfo.biWidth+j] = BMPSaveData[i][j];
            }
    }
    //l = bmpInfo.biHeight * id / size + 1;       // left bound of enumeration
    //r = (bmpInfo.biHeight + 1) * (id + 1) / size + 1; // right bound of enumeration
    MPI_Barrier(MPI_COMM_WORLD);
    if(!MPI_Bcast(&biWidth, 1, MPI_LONG, 0, MPI_COMM_WORLD))
        cout << "id: " << id <<": biWidth received" << endl;
    //MPI_Barrier(MPI_COMM_WORLD);
    if(!MPI_Scatter(disp, 1, MPI_INT, &offset, 1, MPI_INT, 0, MPI_COMM_WORLD))
        cout << "id: " << id <<": offset received" << endl;
    //MPI_Barrier(MPI_COMM_WORLD);
    if(!MPI_Scatter(recvScnt, 1, MPI_INT, &length, 1, MPI_INT, 0, MPI_COMM_WORLD))
        cout << "id: " << id <<": size received" << endl;
    //MPI_Barrier(MPI_COMM_WORLD);

    RECData = alloc_memory_linear(length/biWidth + 2, biWidth);
    RETData = alloc_memory_linear(length/biWidth, biWidth);

    MPI_Barrier(MPI_COMM_WORLD);
    //record the beginning
    startwtime = MPI_Wtime();
    
    for(int count = 0; count < NSmooth; count ++){
        //parellelism
        //MPI_Barrier(MPI_COMM_WORLD);
        if(id == 0){
            cout << "cycle: " << count << " : ";
            //swap pixels with tmp
            //swap(TMPData, BMPData);
            for(int j = 0; j < bmpInfo.biWidth; j++){
                //TMPData[j] = BMPData[(bmpInfo.biHeight-1)*bmpInfo.biWidth+j];
                TMPData[(bmpInfo.biHeight+1)*bmpInfo.biWidth+j] = BMPData[j];
                for (int k = 0; k < bmpInfo.biHeight; k++)
                    TMPData[(k+1)*bmpInfo.biWidth+j] = BMPData[k*bmpInfo.biWidth+j];
            }
            cout << "data copied" << endl;
            //MPI_Scatterv(BMPData, scnt, disp, rgb_triple, RECData, (bmpInfo.biHeight + 2) * bmpInfo.biWidth, rgb_triple, 0, MPI_COMM_WORLD);
            //MPI_Gatherv(RETData, (r - l - 1) * bmpInfo.biWidth, rgb_triple, BMPData, scnt, disp, rgb_triple, 0, MPI_COMM_WORLD);
        }
        MPI_Barrier(MPI_COMM_WORLD);
        if(!MPI_Scatterv(TMPData, scnt, disp, rgb_triple, RECData, length + 2 * biWidth, rgb_triple, 0, MPI_COMM_WORLD))
            cout << "cycle: " << count << " id: " << id <<": data chunk received" << endl;

        //smoothing
        for(int i = 1; i < length/biWidth+1; i++)
            for(int j = 0; j < biWidth; j++){
                /*********************************************************/
                /*set bounds                                             */
                /*********************************************************/
                int Top = i-1;
                int Down = i+1;
                int Left = j > 0 ? j - 1 : biWidth - 1;
                int Right = j < biWidth - 1 ? j + 1 : 0;
                /*********************************************************/
                /*averaging with surrounding pixels and rounding         */
                /*********************************************************/
                RETData[(i-1)*biWidth+j].rgbBlue = double(RECData[i*biWidth+j].rgbBlue + RECData[Top*biWidth+j].rgbBlue + RECData[Top*biWidth+Left].rgbBlue + RECData[Top*biWidth+Right].rgbBlue + RECData[Down*biWidth+j].rgbBlue + RECData[Down*biWidth+Left].rgbBlue + RECData[Down*biWidth+Right].rgbBlue + RECData[i*biWidth+Left].rgbBlue + RECData[i*biWidth+Right].rgbBlue) / 9 + 0.5;
                RETData[(i-1)*biWidth+j].rgbGreen =double(RECData[i*biWidth+j].rgbGreen + RECData[Top*biWidth+j].rgbGreen + RECData[Top*biWidth+Left].rgbGreen + RECData[Top*biWidth+Right].rgbGreen + RECData[Down*biWidth+j].rgbGreen + RECData[Down*biWidth+Left].rgbGreen + RECData[Down*biWidth+Right].rgbGreen + RECData[i*biWidth+Left].rgbGreen + RECData[i*biWidth+Right].rgbGreen) / 9 + 0.5;
                RETData[(i-1)*biWidth+j].rgbRed = double(RECData[i*biWidth+j].rgbRed + RECData[Top*biWidth+j].rgbRed + RECData[Top*biWidth+Left].rgbRed + RECData[Top*biWidth+Right].rgbRed + RECData[Down*biWidth+j].rgbRed + RECData[Down*biWidth+Left].rgbRed + RECData[Down*biWidth+Right].rgbRed + RECData[i*biWidth+Left].rgbRed + RECData[i*biWidth+Right].rgbRed) / 9 + 0.5;
            }
            cout << "cycle: " << count << " id: " << id <<": done calculation" << endl;
        MPI_Barrier(MPI_COMM_WORLD);
        //if(id == 0)
        MPI_Gatherv(RETData, length, rgb_triple, BMPData, recvScnt, disp, rgb_triple, 0, MPI_COMM_WORLD);
    //}
    }
    MPI_Barrier(MPI_COMM_WORLD);
    //write to file
    if(id == 0){
        //MPI_Gatherv(RETData, (r - l - 1) * bmpInfo.biWidth, rgb_triple, BMPData, scnt, disp, rgb_triple, 0, MPI_COMM_WORLD);
        //MPI_Barrier(MPI_COMM_WORLD);
        //record the end and print the duration
        endwtime = MPI_Wtime();
        cout << "The execution time = " << endwtime-startwtime << endl;

        for(int i = 0; i < bmpInfo.biHeight; i++)
            for(int j = 0; j < bmpInfo.biWidth; j++)
                BMPSaveData[i][j] = BMPData[i*bmpInfo.biWidth+j];

        if(saveBMP(outfileName))
            cout << "Save file successfully!!" << endl;
        else
            cout << "Save file fails!!" << endl;

        //free(BMPData);
        //free(BMPSaveData[0]);
        free(BMPData);
        free(TMPData);
        free(BMPSaveData);
    }
    free(RECData);
    free(RETData);

    MPI_Finalize();

    return 0;
}

/**
 * @brief Read the image corresponding to the given fileName
 * 
 * @param fileName path to the image
 * @return int 0 if error occured else 1
 */
int readBMP(char *fileName){
    //create a input stream
    ifstream bmpFile(fileName, ios::in | ios::binary);

    //unable to open the file
    if(!bmpFile){
        cout << "It can't open file!!" << endl;
        return 0;
    }

    //read BMP header
    bmpFile.read((char*)&bmpHeader, sizeof(BMPHEADER));

    //check BMP header
    if(bmpHeader.bfType != 0x4d42){
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    //read BMP info
    bmpFile.read((char*)&bmpInfo, sizeof(BMPINFO));

    //check if the bit depth is 24 bits
    if(bmpInfo.biBitCount != 24){
        cout << "The file is not 24 bits!!" << endl;
        return 0;
    }

    //rectify the width to be multiples of 4
    while(bmpInfo.biWidth % 4 != 0)
        bmpInfo.biWidth++;

    //allocate memory
    BMPSaveData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);

    //read pixel data
    //for(int i = 0; i < bmpInfo.biHeight; i++)
    //	bmpFile.read( (char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE));
    bmpFile.read((char*)BMPSaveData[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    //close the file
    bmpFile.close();

    return 1;
}
/**
 * @brief Save the image to a specific file
 * 
 * @param fileName name of the output file
 * @return int 0 if error occured else 1
 */
int saveBMP(char *fileName){
    //header check (probably unnecessary?)
    if(bmpHeader.bfType != 0x4d42){
        cout << "This file is not .BMP!!" << endl;
        return 0;
    }

    //create output file
    ofstream newFile(fileName, ios::out | ios::binary);

    //unable to create the file
    if(!newFile){
        cout << "The File can't create!!" << endl;
        return 0;
    }

    //write BMP header
    newFile.write((char*)&bmpHeader, sizeof(BMPHEADER));

    //write BMP info
    newFile.write((char*)&bmpInfo, sizeof(BMPINFO));

    //write pixel data
    //for( int i = 0; i < bmpInfo.biHeight; i++ )
    //        newFile.write( ( char* )BMPSaveData[i], bmpInfo.biWidth*sizeof(RGBTRIPLE) );
    newFile.write((char*)BMPSaveData[0], bmpInfo.biWidth * sizeof(RGBTRIPLE) * bmpInfo.biHeight);

    //close the file
    newFile.close();

    return 1;

}


/**
 * @brief Allocate memory and return a Y * X matrix
 * 
 * @param Y rows
 * @param X cols
 * @return RGBTRIPLE** the matrix
 */
RGBTRIPLE **alloc_memory(int Y, int X){
    //array of pointers with the size of Y (rows)
    RGBTRIPLE **temp = new RGBTRIPLE * [Y];
    RGBTRIPLE *temp2 = new RGBTRIPLE [Y * X];
    memset(temp, 0, sizeof(RGBTRIPLE) * Y);
    memset(temp2, 0, sizeof(RGBTRIPLE) * Y * X);

    //array with the size X for each pointer as the matrix rows
    for( int i = 0; i < Y; i++)
        temp[i] = &temp2[i * X];

    return temp;

}

/**
 * @brief Allocate memory and return a Y * X matrix in linear form
 * 
 * @param Y rows
 * @param X cols
 * @return RGBTRIPLE* the matrix in linear form
 */
RGBTRIPLE *alloc_memory_linear(int Y, int X){
    
    return (RGBTRIPLE*) malloc(sizeof(RGBTRIPLE) * Y *X);

}

/**
 * @brief Swap the given pointers
 * 
 * @param a 
 * @param b 
 */
void swap(RGBTRIPLE *a, RGBTRIPLE *b){
    RGBTRIPLE *temp;
    temp = a;
    a = b;
    b = temp;
}