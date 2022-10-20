#include <stdio.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <pthread.h>
#include "../include/lcd.h"


#define RECV_MAX_SIZE 1024 
void Extraction(char *recvbuf, char *result); //语音ＩＤ提取函数
int  send_file(int socket_fd,const char *pcm_file);//tcp数据发送函数  
void send_cmd(char *cmd);//发送命令函数
void mplayer_init(void);//mplayer初始化
void Operate(char * Video_ID);//语音操作
void *thread_touch_ctrl(void * parg); //触屏线程函数
int touch_flag = 0;
int touch_voiceflag =0;
int Piccount = 1;
int socket_fd;  //
int fi;

//语音ID
char TurnON[10] = "1"; 
char TurnOFF[10] = "2"; 
char NextPic[10] = "3"; 
char LastPic[10] = "4";
char Playmusic_ctrl[10] = "5";
char Hello[10] = "7";
char Video[10] = "8"; 
char Playmusic[10] = "9"; 
char Chicken[10] = "10";
char Handsome[10] = "11";
char StopPlay[10] = "12";
char Exit[10] = "999"; 


//主函数
int main (int argc,char **argv)
{
	pthread_t tid;  //线程ID创建
	lcd_open(); 	//打开LCD设备 
	mplayer_init(); //播放器初始化
	//科大讯飞连接部分
	char recvbuf[RECV_MAX_SIZE]= {};
	char Voice[10] = {};
	socket_fd = socket(AF_INET,SOCK_STREAM,0);
	struct sockaddr_in	 dest_addr;
	dest_addr.sin_family 		= AF_INET;					
	dest_addr.sin_port   		= htons(54321);	            //匹配端口			
	dest_addr.sin_addr.s_addr	= inet_addr("192.168.0.101");	//连接科大讯飞本地服务端
	if(socket_fd<0)
	{
		perror("create socket fail:");	
		return -1;
	}
	if(connect(socket_fd,(struct sockaddr *)&dest_addr,sizeof dest_addr) < 0)
	{
		perror("connect fail:");
		return -1;
	}
	printf("connect successful!!\n");//科大讯飞连接成功
	pthread_create(&tid,NULL,thread_touch_ctrl,NULL);//触屏线程函数启动创建
	sleep(1); //等待系统初始化完成
	while(1) //语音交流操作循环
	{
		printf("录音按键位置X:(750-800) 视频&歌曲退出X:(700-750)\n"); //按键提示
		sleep(1);  //提示打印间隔时间
		while(touch_voiceflag) 
		{
			system("arecord -d3 -c1 -r16000 -traw -fS16_LE ../res/live.pcm"); //录音指令
			send_file(socket_fd,"../res/live.pcm");							//发送录音PCM文件
			printf("send succful!\n\n");	
			system("rm -rf ../res/live.pcm");									//发送完毕删除录音文件
			sleep(1); //发送与接收判断间隔1S
			if(recv(socket_fd, recvbuf, RECV_MAX_SIZE , 0) < 0)
			{
				perror("connect fail:");
				goto Exit;                   //接收失败 退出程序
			}
			Extraction(recvbuf, Voice);//获取语音ID
			Operate(Voice); //语音ID匹配操作
			if(strcmp(Voice, Exit) == 0) //需要结束主程序
			{
				lcd_draw_bmp(0,0,"1.bmp");
				system("mplayer haode.m4a &");
				sleep(3); //等待语音恢复完毕
				goto Exit;  //退出程序
			}
			memset(Voice, 0, sizeof(Voice)); //清除写入的ID
			touch_voiceflag = 0;//录音循环控制变量恢复
			touch_flag = 0;  //视频 歌曲 等待位恢复 Operate函数中使用
		}
	}
Exit: //语音ID判断为退出时候 结束客户端程序
	close(socket_fd);
	return 0;
}

//线程函数(LCD)
void *thread_touch_ctrl(void * parg)
{
	//初始化
    struct input_event buf;
    int x,y;
    int mute=0;
    int flag_ts=0;
    int fd_ts = open("/dev/ubuntu_event",O_RDWR);
    int next_pic_temp = 0;
    int last_pic_temp = 0;
    if(fd_ts < 0)
		perror("open /dev/ubuntu_event");

    printf("open ubuntu_event succful!\n");
	//坐标判断及其操作 一直运行
    while(1)
    {   
	    int len = read(fd_ts,&buf,sizeof(buf));
	    if(len <= 0)
	    {
			perror("read");
	    }

	    usleep(10*1000);
	    if(buf.type == EV_ABS && buf.code == ABS_X)
	    {
			x = buf.value;
			flag_ts|=0x01;
	    }

	    if(buf.type == EV_ABS && buf.code == ABS_Y)
	    {
		if(flag_ts & 0x01 == 0)
			y = buf.value;
		flag_ts|=0x02;	
	    }

		if(flag_ts == 0x03)
		{
		    memset(&buf,0,sizeof buf);
		    flag_ts=0;
		    if(x<100)
		    {
		        printf("pause or play\n");
		        send_cmd("pause\n");
		    }
		    if(x>=100 && x<200)
		    {
		        printf("volume 10 1\n");
		        send_cmd("volume 10 1\n");

		    }				
		    if(x>=200 && x<300)
		    {
		        printf("volume 50 1\n");
		        send_cmd("volume 50 1\n");
		    }	
		    if(x>=300 && x<400)
		    {
		        printf("volume 100 1\n");
		        send_cmd("volume 100 1\n");
		    }	
		    if(x>=400 && x<500)
		    {
		        mute=!mute;
		        if(mute)
		        {
		            printf("mute\n");
		            send_cmd("mute 1\n");					
		        }
		        else
		        {
		            printf("no mute\n");
		            send_cmd("mute 0\n");						
		        }
		    }	
		    if(x>=500 && x<600)
		    {
		        printf("seek -10\n");
		        send_cmd("seek -10\n");						
		    }			
		    if(x>=600 && x<700)
		    {
		        printf("seek 10\n");
		        send_cmd("seek 10\n");						
		    }				
		    if(x > 700 && x<750 )
		    {	
		       	printf("quit\n");				
				touch_flag = 1;
		    }
		    if(x > 750)
				touch_voiceflag = 1;
		}
    }
}

//提取语音ID 参数1 收到的数据  参数2 处理结果字符串
void Extraction(char *recvbuf, char *result)
{
	char text[512] = {};
	char id[10] = {};
	int Text_Start;
	int Text_End;
	int ID_Start;
	int ID_End;
	int b = 0;
	int c = 0;
	
	for (int i = 0; i < 1024; i++)
		if(recvbuf[i] == 'i' && recvbuf[i+1] == 'd' && recvbuf[i+2] == '=')
		{
			//ID extraction
			for(int j = i;;j++)
				if(recvbuf[j] == '"')
				{
					ID_Start = j+1;
					break;
				}
			for(int j = i + 4;;j++)
				if(recvbuf[j] == '"')
				{
					ID_End = j;
					break;
				}
			//ID前后定位 ID_Start && ID_End


			//写入结果字符
			for(int a = ID_Start; a < ID_End; a++)
			{
				result[c] = recvbuf[a];
				c++;
			}
			break;		
		}
}

//语音操作函数
void Operate(char * Voice_ID)
{
		//字符串不能使用switch 用if判断对应ID进行匹配操作
    	if(strcmp(Voice_ID, TurnON) == 0)
		{			
			lcd_draw_bmp(0,0,"../res/ONL.bmp");
			system("mplayer ../res/haode.m4a &");
			sleep(3);
			printf("开灯！\n");	
		}
		if(strcmp(Voice_ID, StopPlay) == 0)
		{			
			send_cmd("quit\n");
			system("mplayer ../res/haode.m4a &");
			sleep(3);	
		}
		if(strcmp(Voice_ID, TurnOFF) == 0)
		{
			lcd_draw_bmp(0,0,"../res/OFFL.bmp");
			system("mplayer ../res/haode.m4a &");
			sleep(3);	
			printf("关灯！\n");
				
		}
		if(strcmp(Voice_ID, Playmusic) == 0)

		{
			lcd_draw_bmp(0,0,"../res/OFFL.bmp");
			system("mplayer  ../res/music.m4a &"); 
			sleep(20);
		}
		
		if(strcmp(Voice_ID, Playmusic_ctrl) == 0)

		{
			lcd_draw_bmp(0,0,"../res/videoctrl.bmp");
			//system("mplayer  music.m4a &"); //-ac  mad 
			system("mplayer -slave -quiet -input file=/tmp/fifo -geometry 0:0 -zoom -x 800 -y 480 ../res/qingtian.mp3 &");
			while(touch_flag); //等待操控屏幕退出
		}
		if(strcmp(Voice_ID, NextPic) == 0)
		{
			Piccount++;
			if(Piccount == 7)
				Piccount = 6;
			switch(Piccount)
			{
				case 0:
					lcd_draw_bmp(0,0,"../res/1.bmp");
					break;
				case 1:
					lcd_draw_bmp(0,0,"../res/2.bmp");
					break;
				case 2:
					lcd_draw_bmp(0,0,"../res/3.bmp");
					break;
				case 3:
					lcd_draw_bmp(0,0,"../res/4.bmp");
					break;
				case 4:
					lcd_draw_bmp(0,0,"../res/5.bmp");
					break;
				case 5:
					lcd_draw_bmp(0,0,"../res/6.bmp");
					break;
				case 6:
					lcd_draw_bmp(0,0,"../res/7.bmp");
					break;
				default:
					;
			}
			system("mplayer ../res/haode.m4a &");
			sleep(3);
			printf("下一张图片！\n");
		}
		if(strcmp(Voice_ID, LastPic) == 0)
		{
			Piccount--;
			if(Piccount == -1)
				Piccount = 0;
			switch(Piccount)
			{
				case 0:
					lcd_draw_bmp(0,0,"../res/1.bmp");
					break;
				case 1:
					lcd_draw_bmp(0,0,"../res/2.bmp");
					break;
				case 2:
					lcd_draw_bmp(0,0,"../res/3.bmp");
					break;
				case 3:
					lcd_draw_bmp(0,0,"../res/4.bmp");
					break;
				case 4:
					lcd_draw_bmp(0,0,"../res/5.bmp");
					break;
				case 5:
					lcd_draw_bmp(0,0,"../res/6.bmp");
					break;
				case 6:
					lcd_draw_bmp(0,0,"../res/7.bmp");
					break;
				default:
					;
			}	
			system("mplayer ../res/haode.m4a &");
			sleep(3);
			printf("上一张图片！\n");	
		}
		if(strcmp(Voice_ID, Hello) == 0)

		{
			system("mplayer ../res/wozai.m4a &");
			sleep(3);
		}
		if(strcmp(Voice_ID, Handsome) == 0)
		{
			system("mplayer ../res/handsome.m4a &");
			sleep(3);
		}
		if(strcmp(Voice_ID, Video) == 0)
		{
			touch_flag = 0;
			lcd_draw_bmp(0,0,"../res/videoctrl.bmp");
			system("mplayer -slave -quiet -input file=/tmp/fifo -geometry 0:0 -zoom -x 1280 -y 720 ../res/zjx.flv &");
			printf("mplayer play ...\n");
			while(!touch_flag); //等待操控屏幕退出 主函数路由循环末尾恢复
		}	
}

//发送命令函数 发送到mplayer 从有名管道(fifo)输入控制命令
void send_cmd(char *cmd)
{
	write(fi,cmd,strlen(cmd));
}

//Mplayer播放器初始化函数   
void mplayer_init(void)
{
	if(access("/tmp/fifo",F_OK))
		mkfifo("/tmp/fifo",0777);  //创建有名管道
	
	fi = open("/tmp/fifo",O_RDWR);
}

//Tcp发送函数
int send_file(int socket_fd,const char *pcm_file)
{
	//读取PCM文件到buf
	int fd;
	fd = open(pcm_file,O_RDONLY);
	if(fd < 0)
	{
		printf("open %s\n",pcm_file);
		return -1;
	}
	off_t pcm_size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char *pcm_buf = calloc(1, pcm_size);
	read(fd, pcm_buf, pcm_size);

	//将buf中的数据发送到tcp服务端
	int m = send(socket_fd, pcm_buf, pcm_size,0); 
	free(pcm_buf);	
	return 0;
}

