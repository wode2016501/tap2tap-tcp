/*
 * simple point-to-point L2 (tap) vpn
 *
 * it doesn't do anything fancy: no IP assignment, no crazy CA system, no
 * support for legacy systems. it runs only on modern linux. it is L2 only.
 *
 * features still needed, in order of priority:
 *   - tests
 *   - encryption
 *
 * https://github.com/chriskuehl/tap2tap
 */
#define _GNU_SOURCE

#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <netinet/in.h>

#include "tap2tap.h"
#include "iface.h"

#define RECV_QUEUE 1024
#define SEND_QUEUE 1024


char exit_wanted = 0;
int received_signal = 0;

struct frame {
    size_t len;
    char data[MAX_MTU + ETHERNET_HEADER];
};


int setup_socket(in_addr_t bind_addr, uint16_t bind_port,int yy) {
    int sockfd = socket(AF_INET, SOCK_STREAM,IPPROTO_TCP);
    if (sockfd < 0) {
        perror("socket");
        return sockfd;
    }
    if(yy)
	    return sockfd;
    struct sockaddr_in local;
    memset(&local, 0, sizeof local);
    local.sin_family = AF_INET;
    local.sin_port = htons(bind_port);
    local.sin_addr.s_addr = bind_addr;

    int bind_result = bind(sockfd, (struct sockaddr *) &local, sizeof local);
    if (bind_result < 0) {
        perror("bind");
        return bind_result;
    }
     bind_result = listen(sockfd,2) ;
      if (bind_result < 0) {
	      perror("listen");
	      return bind_result;
      }
    return sockfd;
}


int run_updown(char *script, char *device) {
    pid_t pid = fork();
    if (pid == 0) {  // child
        execlp(script, script, device, NULL);
        // exec failed
        perror("exec");
        exit(1);
    } else if (pid > 0) {  // parent
        int ret;
        waitpid(pid, &ret, 0);
        return WEXITSTATUS(ret);
    } else {
        perror("fork");
        // TODO: exit from here is bad
        exit(1);
    }
}


int drop_privileges(int uid, int gid) {
    if (setgid(gid) != 0) {
        perror("setgid");
        return 1;
    }
    if (setuid(uid) != 0) {
        perror("uetgid");
        return 1;
    }
    return 0;
}

int udata(char *buf, size_t  size){
for (int i=0;i<size;i++)
buf[i]++;
return 0;
}
int edata(char *buf, size_t  size){
for (int i=0;i<size;i++)
buf[i]--;
return 0;
}

int run_tunnel(struct args *args, sigset_t *orig_mask) {
    char device[IFNAMSIZ];
    int fd = create_tap(args->iface, device, args->mtu);
    if (fd < 0) {
        fprintf(stderr, "unable to create tap device\n");
        return 1;
    }
    printf("tap device is: %s\n", device);

    if (args->up_script) {
        int ret = run_updown(args->up_script, device);
        if (ret != 0) {
            fprintf(stderr, "up script exited with status: %d\n", ret);
            return 1;
        }
    }

    int yy=0;
    if(args->remote){
	    yy=1;
    }

    // TODO: make the port and bind address configurable
    int sockfd = setup_socket(inet_addr("0.0.0.0"), dport,yy);
    if (sockfd < 0) {
        fprintf(stderr, "unable to create socket\n");
        return 1;
    }
    fprintf(stderr,"接听端口：%d\n",dport);
    if (drop_privileges(args->uid, args->gid) != 0) {
        fprintf(stderr, "couldn't drop privileges\n");
        return 1;
    }

    // circular queues
    struct frame recv_queue[RECV_QUEUE] = {0};
    size_t recv_idx = 0;
    size_t recv_len = 0;

    struct frame send_queue[SEND_QUEUE] = {0};
    size_t send_idx = 0;
    size_t send_len = 0;

    struct timespec tm;
    memset(&tm, 0, sizeof tm);
    tm.tv_nsec = 10000000;  // 0.01 seconds

    struct pollfd fds[2];
    memset(&fds, 0, sizeof fds);
    fds[0].fd = fd;
    fds[1].fd = sockfd;

    struct sockaddr_in remote;
    memset(&remote, 0, sizeof remote);
    char has_remote = 0;

    if (args->remote) {
        remote.sin_family = AF_INET;
        remote.sin_port = htons(sport);
        has_remote = 1;

        remote.sin_addr.s_addr = inet_addr(args->remote);
        if (remote.sin_addr.s_addr == INADDR_NONE) {
            fprintf(stderr, "无法连接: %s:%d\n", args->remote,sport);
            return 2;
        }

        fprintf(stderr, "链接到服务器: %s:%d\n", args->remote,sport);
    }


    fprintf(stderr, "隧道通了\n");
    //int w=0;
#if WWW
    long Len=0,Len1=0;
#endif
  int ret=0;
    int len=0;
int Data_fb=sockfd;       

while (1){
 len=sizeof(remote);
   if (args->remote) {
    len=sizeof(remote);
    ret=connect(sockfd,(struct sockaddr *)&remote,len);
        if(ret == -1)
    {
        perror("connect");
        return -2;
    }
   }
   else

   {
	   if(Data_fb!=sockfd)
		   close(sockfd);
    sockfd = accept(Data_fb,(struct sockaddr *)&remote,(socklen_t *)&len);
        printf("与%s链接成功！\n",inet_ntoa(remote.sin_addr));
    fds[1].fd = sockfd;
   }
    for (;;) {
	   // printf("%d\n",POLLIN);
	    //sleep(1);
        fds[0].events = POLLIN;
        if (recv_len > 0) {
	//	 printf("%d,|%d=",POLLIN,fds[0].events);
            fds[0].events |= POLLOUT;
	    // printf("%d\n",fds[0].events);
        }
   //printf("%d,%d\n",POLLIN,fds[0].events);

        fds[1].events = POLLIN;
        if (send_len > 0 && has_remote) {
            fds[1].events |= POLLOUT;
        }

        int result = ppoll(fds, 2, &tm, orig_mask);
        if (result < 0) {
            if (errno != EINTR) {
                perror("ppoll");
                return 3;
            }
        }

        if (exit_wanted) {
            fprintf(stderr, "\nreceived signal %d, stopping tunnel\n", received_signal);
            break;
        }

        // tap can handle a write
        if (fds[0].revents & POLLOUT) {
            struct frame *f = &recv_queue[recv_idx];
            assert(f->len <= args->mtu + ETHERNET_HEADER);
            recv_idx = (recv_idx + 1) % RECV_QUEUE;
            recv_len -= 1;
#if WWW
	     fprintf(stderr, "write tap: '%ld,%ld' \n",f->len,Len+=f->len);
#endif      
	     udata(f->data,f->len);
            ssize_t n = write(fd, f->data, f->len);
            if (n < 0) {
                if (errno == EINVAL) {
                    fprintf(stderr, "received garbage frame\n");
                } else {
                    perror("write");
                    return 4;
                }
            } else if (n < f->len) {
                printf("[error] only wrote %zd bytes to tap (out of %zd bytes)\n", n, f->len);
            }
        }

        // udp socket can handle a write
        if (fds[1].revents & POLLOUT) {
            struct frame *f = &send_queue[send_idx];
            assert(f->len <= args->mtu + ETHERNET_HEADER);
            send_idx = (send_idx + 1) % SEND_QUEUE;
            send_len -= 1;
#if WWW
	       fprintf(stderr, "\t\t\twrite udp: '%ld,' \n",f->len);
#endif
   	       //  fprintf(stderr, "write sockfd : '%d,%s'\n",f->len,f->data);
	       edata(f->data,f->len);
           // ssize_t n = sendto(sockfd, f->data, f->len, 0, (struct sockaddr *) &remote, sizeof remote);
	     ssize_t n = write(sockfd, f->data, f->len);
	       if (n < 1) {
                perror("sendto");
		break;
                //return 4;
            } else if (n < f->len) {
                printf("[error] only sent %zd bytes to peer (out of %zd bytes)\n", n, f->len);
            }
        }

        // tap has data for us to read
        if (fds[0].revents & POLLIN) {
            size_t idx = (send_idx + send_len) % SEND_QUEUE;

            if (send_len < SEND_QUEUE) {
                send_len += 1;
            } else {
                assert(send_len == SEND_QUEUE);
                printf("dropping frame from send queue\n");

                // put this packet at the end of the queue;
                // drop the first frame in the queue
                send_idx += 1;
            }

            struct frame *f = &send_queue[idx];
            memset(f, 0, sizeof(struct frame));
            ssize_t n = read(fd, &f->data, args->mtu + ETHERNET_HEADER);
            f->len = n;
#if WWW
	      fprintf(stderr, "\t\t\tread tap: '%ld,' \n",f->len);
#endif
       	}

        // udp socket has data for us to read
        if (fds[1].revents & POLLIN) {
            size_t idx = (recv_idx + recv_len) % RECV_QUEUE;

            if (recv_len < RECV_QUEUE) {
                recv_len += 1;
            } else {
                assert(recv_len == RECV_QUEUE);
                printf("dropping frame from recv queue\n");

                // put this packet at the end of the queue;
                // drop the first frame in the queue
                recv_idx += 1;
            }

            struct frame *f = &recv_queue[idx];
            memset(f, 0, sizeof(struct frame));

            // TODO: handle case where remote changes, in both server+client mode
            //socklen_t l = sizeof(remote);
            has_remote = 1;/*
            ssize_t n = recvfrom(
                sockfd,
                &f->data,
                args->mtu + ETHERNET_HEADER,
                0,
                (struct sockaddr *) &remote,
                &l
            );*/
	     ssize_t n = read(sockfd, &f->data, args->mtu + ETHERNET_HEADER);
//	    fprintf(stderr, "读取 %s:%d %dsize\n",args->remote,sport,( int ) n) ; 
            if(n<1){
		    break;
	    }
		    f->len = n;
#if WWW
	     fprintf(stderr, "read udp: '%ld,%ld' \n",f->len,Len1+=f->len);
#endif
        }
    }
    if (exit_wanted) {
            //fprintf(stderr, "\nreceived signal %d, stopping tunnel\n", received_signal);
            break;
        }

}
    if (args->down_script) {
        int ret = run_updown(args->down_script, device);
        if (ret != 0) {
            fprintf(stderr, "down script exited with status: %d\n", ret);
            return 1;
        }
    }

    return 0;
}
