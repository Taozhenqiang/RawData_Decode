/****************************************************************************
Ŀ�ģ�    ʵ��Novatel��Unicore���ջ��Ķ��������ݽ��룬��ù۲����ݺ͹㲥����

��дʱ�䣺2014.05.09
���ߣ�    ������
�汾:     V1.0
��Ȩ��    �人��ѧ���ѧԺ

�޸�ʱ�䣺2020.08.02
���ߣ�    ��Բ�
�汾:     V1.1
****************************************************************************/

#include <string.h>
#include <math.h>
#include <stdio.h>
#include "Decode.h"

IONOPARA IonPara;


/* get fields (little-endian) ------------------------------------------------*/
#define U1(p) (*((unsigned char *)(p)))
#define I1(p) (*((char *)(p)))
static unsigned short U2(unsigned char *p) {unsigned short u; memcpy(&u,p,2); return u;}
static unsigned int   U4(unsigned char *p) {unsigned int   u; memcpy(&u,p,4); return u;}
static int            I4(unsigned char *p) {int            i; memcpy(&i,p,4); return i;}
static float          R4(unsigned char *p) {float          r; memcpy(&r,p,4); return r;}
static double         R8(unsigned char *p) {double         r; memcpy(&r,p,8); return r;}


/****************************************************************************
  CopyArray
   Ŀ�ģ���һ�����鿽������һ��������( ��ά�������һά��ʾ )  
  ��ţ�01018

  ����:
  n      ����������Ԫ�ظ���
  Dist   Ŀ������
  Sour   Դ����
****************************************************************************/
int CopyArray( int n, double Dist[], const double Sour[] )
{
	int i;

	if( n<=0 )
	{
		//printf( "Error dimension in CopyArray!\n");
		return 1;
    }

    for( i=0; i<n; i++ )
    {
        Dist[i] = Sour[i];
    }
	return 0;
}

/* input oem4 raw data from file ------------------------------------------
* fetch next novatel oem4/oem3 raw data and input a message from file
* args   : EPOCHOBSDATA  *obs   O     raw obs data
           GPSEPHREC     *eph   O     broadcast ephemeris
*          FILE          *fp    I     file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/

int input_oem4f(EPOCHOBSDATA* obs, GPSEPHREC* eph, FILE *fp)
{
	int i, data, len;
	unsigned char buff[MAXRAWLEN];
	double tow;
	int msg,week,type;

	while(!feof(fp))
	{
		for (i=0;;i++)
		{
			if ((data=fgetc(fp))==EOF) return -2;
			if (sync_oem4(buff,(unsigned char)data)) break;
			if (i>=MAXRAWLEN) return 0;
		}
		if (fread(buff+3,7,1,fp)<1) return -2;

		if ((len=U2(buff+8)+OEM4HLEN)>MAXRAWLEN-4) {
			return -1;
		}
		if (fread(buff+10,len-6,1,fp)<1) return -2;
		type=U2(buff+4);

		// check crc32 
		if (crc32(buff,len)!=U4(buff+len)) return -1;
		msg =(U1(buff+6)>>4)&0x3;
		week=U2(buff+14);
		tow =U4(buff+16)*0.001;

		if (msg!=0)     return 0; // message type: 0=binary,1=ascii
		double pos[3];
		memset(obs,0,sizeof(EPOCHOBSDATA));
		obs->Time.Week = week;
		obs->Time.SecOfWeek = tow;

//		printf_s("%4d%10.f%5d\n",week,tow,type);
		switch (type) {
		//case ID_RANGECMP      : return decode_rangebcmp(buff,obs);
		case ID_RANGE         : return decode_rangeb(buff,obs);
		case ID_GPSEPHEM      : return decode_gpsephem(buff,eph);
		case ID_IONUTC        : return decode_ionutc(buff,&IonPara);
		case ID_PSRPOS        : return decode_psrpos(buff,obs->Pos);
		}
	};

	return 0;
}


/* input oem4 raw data from buff ------------------------------------------
* fetch next novatel oem4/oem3 raw data and input a message from file
* args   : EPOCHOBSDATA  *obs   O     raw obs data
           GPSEPHREC     *eph   O     broadcast ephemeris
*          FILE          *fp    I     file pointer
* return : status(-2: end of file, -1...9: same as above)
*-----------------------------------------------------------------------------*/
//int type;
//int input_oem4f(EPOCHOBSDATA* obs, GPSEPHREC *eph, int d,unsigned char Buff[])
//{
//	int i,j, len, val;
//	double tow, pos[3];
//	int msg,week;
//	unsigned char buff[MAXRAWLEN];
//	
//	i=0;
//	val=0;
//	while(1)
//	{
//		for (;i<d;i++)
//		{
//			if(Buff[i]==OEM4SYNC1 && Buff[i+1]==OEM4SYNC2 && Buff[i+2]==OEM4SYNC3)
//			{
//				break;//�˳�forѭ��
//			}
//		}
//
//		if(i+OEM4HLEN>=d)         break;//�˳�whileѭ��
//		for(j=0;j<OEM4HLEN;j++)   buff[j]=Buff[i+j];
//		len=U2(buff+8)+OEM4HLEN;	//�����ܳ���
//		if ((len+4+i)>d) 	      break;//����ĩβλ�ô���buff��С
//		for(j=OEM4HLEN;j<len+4;j++) buff[j]=Buff[i+j];//��ȡ�ö�����		
//		type=U2(buff+4);//��ȡ�ö���������ID
//
//		/* check crc32 */
//		if (crc32(buff,len)!=U4(buff+len))
//		{
//			i+=len+4;
//			continue;
//		}
//		
//		msg =(U1(buff+6)>>4)&0x3;
//		week=U2(buff+14);
//		tow =U4(buff+16)*0.001;
//
//		if (msg!=0)  continue; /* message type: 0=binary,1=ascii */
//
//		printf("%4d%10.f%5d\n",week,tow,type);
//		
//		switch (type) {
//		case ID_RANGE         :
//			CopyArray(3, pos, obs->Pos);
//			memset(obs,0,sizeof(EPOCHOBSDATA));
//			CopyArray(3, obs->Pos, pos);
//			obs->Time.Week = week;
//			obs->Time.SecOfWeek = tow;
//			val=decode_rangeb(buff,obs);
//			break;
//		case ID_GPSEPHEM      : decode_gpsephem(buff,eph);   break;
//		case ID_IONUTC        : decode_ionutc(buff,&IonPara); break;
//		case ID_PSRPOS        : decode_psrpos(buff,obs->Pos);  break;
//		}
//		i+=len+4;
//	};
//	for(j=0;j<d-i;j++)     Buff[j]=Buff[i+j];	//Ϊ�����������ǰ��
//	d=j;
//	return val;
//}

/* sync header ---------------------------------------------------------------*/
int sync_oem4(unsigned char *buff, unsigned char data)
{
	buff[0]=buff[1]; buff[1]=buff[2]; buff[2]=data;
	return buff[0]==OEM4SYNC1&&buff[1]==OEM4SYNC2&&buff[2]==OEM4SYNC3;
}

/* crc-32 parity ---------------------------------------------------------------
* compute crc-32 parity for novatel raw
* args   : unsigned char *buff I data
*          int    len    I      data length (bytes)
* return : crc-32 parity
* notes  : see NovAtel OEMV firmware manual 1.7 32-bit CRC
*-----------------------------------------------------------------------------*/
unsigned int crc32(const unsigned char *buff, int len)
{
	int i,j;
	unsigned int crc=0;

	for (i=0;i<len;i++) 
	{
		crc^=buff[i];
		for (j=0;j<8;j++) 
		{
			if (crc&1) crc=(crc>>1)^POLYCRC32; 
			else crc>>=1;
		}
	}
	return crc;
}

/* decode rangeb -------------------------------------------------------------*/
int decode_rangeb(unsigned char *buff,EPOCHOBSDATA* obs)
{
	double lockt;
	int i,n,nobs,prn,sat,j;
	int track;
	GNSSSys sys;
	unsigned char *p=buff+OEM4HLEN;

	nobs=U4(p);
	n=0;
	for (i=0,p+=4;i<nobs;i++,p+=44) 
	{
		track=U4(p+40)&0x1F;	//ͨ������״̬
		sat=U2(p);						//���� PRN ��
		if(sat>=38 && sat<=61){
			prn=sat-37;
			sys=GLONASS;
		}
		else if(sat>=161 && sat<=197){
			prn=sat-160;
			sys=COMPASS;
		}
		else{
			prn=sat;
			sys=GPS;
		}

		for(j=0;j<=n;j++)
		{
			if(obs->SatObs[j].Prn==prn&&obs->SatObs[j].System==sys)
			{
				n=j;
				break;
			}
		}

		obs->SatObs[n].Prn=prn;
		obs->SatObs[n].System=sys;

		if(track==4) {
			obs->SatObs[n].c1=R8(p+4);	//��α�����ֵ���ף�
			obs->SatObs[n].l1=-R8(p+16);//�ز���λ���ܣ����ֶ����գ�
			obs->SatObs[n].l1*=(sys==GPS)? WL1_GPS: WL1_CPS;  //����Ϊ��λ
			obs->SatObs[n].d1=R4(p+28);	//˲ʱ�����գ�Hz��
			obs->SatObs[n].d1*=(sys==GPS)? WL1_GPS: WL1_CPS;
			lockt=R4(p+36);	//��������ʱ�䣬��
			//printf_s("%4d%9.1f  %C%2d L1%10.1f",obs->Time.Week,obs->Time.SecOfWeek,
			//	obs->SatObs[n].System==GPS? 'G':'C',obs->SatObs[n].Prn,lockt);
		}
		if(track==11)
		{
			obs->SatObs[n].p2=R8(p+4);
			obs->SatObs[n].l2=-R8(p+16);
			obs->SatObs[n].l2*=(sys==GPS)? WL2_GPS: WL2_CPS;
			lockt=R4(p+36);
			//printf_s(" L2%10.f\n",lockt);
		}
		
		n++;
	}
	obs->SatNum=n;
	obs->EpochFlag=0;
	return 1;
}

//������������
GPSEPHREC *gps;
int decode_gpsephem(unsigned char* buff, GPSEPHREC *eph)
{
	unsigned char *p=buff+OEM4HLEN;
	int prn;
	

	prn       =U4(p);           p+=4;		//���� PRN ���
	gps = eph+prn-1;
	gps->PRN = prn;
	gps->TOC.SecOfWeek = R8(p); p+=8;		//��֡ 0 ��ʱ������룩
	gps->SVHealth      = U4(p); p+=4;		//����״̬
	gps->IODE          = U4(p); p+=8;		//�������� ����
	gps->TOC.Week =gps->TOE.Week = U4(p); p+=8;	//���� GPS ʱ������ܼ�����GPS �ܣ�
	gps->TOE.SecOfWeek = R8(p); p+=8;		//�����ο�ʱ�䣬��  
	gps->SqrtA         = sqrt(R8(p)); p+=8;//��������ᣬ�� 
	gps->DetlaN        = R8(p); p+=8;		//����ƽ�����ٶȵĸ���ֵ������/��  
	gps->M0            = R8(p); p+=8;		//�ο�ʱ���ƽ����У�����
	gps->e             = R8(p); p+=8;		//ƫ����
	gps->omega         = R8(p); p+=8;		//���ص���ǣ�����
	gps->Cuc           = R8(p); p+=8;		//γ�ȷ��ǣ�������������ȣ� 
	gps->Cus           = R8(p); p+=8;		//γ�ȷ��ǣ�������������ȣ� 
	gps->Crc           = R8(p); p+=8;		//����뾶������������ף�  
	gps->Crs           = R8(p); p+=8;		//����뾶������������ף�  
	gps->Cic           = R8(p); p+=8;		//��ǣ�������������ȣ� 
	gps->Cis           = R8(p); p+=8;		//��ǣ�������������ȣ�  
	gps->i0            = R8(p); p+=8;		//�ο�ʱʱʱ�̹����ǣ����� 
	gps->iDot          = R8(p); p+=8;		//�����Ǳ仯�ʣ�����/��  
	gps->OMEGA         = R8(p); p+=8;		//������ྭ������
	gps->OMEGADot      = R8(p); p+=8;		//������ྭ�仯�ʣ�����/��  
	gps->AODC          = R8(p); p+=8;		//ʱ����������
	gps->TOC.SecOfWeek = R8(p); p+=8;		//�����Ӳ�ο�ʱ�䣬��  
	gps->TGD           = R8(p); p+=16;	//�豸ʱ�Ӳ�
	gps->ClkBias       = R8(p); p+=8;		//�����Ӳ�������루s�� 
	gps->ClkDrift      = R8(p); p+=8;		//�������ٲ�������s/s��  
	gps->ClkDriftRate  = R8(p); p+=20;	//������Ư��������s/s/s��  
	gps->SVAccuracy    = R8(p);					//�û����뾫��

	return 2;
}
//������������
int decode_ionutc(unsigned char* buff, IONOPARA* para)
{
	unsigned char *p=buff+OEM4HLEN;
	para->alpha[0] = R8(p); p+=8;
	para->alpha[1] = R8(p); p+=8;
	para->alpha[2] = R8(p); p+=8;
	para->alpha[3] = R8(p); p+=8;
	para->beta[0]  = R8(p); p+=8;
	para->beta[1]  = R8(p); p+=8;
	para->beta[2]  = R8(p); p+=8;
	para->beta[3]  = R8(p); p+=8;
	para->IsValid = 1;

	return 3;
}

//α�ඨλ��Ϣ����
int decode_psrpos(unsigned char* buff, double pos[])
{
	unsigned char *p=buff+OEM4HLEN+8;
	double x[3];
	x[0] = R8(p)*Rad;  p+=8;
	x[1] = R8(p)*Rad;  p+=8;
	x[2] = R8(p);  p+=8;
	x[2] += R4(p);

//	BLHToXYZ(x,pos,R_WGS84,F_WGS84);		//ʵ�ִӴ�����굽�ռ�ֱ�������ת��
	return 3;
}

