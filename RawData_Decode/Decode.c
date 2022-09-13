/****************************************************************************
目的：    实现Novatel和Unicore接收机的二进制数据解码，获得观测数据和广播星历

编写时间：2014.05.09
作者：    王甫红
版本:     V1.0
版权：    武汉大学测绘学院

修改时间：2020.08.02
作者：    彭皆彩
版本:     V1.1
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
   目的：将一个数组拷贝到另一个数组中( 多维数组均以一维表示 )  
  编号：01018

  参数:
  n      拷贝的数组元素个数
  Dist   目标数组
  Sour   源数组
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
//				break;//退出for循环
//			}
//		}
//
//		if(i+OEM4HLEN>=d)         break;//退出while循环
//		for(j=0;j<OEM4HLEN;j++)   buff[j]=Buff[i+j];
//		len=U2(buff+8)+OEM4HLEN;	//数据总长度
//		if ((len+4+i)>d) 	      break;//数据末尾位置大于buff大小
//		for(j=OEM4HLEN;j<len+4;j++) buff[j]=Buff[i+j];//获取该段数据		
//		type=U2(buff+4);//获取该段数据数据ID
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
//	for(j=0;j<d-i;j++)     Buff[j]=Buff[i+j];	//为处理过的数据前移
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
		track=U4(p+40)&0x1F;	//通道跟踪状态
		sat=U2(p);						//卫星 PRN 号
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
			obs->SatObs[n].c1=R8(p+4);	//码伪距测量值（米）
			obs->SatObs[n].l1=-R8(p+16);//载波相位，周（积分多普勒）
			obs->SatObs[n].l1*=(sys==GPS)? WL1_GPS: WL1_CPS;  //以米为单位
			obs->SatObs[n].d1=R4(p+28);	//瞬时多普勒（Hz）
			obs->SatObs[n].d1*=(sys==GPS)? WL1_GPS: WL1_CPS;
			lockt=R4(p+36);	//连续跟踪时间，秒
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

//卫星星历解码
GPSEPHREC *gps;
int decode_gpsephem(unsigned char* buff, GPSEPHREC *eph)
{
	unsigned char *p=buff+OEM4HLEN;
	int prn;
	

	prn       =U4(p);           p+=4;		//卫星 PRN 编号
	gps = eph+prn-1;
	gps->PRN = prn;
	gps->TOC.SecOfWeek = R8(p); p+=8;		//子帧 0 的时间戳（秒）
	gps->SVHealth      = U4(p); p+=4;		//健康状态
	gps->IODE          = U4(p); p+=8;		//星历数据 龄期
	gps->TOC.Week =gps->TOE.Week = U4(p); p+=8;	//基于 GPS 时间的整周计数（GPS 周）
	gps->TOE.SecOfWeek = R8(p); p+=8;		//星历参考时间，秒  
	gps->SqrtA         = sqrt(R8(p)); p+=8;//轨道长半轴，米 
	gps->DetlaN        = R8(p); p+=8;		//卫星平均角速度的改正值，弧度/秒  
	gps->M0            = R8(p); p+=8;		//参考时间的平近点叫，弧度
	gps->e             = R8(p); p+=8;		//偏心率
	gps->omega         = R8(p); p+=8;		//近地点幅角，弧度
	gps->Cuc           = R8(p); p+=8;		//纬度幅角（余弦振幅，弧度） 
	gps->Cus           = R8(p); p+=8;		//纬度幅角（正弦振幅，弧度） 
	gps->Crc           = R8(p); p+=8;		//轨道半径（余弦振幅，米）  
	gps->Crs           = R8(p); p+=8;		//轨道半径（正弦振幅，米）  
	gps->Cic           = R8(p); p+=8;		//倾角（余弦振幅，弧度） 
	gps->Cis           = R8(p); p+=8;		//倾角（正弦振幅，弧度）  
	gps->i0            = R8(p); p+=8;		//参考时时时刻轨道倾角，弧度 
	gps->iDot          = R8(p); p+=8;		//轨道倾角变化率，弧度/秒  
	gps->OMEGA         = R8(p); p+=8;		//升交点赤经，弧度
	gps->OMEGADot      = R8(p); p+=8;		//升交点赤经变化率，弧度/秒  
	gps->AODC          = R8(p); p+=8;		//时钟数据龄期
	gps->TOC.SecOfWeek = R8(p); p+=8;		//卫星钟差参考时间，秒  
	gps->TGD           = R8(p); p+=16;	//设备时延差
	gps->ClkBias       = R8(p); p+=8;		//卫星钟差参数，秒（s） 
	gps->ClkDrift      = R8(p); p+=8;		//卫星钟速参数，（s/s）  
	gps->ClkDriftRate  = R8(p); p+=20;	//卫星钟漂参数，（s/s/s）  
	gps->SVAccuracy    = R8(p);					//用户距离精度

	return 2;
}
//电离层改正参数
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

//伪距定位信息解码
int decode_psrpos(unsigned char* buff, double pos[])
{
	unsigned char *p=buff+OEM4HLEN+8;
	double x[3];
	x[0] = R8(p)*Rad;  p+=8;
	x[1] = R8(p)*Rad;  p+=8;
	x[2] = R8(p);  p+=8;
	x[2] += R4(p);

//	BLHToXYZ(x,pos,R_WGS84,F_WGS84);		//实现从大地坐标到空间直角坐标的转换
	return 3;
}

