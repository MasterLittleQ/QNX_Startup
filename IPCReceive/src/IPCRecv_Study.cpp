#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/neutrino.h>
#include <errno.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <sys/resmgr.h>
#include <vector>
#include <memory>
#include <iostream>
/*namespace and ipc*/
#define ATTACH_POINT "ZStudy_AttachPoint"

#include <mqueue.h>
#include <errno.h>
#define QUEUE_NAME "/testqueue"

#include <sys/times.h>

#include <process.h>

#include <signal.h>

#include <sys/mman.h>
/*POSIX message queues*/
/*The Func_MQSend is Client and responsible for send*/

#define TESTSTRING "AAAAAAAAAA"

void sigaction_handler(int signo);
void sigaction_handler2(int signo);

class Z_TestCases
{
public:
    void Func_MQSend(void);
    void Process1s(void);

protected:
private:
};

class memory_sync_test
{
public:
    memory_sync_test()
    {
        fd = open("/usr/bin/Zfile_1.txt", O_CREAT | O_RDWR, 0777);
        write(fd, TESTSTRING, sizeof(TESTSTRING));
        lseek(fd, 0, SEEK_SET);
        this->file_size = lseek(fd, 0, SEEK_END);
        printf("Size of file = %d bytes\n", file_size);
        addr = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd, 0);

        printf("ZStudy3 : memory_sync_test construct done\n");
    }
    ~memory_sync_test()
    {
    }

    void sync_memory(void)
    {
        if (-1 == msync(this->addr, this->file_size, MS_SYNC))
        {
            printf("msync error\n");
        }
    }

    void modify_memory(void)
    {
        memset(this->addr, 'B', this->file_size);
    }

private:
    void *addr;
    int fd;
    int file_size;
};

class signal_test
{
public:
    signal_test()
    {
        /*Sigaction*/
        sigemptyset(&set);
        sigaddset(&set, SIGUSR1);
        sigaddset(&set, SIGUSR2);

        act.sa_flags = 0;
        act.sa_mask = set;
        act.sa_handler = sigaction_handler;
        sigaction(SIGUSR1, &act, NULL);

        act2.sa_flags = 0;
        act2.sa_mask = set;
        act2.sa_handler = sigaction_handler2;
        sigaction(SIGUSR2, &act2, NULL);
    }

    void signal_trigger(void)
    {
        kill(getpid(), SIGUSR1);
    }

private:
    struct sigaction act, act2;
    sigset_t set;
};

void Z_TestCases::Func_MQSend(void)
{
    mqd_t mq;
    char buffer[1024];

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 1024;
    attr.mq_curmsgs = 0;
    errno = 0;
    mq = mq_open(QUEUE_NAME, O_CREAT | O_WRONLY, 0777, attr);
    if (0 != errno)
    {
        printf("ZStudy3 : errno = %d\n", errno);
    }
    printf("ZStudy3 : mq_send : %d\n", mq);

    for (int i = 0; i < 10; i++)
    {
        buffer[i] = i;
    }
    mq_send(mq, buffer, 10, 0);
}

void Z_TestCases::Process1s(void)
{
    //while (1)
    {
        system("date");
        sleep(1);
    }
}

void sigaction_handler(int signo)
{
    printf("ZStudy3 : This is sigaction handler, pid = %d\n", getpid());
    int once = 1;
    if (once)
    {
        once = 0;
        kill(getpid(), SIGUSR2);
    }
}

void sigaction_handler2(int signo)
{
    printf("ZStudy3 : This is sigaction handler2, pid = %d\n", getpid());
    int once = 1;
    if (once)
    {
        once = 0;
        //kill(getpid(), SIGUSR1);
        //kill(getpid(), SIGUSR2);
    }
}

int main(int argc, char *argv[])
{
    using namespace std;
    Z_TestCases ZStudyTest;
    int ret;

    /*-------memory sync test-------*/
    memory_sync_test MMTEST;
    MMTEST.modify_memory();
    MMTEST.sync_memory();
    printf("ZStudy3 : memory test done\n");

    pid_t pid;

    if ((pid = fork()) == -1)
    {
        printf("ZStudy3 : child Process create error\n");
    }
    else if (0 == pid)
    {
        /*Child*/
        printf("ZStudy3 : child pid = %d\n", getpid());
        ZStudyTest.Process1s();

        /*-------Share memory test-------*/
        int fd;
        void *shareBuffer = NULL;
        int buffer[2];
        fd = shm_open("/ZStudy3_ShareMem", O_RDWR | O_CREAT, 077);
        if (-1 == fd)
        {
            printf("ZStudy3 : share memory failed\n");
        }
        if (ftruncate(fd, sizeof(buffer)) == -1)
        {
            printf("ZStudy3 : ftruncate failed\n");
        }
        shareBuffer = mmap(0, sizeof(*buffer), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (shareBuffer == MAP_FAILED)
        {
            printf("ZStudy3 : mmap failed\n");
        }

        printf("ZStudy3 : shareBuffer[0]=%x, shareBuffer[1]=%x\n", *(int *)shareBuffer, *((int *)shareBuffer + 1));

        *(int *)shareBuffer = 0x0A;
        *((int *)shareBuffer + 1) = 0x0B;

        close(fd);

        ret = 1;
    }
    else
    {
        /*Parent*/
        printf("ZStudy3 : parent pid = %d\n", getpid());
        ZStudyTest.Func_MQSend();

        /*-------sigaction test-------*/
        signal_test SIGNALTEST;
        SIGNALTEST.signal_trigger();

        printf("ZStudy3 : parent END\n");

        ret = 2;
    }

    return ret;
}
