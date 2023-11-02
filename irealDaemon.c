/*****************************************
@File    :  irealDaemon.c
@Time    :  2023/11/02 9:51
@Author  :  Hegs
@Version :  1.0
@Contact :  username@163.com
@License :  (C)Copyright 2022-2025
@Desc    :  对BK02的主程序rw_ireal.app进行守护
@other   :  查找守护进程:ps -ef | grep rw_ireal | grep -v grep
			退出守护进程:killall -9 rw_ireal
			编译:arm-linux-gnueabihf-gcc irealDaemon.c -o irealDaemon
/******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
//基本参数定义
#define DAEMON_TIME_DELAY   6
#define DAEMON_LOG_DIR      "/opt/"
#define DAEMON_LOG_FILE     "/opt/log/daemon.log"
#define DAEMON_FILE         "rw_ireal"
#define BUFSIZE             256
//日志添加
void logAdd(int fd,char *argc)
{
	char logBuf[128];
	time_t ti;
	ti = time(&ti);
	//记录				
	sprintf(logBuf,"[%s",ctime(&ti));
	logBuf[strlen(logBuf)-1] = '\0';
	sprintf(logBuf,"%s]:%s\n",logBuf,argc);
	write(fd, logBuf, strlen(logBuf));
	//memset(logBuf,0,sizeof(logBuf));
}
int executeCommand(const char *command, char *result)
{
    // 执行命令并将标准输出连接到文件流中
    FILE* fp = popen(command, "r");
    if (!fp)
    {
        perror("popen failed");
        return -1;
    }

    // 读取文件流中的数据
    char buf[BUFSIZE];
    while (fgets(buf, BUFSIZE, fp))
    {
        strcat(result, buf); // 将读取到的数据添加到结果中
    }
    pclose(fp); // 关闭文件流
    // 如果想去掉末尾的换行符可以直接把最最后一位换成结束符
    result[strlen(result)-1] = '\0';
    return 0;
}

//查询进程
int query_process()
{
	char result[BUFSIZE];
	char daemonFile[BUFSIZE];
	sprintf(daemonFile,"%s%s%s","ps -ef | grep ",DAEMON_FILE," | grep -v grep");
	executeCommand(daemonFile, result);
	//printf("%s", result);
    char *ptr = strstr(result, DAEMON_FILE);
    if (ptr != NULL) {
        //printf("字符串'world'在原字符串中的位置为：%ld\n", ptr - result);
		memset(result, 0, BUFSIZE);
		return 1;
    } else {
        //printf("未找到字符串irealDaemon\n");
		memset(result, 0, BUFSIZE);
		return 0;
    }
}
int main(int argc, const char *argv[])
{
	// 创建子进程
	pid_t pid = fork();
	if(pid > 0)
	{
		// 杀死父进程
		exit(0);
	}else if(0 == pid)
	{
		// 设置子进程为会话组组长
		setsid();
		// 更改工作路径为根目录
		chdir(DAEMON_LOG_DIR);
		// 修改文件权限掩码
		umask(0077);
		int size = getdtablesize();
		int i = 0;
		for(i = 3; i < size; i++)
		{
			// 关闭从父进程拷贝过来的多余的文件描述符
			close(i);
		}
		// 获取时间并写入到文件中
		system("rm -rf /opt/log/daemon.log > /dev/null 2>&1");
		int fd = open(DAEMON_LOG_FILE, O_CREAT | O_WRONLY, 0664);
		while(1)
		{
#if 0
			//测试文件中写时间
			time_t ti;
			sleep(10);
			ti = time(&ti);
			char *p = ctime(&ti);
			write(fd, p, strlen(p));
#else
			//实际监控rw_ireal.app进程
			sleep(DAEMON_TIME_DELAY);
			//检测进程
			if(query_process()==0){
				//system("pwd");
				system("nohup ./rw_ireal.app > /dev/null 2>&1  &");
				logAdd(fd,"rw_ireal.app restart successful!");
			}
#endif
		}
		close(fd);
	}
	return 0;
}
