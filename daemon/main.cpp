#include <vector>
#include <yaml-cpp/yaml.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>  
#include <string>  
#include <sstream>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h> 
#include <fstream> 
#include <vector>
#include <string.h>
#include <algorithm>

#include <cctype>
#include <algorithm>
 
#include <sys/stat.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

#include <signal.h>

#include <iostream>
#include <cstdlib>
#include <unistd.h>

#include<sys/types.h>
#include<sys/wait.h>

#include "daemon.h"
 
using namespace std;
using namespace handy;

struct envv {
    std::string env;
};

struct daemonrun {
    std::string netdev;
	std::string bindvip;
	std::string binrun;
	int  retimes; 
    std::vector <envv> envlist;
	int  runstartimes; 
	bool resetruntimes;
	bool resetdelnetvip;
};
 


static  char baserunpath[FILENAME_MAX];
static  char baselogpath[FILENAME_MAX];
static  char baseconfyaml[FILENAME_MAX];
static  std::vector<daemonrun> daemonrunload;


void nowtime2str(char *str){
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);
	
    int year = local_time->tm_year + 1900;
    int month = local_time->tm_mon + 1;
    int day = local_time->tm_mday;
    int hour = local_time->tm_hour;
    int minute = local_time->tm_min;
    int second = local_time->tm_sec;
	sprintf(str, "current time: %d-%d-%d %d:%d:%d\n", year, month, day, hour, minute, second);
	return ;
}


int writeversion(const char *versionfile) {

    int lfp = open(versionfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (lfp < 0 || lockf(lfp, F_TLOCK, 0) < 0) {
        fprintf(stderr, "Can't write version File: %s", versionfile);
        return -1;
    }
    ExitCaller call1([=] { close(lfp); });
	char* version = "Hello, Lixingbo, this daemon is v1.0\n";
    ssize_t len = strlen(version);
    ssize_t ret = write(lfp, version, len);
    if (ret != len) {
        fprintf(stderr, "Can't Write version File: %s", versionfile);
        return -1;
    }
    return 0;
}


int writeresettime(const char *resetfile) {

    int lfp = open(resetfile, O_WRONLY | O_CREAT | O_APPEND, 0600);
    if (lfp < 0 || lockf(lfp, F_TLOCK, 0) < 0) {
        fprintf(stderr, "Can't write Pid File: %s", resetfile);
        return -1;
    }
    ExitCaller call1([=] { close(lfp); });
	
	char str[256] = {0};
	nowtime2str(str);

    ssize_t len = strlen(str);
    ssize_t ret = write(lfp, str, len);
    if (ret != len) {
        fprintf(stderr, "Can't Write Pid File: %s", resetfile);
        return -1;
    }
    return 0;
}

int writeresetpid(const char *pidfile) {
    char str[32];
    int lfp = open(pidfile, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (lfp < 0 || lockf(lfp, F_TLOCK, 0) < 0) {
        fprintf(stderr, "Can't write Pid File: %s", pidfile);
        return -1;
    }
    ExitCaller call1([=] { close(lfp); });
	
    sprintf(str, "%d\n", getpid());
    ssize_t len = strlen(str);
    ssize_t ret = write(lfp, str, len);
    if (ret != len) {
        fprintf(stderr, "Can't Write Pid File: %s", pidfile);
        return -1;
    }
    return 0;
}

int writeresetenv(int lfp, const char *env) {

	printf("writeresetenv:%s\n", env);
	char str[2048] = {0};
	sprintf(str, "%s\n", env);
	
    ssize_t len = strlen(str);
    ssize_t ret = write(lfp, str, len);
    if (ret != len) {
        fprintf(stderr, "Can't Write env File");
        return -1;
    }
    return 0;
}


// 按delim分割字符串s进入sv
void split(const std::string& s, std::vector<std::string>& sv, const char delim) 
{
    sv.clear();
    std::istringstream iss(s);
    std::string temp;

    while (std::getline(iss, temp, delim)) {
        if (temp == "") {
            continue;//单纯的空字符不要
        }
        sv.emplace_back(std::move(temp));
    }
    return;
}

int daemonsta()
{
    FILE *fp = NULL;
    char buf[100]={0};
    fp = popen("ps -e -o args | cut -f 1 -d' '| grep /daemon |grep -v grep |sort |wc -l", "r");
    if(fp) 
    {
    
        int ret =  fread(buf,1,sizeof(buf)-1,fp);
        if(ret > 0) 
        {
            //printf("<SystemStatus> buf:%s!\n", buf);
        }
        pclose(fp);
    }
	return atoi(buf);
}


// 二进制绝对路径
bool isrun(std::string program){

    int fd = open(program.c_str(),  O_WRONLY);
    if (fd == -1)
	{
		return false;
	}	
	close(fd);
	return true;
}


void setprogrampath(const char* program){
	char path_buff[PATH_MAX];
	strncpy(path_buff,program,strlen(program));
	for(int i= strlen(program);i >=0 ;--i){
		if(path_buff[i]=='/')
        {
            path_buff[i+1]='\0';
            break;
        }
    }
	chdir(path_buff);
	return;
}

const char* getprogramname(const char* program){
	for(int i= strlen(program);i >=0 ;--i){
		if(program[i]=='/')
        {	
            return (char*)program + i + 1;
        }
    }
	return program;
}

void cleanUp(int sig)
{
	int status;
	wait(&status);
	cout << "The parent process has kill the child process successfully!" << endl;
	cout << "The exit num of child process"  << endl;
}
 
/**********设置进程对于SIGCHLD信号的相应*********/
void sigClean()
{
	struct sigaction act, oldact;
	act.sa_handler  = cleanUp;
	sigaction(SIGCHLD,&act,&oldact); //Linux中，子进程退出的时候将自动向父进程发送SIGCHLD信号
}


void pdaemonrunstart(daemonrun &daemonrunone, char* myargv[16], char* mypenv[256]){
		
	if(isrun(myargv[0])){

		printf("myargv[0]: %s, daemonrunone.runstartimes: %d\n", myargv[0], daemonrunone.runstartimes);	

		daemonrunone.resetruntimes = false;
		string programname = getprogramname(myargv[0]);
		string pidpro = programname + ".pid";
		string resetpro = programname + ".reset";
		string resetenv = programname + ".env";

		if (daemonrunone.runstartimes < daemonrunone.retimes){
			pid_t id = fork();//接收返回值
			if(id == 0){

				printf("子进程pid: %d, 父进程pid: %d\n", getpid(), getppid());
				chdir(baselogpath);

				writeresetpid(pidpro.c_str());
				writeresettime(resetpro.c_str());
			
				printf("binrun:%s\n", myargv[0]);

    			int lfp = open(resetenv.c_str(), O_WRONLY | O_CREAT, 0600);
    			if (lfp < 0 || lockf(lfp, F_TLOCK, 0) < 0) {
        			fprintf(stderr, "Can't open env File: %s", resetenv.c_str());
        			return;
    			}
   	 			ExitCaller call1([=] { close(lfp); });

				for(int i = 0; i < 16; ++i){
					if(myargv[i]){
						printf("myargv:%s\n", myargv[i]);
					}
				} 
				for(int i = 0; i < 256; ++i){
					if(mypenv[i]){
						printf("mypenv:%s\n", mypenv[i]);
						writeresetenv(lfp, mypenv[i]);
					}
				} 

				setprogrampath(myargv[0]);
				setuid(0);
				execve(myargv[0], myargv, mypenv);
				exit(0);  //  exit
			}else if(-1 == id){
				printf("fork error\n");
			}else{
				daemonrunone.runstartimes = daemonrunone.runstartimes + 1;
       			
			}
			
		}else{
			daemonrunone.resetruntimes = true;
			char netvip[128] = {0};
			sprintf(netvip, "ip addr del %s dev %s", daemonrunone.bindvip.c_str(), daemonrunone.netdev.c_str());
			if(daemonrunone.resetdelnetvip){
				setuid(0);
				system(netvip);
				daemonrunone.resetdelnetvip = false;
			}
		}
	}else{
		if(daemonrunone.resetruntimes)
			daemonrunone.runstartimes = 0;
		daemonrunone.resetdelnetvip = true;
	}

}

void pdaemonrunenv(char *envp[], daemonrun &daemonrunone, char* myargv[16], char* mypenv[256]){

	vector<string> sp;
	split(daemonrunone.binrun, sp, ' ');

	int i = 0;
	for(i = 0; i < sp.size(); i++){
		strncpy(myargv[i], (char*)sp[i].c_str(), 64);
	}
	for(int n = i; n < 16; ++n){
		delete myargv[n];
		myargv[n] = {0};
	} 

	int m = 0;
	for (m = 0; envp[m] != 0; m++){

		
		char *pDest = strstr(envp[m], "LD_LIBRARY_PATH=");
		if (NULL != pDest) 
		{
			strncpy(mypenv[m], "LD_LIBRARY_PATH_REMOVE=Lixingbo", 2048);
		}else{
			strncpy(mypenv[m], envp[m], 2048);
		}
	}
	for_each(daemonrunone.envlist.begin(),daemonrunone.envlist.end(),[&](envv &envvone){
		strncpy(mypenv[m], (char*)envvone.env.c_str(), 2048);
		m++;
	});
	
	for(int n = m; n < 256; ++n){
		delete mypenv[n];
		mypenv[n] = {0};
	} 
}

void programrun(char *envp[], daemonrun &daemonrunone){

	char *myargv[16];
	for(int i = 0; i < 16; ++i){
		myargv[i] = new char[64];
		memset(myargv[i], 0, 64);
	}
	char *mypenv[256];
	for(int i = 0; i < 256; ++i){
		mypenv[i] = new char[2048];
		memset(mypenv[i], 0, 2048);
	}
	
	pdaemonrunenv(envp, daemonrunone, (char**)myargv, (char**)mypenv);
	pdaemonrunstart(daemonrunone, (char**)myargv, (char**)mypenv);

	for(int i = 0; i < 16; ++i){
		if(myargv[i]){
			delete myargv[i];
		}
	} 
	for(int i = 0; i < 256; ++i){
		if(mypenv[i]){
			delete mypenv[i];
		}
	} 
}

void daemonrunloop(char *envp[]){
	
	while(1){
		printf("");
		for_each(daemonrunload.begin(),daemonrunload.end(),[&](daemonrun &daemonrunone){
			programrun(envp, daemonrunone);
		});
		sleep(2);		
	}
}

// 解析YAML文件，并将结果填充到结构体列表中
void parseyamlfile() {
    YAML::Node doc = YAML::LoadFile(baseconfyaml);
	if(doc["daemonrun"].IsDefined()){
		const YAML::Node& daemonrunnodes = doc["daemonrun"];
		for (const auto& runnode : daemonrunnodes) {
			daemonrun runone;
			if(runnode["netdev"].IsDefined()){
				runone.netdev = runnode["netdev"].as<std::string>();
			}
			if(runnode["bindvip"].IsDefined()){
				runone.bindvip = runnode["bindvip"].as<std::string>();
			}
			if(runnode["binrun"].IsDefined()){
				runone.binrun = runnode["binrun"].as<std::string>();


				printf("parseyamlfile daemonrunone.binrun:%s\n\n", runone.binrun.c_str());

			
			}
			if(runnode["retimes"].IsDefined()){
				runone.retimes = runnode["retimes"].as<int>();
			}

			
			if(runnode["envs"].IsDefined()){
				const YAML::Node& envs = runnode["envs"];
				for (const auto& env : envs) {
					envv envvone;
					if(env["env"].IsDefined()){
						std::string envone = env["env"].as<std::string>();
						envvone.env = envone;
					}
					runone.envlist.push_back(envvone);
				}
			}
			runone.runstartimes = 0;
			runone.resetruntimes = false;
			runone.resetdelnetvip = true;
			daemonrunload.push_back(runone);
		}
	}
}



int main(int argc, const char *argv[], char *envp[]) {

	//SIGCHLD‌,SIGCLD：等价，当子进程的状态发生变化时。
	//父进程忽略这个信号，子进程交给init
	signal(SIGCLD, SIG_IGN);
	signal(SIGCHLD, SIG_IGN);

	//if(daemonsta() > 1){
	//	printf("daemon already runing... \n");
	//	return 1;
   // }

	printf("Hello, Lixingbo, this daemon is v1.0\n");

	setprogrampath(argv[0]);

    if (getcwd(baserunpath, sizeof(baserunpath)) != NULL) {
       std::cout << "current working directory: " << baserunpath << std::endl;
    } else {
        std::cerr << "error getting current working directory" << std::endl;
    }

	writeversion("daemon.version");
	
	mkdir("daemonrun", 0755);
	strncpy(baselogpath, baserunpath, strlen(baserunpath));
	strncpy(baselogpath + strlen(baserunpath) , "/daemonrun", strlen("/daemonrun"));
	strncpy(baseconfyaml, baserunpath, strlen(baserunpath));
	strncpy(baseconfyaml + strlen(baserunpath) , "/daemon.yaml", strlen("/daemon.yaml"));
	parseyamlfile();

    string pidfile = "daemon.pid";
	Daemon::daemonProcess("start", pidfile.c_str());

	sigClean();
	daemonrunloop(envp);
	return 0;
}
