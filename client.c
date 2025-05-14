#include "./classes/structures.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define SERVER_PORT 8080

User current_user;

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

int recv_all(int sd, void *buf, int len) {
    int total = 0;
    char *ptr = buf;
    while (total < len) {
        int recvd = read(sd, ptr + total, len - total);
        if (recvd <= 0) return recvd;
        total += recvd;
    }
    return total;
}

void send_request(int sd, Request request) {
    send_all(sd, &request, sizeof(Request));
}

Response get_response(int sd) {
    Response response;
    memset(&response, 0, sizeof(Response));
    recv_all(sd, &response, sizeof(Response));
    return response;
}

void display_help_menu(int role) {
    if (role == ADMIN) {
        printf("\n=== ADMIN MENU ===\n");
        printf("1. Add Student\n2. View Student Details\n3. Add Faculty\n4. View Faculty Details\n");
        printf("5. Activate Student\n6. Block Student\n7. Modify Student Details\n8. Modify Faculty Details\n9. Logout and Exit\n");
    } else if (role == STUDENT) {
        printf("\n=== STUDENT MENU ===\n");
        printf("1. Enroll to course\n2. Unenroll from course\n3. View enrolled courses\n");
        printf("4. Change Password\n5. Logout and exit\n");
    } else if (role == FACULTY) {
        printf("\n=== FACULTY MENU ===\n");
        printf("1. Add new course\n2. Remove Course from catalog\n3. View enrollments in course\n");
        printf("4. View Offering Courses\n5. Change password\n6. Logout and exit\n");
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    struct sockaddr_in server;
    
    // Create socket
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if(sd < 0) {
        perror("Socket creation error");
        exit(1);
    }
    
    // Configure server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(SERVER_PORT);
    
    // Connect to server
    if(connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr_in)) < 0) {
        perror("Server connection error");
        exit(1);
    }
    
    printf("Connected to server successfully!\n");
    
    // Login process
    Request request;
    memset(&request, 0, sizeof(Request));
    int id;
    char password[200];
    int choice;
    
    printf("\n==================================================\n");
    printf("Welcome to Academia :: Course Registration System\n");
    printf("==================================================\n");
    printf("Login Type\n");
    printf("Enter your choice {1.Admin 2.Faculty 3.Student}: ");
    scanf("%d", &choice);
    printf("Enter your ID: ");
    scanf("%d", &id);
    printf("Enter your password: ");
    scanf("%s", password);
    
    request.info.id = id;
    request.user.id = id;
    strcpy(request.info.password, password);
    strcpy(request.user.password, password);
    strcpy(request.info.type, "login");

    switch (choice) {
    case 1:
        request.user.role = ADMIN;
        break;
    case 2:
        request.user.role = FACULTY;
        break;
    case 3:
        request.user.role = STUDENT;
        break;
    default:
        printf("Invalid choice. Exiting...\n");
        close(sd);
        exit(1);
    }

    send_request(sd, request);
    Response response = get_response(sd);
    
    if (response.status == 200) {
        printf("Login successful! Welcome to Academia.\n");
        current_user.role = request.user.role;
        current_user.id = id;
        strcpy(current_user.password, password);
        
        display_help_menu(current_user.role);
    } else {
        printf("Login failed: Invalid user ID or password.\n");
        close(sd);
        exit(1);
    }

    while (1) {
        int request_choice;
        memset(&request, 0, sizeof(Request));
        memset(&response, 0, sizeof(Response));
        
        // Initialize request with user data
        request.user.role = current_user.role;
        request.user.id = current_user.id;
        strcpy(request.user.password, current_user.password);
        
        printf("\nEnter your choice: (Enter -1 for help menu) ");
        if (scanf("%d", &request_choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            clear_input_buffer();
            continue;
        }

        if (request_choice == -1) {
            display_help_menu(current_user.role);
            continue;
        }

        // Handle ADMIN requests
        if (current_user.role == ADMIN) {
            switch (request_choice) {
            case 1: // Add Student
                {
                    char student_name[100];
                    printf("Enter student name: ");
                    scanf("%s", student_name);
                    
                    strcpy(request.info.type, "add_student");
                    strcpy(request.data.student.student_name, student_name);
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Student added successfully!\n");
                    } else if(response.status==403){
                        printf("Record already exists...\n");
                    } else {
                        printf("Failed to add student.\n");
                    }
                }
                break;
                
            case 2: // View Student Details
                {
                    int student_id;
                    printf("Enter student ID: ");
                    scanf("%d", &student_id);
                    
                    strcpy(request.info.type, "view_student");
                    request.info.id = student_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        Student student = response.data.student;
                        printf("\n==== STUDENT DETAILS ====\n");
                        printf("Student ID: %d\n", student.student_id);
                        printf("Name: %s\n", student.student_name);
                        printf("CGPA: %.2f\n", student.cgpa);
                        printf("Status: %s\n", student.present == PRESENT ? "Active" : "Blocked");
                    } else {
                        printf("Student not found or error retrieving details.\n");
                    }
                }
                break;
                
            case 3: // Add Faculty
                {
                    char faculty_name[100], room_number[10];
                    printf("Enter faculty name: ");
                    scanf("%s", faculty_name);
                    printf("Enter room number: ");
                    scanf("%s", room_number);
                    
                    strcpy(request.info.type, "add_faculty");
                    strcpy(request.data.faculty.faculty_name, faculty_name);
                    strcpy(request.data.faculty.faculty_room_number, room_number);
                    request.user = current_user;

                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Faculty added successfully!\n");
                    }
                    else if(response.status==403){
                        printf("Record already exists...\n");
                    }
                    else {
                        printf("Failed to add faculty.\n");
                    }
                }
                break;
                
            case 4: // View Faculty Details
                {
                    int faculty_id;
                    printf("Enter faculty ID: ");
                    scanf("%d", &faculty_id);
                    
                    strcpy(request.info.type, "view_faculty");
                    request.info.id = faculty_id;
                    request.user = current_user;

                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        Faculty faculty = response.data.faculty;
                        printf("\n==== FACULTY DETAILS ====\n");
                        printf("Faculty ID: %d\n", faculty.faculty_id);
                        printf("Name: %s\n", faculty.faculty_name);
                        printf("Room Number: %s\n", faculty.faculty_room_number);
                    } else {
                        printf("Faculty not found or error retrieving details.\n");
                    }
                }
                break;
                
            case 5: // Activate Student
                {
                    int student_id;
                    printf("Enter student ID to activate: ");
                    scanf("%d", &student_id);
                    
                    strcpy(request.info.type, "activate_student");
                    request.info.id = student_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Student activated successfully!\n");
                    } else {
                        printf("Failed to activate student.\n");
                    }
                }
                break;
                
            case 6: // Block Student
                {
                    int student_id;
                    printf("Enter student ID to block: ");
                    scanf("%d", &student_id);
                    
                    strcpy(request.info.type, "deactivate_student");
                    request.info.id = student_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Student blocked successfully!\n");
                    } else {
                        printf("Failed to block student.\n");
                    }
                }
                break;
                
            case 7: // Modify Student Details
                {
                    int student_id;
                    char student_name[100];
                    float cgpa;
                    
                    printf("Enter student ID to modify: ");
                    scanf("%d", &student_id);
                    printf("Enter new name: ");
                    scanf("%s", student_name);
                    printf("Enter new cgpa: ");
                    scanf("%f",&cgpa);
                    
                    strcpy(request.info.type, "modify_student");
                    request.info.id = student_id;
                    strcpy(request.data.student.student_name, student_name);
                    request.data.student.cgpa = cgpa;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Student details updated successfully!\n");
                    } else {
                        printf("Failed to update student details.\n");
                    }
                }
                break;
                
            case 8: // Modify Faculty Details
                {
                    int faculty_id;
                    char faculty_name[100], room_number[10];
                    
                    printf("Enter faculty ID to modify: ");
                    scanf("%d", &faculty_id);
                    printf("Enter new name: ");
                    scanf("%s", faculty_name);
                    printf("Enter new room number: ");
                    scanf("%s", room_number);
                    
                    strcpy(request.info.type, "modify_faculty");
                    request.info.id = faculty_id;
                    strcpy(request.data.faculty.faculty_name, faculty_name);
                    strcpy(request.data.faculty.faculty_room_number, room_number);
                    request.user = current_user;

                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Faculty details updated successfully!\n");
                    } else {
                        printf("Failed to update faculty details.\n");
                    }
                }
                break;
                
            case 9: // Logout and Exit
                {
                    strcpy(request.info.type, "exit");
                    request.user = current_user;
                    send_request(sd, request);
                    printf("Logging out... Goodbye!\n");
                    close(sd);
                    exit(0);
                }
                break;
                
            default:
                printf("Invalid choice. Try again.\n");
                break;
            }
        }
        // Handle STUDENT requests
        else if (current_user.role == STUDENT) {
            switch (request_choice) {
            case 1: // Enroll to course
                {
                    int course_id;
                    printf("Enter course ID to enroll: ");
                    scanf("%d", &course_id);
                    
                    strcpy(request.info.type, "enroll");
                    request.info.id = current_user.id;
                    request.info.id2 = course_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Successfully enrolled in the course!\n");
                    }
                    else if(response.status==403){
                        printf("Record already exists...\n");
                    } 
                    else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to enroll.\n");
                    }
                    else if(response.status==404){
                        printf("Failed to enroll. Course does not exist.\n");
                    } 
                    else {
                        printf("Failed to enroll. Course might be full or not available.\n");
                    }
                }
                break;
                
            case 2: // Unenroll from course
                {
                    int course_id;
                    printf("Enter course ID to unenroll: ");
                    scanf("%d", &course_id);
                    
                    strcpy(request.info.type, "unenroll");
                    request.info.id = current_user.id;
                    request.info.id2 = course_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Successfully unenrolled from the course!\n");
                    } else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to unenroll.\n");
                    } else {
                        printf("Failed to unenroll from the course.\n");
                    }
                }
                break;
                
            case 3: // View enrolled courses
                {
                    strcpy(request.info.type, "view_enrolled_courses");
                    request.info.id = current_user.id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        Enrollment* enrollments = response.data.enrollments;
                        
                        printf("\n==== YOUR ENROLLED COURSES ====\n");
                        int isThere = 0;
                        for(int i=0;i<MAX_LEN;i++){
                            if(enrollments[i].present==PRESENT){
                                isThere++;
                                printf("%d. %s (ID: %d, Credits, %d, Instructor: %d, Seats: %d)\n", 
                                    i + 1, 
                                    enrollments[i].course.course_name,
                                    enrollments[i].course.course_id,
                                    enrollments[i].course.credits,
                                    enrollments[i].course.instructor_id,
                                    enrollments[i].course.number_of_seats);
                            }
                        }
                        if (isThere == 0) {
                            printf("No courses enrolled.\n");
                        }
                    } else {
                        printf("Failed to fetch enrolled courses.\n");
                    }
                }
                break;
                
            case 4: // Change Password
                {
                    char new_password[100];
                    printf("Enter new password: ");
                    scanf("%s", new_password);

                    request.info.id = current_user.id;
                    request.user = current_user;
                    strcpy(request.info.type, "change_password");
                    strcpy(request.info.password, new_password);
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Password changed successfully!\n");
                        strcpy(current_user.password, new_password);
                    } else {
                        printf("Failed to change password.\n");
                    }
                }
                break;
                
            case 5: // Logout and exit
                {
                    strcpy(request.info.type, "exit");
                    send_request(sd, request);
                    printf("Logging out... Goodbye!\n");
                    close(sd);
                    exit(0);
                }
                break;
                
            default:
                printf("Invalid choice. Try again.\n");
                break;
            }
        }
        // Handle FACULTY requests
        else if (current_user.role == FACULTY) {
            switch (request_choice) {
            case 1: // Add new course
                {
                    char course_name[100];
                    int seats,credits;
                    
                    printf("Enter course name: ");
                    scanf("%s", course_name);
                    printf("Enter number of seats: ");
                    scanf("%d", &seats);
                    printf("Enter course credits: ");
                    scanf("%d",&credits);
                    
                    strcpy(request.info.type, "add_course");
                    strcpy(request.data.course.course_name, course_name);
                    request.data.course.instructor_id = current_user.id;
                    request.data.course.number_of_seats = seats;
                    request.data.course.credits = credits;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        printf("Course added successfully!\n");
                    } else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to add courses.\n");
                    }
                    else if(response.status==403){
                        printf("Record already exists...\n");
                    } else {
                        printf("Failed to add course.\n");
                    }
                }
                break;
                
            case 2: // Remove Course from catalog
                {
                    int course_id;
                    printf("Enter course ID to remove: ");
                    scanf("%d", &course_id);
                    
                    strcpy(request.info.type, "remove_course");
                    request.info.id2 = current_user.id;  // Faculty ID
                    request.info.id = course_id;       // Course ID
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Course removed successfully!\n");
                    } else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to remove this course.\n");
                    } else {
                        printf("Failed to remove course.\n");
                    }
                }
                break;
                
            case 3: // View enrollments in course
                {
                    int course_id;
                    printf("Enter course ID: ");
                    scanf("%d", &course_id);
                    
                    strcpy(request.info.type, "view_course_enrollments");
                    request.info.id = course_id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        Enrollment *enrollments = response.data.enrollments;
                        
                        printf("\n==== STUDENTS ENROLLED IN COURSE %d ====\n", course_id);
                        int isThere = 0;
                        for(int i=0;i<MAX_LEN;i++){
                            if(enrollments[i].present==PRESENT){
                                isThere++;
                                printf("%d. %s (ID: %d)\n", 
                                    i + 1, 
                                    enrollments[i].student.student_name,
                                    enrollments[i].student.student_id);
                            }
                        }
                        if (isThere == 0) {
                            printf("No students enrolled in this course.\n");
                        }
                    } else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to view enrollments.\n");
                    } else {
                        printf("Failed to fetch enrollment data.\n");
                    }
                }
                break;
                
            case 4: // View Offering Courses
                {
                    strcpy(request.info.type, "view_offered_courses");
                    request.info.id = current_user.id;
                    request.user = current_user;
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 200) {
                        Course *courses = response.data.courses;
                        
                        printf("\n==== COURSES OFFERED BY YOU ====\n");
                        int isThere = 0;
                        for(int i=0;i<MAX_LEN;i++){
                            if(courses[i].present==PRESENT){
                                isThere++;
                                printf("%d. %s (ID: %d, Seats: %d, Credits: %d)\n", 
                                    i + 1, 
                                    courses[i].course_name,
                                    courses[i].course_id,
                                    courses[i].number_of_seats,
                                    courses[i].credits);
                            }
                        }
                        if (isThere == 0) {
                            printf("You're not offering any courses.\n");
                        }
                    } else if (response.status == 401) {
                        printf("Unauthorized: You don't have permission to view offered courses.\n");
                    } else {
                        printf("Failed to fetch course data.\n");
                    }
                }
                break;
                
            case 5: // Change password
                {
                    char new_password[100];
                    printf("Enter new password: ");
                    scanf("%s", new_password);
                    
                    request.info.id = current_user.id;
                    request.user = current_user;
                    strcpy(request.info.type, "change_password");
                    strcpy(request.info.password, new_password);
                    
                    send_request(sd, request);
                    response = get_response(sd);
                    
                    if (response.status == 201) {
                        printf("Password changed successfully!\n");
                        strcpy(current_user.password, new_password);
                    } else {
                        printf("Failed to change password.\n");
                    }
                }
                break;
                
            case 6: // Logout and exit
                {
                    strcpy(request.info.type, "exit");
                    request.user = current_user;
                    send_request(sd, request);
                    printf("Logging out... Goodbye!\n");
                    close(sd);
                    exit(0);
                }
                break;
                
            default:
                printf("Invalid choice. Try again.\n");
                break;
            }
        }
    }
    
    close(sd);
    return 0;
}