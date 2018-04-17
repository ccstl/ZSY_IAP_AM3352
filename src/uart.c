#include     "common.h"
#define FALSE 1
#define TRUE 0

int speed_arr[] = {B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200};
int name_arr[] = {115200, 57600, 38400,  19200,  9600,  4800,  2400, 1200};

extern char *secname; 


/********************************************************************\
* ������: set_speed
* ˵��:
* ����:    ���ô��ڲ�����
* ����:     fd:�ļ�������
            speed:������
* ����ֵ:   0:�ɹ�����0:ʧ��
* ������:   Yang Chao Xu
* ����ʱ��: 2014-8-22
\*********************************************************************/
void set_speed(int fd, int speed)
{
    int   i;
    int   status;
    struct termios   Opt;
    tcgetattr(fd, &Opt);
    for(i = 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if(speed == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);
            cfsetispeed(&Opt, speed_arr[i]);
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);
            if(status != 0)
                perror("tcsetattr fd1");
            return;
        }
        tcflush(fd, TCIOFLUSH);
    }
}

/********************************************************************\
* ������: set_Parity
* ˵��:
* ����:    ���ô�������λ��ֹͣλ��У�鷽ʽ
* ����:     fd:�ļ�������
            databits:����λ
            stopbits:ֹͣλ
            parity:У�鷽ʽ
* ����ֵ:   0:�ɹ�����0:ʧ��
* ������:   Yang Chao Xu
* ����ʱ��: 2014-8-22
\*********************************************************************/
int set_Parity(int fd, int databits, int stopbits, int parity)
{
    struct termios options;
    if(tcgetattr(fd, &options)  !=  0)
    {
        perror("SetupSerial 1");
        return(FALSE);
    }
    options.c_cflag &= ~CSIZE;
    switch(databits)
    {
        case 7:
            options.c_cflag |= CS7;
            break;
        case 8:
            options.c_cflag |= CS8;
            break;
        default:
            fprintf(stderr, "Unsupported data size\n");
            return (FALSE);
    }
    switch(parity)
    {
        case 'n':
        case 'N':
            options.c_cflag &= ~PARENB;
            options.c_iflag &= ~INPCK;
            break;
        case 'o':
        case 'O':
            options.c_cflag |= (PARODD | PARENB);
            options.c_iflag |= INPCK;
            break;
        case 'e':
        case 'E':
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            options.c_iflag |= INPCK;
            break;
        case 'S':
        case 's':
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return (FALSE);
    }
    switch(stopbits)
    {
        case 1:
            options.c_cflag &= ~CSTOPB;
            break;
        case 2:
            options.c_cflag |= CSTOPB;
            break;
        default:
            fprintf(stderr, "Unsupported stop bits\n");
            return (FALSE);
    }

    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    options.c_oflag &= ~OPOST;
    options.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);

    /* Set input parity option */

    if(parity != 'n')
        options.c_iflag |= INPCK;
    options.c_cc[VTIME] = 150; // 15 seconds
    options.c_cc[VMIN] = 0;

    tcflush(fd, TCIFLUSH); /* Update the options and do it NOW */
    if(tcsetattr(fd, TCSANOW, &options) != 0)
    {
        perror("SetupSerial 3");
        return (FALSE);
    }
    return (TRUE);
}

void DealUartRecvData()
{
    if(Uart_Rx_Buff[0]==0xBB&&Uart_Rx_Buff[1]==0xEE&&Uart_Rx_Buff[4]==0xBB)//��ӡ�汾��
        printf("%s\n", &Uart_Rx_Buff[7]);
    else
        printf("%s\n", &Uart_Rx_Buff[0]);
    fflush(stdout);
}

void Uart_Receive_Thread_Select()
{
    int nread, ret;
    fd_set readfds;
    struct timeval timeout;
    timeout.tv_sec = 0;         //������
    timeout.tv_usec = 0;
    while(1)
    {
        if(UartStopRcvFlag==0)//�߳̽������ڴ�ӡ
        {
            FD_ZERO(&readfds);       //ÿ��ѭ����Ҫ��ռ��ϣ�select��������ļ����Ƿ��пɶ��ģ��Ӷ��ܼ���������仯
            FD_SET(Uart_fd, &readfds);
            ret = select(Uart_fd + 1, &readfds, NULL, NULL, &timeout); //select��������ļ����Ƿ��пɶ���
            usleep(10 * 1000); //�ȴ���������
            if(ret < 0)
            {
                printf("select error!\n");
            }
            //�����ļ����������ϣ�һֱ��ѭ���������������ı仯
            ret = FD_ISSET(Uart_fd, &readfds);
            if(ret > 0)
            {
                bzero(Uart_Rx_Buff, LENGTH);
                if((nread = read(Uart_fd, Uart_Rx_Buff, LENGTH)) > 0)   //��������
                {
                    Uart_Data_Len = nread;
                    DealUartRecvData();                    
                }
            }
        }
        else//�����ط�����
        {
            usleep(5 * 1000);
        }
    }
    return;
}



/********************************************************************\
* ��������: Uart_send_Thread
* ˵��:
* ����:     �������ݷ��ʹ���
* ����:     ��
* ����ֵ:   ��
* ������:   Yang Chao Xu
* ����ʱ��:  2014-8-22
\*********************************************************************/
void Uart_send_Thread(void)
{
    while(1)
    {
        if(Uart_Tx_Flag == 1)
        {
            Uart_Tx_Flag = 0;
            write(Uart_fd, Uart_Tx_Buff, Uart_Data_Len);
        }
        usleep(10 * 1000);
    }
    return;
}


/********************************************************************\
* ������: Uart_Pthread_Creat
* ˵��:
* ����:  ���ڵĽ��ܺͷ����̴߳���
* ����:
* ����ֵ:   0:�ɹ���0:ʧ��
* ������:   Yang Chao Xu
* ����ʱ��: 2014-8-22
\*********************************************************************/
int Uart_Pthread_Creat()
{
    int err;
    pthread_t receiveid, send;
    err = pthread_create(&receiveid, NULL, (void*)Uart_Receive_Thread_Select, NULL); //���������߳�
    if(err != 0)
    {
        printf("can't create Uart_Receive thread thread: %s\n", strerror(err));
        return 1;
    }
    else
        printf("create Uart_Receive thread thread OK\n");
    err = pthread_create(&send, NULL, (void*)Uart_send_Thread, NULL); //���������߳�
    if(err != 0)
    {
        printf("can't create Uart_send thread: %s\n", strerror(err));
        return 1;
    }
    else
        printf("create Uart_send thread OK\n");

    return 0;
}


/********************************************************************\
* ��������: Open_Uart
* ˵��:
* ����:     �򿪴���
* ����:     ���ں�
* ����ֵ:   0:�ɹ�-1:ʧ��
* ������:   Yang Chao Xu
* ����ʱ��:  2014-8-22
\*********************************************************************/
int Open_Uart(char *uart_no)
{
    Uart_fd= open(uart_no, O_RDWR);
    if(Uart_fd < 0)
    {
        printf("open device %s faild\n", uart_no);
        return(-1);
    }
    return 0;
}

void FCS_To_OPT_HeadandTail(Uchar cmdID,Ushort len)//֡ͷ+֡β
{
    Ushort crc;
    //֡ͷ
    Uart_Tx_Buff[0] = 0xAA;       //��ʼ�� AAH
    Uart_Tx_Buff[1] = 0xFF;       //��ʼ�� FFH
    Uart_Tx_Buff[2] = conf.Gun_num;//ǹ�ź� ��Ŀ���豸
    Uart_Tx_Buff[3] = 0x00;       //Դ�豸
    Uart_Tx_Buff[4] = cmdID;      //������ 01H~90H
    Uart_Tx_Buff[5] = len>>8;
    Uart_Tx_Buff[6] = (Uchar)len;
    //֡β
    crc=ModbusCrc16(&Uart_Tx_Buff[2], len+5);
    Uart_Tx_Buff[len+7] = crc>>8;
    Uart_Tx_Buff[len+8] = (Uchar)crc;
    Uart_Tx_Buff[len+9] = 0xCC; //������CC
    Uart_Tx_Buff[len+10] = 0xFF; //������FF
    Uart_Tx_Flag=1;
    Uart_Data_Len=len+11;
}


//����reboot dispenser����
void SendRebootDispenser()
{
    Uchar tt=7;
    Uart_Tx_Buff[tt++]=0xAA;
    Uart_Tx_Buff[tt++]=0x00;
    Uart_Tx_Buff[tt++]=0xAA;
    Uart_Tx_Buff[tt++]=0x00;
    FCS_To_OPT_HeadandTail(0xAA,tt-7);    
}

//����IAP����
void SendIAP()
{
    Uchar tt=0;
    Uart_Tx_Buff[tt++]='I';
    Uart_Tx_Buff[tt++]='A';
    Uart_Tx_Buff[tt++]='P';
    Uart_Tx_Flag=1;
    Uart_Data_Len=tt;
}

//quit bootloader
void QuitBootloader()
{
    Uchar tt=0;
    Uart_Tx_Buff[tt++]='q';
    Uart_Tx_Flag=1;
    Uart_Data_Len=tt;
}
//��ȡ�汾��
void GetVersion()
{
    Uchar tt=7;
    Uart_Tx_Buff[tt++]=0xBB;
    Uart_Tx_Buff[tt++]=0x00;
    Uart_Tx_Buff[tt++]=0xBB;
    Uart_Tx_Buff[tt++]=0x00;
    FCS_To_OPT_HeadandTail(0xBB,tt-7); 
}
