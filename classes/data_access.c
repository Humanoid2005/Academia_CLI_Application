#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <semaphore.h>
#include "structures.h"


int getRecord(int id,void* rec,int rec_size,char filename[],sem_t *sem){
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;


    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return FILE_ERROR;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return FILE_ERROR;
    }

    sem_wait(sem);

    if(lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET)==-1){
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        sem_post(sem);
        close(fd);
        return FILE_ERROR;
    }

    int key;
    read(fd,&key,sizeof(int));
    if(key!=id){
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        sem_post(sem);
        close(fd);
        return KEY_NOT_FOUND;
    }

    read(fd,rec,rec_size);

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    sem_post(sem);
    close(fd);
    return SUCCESS;
}

int getNumberOfRecords(int rec_size,char filename[],sem_t *sem){
    struct flock lock;
    lock.l_type = F_RDLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;

    fd = open(filename, O_RDONLY);
    if (fd == -1) {
        return 0;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return 0;
    }

    sem_wait(sem);
    
    int count  = 0;
    int key;
    void * record = malloc(rec_size);

    while(read(fd,&key,sizeof(int))==sizeof(int)){
        read(fd,record,rec_size);
        count++;
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    sem_post(sem);
    close(fd);

    return count;
}

int addRecord(int id,void * rec,int rec_size,char filename[],sem_t *sem){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;

    fd = open(filename, O_CREAT|O_RDWR|O_APPEND);
    if (fd == -1) {
        return FILE_ERROR;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return FILE_ERROR;
    }

    sem_wait(sem);

    lseek(fd,0,SEEK_END);
    int record_key = id;
    write(fd,&record_key,sizeof(int));
    write(fd,rec,rec_size);

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    sem_post(sem);
    close(fd);
    return SUCCESS;  
}

int updateRecord(int id,void * rec,int rec_size,char filename[],sem_t * sem){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        return FILE_ERROR;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return FILE_ERROR;
    }

    sem_wait(sem);

    int key;
    void * record = malloc(rec_size);

    while(read(fd,&key,sizeof(int))==sizeof(int)){
        if(key==id){
            write(fd,rec,rec_size);
            break;
        }
        else{
            read(fd,record,rec_size);
        }
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    sem_post(sem);
    close(fd);
    return SUCCESS;  
}

int deleteRecord(int id,int rec_size,char filename[],Class class,sem_t *sem,int flag){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;
    int bytes_read;

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        return FILE_ERROR;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return FILE_ERROR;
    }

    sem_wait(sem);

    int key;
    void * record = malloc(rec_size);
    if(lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET)<0){
        return DELETION_FAILED;
    }
    
    read(fd,&key,sizeof(int));
    read(fd,record,rec_size);
    if(class==STUDENT){
        Student rec = *((Student*)record);
        rec.present = flag;
        lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET);
        write(fd,&id,sizeof(int));
        write(fd,(void*)&rec,rec_size);
    }
    else if(class==FACULTY){
        Faculty rec = *((Faculty*)record);
        rec.present = flag;
        lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET);
        write(fd,&id,sizeof(int));
        write(fd,(void*)&rec,rec_size);
    }
    else if(class==ADMIN){
        Admin rec = *((Admin*)record);
        rec.present = flag;
        lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET);
        write(fd,&id,sizeof(int));
        write(fd,(void*)&rec,rec_size);
    }
    else if(class==COURSE){
        Course *rec = ((Course*)record);
        printf("Course found: %s %d %d\n",rec->course_name,rec->course_id,rec->instructor_id);
        rec->present = flag;
        lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET);
        write(fd,&id,sizeof(int));
        write(fd,(void*)rec,rec_size);
    }
    else if(class==ENROLLMENT){
        Enrollment rec = *((Enrollment*)record);
        rec.present = flag;
        lseek(fd,(id-1)*(rec_size+sizeof(int)),SEEK_SET);
        write(fd,&id,sizeof(int));
        write(fd,(void*)&rec,rec_size);
    }

    lock.l_type = F_UNLCK;
    fcntl(fd, F_SETLK, &lock);
    sem_post(sem);
    close(fd);
    return SUCCESS;
}

int getMutipleRecords(int id,MultiSelector selector,void * records,char filename[],sem_t * sem){
    struct flock lock;
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    int fd;
    int bytes_read;

    fd = open(filename, O_RDWR);
    if (fd == -1) {
        return FILE_ERROR;
    }

    if (fcntl(fd, F_SETLKW, &lock) == -1) {
        close(fd);
        return FILE_ERROR;
    }

    sem_wait(sem);

    //get multiple courses --> for student (enrollments check student_id==id)
    if(selector==MULTI_STUDENT_COURSE){
        Enrollment * enrollments = (Enrollment*)records;
        int i = 0;
        Enrollment e;
        int key;

        while(read(fd,&key,sizeof(int))==sizeof(int)){
            read(fd,&e,sizeof(Enrollment));
            if(e.student.student_id==id){
                enrollments[i] = e;
                i++;
            }
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        sem_post(sem);
        close(fd);
        return i;
    }
    //get multiple courses --> for faculty (courses check faculty_id==id)
    else if(selector==MULTI_FACULTY_COURSE){
        Course * courses = (Course*)records;
        int i = 0;
        Course c;
        int key;

        while(read(fd,&key,sizeof(int))==sizeof(int)){
            read(fd,&c,sizeof(Course));
            if(c.instructor_id==id){
                courses[i] = c;
                i++;
            }
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        sem_post(sem);
        close(fd);
        return i;
    }
    //get multiple students --> for a course (enrollements check course_id==id)
    else if(selector==MULTI_COURSE_STUDENT){
        Enrollment * enrollments = (Enrollment*)records;
        int i = 0;
        Enrollment e;
        int key;

        while(read(fd,&key,sizeof(int))==sizeof(int)){
            read(fd,&e,sizeof(Enrollment));
            if(e.course.course_id==id){
                enrollments[i] = e;
                i++;
            }
        }
        lock.l_type = F_UNLCK;
        fcntl(fd, F_SETLK, &lock);
        sem_post(sem);
        close(fd);
        return i;
    }
    else{
        return INVALID_SELECTOR;
    }
    
    return SUCCESS;  
}

void convertCoursePtrToArray(Course courses[],Course * ptr){
    int i =0;
    while(i<=MAX_LEN){
        courses[i] = *ptr;
        ptr++;
        i++;
    }
}

void convertStudentPtrToArray(Student students[],Student * ptr){
    int i =0;
    while(i<=MAX_LEN){
        students[i] = *ptr;
        ptr++;
        i++;
    }
}

void convertAdminPtrToArray(Admin admins[],Admin * ptr){
    int i =0;
    while(i<=MAX_LEN){
        admins[i] = *ptr;
        ptr++;
        i++;
    }
}

void convertFacultyPtrToArray(Faculty faculties[],Faculty * ptr){
    int i =0;
    while(i<=MAX_LEN){
        faculties[i] = *ptr;
        ptr++;
        i++;
    }
}

void convertEnrollmentsPtrToArray(Enrollment enrollments[],Enrollment * ptr){
    int i =0;
    while(i<=MAX_LEN){
        enrollments[i] = *ptr;
        ptr++;
        i++;
    }
}
