#include "structures.h"
#include "data_access.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int get_student(int s_id,Student* student,sem_t * student_sem){
    void * record = malloc(sizeof(Student));
    int getr = getRecord(s_id,record,sizeof(Student),STUDENT_FILE,student_sem);
    *student = *((Student*)record);
    free(record);
    if(student->present==DELETED){
        return KEY_NOT_FOUND;
    }
    return getr;
}

int get_faculty(int f_id,Faculty* faculty,sem_t * faculty_sem){
    void * record = malloc(sizeof(Faculty));
    int getr = getRecord(f_id,record,sizeof(Faculty),FACULTY_FILE,faculty_sem);
    *faculty = *((Faculty*)record);
    free(record);
    if(faculty->present==DELETED){
        return KEY_NOT_FOUND;
    }
    return getr;
}

int get_admin(int a_id,Admin * admin,sem_t * admin_sem){
    void * record = malloc(sizeof(Admin));
    int getr = getRecord(a_id,record,sizeof(Admin),ADMIN_FILE,admin_sem);
    *admin = *((Admin*)record);
    free(record);
    if(admin->present==DELETED){
        return KEY_NOT_FOUND;
    }
    return getr;
}

int get_course(int c_id,Course* course,sem_t * course_sem){
    int getr = getRecord(c_id,course,sizeof(Course),COURSE_FILE,course_sem);
    if(course->present==DELETED){
        return KEY_NOT_FOUND;
    }
    return getr;
}

int get_enrolled_courses(int s_id,Enrollment enrollments[],sem_t * enrollment_sem){
    void * records = malloc(sizeof(Enrollment)*MAX_LEN);
    int getr = getMutipleRecords(s_id,MULTI_STUDENT_COURSE,records,ENROLLMENT_FILE,enrollment_sem);
    convertEnrollmentsPtrToArray(enrollments,(Enrollment*)records);
    free(records);
    return getr;
}

int get_offering_courses(int f_id,Course courses[],sem_t * course_sem){
    void * records = malloc(sizeof(Course)*MAX_LEN);
    int getr = getMutipleRecords(f_id,MULTI_FACULTY_COURSE,records,COURSE_FILE,course_sem);
    convertCoursePtrToArray(courses,(Course*)records);
    free(records);
    return getr;
}

int get_students_enrolled(int c_id,Enrollment enrollments[],sem_t * enrollment_sem){
    void * records = malloc(sizeof(Enrollment)*MAX_LEN);
    int getr = getMutipleRecords(c_id,MULTI_COURSE_STUDENT,records,ENROLLMENT_FILE,enrollment_sem);
    convertEnrollmentsPtrToArray(enrollments,(Enrollment*)records);
    return getr;
}

int add_student(char student_name[],float cgpa,sem_t * student_sem){
    for(int i=0;i<MAX_LEN;i++){
        Student student;
        int getr = get_student(i,&student,student_sem);
        if(getr==SUCCESS && strcmp(student.student_name,student_name)==0){
            return ALREADY_ADDED;
        }
    }
    int key = getNumberOfRecords(sizeof(Student),STUDENT_FILE,student_sem)+1;
    Student student;
    student.student_id = key;
    student.cgpa = cgpa;
    strcpy(student.student_name,student_name);
    strcpy(student.student_password,"changeme");
    student.present = PRESENT;

    int addr = addRecord(key,&student,sizeof(Student),STUDENT_FILE,student_sem);
    return addr;
}

int add_faculty(char faculty_name[],char room_number[],sem_t * faculty_sem){
    for(int i=0;i<MAX_LEN;i++){
        Faculty faculty;
        int getr = get_faculty(i,&faculty,faculty_sem);
        if(getr==SUCCESS && strcmp(faculty.faculty_name,faculty_name)==0 && strcmp(faculty.faculty_room_number,room_number)==0){
            return ALREADY_ADDED;
        }
    }
    int key = getNumberOfRecords(sizeof(Faculty),FACULTY_FILE,faculty_sem)+1;
    Faculty faculty;
    faculty.faculty_id = key;
    strcpy(faculty.faculty_name,faculty_name);
    strcpy(faculty.faculty_password,"changeme");
    strcpy(faculty.faculty_room_number,room_number);
    faculty.present = PRESENT;

    int addr = addRecord(key,(void*)&faculty,sizeof(Faculty),FACULTY_FILE,faculty_sem);
    return addr;
}

int add_admin(char admin_name[],sem_t * admin_sem){
    for(int i=0;i<MAX_LEN;i++){
        Admin admin;
        int getr = get_admin(i,&admin,admin_sem);
        if(getr==SUCCESS && strcmp(admin.admin_name,admin_name)==0){
            return ALREADY_ADDED;
        }
    }
    int key = getNumberOfRecords(sizeof(Admin),ADMIN_FILE,admin_sem)+1;
    Admin admin;
    admin.admin_id = key;
    strcpy(admin.admin_name,admin_name);
    strcpy(admin.admin_password,"changeme");
    admin.present = PRESENT;

    int addr = addRecord(key,(void*)&admin,sizeof(Admin),ADMIN_FILE,admin_sem);
    return addr;
}

int add_course(char course_name[],int seats,int instructor_id,int credits,sem_t * course_sem){
    for(int i=0;i<MAX_LEN;i++){
        Course course;
        int getr = get_course(i,&course,course_sem);
        if(getr==SUCCESS && strcmp(course.course_name,course_name)==0 && course.instructor_id==instructor_id){
            return ALREADY_ADDED;
        }
    }
    int key = getNumberOfRecords(sizeof(Course),COURSE_FILE,course_sem)+1;
    Course course;
    course.course_id = key;
    strcpy(course.course_name,course_name);
    course.instructor_id = instructor_id;
    course.number_of_seats = seats;
    course.credits = credits;
    course.present = PRESENT;

    int addr = addRecord(key,(void*)&course,sizeof(Course),COURSE_FILE,course_sem);
    return addr;  
}

int enroll(int s_id,int c_id,sem_t* course_sem,sem_t * student_sem,sem_t * enrollment_sem){
    for(int i=0;i<MAX_LEN;i++){
        Enrollment enrollment;
        int getr = getRecord(i,(void*)&enrollment,sizeof(Enrollment),ENROLLMENT_FILE,enrollment_sem);
        if(getr==SUCCESS && enrollment.student.student_id==s_id && enrollment.course.course_id==c_id){
            int readdr = deleteRecord(i,sizeof(Enrollment),ENROLLMENT_FILE,ENROLLMENT,enrollment_sem,PRESENT);
            if(readdr!=SUCCESS){
                return ADDITION_ERROR;
            }
            else{
                return SUCCESS;
            }
            return ALREADY_ADDED;
        }
    }
    int key = getNumberOfRecords(sizeof(Enrollment),ENROLLMENT_FILE,enrollment_sem)+1;
    Course course;
    Student student;
    int getr1 = get_student(s_id,&student,student_sem);
    int getr2 = get_course(c_id,&course,course_sem);
    if(getr1!=SUCCESS){
        return KEY_NOT_FOUND;
    }
    if(getr2!=SUCCESS){
        return KEY_NOT_FOUND;
    }

    Enrollment enrollment;
    enrollment.enrollment_id = key;
    enrollment.course = course;
    enrollment.student = student;
    enrollment.present = PRESENT;

    int addr = addRecord(key,(void*)&enrollment,sizeof(Enrollment),ENROLLMENT_FILE,enrollment_sem);
    return addr;
}

int unenroll(int s_id,int c_id,sem_t * enrollment_sem){
    Enrollment enrollment;
    int found = KEY_NOT_FOUND;
    for(int i=0;i<MAX_LEN;i++){
        int getr = getRecord(i,(void*)&enrollment,sizeof(Enrollment),ENROLLMENT_FILE,enrollment_sem);
        if(getr==SUCCESS && enrollment.student.student_id==s_id && enrollment.course.course_id==c_id){
            found = SUCCESS;
            break;
        }
    }
    if(found!=SUCCESS){
        return KEY_NOT_FOUND;
    }
    if(enrollment.present==DELETED){
        return KEY_NOT_FOUND;
    }
    if(enrollment.student.student_id!=s_id){
        return UNAUTHORISED_ACCESS;
    }
    int deleter = deleteRecord(enrollment.enrollment_id,sizeof(Enrollment),ENROLLMENT_FILE,ENROLLMENT,enrollment_sem,DELETED);
    return deleter;
}

int delete_course(int c_id,int f_id,sem_t * course_sem){
    Course course; 
    int getr = getRecord(c_id,(void*)&course,sizeof(Course),COURSE_FILE,course_sem);

    if(getr!=SUCCESS){
        return KEY_NOT_FOUND;
    }
    if(f_id!=course.instructor_id){
        return UNAUTHORISED_ACCESS;
    }

    int remover = deleteRecord(c_id,sizeof(Course),COURSE_FILE,COURSE,course_sem,DELETED);
    return remover;
}

int delete_student(int s_id,sem_t * student_sem){
    int remover = deleteRecord(s_id,sizeof(Student),STUDENT_FILE,STUDENT,student_sem,DELETED);
    return remover;
}

int activate_student(int s_id,sem_t * student_sem){
    int addr = deleteRecord(s_id,sizeof(Student),STUDENT_FILE,STUDENT,student_sem,PRESENT);
    return addr;
}

int update_student(int s_id,Student student,sem_t * student_sem){
    int updater = updateRecord(s_id,(void*)&student,sizeof(Student),STUDENT_FILE,student_sem);
    return updater;
}

int change_student_password(int s_id,char new_password[],sem_t * student_sem){
    Student student;
    int getr = get_student(s_id,&student,student_sem);
    if(getr!=SUCCESS){
        return KEY_NOT_FOUND;
    }
    strcpy(student.student_password,new_password);
    int updater = update_student(s_id,student,student_sem);
    return updater;
}

int update_faculty(int f_id,Faculty faculty,sem_t * faculty_sem){
    int updater = updateRecord(f_id,(void*)&faculty,sizeof(Faculty),FACULTY_FILE,faculty_sem);
    return updater;  
}

int change_faculty_password(int f_id,char new_password[],sem_t * faculty_sem){
    Faculty faculty;
    int getr = get_faculty(f_id,&faculty,faculty_sem);
    if(getr!=SUCCESS){
        return KEY_NOT_FOUND;
    }
    strcpy(faculty.faculty_password,new_password);
    int updater = update_faculty(f_id,faculty,faculty_sem);
    return updater;
}
