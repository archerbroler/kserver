#include <linux/module.h>
#include <linux/init.h>

#include <linux/in.h>
#include <linux/inet.h>
#include <linux/socket.h>
#include <net/sock.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("broler");

struct socket* createServerSocket(void)
{
	struct socket *_sock;
        struct sockaddr_in _saddr;
	unsigned short _port=0x8080;
        int ret = 0;
        memset(&_saddr,0,sizeof(_saddr));
        _saddr.sin_family = AF_INET;
        _saddr.sin_port = htons(_port);
        _saddr.sin_addr.s_addr = htonl(INADDR_ANY);
        _sock = (struct socket*)kmalloc(sizeof(struct socket),GFP_KERNEL);
        //_clientSock = (struct socket*)kmalloc(sizeof(struct socket),GFP_KERNEL);
        ret = sock_create_kern(AF_INET, SOCK_STREAM,0,&_sock);
	if(ret )
	{
		printk("server socket created error\n");
		return NULL;
	}
	int value=1;
	if(kernel_setsockopt(_sock, SOL_SOCKET, SO_REUSEPORT, &value, 4))
	{
		printk("set reuseport option failed\n");
		return NULL;
	}
	if(kernel_setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, &value, 4))
        {
                printk("set option reuseaddr failed\n");
                return NULL;
        }     
	printk("reuse port set\n");
	ret = _sock->ops->bind(_sock,(struct sockaddr *)&_saddr,sizeof(struct sockaddr_in));	
	if(ret<0){
                printk("server: bind error\n");
                return NULL;
        }
        printk("server:bind ok!\n");
	return _sock;
}
void processConnection(struct socket* _clientSocket){
 /*kmalloc a receive buffer*/
        char *recvbuf=NULL;
	struct kvec vec;
        struct msghdr msg;
        recvbuf=kmalloc(1024,GFP_KERNEL);
        if(recvbuf==NULL){
                printk("server: recvbuf kmalloc error!\n");
                return ;
        }
        memset(recvbuf, 0, 1024);

        /*receive message from client*/
        memset(&vec,0,sizeof(vec));
        memset(&msg,0,sizeof(msg));
        vec.iov_base=recvbuf;
        vec.iov_len=1024;
        if(kernel_recvmsg(_clientSocket,&msg,&vec,1,1024,0)){
		printk("receive msg failed\n");
		return;
	}
        printk("receive message:\n %s\n",recvbuf);
        
	/*release socket*/
        sock_release(_clientSocket);
}
static int server(void){
	int ret=0;
	struct socket* _clientSocket;
	struct socket * _socket = createServerSocket();
	if(!_socket)
	{
		printk("server socket created failed\n");
		return -1;
	}
	while	(1){
		 /*listen*/
        	ret=_socket->ops->listen(_socket,10);
        	if(ret<0){
                	printk("server: listen error\n");
                	return ret;
        	}
        	printk("server:listen ok!\n");

 	        ret=_socket->ops->accept(_socket,_clientSocket,10);
		if(ret<0){
                	printk("server:accept error!\n");
                	return ret;
        	}
		printk("server: accept ok, Connection Established\n");
		processConnection(_clientSocket);
	}
	sock_release(_socket);

}

static void __exit hello_exit(void)
{
	printk(KERN_ALERT "Goodbye,cruel world!");
}
static int __init hello_init(void)
{
        printk(KERN_ALERT "Hello world!\n");
        server();
        return 0;
}

module_init(hello_init);
module_exit(hello_exit);
