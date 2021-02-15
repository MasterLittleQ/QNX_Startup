#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <sys/slog2.h>
#include <sys/resmgr.h>
#include <process.h>
#include <sched.h>
#include <pthread.h>
/*namespace and ipc*/
#define ATTACH_POINT "ZStudy_AttachPoint"

/*---------------------------------------------------------------*/
/*-------ZStudy mainly for Resource Manager study----------------*/
/*-1.Resouce manager provide for file operation------------------*/
/*-2.IPCServerThread communicate to the ZStudy2's client---------*/
/*---------------------------------------------------------------*/

static resmgr_connect_funcs_t connect_funcs;
static resmgr_io_funcs_t io_funcs;
static iofunc_attr_t attr;

int server()
{
    name_attach_t *attach;
    char msg_server[20];
    int rcvid;

    attach = name_attach(NULL, ATTACH_POINT, 0);

    while (1)
    {
        rcvid = MsgReceive(attach->chid, &msg_server, sizeof(msg_server), NULL);

        printf("ZStudy : server rcvid = %d\n", rcvid);
        printf("ZStudy : server received fist data:%d\n", *msg_server);
        MsgReply(rcvid, EOK, NULL, 0);
    }
    printf("ZStudy : server end\n");
    name_detach(attach, 0);
    return 0;
}

int io_open(resmgr_context_t *ctp, io_open_t *msg, RESMGR_HANDLE_T *handle, void *extra)
{
    printf("ZStudy : io_open\n");
    creat("/usr/bin/Zfile.txt", S_IRUSR | S_IWUSR);
    return 0; //(iofunc_open_default(ctp, msg, handle, extra));
}

int io_read(resmgr_context_t *ctp, io_read_t *msg, RESMGR_OCB_T *ocb)
{
    printf("ZStudy : io_read\n");
    return 1;
}

int io_write(resmgr_context_t *ctp, io_write_t *msg, RESMGR_OCB_T *ocb)
{
    printf("ZStudy : io_write's pid : %d\n", getpid());

    FILE *fp;
    fp = fopen("/usr/bin/Zfile.txt", "a");
    if (NULL == fp)
    {
        printf("ZStudy : fopen failed \n");
    }
    else
    {
        int i = fwrite(msg + 1, msg->i.nbytes, 1, fp);
        printf("ZStudy : Successfully wrote %d records, should be %d\n", i, msg->i.nbytes);
    }

    fwrite("\r\n", 1, 2, fp);

    fclose(fp);
    return 2;
}

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, RESMGR_OCB_T *ocb)
{
    printf("ZStudy : io_devctl\n");
    return 3;
}

void PrepareThreadAttributes(pthread_attr_t *thread_attr, int Prio, int StackSize)
{
    //sched_param priority;
    //memset((void*)&priority,0,sizeof(sched_param));
    pthread_attr_init(thread_attr);
    pthread_attr_setscope(thread_attr, PTHREAD_SCOPE_SYSTEM);
    //priority.sched_priority = Prio;
    //pthread_attr_setschedparam(thread_attr, &priority);//needed for SCHED_FIFO or SCHED_RR
    pthread_attr_setschedpolicy(thread_attr, SCHED_OTHER);
    pthread_attr_setdetachstate(thread_attr, PTHREAD_CREATE_JOINABLE);
    pthread_attr_setstacksize(thread_attr, (size_t)StackSize);
}

void *IPCServerThread(void *pArg)
{
    (void)server();

    pthread_exit(NULL);
}

void *LogEmTrace(void *pArg)
{
    while (1)
    {
#if 0
        if (1 == gZStudyBuffFlag)
        {
            int fd;

            fd = open("/usr/bin/Zfile.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

            write(fd, gZstudyBuff, gTestBufLength + 1);

            gZStudyBuffFlag = 0;
        }
#endif
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    resmgr_attr_t resmgr_attr;
    dispatch_t *dpp;
    dispatch_context_t *ctp;
    int id;

    if ((dpp = dispatch_create()) == NULL)
    {
        printf("ZStudy : dispacth error");
        return EXIT_FAILURE;
    }

    /* initialize resource manager attributes */
    memset(&resmgr_attr, 0, sizeof resmgr_attr);
    resmgr_attr.nparts_max = 3;
    resmgr_attr.msg_max_size = 2048;

    /* initialize functions for handling messages */
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_funcs,
                     _RESMGR_IO_NFUNCS, &io_funcs);
    //connect_funcs.open = io_open;
    io_funcs.read = io_read;
    io_funcs.write = io_write;
    io_funcs.devctl = io_devctl;

    /* initialize attribute structure used by the device */
    iofunc_attr_init(&attr, S_IFNAM | 0662, 0, 0);

    /* attach our device name */
    id = resmgr_attach(
        dpp,               /* dispatch handle        */
        &resmgr_attr,      /* resource manager attrs */
        "/dev/ZStudy_dev", /* device name            */
        _FTYPE_ANY,        /* open type              */
        0,                 /* flags                  */
        &connect_funcs,    /* connect routines       */
        &io_funcs,         /* I/O routines           */
        &attr);            /* handle                 */

    if (id == -1)
    {
        printf("ZStudy : resmgr error");
        return EXIT_FAILURE;
    }

    /* create a backend thread for error log*/
    pthread_t thread;
    pthread_attr_t thread_attr;

    pthread_attr_setstacksize(&thread_attr, (size_t)0x1000);
    pthread_attr_init(&thread_attr);
    //pthread_attr_setstack()
    int ret = pthread_create(&thread, &thread_attr, LogEmTrace, NULL);
    printf("ZStudy : LogEmTrace tread created : %d\n", ret);

    ret = pthread_create(&thread, &thread_attr, IPCServerThread, NULL);
    printf("ZStudy : IPCServerThread tread created : %d\n", ret);

    /* allocate a context structure */
    ctp = dispatch_context_alloc(dpp);

    while (1)
    {
        ctp = dispatch_block(ctp);
        dispatch_handler(ctp);
    }

    printf("ZStudy : First Time to Build Zxz and it ends here.\n");
    return 0;
}
