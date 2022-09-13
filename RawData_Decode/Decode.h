#ifndef __DECODE_H
#define __DECODE_H

/**********************************************************************
Ŀ�ģ�������λ�����ͷ�ļ��������۲����ݽṹ������������ṹ��Ķ��壬
      �������ݺ�����λ�ý��㺯��������
���ߣ�Ф����
ʱ�䣺2015.3.15

�޸�ʱ�䣺2020.08.02
���ߣ�    ��Բ�
�汾:     V1.1
**********************************************************************/

//#include "stm32f10x.h"
//#include "USART.h"


//���峣��
#define PI 3.1415926535897932384626433832795    
#define C 2.99792458e8          // ����й��� m/s                
#define GM 3.986005e14          //  GM  m3/s2
#define Omega 7.2921151467e-5    //������ת���ٶ� rad/s


#define WL1_GPS  C/(1575.42e6)
#define WL2_GPS  C/(1227.60e6)
#define WL1_CPS  C/(1561.098e6)
#define WL2_CPS  C/(1207.14e6)

#define Rad  PI/180.0

#define OEM4SYNC1   0xAA        /* oem4 and Unicore message start sync code 1 */
#define OEM4SYNC2   0x44        /* oem4 and Unicore message start sync code 2 */
#define OEM4SYNC3   0x12        /* oem4 and Unicore message start sync code 3 */

#define OEM4HLEN    28          /* oem4 and Unicore message header length (bytes) */
#define ID_RANGE    43          /* message id: oem4 range measurement */
#define ID_GPSEPHEM 7           /* message id: oem4 raw ephemeris */
#define ID_IONUTC   8           /* message id: Ionospheric and UTC model information */
#define ID_PSRPOS   47          /* message id: Pseudorange Position, BESTPOS 42,PDPPOS 469, PSRPOS 47*/
#define MAXRAWLEN   4096
#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */
#define MAXVAL      8388608.0






typedef enum
{
	FALSE = 0,
	TRUE = 1
}bool;


//���嵼��ϵͳ��ö������
typedef enum 
{
	GPS,GLONASS,COMPASS,UNKNOWN
}GNSSSys;





typedef struct 
{
	double alpha[4];
	double beta[4];
	bool IsValid;
}IONOPARA;



//α��۲�ֵ�ṹ��
typedef struct 
{
	int SatNum;          //������
	int EpochFlag;
	struct 
	{
		int Week;
		double SecOfWeek;
	}Time;

	double Pos[3];
	struct  
	{
		int Prn;
		GNSSSys System;
		double c1;		//��α�����ֵ���ף�
		double l1;		//�ز���λ  ��Ϊ��λ
		float d1;			//˲ʱ������
		double p2;
		double l2;
	}SatObs[32];
	
}EPOCHOBSDATA;



typedef struct 
{
	int PRN;            //���Ǻ�
	int SVHealth;       //���ǽ���״��
	int IODE;						//�������� 1 ����
	int AODC;						//ʱ����������
	double TGD;
	double SqrtA;
	double DetlaN;
	double M0;
	double e;
	double omega;
	double Cuc;
	double Cus;
	double Crc;
	double Crs;
	double Cic;
	double Cis;
	double i0;
	double iDot;
	double OMEGA;
	double OMEGADot;
	double ClkBias;
	double ClkDrift;
	double ClkDriftRate;
	double SVAccuracy;

	struct 
	{
		int Week;
		double SecOfWeek;
	}TOC;
	
	struct
	{
		int Week;
		double SecOfWeek;
	}TOE;
}GPSEPHREC;


//��������
//���뺯������
int input_oem4f(struct EPOCHOBSDATA* obs, struct GPSEPHREC* eph, struct FILE *fp);
//int input_oem4f(EPOCHOBSDATA* obs, GPSEPHREC *eph, int d,unsigned char Buff[]);
int sync_oem4(unsigned char *buff, unsigned char data);
unsigned int crc32(const unsigned char *buff, int len);
int decode_rangeb(unsigned char *buff, EPOCHOBSDATA* obs);
int decode_gpsephem(unsigned char* buff, GPSEPHREC *eph);
int decode_ionutc(unsigned char* buff, IONOPARA* para);
int decode_psrpos(unsigned char* buff, double pos[]);



#endif
