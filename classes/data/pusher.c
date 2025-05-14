#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

/*
Use this file to add an admin entity to the admin file. Initially one admin entity has been added to the admin.dat file with id 1 and password changeme
*/

#define PRESENT 101

typedef struct {
    char admin_name[200];
    char admin_password[200];
    int admin_id;
    int present;
} Admin;

int main(){
    int fd = open("admin.txt",O_RDWR,0644);
    Admin admin;
    char admin_name[200],admin_password[200];
    printf("Enter admin name and password: ");
    scanf("%s %s",admin_name,admin_password);
    admin.admin_id = 2;
    strcpy(admin.admin_name,admin_name);
    strcpy(admin.admin_password,admin_password);
    admin.present = PRESENT;
    int key = 1;
    write(fd,&key,sizeof(int));
    write(fd,&admin,sizeof(Admin));
    lseek(fd,0,SEEK_SET);
    read(fd,&admin,sizeof(Admin));
    printf("Admin from file: %d %s %s %d\n",admin.admin_id,admin.admin_name,admin.admin_password,admin.present);
    close(fd);
}
