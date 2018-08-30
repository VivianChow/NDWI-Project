#include <iostream>
#include "gdal_priv.h"
#include "ogr_spatialref.h"

using namespace std;

//NDWI����
//NDWI=(Green-NIR)/(Green+NIR)=(Band2-Band4)/(Band2+Band4)
extern "C"{
int NDWI(float threshold, const char *inFileName, const char *outFileName)
{
	//��������Ϊ�ü��Ժ������
	const char *inputFileName = inFileName;
	const char *outputFileName = outFileName;

	GDALDataset *inputDataset;
	GDALDataset *outputDataset;

	GDALAllRegister();

	cout<<"Reading image..."<<endl;
	inputDataset = (GDALDataset *)GDALOpen(inputFileName,GA_ReadOnly);
	if (inputDataset == NULL)
	{
		cout<<"File open failed!"<<endl;
		return 1;
	}

	int imgSizeX = inputDataset->GetRasterXSize();
	int imgSizeY = inputDataset->GetRasterYSize();

	const char *imgFormat = "GTiff";
	GDALDriver *gdalDriver = GetGDALDriverManager()->GetDriverByName(imgFormat);
	if (gdalDriver == NULL)
	{ 
		cout<<"File create failed!"<<endl;
		return 1;
	}

	outputDataset = gdalDriver->Create(outputFileName,imgSizeX,imgSizeY,1,GDT_Float32,NULL);

	//��ȡ�������ݵĵ���仯��Ϣ
	double goeInformation[6];
	inputDataset->GetGeoTransform(goeInformation);
	//��ȡ�������ݵĵ�����Ϣ��д������ļ�
	const char * gdalProjection = inputDataset->GetProjectionRef();

	outputDataset->SetGeoTransform(goeInformation);
	outputDataset->SetProjection(gdalProjection);

	cout<<"Image Processing..."<<endl;
	//ȡ���̲��κͽ����Ⲩ��
	GDALRasterBand *raseterBandGreen = inputDataset->GetRasterBand(2);
	GDALRasterBand *raseterBandNIR = inputDataset->GetRasterBand(4);
	GDALRasterBand *outputRasterBand = outputDataset->GetRasterBand(1);
	//����洢�ռ䣬Ϊһ�еĴ�С
	float *bufferBlockGreen = (float *)CPLMalloc(sizeof(float) * imgSizeX);
	float *bufferBlockNIR = (float *)CPLMalloc(sizeof(float) * imgSizeX);
	float *outputBufferBlock = (float *)CPLMalloc(sizeof(float) * imgSizeX);

	//����NDWI�ļ���
	for (int i = 0; i < imgSizeY; i++)
	{
		raseterBandGreen->RasterIO(GF_Read,0,i,imgSizeX,1,bufferBlockGreen,imgSizeX,1,GDT_Float32,0,0);
		raseterBandNIR->RasterIO(GF_Read,0,i,imgSizeX,1,bufferBlockNIR,imgSizeX,1,GDT_Float32,0,0);
		for (int j = 0; j < imgSizeX; j++)
		{
			outputBufferBlock[j] = (bufferBlockGreen[j] - bufferBlockNIR[j]) / (bufferBlockGreen[j] + bufferBlockNIR[j]);
			// 20141008 0.03
			// 20141119 0.1
			if (outputBufferBlock[j] <= threshold)
			{
				outputBufferBlock[j] = 0;
			} 
		}
		outputRasterBand->RasterIO(GF_Write,0,i,imgSizeX,1,outputBufferBlock,imgSizeX,1,GDT_Float32,0,0);
	}
	//�ͷ���Դ
	CPLFree(bufferBlockGreen);
	CPLFree(bufferBlockNIR);
	CPLFree(outputBufferBlock);
	GDALClose(inputDataset);
	GDALClose(outputDataset);
	return 0;
}}
