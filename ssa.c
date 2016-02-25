// ssa.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"

#include "stdio.h"

typedef short                       INT16;
typedef int       			INT32;
typedef unsigned int               UINT32;
typedef  long long				INT64;
//typedef  __int64				INT64;
#define APP_INPUT_BUFF_SIZE  (0x1000)
#define FM_SSA_THRESHOLD (0x20)
#define LINE_IN_SSA_THRESHOLD (0x10)
#define SSA_FIRST_ORDER_CNT_BITS (12)
#define SSA_SECOND_ORDER_CNT_BITS (6)
#define SSA_FIRST_ORDER_CNT (0x1<<SSA_FIRST_ORDER_CNT_BITS)
#define SSA_SECOND_ORDER_CNT (0x1<<SSA_SECOND_ORDER_CNT_BITS)
#define SSA_ATTENUATION (4)
#define SSA_UNATTEN_SAMPLE_NUM  10
#define SSA_UNATTEN_SAMPLE_THRESHOLD    32


INT16 app_in_buff_left[APP_INPUT_BUFF_SIZE];//0x1000
INT16 app_in_buff_right[APP_INPUT_BUFF_SIZE];//0x1000
typedef enum {
    AIO_CH_SRC_MIDIA = 0, //media mode
    AIO_CH_SRC_SIF,       ///ATV mode
    AIO_CH_SRC_I2S,     //I2s mode
    AIO_CH_SRC_LINEIN0,
    AIO_CH_SRC_LINEIN1,
    APP_SCR_TYPE_NB            ///< Number of app of source type
} AIO_CH_SRC_ENUM;
static UINT32 ssa_attenuation_r=0;
static UINT32 ssa_attenuation_l=0;
INT16 ssa_threshold;
void serv_app_msg_ssa(INT16 *databufferIn_l,INT16 *databufferIn_r,INT16 len)
{
    static UINT32 ssa_count_l1 = 0,ssa_count_r1 = 0;
    static UINT32 ssa_count_l2 = 0,ssa_count_r2 = 0;
    static INT32 ssa_average_r1 = 0 ,ssa_average_l1 = 0,ssa_average_r2 = 0 ,ssa_average_l2 = 0;
    static INT64 ssa_sum_l1=0,ssa_sum_r1 = 0,ssa_sum_r2=0 ,ssa_sum_l2 = 0;
    static UINT32 ssa_release_cnt_l =0,ssa_release_cnt_r =0;

    static UINT32 dcf_count_l1 = 0,dcf_count_r1 = 0;
    static UINT32 dcf_count_l2 = 0,dcf_count_r2 = 0;
    static INT16 dcf_average_r1 = 0 ,dcf_average_l1 = 0,dcf_average_r2 = 0 ,dcf_average_l2 = 0;
    static INT64 dcf_sum_l1=0,dcf_sum_r1 = 0,dcf_sum_r2=0 ,dcf_sum_l2 = 0;

    INT16 i,j;

    if(len > 0) {
        for(j = 0; j < len; j++) {
            dcf_sum_l1 += databufferIn_l[j];
            databufferIn_l[j] -= dcf_average_l2;
            dcf_sum_r1 += databufferIn_r[j];
            databufferIn_r[j] -= dcf_average_r2;
            switch (APP_SCR_TYPE_NB) {
            case AIO_CH_SRC_I2S:
            case AIO_CH_SRC_MIDIA: {
                databufferIn_l[j] = (((INT32)databufferIn_l[j])*2150)>>13;
                databufferIn_r[j] = (((INT32)databufferIn_r[j])*2150)>>13;
                break;
            }
            case AIO_CH_SRC_SIF:
            case AIO_CH_SRC_LINEIN0:
            case AIO_CH_SRC_LINEIN1: {
                break;
            }
            default:
                break;
            }
            if(databufferIn_l[j] >=0) {
                ssa_sum_l1 += databufferIn_l[j];
            } else {
                ssa_sum_l1 += (-databufferIn_l[j]);
            }
            if((ssa_attenuation_l>0)&&
                    (databufferIn_l[j] > SSA_UNATTEN_SAMPLE_THRESHOLD||
                     (-databufferIn_l[j])> SSA_UNATTEN_SAMPLE_THRESHOLD)) {
                if(++ssa_release_cnt_l > SSA_UNATTEN_SAMPLE_NUM) {
                    ssa_attenuation_l = 0;
                    ssa_count_l1 = 0;
                    ssa_count_l2 = 0;
                    ssa_average_l1 = 0;
                    ssa_average_l2 = 0;
                    ssa_sum_l1 = 0;
                    ssa_sum_l2 = 0;
                    ssa_release_cnt_l = 0;
                }
            } else {
                ssa_release_cnt_l = 0;
            }

            if(++ssa_count_l1 == SSA_FIRST_ORDER_CNT) {

                ssa_average_l1 =ssa_sum_l1 >> SSA_FIRST_ORDER_CNT_BITS;
                ssa_sum_l1=0;
                ssa_count_l1 = 0;
                ssa_sum_l2 += ssa_average_l1;
		 //printf("ssa_average_l1 = %d\n",ssa_average_l1);
                dcf_average_l1 =dcf_sum_l1 >> SSA_FIRST_ORDER_CNT_BITS;
                dcf_sum_l1=0;
                dcf_sum_l2 += dcf_average_l1;

                if(++ssa_count_l2 == SSA_SECOND_ORDER_CNT) {
                    ssa_average_l2 =ssa_sum_l2 >> SSA_SECOND_ORDER_CNT_BITS;
                    ssa_sum_l2=0;
                    ssa_count_l2 = 0;

                    dcf_average_l2 =dcf_sum_l2 >> SSA_SECOND_ORDER_CNT_BITS;
                    dcf_sum_l2=0;
                    dcf_count_l2 = 0;
			printf("ssa_average_l2 = %d\n",ssa_average_l2);		
                    if(ssa_average_l2 < ssa_threshold) {
                        ssa_attenuation_l = SSA_ATTENUATION;
                    } else {
                        ssa_attenuation_l = 0;
                    }
		      printf("ssa_attenuation_l = %d\n",ssa_attenuation_l);
                }

            }
            if(databufferIn_r[j] >=0) {
                ssa_sum_r1 += databufferIn_r[j];
            } else {
                ssa_sum_r1 += (-databufferIn_r[j]);
            }

            if((ssa_attenuation_r>0)&&
                    (databufferIn_r[j] > SSA_UNATTEN_SAMPLE_THRESHOLD||
                     (-databufferIn_r[j])> SSA_UNATTEN_SAMPLE_THRESHOLD)) {
                if(++ssa_release_cnt_r > SSA_UNATTEN_SAMPLE_NUM) {
                    ssa_attenuation_r = 0;
                    ssa_count_r1 = 0;
                    ssa_count_r2 = 0;
                    ssa_average_r1 = 0;
                    ssa_average_r2 = 0;
                    ssa_sum_r1 = 0;
                    ssa_sum_r2 = 0;
                    ssa_release_cnt_r = 0;
                }
            } else {
                ssa_release_cnt_r = 0;
            }

            if(++ssa_count_r1 == SSA_FIRST_ORDER_CNT) {
                ssa_average_r1 =ssa_sum_r1 >> SSA_FIRST_ORDER_CNT_BITS;
                ssa_sum_r1=0;
                ssa_count_r1 = 0;
                ssa_sum_r2 += ssa_average_r1;

                dcf_average_r1 =dcf_sum_r1 >> SSA_FIRST_ORDER_CNT_BITS;
                dcf_sum_r1=0;
                dcf_sum_r2 += dcf_average_r1;
                if(++ssa_count_r2 == SSA_SECOND_ORDER_CNT) {
                    ssa_average_r2 =ssa_sum_r2 >> SSA_SECOND_ORDER_CNT_BITS;
                    ssa_sum_r2=0;
                    ssa_count_r2 = 0;

                    dcf_average_r2 =dcf_sum_r2 >> SSA_SECOND_ORDER_CNT_BITS;
                    dcf_sum_r2=0;
                    dcf_count_r2 = 0;
                    if(ssa_average_r2 < ssa_threshold) {
                        ssa_attenuation_r = SSA_ATTENUATION;
                    } else {
                        ssa_attenuation_r = 0;
                    }

                }

            }

        }

    }
}
int main(int argc, char* argv[])
{
	FILE *fp_in,*fp_out,*fp_test;
	int cnt,n;
	fp_in  = fopen("ssa_small.bin","rb");//4//ssa_small.bin ssa_large.bin
	if (fp_in == NULL)
	{
		printf("aaaaab\n");
		return -1;
	}
	fp_out  = fopen("48_1K_16bit_out.bin","wb");//48_1K_16bit_out.bin 44_1K_16bit_out.bin 96_1K_16bit_out.bin 
	if (fp_out == NULL)
	{	
		printf("ddddd\n");
		return -2;
	}
	ssa_threshold=LINE_IN_SSA_THRESHOLD;
	while(1)
	{
		n = fread(&app_in_buff_left,2,APP_INPUT_BUFF_SIZE,fp_in);
		//printf("read n = %d\n",n);
		if (n != APP_INPUT_BUFF_SIZE)
		{
			serv_app_msg_ssa(app_in_buff_left,app_in_buff_right,n);
			fwrite(app_in_buff_left,2,n,fp_out);
			printf("finished \n");
			break;
		}
		serv_app_msg_ssa(app_in_buff_left,app_in_buff_right,n);
		fwrite(app_in_buff_left,2,n,fp_out);
	}
	
	fclose(fp_in);
	fclose(fp_out);
	printf("Hello World!\n");
	return 0;
}


