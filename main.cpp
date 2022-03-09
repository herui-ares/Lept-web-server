#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include "locker.h"
#include "threadpool.h"
#include "http_conn.h"

#define MAX_FD 65536   // 最大的文件描述符个数
#define MAX_EVENT_NUMBER 10000  // 监听的最大的事件数量

// 添加文件描述符
extern void addfd( int epollfd, int fd, bool one_shot );
extern void removefd( int epollfd, int fd );

void addsig(int sig, void( handler )(int)){
    struct sigaction sa;
    memset( &sa, '\0', sizeof( sa ) );
    sa.sa_handler = handler;
    sigfillset( &sa.sa_mask );
    assert( sigaction( sig, &sa, NULL ) != -1 );
}

int main( int argc, char* argv[] ) {
    
    if( argc <= 1 ) {
        printf( "usage: %s port_number\n", basename(argv[0]));
        return 1;
    }

    int port = atoi( argv[1] );//将端口地址取出，字符串转整数
    addsig( SIGPIPE, SIG_IGN );  //，（默认的话，当服务器在处理数据时，client关闭连接，服务器会中止线程）将SIGPIPE信号设置为SIG_IFN，当客户端退出，SIGPIPE交给系统处理

    threadpool< http_conn >* pool = NULL;//创建线程池POOL
    try {
        pool = new threadpool<http_conn>;
    } catch( ... ) {
        return 1;
    }

    http_conn* users = new http_conn[ MAX_FD ];//创建保存客户端信息的数组

    int listenfd = socket( PF_INET, SOCK_STREAM, 0 );//创建监听的套接字

    int ret = 0;
    struct sockaddr_in address;
    address.sin_addr.s_addr = INADDR_ANY;//本机IP
    address.sin_family = AF_INET;  //TCP/IPV4协议族
    address.sin_port = htons( port );//从主机字节顺序转变成网络字节顺序，网络字节顺序采用big-endian排序方式

    int reuse = 1;
    setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof( reuse ) );// 端口复用
    // 防止服务器重启时之前绑定的端口还未释放
    //程序突然退出而系统没有释放端口
    ret = bind( listenfd, ( struct sockaddr* )&address, sizeof( address ) );//绑定监听的套接字  IP+端口绑定
    ret = listen( listenfd, 5 );

    // 创建epoll对象，和事件数组，添加
    epoll_event events[ MAX_EVENT_NUMBER ];
    int epollfd = epoll_create( 5 );
    // 添加到epoll对象中
    addfd( epollfd, listenfd, false ); //将监听SOCKET时间添加到epoll对象中
    http_conn::m_epollfd = epollfd;

    while(true) {
        
        int number = epoll_wait( epollfd, events, MAX_EVENT_NUMBER, -1 );//阻塞的检测事件到来
        
        if ( ( number < 0 ) && ( errno != EINTR ) ) {
            printf( "epoll failure\n" );
            break;
        }

        for ( int i = 0; i < number; i++ ) {
            
            int sockfd = events[i].data.fd;
            
            if( sockfd == listenfd ) {//如果事件对应的socket说监听socket,说明有新的连接到来
                
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof( client_address );
                int connfd = accept( listenfd, ( struct sockaddr* )&client_address, &client_addrlength );//建立连接
                
                if ( connfd < 0 ) {
                    printf( "errno is: %d\n", errno );
                    continue;
                } 

                if( http_conn::m_user_count >= MAX_FD ) {//如果用户数量大于最大的文件描述符，就不会建立连接，直接关闭soclet
                    close(connfd);
                    continue;
                }
                users[connfd].init( connfd, client_address);

            } else if( events[i].events & ( EPOLLRDHUP | EPOLLHUP | EPOLLERR ) ) {
//异常断开错误，关闭连接
                users[sockfd].close_conn();

            } else if(events[i].events & EPOLLIN) {
                //有读事件发生EPOLLIN
                if(users[sockfd].read()) {
                    pool->append(users + sockfd);//请求工作队列添加一个事件
                } else {
                    users[sockfd].close_conn();
                }

            }  else if( events[i].events & EPOLLOUT ) {
                //写事件发生EPOLLOUT
                if( !users[sockfd].write() ) {
                    users[sockfd].close_conn();
                }

            }
        }
    }
    
    close( epollfd );
    close( listenfd );
    delete [] users;
    delete pool;
    return 0;
}