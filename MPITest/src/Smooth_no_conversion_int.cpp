#include <mpi.h>
#include <iostream>
#include <string>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include "bmp.h"

using std::ifstream;
using std::ofstream;
using std::ios;
using std::string;

using std::cin;
using std::cout;

using std::swap;
using std::endl;


// Intensity of smoothing
#define NSmooth 1000

BMPHEADER bmpHeader;                    // BMP header
BMPINFO bmpInfo;                        // BMP information
RGBTRIPLE **BMPSaveData = NULL;         // buffer for loading/saving the BMP file
RGBTRIPLE **BMPData = NULL;             // buffer for transmissions
RGBTRIPLE **TMPData = NULL;             // enlarged buffer for transmissions
RGBTRIPLE **RECData = NULL;             // buffer for segments to be processed
RGBTRIPLE **RETData = NULL;             // buffer for processed segments
MPI_Datatype rgb_triple;                // reference for MPI datatype warrper for RGBTRIPLE
LONG biWidth;                           // common variable for reference to the width of the image

int readBMP(char *fileName);        //read file
int saveBMP(char *fileName);        //save file
void swap(RGBTRIPLE *a, RGBTRIPLE *b);
RGBTRIPLE **alloc_memory(int Y, int X);
RGBTRIPLE *alloc_memory_linear(int Y, int X);        //allocate memory

int main(int argc, char *argv[]) {
    // slight modifications of the paths for different directory structure
    char infileName[] = "../input.bmp"; 
    char outfileName[] = "../output.bmp";
    double startwtime = 0.0, endwtime = 0;      // time records
    int id, size, *sendCounts, *displacements, *sendCounts_intercomm, *displacements_intercomm; // self-explanatory
    int length; // length of the segment this processor currently working on
    int *receiveCounts, *receiveCounts_intercomm;
    RGBTRIPLE *upperBorders, *lowerBorders;     // buffer used for collecting and redistributing upper and lower borders for each segment
    int *sectorSize, *sectorOffset, *upperReceiveOffset, *lowerReceiveOffset;   // parameters for transmission

    // initialize MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // MPI wrapper for RGBTRIPLE
    MPI_Type_contiguous(3, MPI_BYTE, &rgb_triple);
    MPI_Type_commit(&rgb_triple);

    // load and process file on processor 0
    if(id == 0){
        // manual break point, comment out if not needed
        // string nil; cin >> nil;
        if (readBMP(infileName))
            cout << "File laoded successfully!!" << endl;
        else
            cout << "Error occured when loading the file!!" << endl;
        
        // register for processor 0 (main controller) preemptively
        biWidth = bmpInfo.biWidth;

        // allocate memory for tmp
        BMPData = alloc_memory(bmpInfo.biHeight, bmpInfo.biWidth);
        TMPData = alloc_memory(bmpInfo.biHeight + 2, bmpInfo.biWidth);

        // variables used for transmission
        sendCounts = (int*)malloc(sizeof(int) * size);
        displacements = (int*)malloc(sizeof(int) * size);
        sendCounts_intercomm = (int*)malloc(sizeof(int) * size);
        displacements_intercomm = (int*)malloc(sizeof(int) * size);
        receiveCounts = (int*)malloc(sizeof(int) * size);
        receiveCounts_intercomm = (int*)malloc(sizeof(int) * size);
        upperBorders = alloc_memory_linear(size, bmpInfo.biWidth);
        lowerBorders = alloc_memory_linear(size, bmpInfo.biWidth);
        sectorSize = (int*)malloc(sizeof(int)*size);
        sectorOffset = (int*)malloc(sizeof(int)*size);
        upperReceiveOffset = (int*)malloc(sizeof(int)*size);
        lowerReceiveOffset = (int*)malloc(sizeof(int)*size);

        for(int i = 0; i < size; i++){
            int l = bmpInfo.biHeight * i / size + 1;       // left bound of enumeration
            int r = (bmpInfo.biHeight) * (i + 1) / size + 1; // right bound of enumeration
            sendCounts[i] = (r - l + 2);
            displacements[i] = (l - 1);
            sendCounts_intercomm[i] = sendCounts[i]*biWidth;
            displacements_intercomm[i] = displacements[i]*biWidth;
            receiveCounts[i] = (r - l);
            receiveCounts_intercomm[i] = receiveCounts[i]*biWidth;
            sectorSize[i] = bmpInfo.biWidth;
            sectorOffset[i] = i*bmpInfo.biWidth;
            upperReceiveOffset[i] = (bmpInfo.biWidth*size+(i+1)*bmpInfo.biWidth)%(bmpInfo.biWidth*size);
            lowerReceiveOffset[i] = (bmpInfo.biWidth*size+(i-1)*bmpInfo.biWidth)%(bmpInfo.biWidth*size);
        }

        // copy the image from IO buffer to transmission buffer
        for(int i = 0; i < bmpInfo.biHeight; i++)
            for(int j = 0; j < bmpInfo.biWidth; j++)
                BMPData[i][j] = BMPSaveData[i][j];

        // copy the image and fill the upper and lower border of the image
        for(int j = 0; j < bmpInfo.biWidth; j++)
        {
            TMPData[0][j] = BMPData[bmpInfo.biHeight-1][j];
            TMPData[bmpInfo.biHeight+1][j] = BMPData[0][j];
            for (int k = 0; k < bmpInfo.biHeight; k++)
                TMPData[k+1][j] = BMPData[k][j];
        }

        // collects upperBorders and lowerBorders for the first cycle
        for (int i = 0; i < size; i++)
            for (int j = 0; j < bmpInfo.biWidth; j++)
            {
                upperBorders[i*bmpInfo.biWidth+j] = TMPData[displacements[i]][j];
                lowerBorders[i*bmpInfo.biWidth+j] = TMPData[displacements[i]+sendCounts[i]-1][j];
            }
    }

    // boradcast some properties to all processors, logged
    MPI_Barrier(MPI_COMM_WORLD);
    if(!MPI_Bcast(&biWidth, 1, MPI_LONG, 0, MPI_COMM_WORLD))
        cout << "id: " << id <<": biWidth received" << endl;
    if(!MPI_Scatter(receiveCounts, 1, MPI_INT, &length, 1, MPI_INT, 0, MPI_COMM_WORLD))
        cout << "id: " << id <<": size received" << endl;

    // creating buffer for calculation
    RECData = alloc_memory(length + 2, biWidth);
    RETData = alloc_memory(length + 2, biWidth);

    // as reference
    if(id)
    {
        TMPData = alloc_memory(1, 1);
        BMPData = alloc_memory(1, 1);
    }

    // scatter image base, logged
    if(!MPI_Scatterv(TMPData[0], sendCounts_intercomm, displacements_intercomm, rgb_triple, RECData[0], (length + 2)*biWidth, rgb_triple, 0, MPI_COMM_WORLD))
        cout << "id: " << id << ": base chunk received" << endl;

    // make empty buffers for borders
    RECData[0] = (RGBTRIPLE*) malloc(sizeof(RGBTRIPLE)*biWidth);
    RECData[length+1] = (RGBTRIPLE*) malloc(sizeof(RGBTRIPLE)*biWidth);

    MPI_Barrier(MPI_COMM_WORLD);
    //record the beginning
    startwtime = MPI_Wtime();
    
    for(int count = 0; count < NSmooth; count ++){
        if(!(id || (count+1)*100%NSmooth))
            cout << "cycle " << count + 1 << " of " << NSmooth << " (" << (count+1)*100/NSmooth <<"%)" << endl;
        // scatter collected borders, logged
        MPI_Barrier(MPI_COMM_WORLD);
        if(!MPI_Scatterv(upperBorders, sectorSize, sectorOffset, rgb_triple, RECData[0], biWidth, rgb_triple, 0, MPI_COMM_WORLD))
            //cout << "cycle: " << count << " id: " << id <<": upperBorder received" << endl;
        if(!MPI_Scatterv(lowerBorders, sectorSize, sectorOffset, rgb_triple, RECData[length+1], biWidth, rgb_triple, 0, MPI_COMM_WORLD))
            //cout << "cycle: " << count << " id: " << id <<": lowerBorder received" << endl;

        //smoothing, logged
        for(int i = 1; i < length+1; i++)
            for(int j = 0; j < biWidth; j++){
                /*********************************************************/
                /* direction reference                                   */
                /*********************************************************/
                int Top = i-1;
                int Down = i+1;
                int Left = j > 0 ? j - 1 : biWidth - 1;
                int Right = j < biWidth - 1 ? j + 1 : 0;
                /*********************************************************/
                /*averaging with surrounding pixels and rounding         */
                /*********************************************************/
                RETData[i][j].rgbBlue = ((RECData[i][j].rgbBlue + RECData[Top][j].rgbBlue + RECData[Top][Left].rgbBlue + RECData[Top][Right].rgbBlue + RECData[Down][j].rgbBlue + RECData[Down][Left].rgbBlue + RECData[Down][Right].rgbBlue + RECData[i][Left].rgbBlue + RECData[i][Right].rgbBlue) *10 / 9 + 5) / 10;
                RETData[i][j].rgbGreen = ((RECData[i][j].rgbGreen + RECData[Top][j].rgbGreen + RECData[Top][Left].rgbGreen + RECData[Top][Right].rgbGreen + RECData[Down][j].rgbGreen + RECData[Down][Left].rgbGreen + RECData[Down][Right].rgbGreen + RECData[i][Left].rgbGreen + RECData[i][Right].rgbGreen) *10 / 9 + 5) / 10;
                RETData[i][j].rgbRed = ((RECData[i][j].rgbRed + RECData[Top][j].rgbRed + RECData[Top][Left].rgbRed + RECData[Top][Right].rgbRed + RECData[Down][j].rgbRed + RECData[Down][Left].rgbRed + RECData[Down][Right].rgbRed + RECData[i][Left].rgbRed + RECData[i][Right].rgbRed) *10 / 9 + 5) / 10;
            }
            //cout << "cycle: " << count << " id: " << id <<": done calculation" << endl;
        MPI_Barrier(MPI_COMM_WORLD);

        // collect the new borders and swap non-border parts with new ones
        MPI_Gatherv(RETData[1], biWidth, rgb_triple, lowerBorders, sectorSize, lowerReceiveOffset, rgb_triple, 0, MPI_COMM_WORLD);
        MPI_Gatherv(RETData[length], biWidth, rgb_triple, upperBorders, sectorSize, upperReceiveOffset, rgb_triple, 0, MPI_COMM_WORLD);
        swap(RECData, RETData);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    // collect processed image
    MPI_Gatherv(RETData[1], length*biWidth, rgb_triple, BMPData[0], receiveCounts_intercomm, displacements_intercomm, rgb_triple, 0, MPI_COMM_WORLD);

    // free no longer used custom datatype
    MPI_Type_free(&rgb_triple);
    //write to file
    if(id == 0){
        //record the end and print the duration
        endwtime = MPI_Wtime();
        cout << "The execution time = " << endwtime-startwtime << endl;

        // copy the image from transmission buffer to IO buffer
        for(int i = 0; i < bmpInfo.biHeight; i++)
            for(int j = 0; j < bmpInfo.biWidth; j++)
                BMPSaveData[i][j] = BMPData[i][j];

        if(saveBMP(outfileName))
            cout << "File saved successfully!!" << endl;
        else
            cout << "Error occured when saving the file!!" << endl;

        // free pointers?

        free(BMPSaveData);
        free(sendCounts);
        free(displacements);
        free(sendCounts_intercomm);
        free(displacements_intercomm);
        free(receiveCounts);
        free(upperBorders);
        free(lowerBorders);
        free(sectorSize);
        free(sectorOffset);
        free(upperReceiveOffset);
        free(lowerReceiveOffset);
    }
    free(BMPData);
    free(TMPData);
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