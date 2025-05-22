#ifndef SERVER_HELPER_H
#define SERVER_HELPER_H

#include "structures.h"
#include <semaphore.h>

int get_student(int s_id,Student* student,sem_t * student_sem);

int get_faculty(int f_id,Faculty* faculty,sem_t * faculty_sem);

int get_course(int c_id,Course* course,sem_t * course_sem);

int get_admin(int a_id,Admin * admin,sem_t * admin_sem);

int get_enrolled_courses(int s_id,Enrollment enrollments[],sem_t * enrollment_sem);

int get_offering_courses(int f_id,Course courses[],sem_t * course_sem);

int get_students_enrolled(int c_id,Enrollment enrollments[],sem_t * enrollment_sem);

int add_student(char student_name[],float cgpa,sem_t * student_sem);

int add_faculty(char faculty_name[],char room_number[],sem_t * faculty_sem);

int add_admin(char admin_name[],sem_t * admin_sem);

int add_course(char course_name[],int seats,int instructor_id,int credits,sem_t * course_sem);

int enroll(int s_id,int c_id,sem_t* course_sem,sem_t * student_sem,sem_t * enrollment_sem);

int unenroll(int s_id,int e_id,sem_t * enrollment_sem);

int delete_course(int c_id,int f_id,sem_t * course_sem);

int delete_student(int s_id,sem_t * student_sem);

int activate_student(int s_id,sem_t * student_sem);

int update_student(int s_id,Student student,sem_t * student_sem);

int update_faculty(int f_id,Faculty faculty,sem_t * faculty_sem);

int change_student_password(int s_id,char new_password[],sem_t * student_sem);

int change_faculty_password(int f_id,char new_password[],sem_t * faculty_sem);

#endif
