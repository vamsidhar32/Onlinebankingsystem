#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "constants.h"

int sign_up_admin(char* Name, char* Password) {
    int fd = open("Admin.txt", O_CREAT | O_RDWR | O_APPEND, 0744);
    if (fd == -1) {
        perror("Error opening Admin.txt");
        return FAILURE;
    }

    struct Admin_account *A = malloc(sizeof(struct Admin_account));
    if (A == NULL) {
        perror("Memory allocation failed");
        close(fd);
        return FAILURE;
    }

    strcpy(A->Name, Name);
    strcpy(A->Password, Password);

    if (write(fd, A, sizeof(struct Admin_account)) == -1) {
        perror("Error writing to Admin.txt");
        free(A);
        close(fd);
        return FAILURE;
    }

    free(A);
    close(fd);
    return SUCCESS;
}

int sign_in_admin(char *Name, char *Password) {
    struct Admin_account *A = malloc(sizeof(struct Admin_account));
    if (A == NULL) {
        perror("Memory allocation failed");
        return FAILURE;
    }

    int fd = open("Admin.txt", O_RDWR);
    if (fd == -1) {
        perror("Error opening Admin.txt");
        free(A);
        return FAILURE;
    }

    while (read(fd, A, sizeof(struct Admin_account))) {
        if (!strcmp(A->Name, Name) && !strcmp(A->Password, Password)) {
            free(A);
            close(fd);
            return SUCCESS;
        }
    }

    free(A);
    close(fd);
    return FAILURE;
}

int add_user(int sock, int t) {
    float balance;

    if (t == User) {
        int fd = open("user.txt", O_CREAT | O_RDWR | O_APPEND, 0744);
        if (fd == -1) {
            perror("Error opening user.txt");
            return FAILURE;
        }

        struct User_account *u = malloc(sizeof(struct User_account));
        if (u == NULL) {
            perror("Memory allocation failed");
            close(fd);
            return FAILURE;
        }

        // Read user details from socket
        read(sock, u->Name, sizeof(u->Name));
        read(sock, u->Password, sizeof(u->Password));
        read(sock, &balance, sizeof(balance));
        u->balance = balance;
        u->flag = PRESENT;

        // Write user struct to file
        if (write(fd, u, sizeof(struct User_account)) == -1) {
            perror("Error writing to user.txt");
            free(u);
            close(fd);
            return FAILURE;
        }

        free(u);
        close(fd);
        return SUCCESS;
    } else if (t == Joint) {
        int fd = open("joint.txt", O_CREAT | O_RDWR | O_APPEND, 0744);
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

        // Read joint account details from socket
        int count = 0;
        read(sock, &count, sizeof(count));
        j->count = count;

        for (int i = 0; i < count; i++) {
            read(sock, j->holders[i].Name, sizeof(j->holders[i].Name));
            read(sock, j->holders[i].Password, sizeof(j->holders[i].Password));
        }

        read(sock, &balance, sizeof(balance));
        j->balance = balance;
        j->flag = PRESENT;

        // Write joint account struct to file
        if (write(fd, j, sizeof(struct Joint_account)) == -1) {
            perror("Error writing to joint.txt");
            free(j);
            close(fd);
            return FAILURE;
        }

        free(j);
        close(fd);
        return SUCCESS;
    }

    return FAILURE; 
}


int del_user(int sock, int t) {
    char Name[Max_Size];
    read(sock, Name, sizeof(Name));

    if (t == User) {
        int fd = open("user.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening user.txt");
            return FAILURE;
        }

        struct User_account u;
        int count = 0;
        while (read(fd, &u, sizeof(struct User_account))) {
            count++;
            if (u.flag == PRESENT && !strcmp(u.Name, Name)) {
                User_lock(fd, count, 1);
                u.flag = DELETED;
                lseek(fd, -sizeof(struct User_account), SEEK_CUR);
                if (write(fd, &u, sizeof(struct User_account)) == -1) {
                    perror("Error writing to user.txt");
                    User_Unlock(fd);
                    close(fd);
                    return FAILURE;
                }
                User_Unlock(fd);
                close(fd);
                return SUCCESS;
            }
        }

        close(fd);
        return FAILURE;
    } else if (t == Joint) {
        int fd = open("joint.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening joint.txt");
            return FAILURE;
        }

        struct Joint_account j;
        int count = 0;
        while (read(fd, &j, sizeof(struct Joint_account))) {
            count++;
            if (j.flag == PRESENT) {
                for (int i = 0; i < j.count; i++) {
                    if (!strcmp(j.holders[i].Name, Name)) {
                        Joint_lock(fd, count, 0);
                        j.flag = DELETED;
                        lseek(fd, -sizeof(struct Joint_account), SEEK_CUR);
                        if (write(fd, &j, sizeof(struct Joint_account)) == -1) {
                            perror("Error writing to joint.txt");
                            Joint_Unlock(fd);
                            close(fd);
                            return FAILURE;
                        }
                        Joint_Unlock(fd);
                        close(fd);
                        return SUCCESS;
                    }
                }
            }
        }

        close(fd);
        return FAILURE;
    }

    return FAILURE;  
}

int modify_user(int sock, int t) {
    char *Name = malloc(Max_Size * sizeof(char));
    read(sock, Name, Max_Size);

    if (t == User) {
        int fd = open("user.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening user.txt");
            free(Name);
            return FAILURE;
        }

        struct User_account *u = malloc(sizeof(struct User_account));
        if (u == NULL) {
            perror("Memory allocation failed");
            free(Name);
            close(fd);
            return FAILURE;
        }

        int count = 0;
        while (read(fd, u, sizeof(struct User_account))) {
            count++;
            if (u->flag == PRESENT && strcmp(u->Name, Name) == 0) {
                User_lock(fd, count, 0);
                read(sock, Name, Max_Size);
                strcpy(u->Name, Name);
                lseek(fd, -sizeof(struct User_account), SEEK_CUR);
                if (write(fd, u, sizeof(struct User_account)) == -1) {
                    perror("Error writing to user.txt");
                    free(u);
                    free(Name);
                    User_Unlock(fd);
                    close(fd);
                    return FAILURE;
                }
                User_Unlock(fd);
                free(u);
                free(Name);
                close(fd);
                return SUCCESS;
            }
        }

        free(u);
        free(Name);
        close(fd);
        return FAILURE;
    } else {
        int fd = open("joint.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening joint.txt");
            free(Name);
            return FAILURE;
        }

        struct Joint_account *j = malloc(sizeof(struct Joint_account));
        if (j == NULL) {
            perror("Memory allocation failed");
            free(Name);
            close(fd);
            return FAILURE;
        }

        int count = 0;
        while (read(fd, j, sizeof(struct Joint_account))) {
            count++;
            if (j->flag == PRESENT) {
                for (int i = 0; i < j->count; i++) {
                    if (strcmp(j->holders[i].Name, Name) == 0) {
                        Joint_lock(fd, count, 0);
                        read(sock, Name, Max_Size);
                        strcpy(j->holders[i].Name, Name);
                        lseek(fd, -sizeof(struct Joint_account), SEEK_CUR);
                        if (write(fd, j, sizeof(struct Joint_account)) == -1) {
                            perror("Error writing to joint.txt");
                            free(j);
                            free(Name);
                            Joint_Unlock(fd);
                            close(fd);
                            return FAILURE;
                        }
                        Joint_Unlock(fd);
                        free(j);
                        free(Name);
                        close(fd);
                        return SUCCESS;
                    }
                }
            }
        }

        free(j);
        free(Name);
        close(fd);
        return FAILURE;
    }
}

int search_user(int sock, int t) {
    char *Name = malloc(Max_Size * sizeof(char));
    read(sock, Name, Max_Size);

    if (t == User) {
        int fd = open("user.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening user.txt");
            free(Name);
            return FAILURE;
        }

        struct User_account *u = malloc(sizeof(struct User_account));
        if (u == NULL) {
            perror("Memory allocation failed");
            free(Name);
            close(fd);
            return FAILURE;
        }

        while (read(fd, u, sizeof(struct User_account))) {
            if (u->flag == PRESENT && strcmp(u->Name, Name) == 0) {
                free(u);
                free(Name);
                close(fd);
                return SUCCESS;
            }
        }

        free(u);
        free(Name);
        close(fd);
        return FAILURE;
    } else {
        int fd = open("joint.txt", O_RDWR);
        if (fd == -1) {
            perror("Error opening joint.txt");
            free(Name);
            return FAILURE;
        }

        struct Joint_account *j = malloc(sizeof(struct Joint_account));
        if (j == NULL) {
            perror("Memory allocation failed");
            free(Name);
            close(fd);
            return FAILURE;
        }

        while (read(fd, j, sizeof(struct Joint_account))) {
            if (j->flag == PRESENT) {
                for (int i = 0; i < j->count; i++) {
                    if (strcmp(j->holders[i].Name, Name) == 0) {
                        free(j);
                        free(Name);
                        close(fd);
                        return SUCCESS;
                    }
                }
            }
        }

        free(j);
        free(Name);
        close(fd);
        return FAILURE;
    }
}
