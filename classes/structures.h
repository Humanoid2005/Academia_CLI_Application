#ifndef STRUCTURES_H
#define STRUCTURES_H

#define STUDENT_FILE "./classes/data/student.dat"
#define TEMP_FILE "./classes/data/temp.dat"
#define FACULTY_FILE "./classes/data/faculty.dat"
#define COURSE_FILE "./classes/data/course.dat"
#define ADMIN_FILE "./classes/data/admin.dat"
#define ENROLLMENT_FILE "./classes/data/enrollment.dat"
#define SUCCESS 100
#define FILE_ERROR 102
#define ADDITION_ERROR 103
#define INVALID_SELECTOR 104
#define UNAUTHORISED_ACCESS 105
#define DELETION_FAILED 106
#define KEY_NOT_FOUND 107
#define UPDATION_FAILED 108
#define DELETED 99
#define PRESENT 101
#define ALREADY_ADDED 109
#define MAX_LEN 100
#define SERVER_PORT 8000
#define CONNECTIONS_HANDLED 5

typedef struct {
    char admin_name[200];
    char admin_password[200];
    int admin_id;
    int present;
} Admin;

typedef struct {
    char course_name[200];
    int course_id;
    int instructor_id;
    int credits;
    int number_of_seats;
    int present;
} Course;

typedef struct {
    char faculty_name[200];
    char faculty_password[200];
    int faculty_id;
    char faculty_room_number[10];
    int present;
} Faculty;

typedef struct {
    char student_name[200];
    int student_id;
    float cgpa;
    char student_password[200];
    int present;
} Student;

typedef struct {
    int enrollment_id;
    Student student;
    Course course;
    int present;
} Enrollment;

typedef enum{
    STUDENT,
    COURSE,
    FACULTY,
    ADMIN,
    ENROLLMENT
} Class;

typedef enum {
    MULTI_COURSE_STUDENT,
    MULTI_STUDENT_COURSE,
    MULTI_FACULTY_COURSE
} MultiSelector;

typedef struct {
    char type[50];
    int id;
    int id2;
    char password[100];
} Request_Info;

typedef struct{
    Class role;
    int id;
    char password[200];
} User;

typedef struct{
    Admin admin;
    Faculty faculty;
    Student student;
    Course course;
    Enrollment enrollment;
    Course  courses[MAX_LEN];
    Admin  admins[MAX_LEN];
    Faculty  faculties[MAX_LEN];
    Student  students[MAX_LEN];
    Enrollment enrollments[MAX_LEN];
} Data;

typedef struct {
    User user;
    Request_Info info;
    Data data;
} Request;

typedef struct{
    Data data;
    int status;
    int count;
} Response;

#endif
