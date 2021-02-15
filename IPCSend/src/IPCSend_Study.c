#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>
/*namespace and ipc*/
#define ATTACH_POINT "ZStudy_AttachPoint"

#include <mqueue.h>
#define QUEUE_NAME "/testqueue"

#include <sys/mman.h>

/*Synchronous message passing*/
/*The Clinet is resposible for send*/
/*The Server need to reply, or the server may stucked*/
int client()
{
    char msg_client[20];
    int server_coid;

    server_coid = name_open(ATTACH_POINT, 0);

    msg_client[0] = 0xFA;

    int ret = MsgSend(server_coid, &msg_client, 1u, NULL, 0);
    printf("ZStudy2 : MsgSend return value = %d\n", ret);

    name_close(server_coid);
    return 0;
}

/*POSIX message queues*/
/*The Func_MQReceive_Async is Server and responsible for recv*/
void Func_MQReceive_Async(void)
{
    mqd_t mq;
    char buffer[1024];

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;
    mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY | O_NONBLOCK, 0777, attr);
    printf("ZStudy2 : mq_receive : %d\n", mq);
    memset(buffer, 0x00, sizeof(buffer));

    /*It shouldn't in a while(1)*/
    int bytes_read = mq_receive(mq, buffer, 1024, NULL);
    if (bytes_read >= 0)
    {
        for (int i = 0; i < bytes_read; i++)
            printf("ZStudy2 : SERVER: Received message: %d\n", buffer[i]);
    }
    else
    {
        //printf("SERVER: None \n");
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
}

/*POSIX message queues*/
/*The Func_MQReceive_Async is Server and responsible for recv and this case will while(1) receive*/
void Func_MQReceive_Sync(void)
{
    mqd_t mq;
    char buffer[1024];

    //struct mq_attr attr;
    //attr.mq_flags = 0;
    //attr.mq_maxmsg = 10;
    //attr.mq_msgsize = 1024;
    //attr.mq_curmsgs = 0;
    mq = mq_open(QUEUE_NAME, O_RDONLY); //O_CREAT | O_RDONLY | O_NONBLOCK, 0777, attr);
    printf("ZStudy2 : mq_receive sync : %d\n", mq);
    memset(buffer, 0x00, sizeof(buffer));

    while (1)
    {
        int bytes_read = mq_receive(mq, buffer, 1024, NULL);
        if (bytes_read >= 0)
        {
            for (int i = 0; i < bytes_read; i++)
                printf("ZStudy2 : SERVER: Received message: %d\n", buffer[i]);
        }
        else
        {
            //printf("SERVER: None \n");
        }
    }

    mq_close(mq);
    mq_unlink(QUEUE_NAME);
}

/*Resource Manager*/
/*RMtest_Write is try to use /dev/ZStudy */
void RMtest_Write(void)
{
    int fd = -1;
    int len = 0;
    char *TestData = (char *)malloc(10);

    fd = open("/dev/ZStudy_dev", O_RDWR);
    printf("ZStudy2 : Open: now fd = %d\n", fd);

    len = sprintf(TestData, "Hi");
    write(fd, TestData, len);

    len = sprintf(TestData, "ZhangTest");
    write(fd, TestData, len);

    free(TestData);
    close(fd);
}

/*Share Memory test*/
/*Share memory started in ZStudy3 and shall be read by ZStudy2*/
void ShareMemTest(void)
{
    int fd;
    void *ReadTestBuf = NULL;
    int buff[2];

    fd = shm_open("/ZStudy3_ShareMem", O_RDWR | O_CREAT, 077);
    if (-1 == fd)
    {
        printf("ZStudy2 : share memory failed\n");
    }
    if (ftruncate(fd, sizeof(buff)) == -1)
    {
        printf("ZStudy2 : ftruncate failed\n");
    }
    ReadTestBuf = mmap(0, sizeof(buff), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ReadTestBuf == MAP_FAILED)
    {
        printf("ZStudy2 : mmap failed\n");
    }
    //printf("ZStudy2 : Map addr is 0x%08x\n", ReadTestBuf);

    printf("ZStudy2 : ReadTestBuf[0]=%x, ReadTestBuf[1]=%x\n", *(int *)ReadTestBuf, *((int *)ReadTestBuf + 1));

    *(int *)ReadTestBuf = 0xFA;
    *((int *)ReadTestBuf + 1) = 0xFB;

    /*If I called shm_unlink, the share mem will be cleared and closed*/
    //shm_unlink("/ZStudy3_ShareMem");
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "abcdefhz:")) != -1)
    {
        switch (opt)
        {
        case 'a':
            RMtest_Write();
            break;
        case 'b':
            (void)client();
            break;
        case 'c':
            Func_MQReceive_Async();
            break;
        case 'd':
            Func_MQReceive_Sync();
            break;
        case 'e':
            break;
        case 'f':
            ShareMemTest();
            break;

        case 'h':
            printf("ZStudy2 : a: (Resource Manager) Test, To write\n"
                   "ZStudyb : b: (MsgSend)Synchronous message pass Test, Client send\n"
                   "ZStudy2 : c: (mq_receive)Message queue Test, To receive Asynchronously\n"
                   "ZStudy2 : d: (mq_receive)Message queue Test, To receive Synchronously\n"
                   "ZStudy2 : f: (shm_open,ftruncate,mmap)Share memory Test, \n"
                   "ZStudy2 : h: help\n");
            break;
        default:
            printf("ZStudy2 : No parameters and then do nothing\n");
            break;
        }
    }
    return 0;
}
