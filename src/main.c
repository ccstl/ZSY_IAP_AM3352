#include "common.h"


Uchar Gun_num=0;//�ӳ�������ʱ�õ��Ĳ����������ý���ʹ���ĸ�����


void Print_Conf()
{
    printf("Gun_num=%d\n", conf.Gun_num); 
    printf("Uart_Speed=%d\n", conf.Uart_Speed);
    printf("Uart_NO=%s\n", conf.Uart_NO);
    printf("Uart_Data_Bits=%d\n", conf.Uart_Data_Bits);
    printf("Uart_Stop_Bits=%d\n", conf.Uart_Stop_Bits);
    printf("Uart_Parity=%c\n", conf.Uart_Parity[0]);
}


char *secname; 
void init_Read_Conf()
{    
    iniFileLoad(inifile_Name);
          
    //����ǹ��Ϣ
    switch(Gun_num)
    {
        case 1:
            secname="Gun_Info1";
        break;
        case 2:
            secname="Gun_Info2";
        break;
        case 3:
            secname="Gun_Info3";
        break;
        case 4:
            secname="Gun_Info4";
        break;
    }
    conf.Gun_num = iniGetInt(secname, "Gun_num", 255);   
    iniGetString(secname, "Uart_NO", conf.Uart_NO,sizeof(conf.Uart_NO), "Uart_NO fail");
    conf.Uart_Speed = iniGetInt(secname, "Uart_Speed", 38400);
    conf.Uart_Data_Bits = iniGetInt(secname, "Uart_Data_Bits", 8);
    conf.Uart_Stop_Bits = iniGetInt(secname, "Uart_Stop_Bits", 1);
    iniGetString(secname, "Uart_Parity", (char *)conf.Uart_Parity,sizeof(conf.Uart_Parity), "Uart_Parity fail");
}

void init_Uart()
{
    Open_Uart(conf.Uart_NO);
    set_speed(Uart_fd, conf.Uart_Speed); //���ô��ڲ�����
    set_Parity(Uart_fd, conf.Uart_Data_Bits, conf.Uart_Stop_Bits, conf.Uart_Parity[0]); //����8λ����λ��1λֹͣλ����У����������á�
    Uart_Pthread_Creat();
}

void init_sys()
{
    
#ifdef Debug
    Print_Conf();
#endif
    //���ڳ�ʼ��
    init_Uart();  
}

static char *BinFileBuffer;
static int BinFilelen;
int InitBinFile(const char *filename)
{
    FILE *file;
    int len;

    if (strlen(filename) >= 256)
        return 0;

    file = fopen(filename, "rb");
    if (file == NULL) 
        return 0;

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    BinFileBuffer = malloc(len);
    if (BinFileBuffer == NULL) {
        fclose(file);
        return 0;
    }

    fseek(file, 0, SEEK_SET);
    len = fread(BinFileBuffer, 1, len, file);
    fclose(file);
    BinFilelen = len;
    printf("FileLen=%d\n",BinFilelen);
    return 1;       
}

extern char *gBuffer;
extern int gBuflen;
//�ͷ��ļ���ռ��Դ
void FileFree()
{
	if (gBuffer != NULL) {
		free(gBuffer);
		gBuffer = 0;
		gBuflen = 0;
	}
    if (BinFileBuffer != NULL) {
		free(BinFileBuffer);
		BinFilelen = 0;
		BinFilelen = 0;
	}
}


int main(int argc, char *argv[])
{
    char c;
    if(argc!=3) //�����ǹ��Ϣ�������ļ��� 
    {
        printf("Usage:./iap_zsy gun_num filename\n");
        return 0;   
    }
    Gun_num=atoi(argv[1]);
    InitBinFile(argv[2]);
    if((Gun_num==0)||(Gun_num>4)) return 0;//���4��ǹ
    system("jksoft stop");
    sleep(1);
    //��ȡ�����ļ�
    init_Read_Conf(Gun_num);
    init_sys();
    while(1)
    {
        printf("*****************JXGF_IAP*****************\n");
        printf("******************************************\n");
        printf("****        1-Reboot Dispenser        ****\n");
        printf("****        2-Send   IAP Command      ****\n");
        printf("****        3-Update Program          ****\n");
        printf("****        4-Quit   Bootloader       ****\n");
        printf("****        5-Get    Version          ****\n");
        printf("****        E-Exit   Application      ****\n");
        printf("******************************************\n");
        printf("*****************JXGF_IAP*****************\n");
        c=getchar();
        switch(c)
        {
            case '1':
                SendRebootDispenser();
            break;
            case '2':
                SendIAP();
            break;
            case '3':
                xmodemTransmit((Uchar *)BinFileBuffer, BinFilelen);
            break;
            case '4': 
                QuitBootloader();
            break;
            case '5': 
                GetVersion();
            break;
            case 'e':
            case 'E':
                FileFree();
                return 0;
            break;
        }
    }
    
    return 0;
}
