# ServerDaemon
这是一个轻量级的守护进程监控工具，能够自动监控和管理多个应用进程的生命周期。当配置的应用进程异常退出时，守护进程会自动重新拉起，并支持虚拟IP（VIP）的自动释放，确保系统的高可用性。

## 给予执行权限
roo8.sh

## 启动守护进程
./daemon

配置参数详解
### 参数名	必填	类型	说明
1. binrun	是	字符串	要启动的应用程序的完整命令行路径，支持带参数
2. netdev	是	字符串	网络设备名称，用于绑定和释放VIP
3. bindvip	是	字符串	要绑定的虚拟IP地址和子网掩码（格式：IP/掩码）
4. retimes	否	整数	进程异常退出时的最大重试次数，默认为0（无限重试）
5. envs	否	数组	要设置的环境变量列表，每个元素格式为：env: 变量值
工作原理
进程监控流程
配置解析：启动时读取 daemon.yaml 配置文件

进程拉起：按照配置顺序启动所有应用进程

状态监控：定期检查每个应用进程的运行状态

异常处理：当进程异常退出时：

检查重试次数限制

释放该进程绑定的虚拟IP（VIP）

根据配置的重试次数重新拉起进程

### daemon 守护进程
1. daemon 守护进程
2. 启动一般为  ./daemon
3. 启动daemon进程后会加载当前路径下daemon.yaml 配置文件
4. 并拉起daemon.yaml配置的每一个进程,并一直监测

### daemon 守护进程 开机自启动
1. myserver.sh 开机自启动脚本 
2. 修改 myserver.sh "/usr/local/daemon/daemon" 
3. 修改守护进程为生产环境的绝对路径
4. 将myserver.sh拷贝到/etc/init.d/目录下，否则添加服务不成功
5. cd /etc/init.d/
6. chkconfig --add myserver.sh
7. chkconfig myserver.sh on

### 重启linux
1. ps -ef | grep ...
2. 查看被daemon守护进程监控的应用是否拉起



systemctl status myservice
