#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void DepositMoney(int *);
void WithdrawMoney(int *);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }
    printf("Shared memory created for BankAccount and Turn variables...\n");

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if ((long)ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }
    printf("Shared memory attached...\n");

    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn
    printf("BankAccount and Turn initialized to 0...\n");

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } else if (pid == 0) {
        // Child Process
        for (int i = 0; i < 25; i++) {
            WithdrawMoney(ShmPTR);
        }
        exit(0);
    } else {
        // Parent Process
        for (int i = 0; i < 25; i++) {
            DepositMoney(ShmPTR);
        }
    }

    wait(&status);
    printf("Child process has completed...\n");
    shmdt((void *)ShmPTR);
    printf("Shared memory detached...\n");
    shmctl(ShmID, IPC_RMID, NULL);
    printf("Shared memory removed...\n");

    return 0;
}

void DepositMoney(int *ShmPTR) {
    srand(time(NULL) ^ (getpid() << 16));
    sleep(rand() % 6);

    int account;
    while (ShmPTR[1] != 0);

    account = ShmPTR[0];
    if (account <= 100) {
        int balance = rand() % 101;
        if (balance % 2 == 0) {
            account += balance;
            printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Dear old Dad: Doesn't have any money to give\n");
        }
    } else {
        printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
    }

    ShmPTR[0] = account;
    ShmPTR[1] = 1;
}

void WithdrawMoney(int *ShmPTR) {
    srand(time(NULL) ^ (getpid() << 16));
    sleep(rand() % 6);

    int account;
    while (ShmPTR[1] != 1);

    account = ShmPTR[0];
    int balance = rand() % 51;
    printf("Poor Student needs $%d\n", balance);
    if (balance <= account) {
        account -= balance;
        printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
    } else {
        printf("Poor Student: Not Enough Cash ($%d)\n", account);
    }

    ShmPTR[0] = account;
    ShmPTR[1] = 0;
}
