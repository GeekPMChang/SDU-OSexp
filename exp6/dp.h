#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

using namespace std;
// 信号灯控制用的共同体
typedef union semuns {
    int val;
} Sem_uns;

//管程中使用的信号量
class Sema {
private:
    int sem_id; //信号量标识符
public:
    Sema(int id) {
        sem_id = id;
    }
    ~Sema() {}

   //信号量加 1
    int down() {
        struct sembuf buf;
        buf.sem_op = -1;
        buf.sem_num = 0;
        buf.sem_flg = SEM_UNDO;
        if ((semop(sem_id, &buf, 1)) < 0) {
            perror("down error ");
            exit(EXIT_FAILURE);
        }
        return EXIT_SUCCESS;
    }
    //信号量减 1
    int up() { 
        Sem_uns arg;
        struct sembuf buf;
        buf.sem_op = 1;
        buf.sem_num = 0;
        buf.sem_flg = SEM_UNDO;
        if ((semop(sem_id, &buf, 1)) < 0)
        {
            perror("up error ");
            exit(EXIT_FAILURE);
        }
        return EXIT_SUCCESS;
    }
};

//管程中使用的锁
class Lock {
private:
    Sema *sema; //锁使用的信号量
public:
    Lock(Sema *s) {
        sema = s;
    }
    ~Lock(){ }

    //上锁
    void close_lock() { sema->down(); }

    //开锁
    void open_lock() { sema->up(); }
};

class Condition {
public:
    Condition(Sema *semax1, Sema *semax2) {
        sema0 = semax1;
        sema1 = semax2;
    }
    ~Condition(){};
    // 过路条件不足时阻塞 看看是否能通过
    void Wait(Lock *lock, int direction) {

        if (direction == 0) {
            lock->open_lock();
            cout <<"Direction -> "<< getpid() << "WAIT!" << "\n";//正向->
            sema0->down();
            lock->close_lock();
        }
        else if (direction == 1) {
            lock->open_lock();
            cout <<"Direction <- "<< getpid() << "WAIT!" << "\n";//反向<-
            sema1->down();
            lock->close_lock();
        }
    }

    //唤醒相反方向阻塞车辆
    int Signal(int direction) {
        int sig_ren;
        if (direction == 0) { //唤醒一个方向
            sig_ren = sema0->up();
        }
        else if (direction == 1) {
            sig_ren = sema1->up();
        }
        return sig_ren;
    }
private:
    Sema *sema0; // 一个方向阻塞队列
    Sema *sema1; // 另一方向阻塞队列
    Lock *lock;  // 进入管程时获取的锁
};

class OneWay  {
private:
    // 调用函数
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);

    int MaxCars;         // 最大同向车数
    int *NumCars;        // 当前正在通过的车辆数
    int *CurrentDirect;  // 当前通过的车辆的方向
    int *Frontward;      // 正向等待人数
    int *Backward;       // 反向等待人数
    int *SumPassedCars;  // 已经通过的车辆总数
    Condition *condition;// 通过单行道的条件变量
    Lock *lock;          // 单行道管程锁
public:
    OneWay(int CarLimit);
    ~OneWay() { delete condition; }
    // 车辆准备上单行道,direction 为行车方向
    void Arrive(int direction,int limit);
    // 车辆正在通过单行道
    void Cross(int direction);
    // 车辆通过了单行道
    void Quit(int direction);
};

OneWay::OneWay(int CarLimit) {
    int ipc_flg = IPC_CREAT | 0644;
    MaxCars = CarLimit;

    // 共享内存
    NumCars = (int *)set_shm(201, 1, ipc_flg);     //当前方向上通过的总的车辆数
    CurrentDirect = (int *)set_shm(301, 1, ipc_flg); //当前方向 0正向 1反向
    SumPassedCars = (int *)set_shm(401, 1, ipc_flg);//已经通过的车辆总数
    Frontward = (int *)set_shm(501, 1, ipc_flg);// 等待人数
    Backward = (int *)set_shm(502, 1, ipc_flg);// 等待人数
    //初始化
    *NumCars = 0;
    *CurrentDirect = 0;
    *SumPassedCars = 0;
    *Frontward = 0;
    *Backward = 0;
    // 信号量
    int sema0_id = set_sem(601, 0, ipc_flg);
    int sema1_id = set_sem(602, 0, ipc_flg);
    int semaLock_id = set_sem(701, 1, ipc_flg);
    //锁和信号量
    lock = new Lock(new Sema(semaLock_id));
    condition = new Condition(new Sema(sema0_id), new Sema(sema1_id));
}

void OneWay::Arrive(int direction,int limit) {
    lock->close_lock();
    if(direction==0){
        cout<<"Car born: "<<getpid()<<"->"<<endl;
    }else{
        cout<<"Car born: "<<getpid()<<"<-"<<endl;
    }
    //没有等待的车 --> 到达的车直接通行
    if (*NumCars == 0 && *Frontward == 0 && *Backward == 0) *CurrentDirect = direction;
    //方向不对->等待防止撞车 || 在路上的车太多->防止超过最大行车数量 || 某边等待大于队列长度的一半 && 路上有车
    if (*CurrentDirect != direction || *NumCars >= MaxCars || ((*Frontward > limit/2 || *Backward > limit/2) && *NumCars > 0)) {
        if (direction == 0){
            *Frontward += 1;
        } else{
             *Backward += 1;
        } 
        cout << "等待的汽车队列: " <<"正向: "<<*Frontward<< " " <<"反向: "<< *Backward  << endl;
        condition->Wait(lock, direction);
    }

    *NumCars = *NumCars + 1;
    *CurrentDirect = direction;

    lock->open_lock();
}

void OneWay::Cross(int direction) {
    lock->close_lock();
    if(direction==0){
        cout<<"正向: "<<getpid()<<"-->>:";
    }else{
        cout<<"反向: "<<getpid()<<"<<--:";
    }
    int num=1;
    for (int i = 1; i <= *NumCars; ++i) {
        cout<<"car"<<getpid()<<num++<<" ";
    }
    cout << endl;
    lock->open_lock();
    sleep(10);
}

void OneWay::Quit(int direction) {
    lock->close_lock();
    *NumCars -= 1;
    if(direction==0){
        cout<<getpid()<<"结束,正在行驶的汽车正向剩余" << *NumCars << endl;
    }else
    {
        cout<<getpid()<<"结束,正在行驶的汽车逆向剩余" << *NumCars << endl;
    }
    
        
    if (*NumCars == 0) {
        int max = MaxCars - *NumCars;
        cout <<"开始调度:"<<endl;
        cout << "等待的汽车队列: " <<"正向: "<<*Frontward<< " " <<"反向: "<< *Backward  << endl;
        if (direction == 0) {
            if (*Backward > 0) while (max && (*Backward > 0)) {
                condition->Signal(1);
                *Backward = *Backward - 1;
                max--;
                cout << "唤醒一个逆向 等待队列剩余" << *Backward << endl;
            }
            else while (max && (*Frontward > 0)) {
                condition->Signal(0);
                *Frontward = *Frontward - 1;
                max--;
                cout << "唤醒一个正向 等待队列剩余" << *Frontward << endl;
            }
        }
        else if (direction == 1) {
            if (*Frontward > 0) while (max && (*Frontward > 0)) {
                condition->Signal(0);
                *Frontward = *Frontward - 1;
                max--;
                cout << "唤醒一个正向 等待队列剩余" << *Frontward << endl;
            }
            else while (max && (*Backward > 0)) {
                condition->Signal(1);
                *Backward = *Backward - 1;
                max--;
                cout << "唤醒一个逆向 等待队列剩余" << *Backward << endl;
            }
        }
        cout << "调度结束"<<endl;
    }
    lock->open_lock();
}



int OneWay::get_ipc_id(char *proc_file, key_t key) {
#define BUFSZ 256
    FILE *pf;
    int i, j;
    char line[BUFSZ], colum[BUFSZ];
    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }
    fgets(line, BUFSZ, pf);
    while (!feof(pf)) {
        i = j = 0;
        fgets(line, BUFSZ, pf);
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        if (atoi(colum) != key) continue;
        j = 0;
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        i = atoi(colum);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}

char *OneWay::set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;
    //测试由 shm_key 标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        //shmget 新建 一个长度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        //shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++) shm_buf[i] = 0; //初始为 0
    }
    //共享内存区已经建立,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}
int OneWay::set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        //semget 新建一个信号灯,其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    //设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {

        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}
