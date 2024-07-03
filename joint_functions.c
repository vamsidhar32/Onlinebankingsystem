#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include "constants.h"

int joint_sign_in(char *Name, char *Password) {
    int count = 0;
    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        return FAILURE;
    }

    int fd = open("joint.txt", O_CREAT | O_RDWR, 0744);
    if (fd == -1) {
        perror("Error opening joint.txt");
        free(j);
        return FAILURE;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        if (j->flag == PRESENT) {
            for (int i = 0; i < j->count; i++) {
                if (!strcmp(j->holders[i].Name, Name) && !strcmp(j->holders[i].Password, Password)) {
                    Joint_lock(fd, count, 0);
                    strcpy(Joint_user.Name, j->holders[i].Name);
                    strcpy(Joint_user.Password, j->holders[i].Password);
                    Joint_Unlock(fd);
                    close(fd);
                    free(j);
                    return SUCCESS;  
                }
            }
        }
    }

    close(fd);
    free(j);
    return FAILURE;
}

int Joint_Deposit_Amount(float amount) {
    int count = 0;
    int fd = open("joint.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening joint.txt");
        return FAILURE;
    }

    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        for (int i = 0; i < j->count; i++) {
            if (j->flag == PRESENT && !strcmp(Joint_user.Name, j->holders[i].Name)) {
                Joint_lock(fd, count, 1);
                j->balance += amount;
                lseek(fd, -sizeof(struct Joint_account), SEEK_CUR);
                if (write(fd, j, sizeof(struct Joint_account)) == -1) {
                    perror("Error writing to joint.txt");
                    Joint_Unlock(fd);
                    close(fd);
                    free(j);
                    return FAILURE;
                }
                Joint_Unlock(fd);
                close(fd);
                free(j);
                return SUCCESS; 
            }
        }
    }

    close(fd);
    free(j);
    return FAILURE;
}

int Joint_Withdraw_Amount(float amount) {
    int count = 0;
    int fd = open("joint.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening joint.txt");
        return FAILURE;
    }

    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        for (int i = 0; i < j->count; i++) {   
            if (j->flag == PRESENT && !strcmp(Joint_user.Name, j->holders[i].Name)) {
                if (j->balance >= amount) {
                    Joint_lock(fd, count, 1);
                    j->balance -= amount;
                    lseek(fd, -sizeof(struct Joint_account), SEEK_CUR);
                    if (write(fd, j, sizeof(struct Joint_account)) == -1) {
                        perror("Error writing to joint.txt");
                        Joint_Unlock(fd);
                        close(fd);
                        free(j);
                        return FAILURE;
                    }
                    Joint_Unlock(fd);
                    close(fd);
                    free(j);
                    return SUCCESS;
                } else {
                    close(fd);
                    free(j);
                    return FAILURE;
                }
            }
        }
    }

    close(fd);
    free(j);
    return FAILURE;
}

float Joint_Balance_Enquiry() {
    int count = 0;
    float balance = -1;
    int fd = open("joint.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening joint.txt");
        return -1;
    }

    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return -1;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        for (int i = 0; i < j->count; i++) {
            if (j->flag == PRESENT && !strcmp(Joint_user.Name, j->holders[i].Name)) {
                Joint_lock(fd, count, 0);
                balance = j->balance;
                printf("Balance: %f\n", j->balance);
                Joint_Unlock(fd);
                close(fd);
                free(j);
                return balance;
            }
        }  
    }

    close(fd);
    free(j);
    return -1;
}

int Joint_Change_Password(char *Password) {
    int count = 0;
    int fd = open("joint.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening joint.txt");
        return FAILURE;
    }

    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        for (int i = 0; i < j->count; i++) {
            if (j->flag == PRESENT && !strcmp(j->holders[i].Name, Joint_user.Name)) {
                Joint_lock(fd, count, 1);
                strcpy(j->holders[i].Password, Password);
                strcpy(Joint_user.Password, Password);
                lseek(fd, -sizeof(struct Joint_account), SEEK_CUR);
                if (write(fd, j, sizeof(struct Joint_account)) == -1) {
                    perror("Error writing to joint.txt");
                    Joint_Unlock(fd);
                    close(fd);
                    free(j);
                    return FAILURE;
                }
                Joint_Unlock(fd);
                close(fd);
                free(j);
                return SUCCESS;
            }
        }
    }

    close(fd);
    free(j);
    return FAILURE;
}

void Joint_View_Details(int sock) {
    int i = 0;
    char *return_str = malloc(sizeof(char) * Buf_Size);
    if (return_str == NULL) {
        perror("Memory allocation failed");
        return;
    }

    int count = 0;
    int fd = open("joint.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening joint.txt");
        free(return_str);
        return;
    }

    struct Joint_account *j = malloc(sizeof(struct Joint_account));
    if (j == NULL) {
        perror("Memory allocation failed");
        close(fd);
        free(return_str);
        return;
    }

    while (read(fd, j, sizeof(struct Joint_account))) {
        count++;
        for (int i = 0; i < j->count; i++) {
            if (j->flag == PRESENT && !strcmp(Joint_user.Name, j->holders[i].Name)) {
                i = 1;
                write(sock, &i, sizeof(int));
                Joint_lock(fd, count, 0);
                write(sock, &j->count, sizeof(j->count));
                for (int k = 0; k < j->count; k++)
                    write(sock, j->holders[k].Name, sizeof(j->holders[k].Name));
                write(sock, &j->balance, sizeof(j->balance));
                Joint_Unlock(fd);
            }
        }     
    }

    write(sock, &i, sizeof(i));
    close(fd);
    free(j);
    free(return_str);
}

void Joint_lock(int fd, int n, int x) {
    struct flock lock;
    lock.l_type = (x == 0) ? F_RDLCK : F_WRLCK;
    lock.l_len = sizeof(struct Joint_account);
    lock.l_start = (n - 1) * sizeof(struct Joint_account);
    lock.l_pid = getpid();
    if (fcntl(fd, F_SETLK, &lock) == -1) {
        perror("Error locking file");
    }
}

int Joint_Unlock(int fd) {
    struct flock lock;
    lock.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        perror("Error unlocking file");
        return FAILURE;
    }
    return SUCCESS;
}
