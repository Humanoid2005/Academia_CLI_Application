#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <asm-generic/fcntl.h>

#include "./classes/structures.h"
#include "./classes/data_access.h"
#include "./classes/server_helper.h"

#define PORT 8080

sem_t *student_sem;
sem_t *faculty_sem;
sem_t *course_sem;
sem_t *admin_sem;
sem_t * enrollment_sem;

int send_all(int sd, const void *buf, int len) {
    int total = 0;
    const char *ptr = buf;
    while (total < len) {
        int sent = write(sd, ptr + total, len - total);
        if (sent <= 0) return sent;
        total += sent;
    }
    return total;
}

int recv_all(int sd, void *buf,int len) {
    int total = 0;
    char *ptr = buf;
    while (total < len) {
        int recvd = read(sd, ptr + total, len - total);
        if (recvd <= 0) return recvd;
        total += recvd;
    }
    return total;
}

void send_response(int sd, Response response) {
    send_all(sd, &response, sizeof(Response));
}

Request get_request(int sd) {
    Request request;
    recv_all(sd, &request, sizeof(Request));
    return request;
}

void cleanup_handler(){
    sem_close(student_sem);
    sem_close(faculty_sem);
    sem_close(course_sem);
    sem_close(admin_sem);
    sem_close(enrollment_sem);

    sem_unlink("/student_sem");
    sem_unlink("/faculty_sem");
    sem_unlink("/course_sem");
    sem_unlink("/admin_sem");
    sem_unlink("/enrollment_sem");

    exit(0);
}

void setup_semaphores(){
    student_sem = sem_open("/student_sem", O_CREAT, 0644, 1);
    faculty_sem = sem_open("/faculty_sem", O_CREAT, 0644, 1);
    course_sem = sem_open("/course_sem", O_CREAT, 0644, 1);
    admin_sem = sem_open("/admin_sem", O_CREAT, 0644, 1);
    enrollment_sem = sem_open("/enrollment_sem", O_CREAT, 0644, 1);
}

void * handle_client(void * nsd){
    int client_sd = *((int*)nsd);
    Request request;
    Response response;
    int bytes_read;

    while (1) {
        memset(&request, 0, sizeof(Request));
        memset(&response, 0, sizeof(Response));
        
        bytes_read = recv_all(client_sd, &request, sizeof(Request));
        if (bytes_read <= 0) {
            break;
        }
        
        char type[50];
        strcpy(type, request.info.type);

        if(strcmp(type,"add_student")==0 ){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                char student_name[100];
                strcpy(student_name,request.data.student.student_name);
                int addr = add_student(student_name,student_sem);
                if(addr==SUCCESS){
                    response.status = 201;
                }
                else if(addr==ALREADY_ADDED){
                    response.status = 403;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"activate_student")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                int s_id = request.info.id;
                int addr = activate_student(s_id,student_sem);
                if(addr==SUCCESS){
                    response.status = 201;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"add_faculty")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                char faculty_name[100],room_number[10];
                strcpy(faculty_name,request.data.faculty.faculty_name);
                strcpy(room_number,request.data.faculty.faculty_room_number);
                int addr = add_faculty(faculty_name,room_number,faculty_sem);
                if(addr==SUCCESS){
                    response.status = 201;
                }
                else if(addr==ALREADY_ADDED){
                    response.status = 403;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"login")==0){
            if(request.user.role==ADMIN){
                Admin admin;
                int getr = get_admin(request.user.id,&admin,admin_sem);

                if(getr!=SUCCESS){
                    response.status = 401;
                }
                else{
                    if(strcmp(admin.admin_password,request.user.password)==0){
                        response.status = 200;
                    }
                    else{
                        response.status = 401;
                    }
                }
            }
            else if(request.user.role==STUDENT){
                Student student;
                int getr = get_student(request.user.id,&student,student_sem);
                if(getr!=SUCCESS){
                    response.status = 401;
                }
                else{
                    if(strcmp(student.student_password,request.user.password)==0){
                        response.status = 200;
                    }
                    else{
                        response.status = 401;
                    }
                }
            }
            else if(request.user.role==FACULTY){
                Faculty faculty;
                int getr = get_faculty(request.user.id,&faculty,faculty_sem);
                if(getr!=SUCCESS){
                    response.status = 401;
                }
                else{
                    if(strcmp(faculty.faculty_password,request.user.password)==0){
                        response.status = 200;
                    }
                    else{
                        response.status = 401;
                    }
                }
            }
        }
        else if(strcmp(type,"exit")==0){
            break;
        }
        else if(strcmp(type,"modify_faculty")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                Faculty faculty;
                int getr = get_faculty(request.info.id,&faculty,faculty_sem);
                if(getr!=SUCCESS){
                    response.status = 400;
                }
                else{
                    request.data.faculty.present = faculty.present;
                    request.data.faculty.faculty_id = faculty.faculty_id;
                    strcpy(request.data.faculty.faculty_password,faculty.faculty_password);
                    int updater = update_faculty(request.info.id,request.data.faculty,faculty_sem);
                    if(updater==SUCCESS){
                        response.status = 201;
                    }
                    else{
                        response.status = 400;
                    }
                }
            }
        }
        else if(strcmp(type,"modify_student")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                Student student;
                int getr = get_student(request.info.id,&student,student_sem);
                if(getr!=SUCCESS){
                    response.status = 400;
                }
                else{
                    request.data.student.present = student.present;
                    request.data.student.student_id = student.student_id;
                    strcpy(request.data.student.student_password,student.student_password);
                    int updater = update_student(request.info.id,request.data.student,student_sem);
                    if(updater==SUCCESS){
                        response.status = 201;
                    }
                    else{
                        response.status = 400;
                    }
                }
            }
        }
        else if(strcmp(type,"change_password")==0){
            if(request.user.role==STUDENT){
                Student student;
                int getr = get_student(request.info.id,&student,student_sem);
                if(getr!=SUCCESS){
                    return KEY_NOT_FOUND;
                }
                strcpy(student.student_password,request.info.password);
                int updater = update_student(request.info.id,student,student_sem);
                if(updater==SUCCESS){
                    response.status = 201;
                }
                else{
                    response.status = 400;
                }
            }
            else if(request.user.role==FACULTY){
                Faculty faculty;
                int getr = get_faculty(request.info.id,&faculty,faculty_sem);
                if(getr!=SUCCESS){
                    return KEY_NOT_FOUND;
                }
                strcpy(faculty.faculty_password,request.info.password);
                int updater = update_faculty(request.info.id,faculty,faculty_sem);
                if(updater==SUCCESS){
                    response.status = 201;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"deactivate_student")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                int deleter = delete_student(request.info.id,student_sem);
                if(deleter==SUCCESS){
                    response.status = 201;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"enroll")==0){
            if(request.user.role!=STUDENT){
                response.status = 401;
            }
            else{
                Course course;
                int getr = get_course(request.info.id2,&course,course_sem);
                if(getr!=SUCCESS){
                    response.status = 404;
                }
                else if(getr==ALREADY_ADDED){
                    response.status = 403;
                }
                else if(course.number_of_seats==get_students_enrolled(course.course_id,request.data.students,enrollment_sem)){
                    response.status = 400;
                }
                else {
                    int addr = enroll(request.info.id,request.info.id2,course_sem,student_sem,enrollment_sem);
                    if(addr==SUCCESS){
                        response.status = 201;
                    }
                    else if(addr==ALREADY_ADDED){
                        response.status = 403;
                    }
                    else{
                        response.status = 400;
                    }
                }
            }
        }
        else if(strcmp(type,"unenroll")==0){
            if(request.user.role!=STUDENT){
                response.status = 401;
            }
            else{
                int remover = unenroll(request.info.id,request.info.id2,enrollment_sem);
                if(remover==SUCCESS){
                    response.status = 201;
                }
                else if(remover==UNAUTHORISED_ACCESS){
                    response.status = 401;
                }
                else{
                    printf("status: %d\n",remover);
                    response.status = 400;
                }
            }        
        }
        else if(strcmp(type,"view_enrolled_courses")==0){
            if(request.user.role!=STUDENT){
                response.status = 401;
            }
            else{
                int getr = get_enrolled_courses(request.info.id,response.data.enrollments,enrollment_sem);
                if(getr>0){
                    response.status = 200;
                }
                else{
                    printf("Messed up %d\n",getr);
                    response.status = 400;
                }
            }   
        }
        else if(strcmp(type,"add_course")==0){
            if(request.user.role!=FACULTY){
                response.status = 401;
            }
            else{
                int addr = add_course(request.data.course.course_name,request.data.course.number_of_seats,request.data.course.instructor_id,request.data.course.credits,course_sem);
                if(addr==SUCCESS){
                    response.status = 200;
                }
                else if(addr==ALREADY_ADDED){
                    response.status = 403;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"remove_course")==0){
            if(request.user.role!=FACULTY){
                response.status = 401;
            }
            else{
                int deleter = delete_course(request.info.id,request.info.id2,course_sem);
                if(deleter==SUCCESS){
                    response.status = 201;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"view_course_enrollments")==0){
            if(request.user.role!=FACULTY){
                response.status = 401;
            }
            else{

                int getr = get_students_enrolled(request.info.id,response.data.enrollments,enrollment_sem);
                if(getr>0){
                    response.status = 200;
                }
                else{
                    printf("Status: %d\n",getr);
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"view_student")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{
                int getr = get_student(request.info.id,&response.data.student,student_sem);
                if(getr==SUCCESS){
                    response.status = 200;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else if(strcmp(type,"view_faculty")==0){
            if(request.user.role!=ADMIN){
                response.status = 401;
            }
            else{

                int getr = get_faculty(request.info.id,&response.data.faculty,faculty_sem);
                if(getr==SUCCESS){
                    response.status = 200;
                }
                else{
                    response.status = 400;
                }
            }  
        }
        else if(strcmp(type,"view_course")==0){
            int getr = get_course(request.info.id,&response.data.course,course_sem);
            if(getr==SUCCESS){
                response.status = 200;
            }
            else{
                response.status = 400;
            }  
        }
        else if(strcmp(type,"view_offered_courses")==0){
            if(request.user.role!=FACULTY){
                response.status = 401;
            }
            else{

                int getr = get_offering_courses(request.info.id,response.data.courses,course_sem);
                printf("Status: %d\n",getr);
                if(getr>0){
                    response.status = 200;
                }
                else{
                    response.status = 400;
                }
            }
        }
        else{
            response.status = 404;
        }

        printf("%s Response: %d\n",type,response.status);
        send_response(client_sd,response);
    }

    close(client_sd);
    free(nsd);
    return NULL;
}

int main(){
    setup_semaphores();
    signal(SIGINT, cleanup_handler);
    signal(SIGTERM, cleanup_handler);

    struct sockaddr_in server,client;

    int sd = socket(AF_INET,SOCK_STREAM,0);
    int nsd;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if(bind(sd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind error");
        exit(1);
    }

    if(listen(sd, 5) < 0) {
        perror("Listen error");
        exit(1);
    }

    printf("Server listening on port %d...\n",PORT);

    while(1){
        int c_size = sizeof(client);
        if ((nsd = accept(sd, (struct sockaddr *)&client,&c_size)) < 0) {
            perror("Accept failed");
            continue;
        }

        int *client_sd = malloc(sizeof(int));
        *client_sd = nsd;
        char client_address[100];
        inet_ntop(AF_INET,&client.sin_addr,client_address,INET_ADDRSTRLEN);
        printf("Received connection request from : %s:%d\n",client_address,ntohs(client.sin_port));
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)client_sd) != 0) {
            perror("Thread creation failed");
            free(client_sd);
            close(nsd);
        }
        pthread_detach(thread_id);
    }
    close(sd);
}