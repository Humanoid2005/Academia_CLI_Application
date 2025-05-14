#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
    char admin_name[200];
    char admin_password[200];
    int admin_id;
    int present;
} Admin;

int main(){
    int fd = open("admin.txt",O_RDWR,0644);
    Admin admin;
    admin.admin_id = 1;
    strcpy(admin.admin_name,"Sriram");
    strcpy(admin.admin_password,"changeme");
    admin.present = 101;
    int key = 1;
    write(fd,&key,sizeof(int));
    write(fd,&admin,sizeof(Admin));
    lseek(fd,0,SEEK_SET);
    read(fd,&admin,sizeof(Admin));
    printf("Admin from file: %d %s %s %d\n",admin.admin_id,admin.admin_name,admin.admin_password,admin.present);
    close(fd);
}