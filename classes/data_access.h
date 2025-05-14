#ifndef DATA_ACCESS_H
#define DATA_ACCESS_H

#include "structures.h"
#include <semaphore.h>

int getRecord(int id,void* rec,int rec_size,char filename[],sem_t *sem);

int addRecord(int id,void * rec,int rec_size,char filename[],sem_t *sem);

int getNumberOfRecords(int rec_size,char filename[],sem_t *sem);

int updateRecord(int id,void * rec,int rec_size,char filename[],sem_t * sem);

int deleteRecord(int id,int rec_size,char filename[],Class class,sem_t *sem,int flag);

int getMutipleRecords(int id,MultiSelector selector,void * records,char filename[],sem_t * sem);

void convertCoursePtrToArray(Course courses[],Course * ptr);

void convertStudentPtrToArray(Student students[],Student * ptr);

void convertAdminPtrToArray(Admin admins[],Admin * ptr);

void convertFacultyPtrToArray(Faculty faculties[],Faculty * ptr);

void convertEnrollmentsPtrToArray(Enrollment enrollments[],Enrollment * ptr);

#endif