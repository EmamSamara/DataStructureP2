//Emam Samara
//1220022
//section 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char g_baseDir[512] = "";

// get the folder of the running executable using argv[0].
static void initBaseDir(const char* argv0) {
    if (!argv0) return;
    const char* slash = strrchr(argv0, '\\');
    if (!slash) slash = strrchr(argv0, '/');
    if (!slash) return;
    size_t len = (size_t)(slash - argv0);
    if (len >= sizeof(g_baseDir)) len = sizeof(g_baseDir) - 1;
    memcpy(g_baseDir, argv0, len);
    g_baseDir[len] = '\0';
}

// build a path using the executable folder if available.
static void buildPath(const char* filename, char* out, size_t size) {
    if (!filename || !out || size == 0) return;
    if (g_baseDir[0] == '\0') {
        strncpy(out, filename, size - 1);
        out[size - 1] = '\0';
        return;
    }
    snprintf(out, size, "%s\\%s", g_baseDir, filename);
}

// check for basic ASCII whitespace.
static int isSpaceChar(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

// check for ASCII letter.
static int isAlphaChar(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// check for ASCII digit.
static int isDigitChar(char c) {
    return c >= '0' && c <= '9';
}

// check for ASCII alphanumeric.
static int isAlnumChar(char c) {
    return isAlphaChar(c) || isDigitChar(c);
}

// convert ASCII letter to lowercase.
static char toLowerChar(char c) {
    if (c >= 'A' && c <= 'Z') return (char)(c + ('a' - 'A'));
    return c;
}

#define MAX_NAME_LEN 128
#define MAX_ID_LEN 32
#define MAX_MAJOR_LEN 128
#define MAX_CODE_LEN 32
#define MAX_TITLE_LEN 128
#define MAX_SEMESTER_LEN 64
#define LINE_BUFFER 512

typedef struct CourseRecord {
    char courseCode[MAX_CODE_LEN];
    char courseTitle[MAX_TITLE_LEN];
    int creditHours;
    char semester[MAX_SEMESTER_LEN];
    struct CourseRecord* next;
} CourseRecord;

typedef struct StudentRecord {
    char name[MAX_NAME_LEN];
    char studentID[MAX_ID_LEN];
    char major[MAX_MAJOR_LEN];
    CourseRecord* courses;
} StudentRecord;

typedef struct AVLNode {
    StudentRecord data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

typedef enum {
    HASH_EMPTY,
    HASH_OCCUPIED,
    HASH_DELETED
} SlotState;

typedef struct {
    SlotState state;
    StudentRecord data;
} HashEntry;

typedef struct {
    HashEntry* entries;
    int capacity;
    int size;
} HashTable;

typedef struct {
    char (*data)[MAX_ID_LEN];
    int size;
    int capacity;
} NodeList;

// remove spaces at the start and end of a string.
static void trim(char* s) {
    if (!s) return;
    size_t start = 0;
    size_t len = strlen(s);
    while (start < len && isSpaceChar(s[start])) start++;
    size_t end = len;
    while (end > start && isSpaceChar(s[end - 1])) end--;
    if (start > 0) memmove(s, s + start, end - start);
    s[end - start] = '\0';
}

// read one line from input and trim it.
static void readLine(char* buffer, size_t size) {
    if (!fgets(buffer, (int)size, stdin)) {
        buffer[0] = '\0';
        return;
    }
    buffer[strcspn(buffer, "\n")] = '\0';
    trim(buffer);
}

// check that a string has only letters and spaces.
static int isLettersSpaces(const char* s) {
    if (!s || *s == '\0') return 0;
    int hasLetter = 0;
    for (size_t i = 0; s[i]; ++i) {
        if (isAlphaChar(s[i])) {
            hasLetter = 1;
            continue;
        }
        if (s[i] == ' ') continue;
        return 0;
    }
    return hasLetter;
}

// check that a string has only digits.
static int isDigitsOnly(const char* s) {
    if (!s || *s == '\0') return 0;
    for (size_t i = 0; s[i]; ++i) {
        if (!isDigitChar(s[i])) return 0;
    }
    return 1;
}

// check that a course code is alphanumeric.
static int isCourseCode(const char* s) {
    if (!s || *s == '\0') return 0;
    for (size_t i = 0; s[i]; ++i) {
        if (!isAlnumChar(s[i])) return 0;
    }
    return 1;
}

// check that a semester string is valid.
static int isSemesterString(const char* s) {
    if (!s || *s == '\0') return 0;
    for (size_t i = 0; s[i]; ++i) {
        if (!isAlnumChar(s[i]) && s[i] != ' ') return 0;
    }
    return 1;
}

typedef int (*ValidatorFn)(const char*);

// compare two strings without case sensitivity.
static int equalsIgnoreCase(const char* a, const char* b) {
    if (!a || !b) return 0;
    while (*a && *b) {
        char ca = toLowerChar(*a);
        char cb = toLowerChar(*b);
        if (ca != cb) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}

// check if the user typed the exit command.
static int isExitCommand(const char* s) {
    return s && equalsIgnoreCase(s, "exit");
}

// read input and validate it with a rule.
static int getValidatedInput(const char* prompt, char* buffer, size_t size, ValidatorFn validator, const char* errorMsg) {
    while (1) {
        if (prompt) printf("%s", prompt);
        readLine(buffer, size);
        if (isExitCommand(buffer)) return 0;
        if (validator(buffer)) break;
        printf("%s", errorMsg);
    }
    return 1;
}

// read an integer with a min and max limit.
static int readIntWithPrompt(const char* prompt, int minValue, int maxValue) {
    char buffer[64];
    for (;;) {
        if (prompt) printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) return minValue;
        char* endptr = NULL;
        long value = strtol(buffer, &endptr, 10);
        if (endptr == buffer) {
            printf("[ERROR] Invalid number. Try again.\n");
            continue;
        }
        while (*endptr) {
            if (!isSpaceChar(*endptr)) break;
            endptr++;
        }
        if (*endptr && *endptr != '\n') {
            printf("[ERROR] Invalid characters detected. Try again.\n");
            continue;
        }
        if (value < minValue || value > maxValue) {
            printf("[ERROR] Value must be between %d and %d.\n", minValue, maxValue);
            continue;
        }
        return (int)value;
    }
}

// read an integer or allow exit.
static int readIntWithPromptOrExit(const char* prompt, int minValue, int maxValue, int* canceled) {
    char buffer[64];
    if (canceled) *canceled = 0;
    for (;;) {
        if (prompt) printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            if (canceled) *canceled = 1;
            return minValue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';
        trim(buffer);
        if (isExitCommand(buffer)) {
            if (canceled) *canceled = 1;
            return minValue;
        }
        char* endptr = NULL;
        long value = strtol(buffer, &endptr, 10);
        if (endptr == buffer) {
            printf("[ERROR] Invalid number. Try again.\n");
            continue;
        }
        while (*endptr) {
        if (!isSpaceChar(*endptr)) break;
            endptr++;
        }
        if (*endptr && *endptr != '\n') {
            printf("[ERROR] Invalid characters detected. Try again.\n");
            continue;
        }
        if (value < minValue || value > maxValue) {
            printf("[ERROR] Value must be between %d and %d.\n", minValue, maxValue);
            continue;
        }
        return (int)value;
    }
}

// create a new course record node.
static CourseRecord* createCourseRecord(const char* code, const char* title, int hours, const char* semester) {
    CourseRecord* record = (CourseRecord*)malloc(sizeof(CourseRecord));
    if (!record) return NULL;
    strncpy(record->courseCode, code, MAX_CODE_LEN - 1);
    record->courseCode[MAX_CODE_LEN - 1] = '\0';
    strncpy(record->courseTitle, title, MAX_TITLE_LEN - 1);
    record->courseTitle[MAX_TITLE_LEN - 1] = '\0';
    record->creditHours = hours;
    strncpy(record->semester, semester, MAX_SEMESTER_LEN - 1);
    record->semester[MAX_SEMESTER_LEN - 1] = '\0';
    record->next = NULL;
    return record;
}

// free a linked list of course records.
static void freeCourseRecords(CourseRecord* head) {
    while (head) {
        CourseRecord* temp = head;
        head = head->next;
        free(temp);
    }
}

// make a copy of a course list.
static CourseRecord* cloneCourseRecords(const CourseRecord* head) {
    CourseRecord* newHead = NULL;
    CourseRecord* tail = NULL;
    while (head) {
        CourseRecord* node = createCourseRecord(head->courseCode, head->courseTitle, head->creditHours, head->semester);
        if (!node) break;
        if (!newHead) newHead = node;
        else tail->next = node;
        tail = node;
        head = head->next;
    }
    return newHead;
}

// add or update a course in the list.
static CourseRecord* appendCourseRecord(CourseRecord* head, CourseRecord* record, int* added) {
    if (added) *added = 0;
    if (!record) return head;
    CourseRecord* current = head;
    while (current) {
        if (equalsIgnoreCase(current->courseCode, record->courseCode) &&
            equalsIgnoreCase(current->semester, record->semester)) {
            free(record);
            return head;
        }
        current = current->next;
    }
    if (!head) {
        head = record;
    } else {
        CourseRecord* tail = head;
        while (tail->next) tail = tail->next;
        tail->next = record;
    }
    if (added) *added = 1;
    return head;
}

// remove a course from the list.
static CourseRecord* removeCourseRecord(CourseRecord* head, const char* courseCode, const char* semester, int* removed) {
    if (removed) *removed = 0;
    CourseRecord* current = head;
    CourseRecord* prev = NULL;
    while (current) {
        if (equalsIgnoreCase(current->courseCode, courseCode) &&
            equalsIgnoreCase(current->semester, semester)) {
            if (prev) prev->next = current->next;
            else head = current->next;
            free(current);
            if (removed) *removed = 1;
            return head;
        }
        prev = current;
        current = current->next;
    }
    return head;
}

// check if a student has a course.
static int hasCourse(const CourseRecord* head, const char* courseCode) {
    while (head) {
        if (equalsIgnoreCase(head->courseCode, courseCode)) return 1;
        head = head->next;
    }
    return 0;
}

// get the height of a tree node.
static int nodeHeight(AVLNode* node) {
    return node ? node->height : -1;
}

// return the bigger of two numbers.
static int maxInt(int a, int b) {
    return a > b ? a : b;
}

// create a new AVL tree node for a student.
static AVLNode* createNode(const char* name, const char* id, const char* major, CourseRecord* course) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    if (!node) return NULL;
    strncpy(node->data.name, name, MAX_NAME_LEN - 1);
    node->data.name[MAX_NAME_LEN - 1] = '\0';
    strncpy(node->data.studentID, id, MAX_ID_LEN - 1);
    node->data.studentID[MAX_ID_LEN - 1] = '\0';
    strncpy(node->data.major, major, MAX_MAJOR_LEN - 1);
    node->data.major[MAX_MAJOR_LEN - 1] = '\0';
    node->data.courses = course;
    node->left = node->right = NULL;
    node->height = 0;
    return node;
}

// copy student fields and courses.
static void copyStudentRecord(StudentRecord* dest, const StudentRecord* src) {
    strncpy(dest->name, src->name, MAX_NAME_LEN - 1);
    dest->name[MAX_NAME_LEN - 1] = '\0';
    strncpy(dest->studentID, src->studentID, MAX_ID_LEN - 1);
    dest->studentID[MAX_ID_LEN - 1] = '\0';
    strncpy(dest->major, src->major, MAX_MAJOR_LEN - 1);
    dest->major[MAX_MAJOR_LEN - 1] = '\0';
    freeCourseRecords(dest->courses);
    dest->courses = cloneCourseRecords(src->courses);
}

// rotate the tree to the right.
static AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    x->right = y;
    y->left = T2;
    y->height = maxInt(nodeHeight(y->left), nodeHeight(y->right)) + 1;
    x->height = maxInt(nodeHeight(x->left), nodeHeight(x->right)) + 1;
    return x;
}

// rotate the tree to the left.
static AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    y->left = x;
    x->right = T2;
    x->height = maxInt(nodeHeight(x->left), nodeHeight(x->right)) + 1;
    y->height = maxInt(nodeHeight(y->left), nodeHeight(y->right)) + 1;
    return y;
}

// left-right rotation for AVL balance.
static AVLNode* doubleRotateWithLeft(AVLNode* node) {
    node->left = leftRotate(node->left);
    return rightRotate(node);
}

// right-left rotation for AVL balance.
static AVLNode* doubleRotateWithRight(AVLNode* node) {
    node->right = rightRotate(node->right);
    return leftRotate(node);
}
// get the balance factor of a node.
static int getBalance(AVLNode* node) {
    if (!node) return 0;
    return nodeHeight(node->left) - nodeHeight(node->right);
}

// insert or update a student in the AVL tree.
static AVLNode* insertRegistration(AVLNode* node, const char* name, const char* id, const char* major, CourseRecord* course, int* newStudent, int* newCourse) {
    if (!node) {
        if (newStudent) *newStudent = 1;
        if (newCourse) *newCourse = 1;
        return createNode(name, id, major, course);
    }
    int cmp = strcmp(id, node->data.studentID);
    if (cmp < 0) {
        node->left = insertRegistration(node->left, name, id, major, course, newStudent, newCourse);
        if (nodeHeight(node->left) - nodeHeight(node->right) == 2) {
            if (strcmp(id, node->left->data.studentID) < 0) node = rightRotate(node);
            else node = doubleRotateWithLeft(node);
        }
    } else if (cmp > 0) {
        node->right = insertRegistration(node->right, name, id, major, course, newStudent, newCourse);
        if (nodeHeight(node->right) - nodeHeight(node->left) == 2) {
            if (strcmp(id, node->right->data.studentID) > 0) node = leftRotate(node);
            else node = doubleRotateWithRight(node);
        }
    } else {
        if (newStudent) *newStudent = 0;
        strncpy(node->data.name, name, MAX_NAME_LEN - 1);
        node->data.name[MAX_NAME_LEN - 1] = '\0';
        strncpy(node->data.major, major, MAX_MAJOR_LEN - 1);
        node->data.major[MAX_MAJOR_LEN - 1] = '\0';
        int added = 0;
        node->data.courses = appendCourseRecord(node->data.courses, course, &added);
        if (newCourse) *newCourse = added;
        if (!added) return node;
    }
    node->height = 1 + maxInt(nodeHeight(node->left), nodeHeight(node->right));
    return node;
}
// find the smallest node in a subtree.
static AVLNode* minValueNode(AVLNode* node) {
    AVLNode* current = node;
    while (current && current->left) current = current->left;
    return current;
}

// remove a student from the AVL tree.
static AVLNode* deleteStudent(AVLNode* root, const char* studentID) {
    if (!root) return NULL;
    int cmp = strcmp(studentID, root->data.studentID);
    if (cmp < 0) root->left = deleteStudent(root->left, studentID);
    else if (cmp > 0) root->right = deleteStudent(root->right, studentID);
    else {
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) {
                freeCourseRecords(root->data.courses);
                free(root);
                return NULL;
            } else {
                AVLNode tempNode = *temp;
                freeCourseRecords(root->data.courses);
                *root = tempNode;
                free(temp);
            }
        } else {
            AVLNode* temp = minValueNode(root->right);
            copyStudentRecord(&root->data, &temp->data);
            root->right = deleteStudent(root->right, temp->data.studentID);
        }
    }
    if (!root) return NULL;
    root->height = 1 + maxInt(nodeHeight(root->left), nodeHeight(root->right));
    int balance = getBalance(root);
    if (balance > 1 && getBalance(root->left) >= 0) return rightRotate(root);
    if (balance > 1 && getBalance(root->left) < 0) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }
    if (balance < -1 && getBalance(root->right) <= 0) return leftRotate(root);
    if (balance < -1 && getBalance(root->right) > 0) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }
    return root;
}

// search the AVL tree by student ID.
static AVLNode* findByID(AVLNode* root, const char* studentID) {
    while (root) {
        int cmp = strcmp(studentID, root->data.studentID);
        if (cmp == 0) return root;
        root = cmp < 0 ? root->left : root->right;
    }
    return NULL;
}

// initialize a simple list for search results.
static void initNodeList(NodeList* list) {
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
}

// add a student ID to the result list.
static void appendNode(NodeList* list, const char* studentID) {
    if (list->size == list->capacity) {
        int newCap = list->capacity == 0 ? 4 : list->capacity * 2;
        char (*resized)[MAX_ID_LEN] = (char (*)[MAX_ID_LEN])realloc(list->data, newCap * sizeof(*list->data));
        if (!resized) return;
        list->data = resized;
        list->capacity = newCap;
    }
    strncpy(list->data[list->size], studentID, MAX_ID_LEN - 1);
    list->data[list->size][MAX_ID_LEN - 1] = '\0';
    list->size++;
}

// collect matching names from the AVL tree.
static void collectByName(AVLNode* root, const char* name, NodeList* list) {
    if (!root) return;
    collectByName(root->left, name, list);
    if (equalsIgnoreCase(root->data.name, name)) appendNode(list, root->data.studentID);
    collectByName(root->right, name, list);
}

// free the result list memory.
static void freeNodeList(NodeList* list) {
    free(list->data);
    list->data = NULL;
    list->size = 0;
    list->capacity = 0;
}

// print a list of courses.
static void printCourses(const CourseRecord* head) {
    int index = 1;
    while (head) {
        printf("    [%d] %s | %s | %d CH | %s\n", index, head->courseCode, head->courseTitle, head->creditHours, head->semester);
        head = head->next;
        index++;
    }
}

// print one student from the AVL tree.
static void printStudent(const AVLNode* node) {
    if (!node) return;
    printf("Name: %s\n", node->data.name);
    printf("Student ID: %s\n", node->data.studentID);
    printf("Major: %s\n", node->data.major);
    printf("Courses:\n");
    if (!node->data.courses) printf("    None\n");
    else printCourses(node->data.courses);
}

// print one student record from the hash table.
static void printStudentRecord(const StudentRecord* student) {
    if (!student) return;
    printf("Name: %s\n", student->name);
    printf("Student ID: %s\n", student->studentID);
    printf("Major: %s\n", student->major);
    printf("Courses:\n");
    if (!student->courses) printf("    None\n");
    else printCourses(student->courses);
}

// list students registered in a course.
static void listStudentsByCourse(AVLNode* root, const char* courseCode, int* count) {
    if (!root) return;
    listStudentsByCourse(root->left, courseCode, count);
    if (hasCourse(root->data.courses, courseCode)) {
        printf("\n\n");
        printStudent(root);
        (*count)++;
    }
    listStudentsByCourse(root->right, courseCode, count);
}

// write one student with all courses to a file.
static void saveStudentCourses(FILE* file, const StudentRecord* student) {
    const CourseRecord* course = student->courses;
    while (course) {
        fprintf(file, "%s#%s#%s#%s#%s#%d#%s\n",
            student->name, student->studentID, student->major,
            course->courseCode, course->courseTitle, course->creditHours, course->semester);
        course = course->next;
    }
}

// save an AVL node to file lines.
static void saveNodeCourses(FILE* file, const AVLNode* node) {
    saveStudentCourses(file, &node->data);
}

// traverse the tree and save all nodes.
static void saveTreeRecursive(FILE* file, AVLNode* node) {
    if (!node) return;
    saveTreeRecursive(file, node->left);
    saveNodeCourses(file, node);
    saveTreeRecursive(file, node->right);
}

// forward declaration for hash insertion.
static int hashInsert(HashTable* table, const char* name, const char* id, const char* major, CourseRecord* course);

// save all AVL data to a file.
static void saveTreeToFile(const char* filename, AVLNode* root) {
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("[ERROR] Unable to write to %s\n", filename);
        return;
    }
    saveTreeRecursive(file, root);
    fclose(file);
    printf("[OK] Data saved to %s\n", filename);
}

// count total students in the AVL tree.
static int countStudents(AVLNode* root) {
    if (!root) return 0;
    return 1 + countStudents(root->left) + countStudents(root->right);
}

// export student base data to a file.
static void exportStudentsRecursive(FILE* file, AVLNode* node) {
    if (!node) return;
    exportStudentsRecursive(file, node->left);
    fprintf(file, "%s#%s#%s\n", node->data.name, node->data.studentID, node->data.major);
    exportStudentsRecursive(file, node->right);
}

// write student data to students_hash.data.
static void exportStudentsData(const char* filename, AVLNode* root) {
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("[ERROR] Cannot open %s\n", filename);
        return;
    }
    exportStudentsRecursive(file, root);
    fclose(file);
    printf("[OK] Student data exported to %s\n", filename);
}

// count non-empty lines in a file.
static int countLinesInFile(const char* filename) {
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "r");
    if (!file) return 0;
    int count = 0;
    char line[LINE_BUFFER];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        trim(line);
        if (strlen(line) > 0) count++;
    }
    fclose(file);
    return count;
}

// load hash table entries from a file.
static void loadHashTableFromFile(HashTable* table, const char* filename) {
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("[ERROR] Cannot open %s\n", filename);
        return;
    }
    char line[LINE_BUFFER];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        trim(line);
        if (strlen(line) == 0) continue;
        char* tokens[7];
        int count = 0;
        char* token = strtok(line, "#");
        while (token && count < 7) {
            tokens[count++] = token;
            token = strtok(NULL, "#");
        }
        if (count != 7) continue;
        char* name = tokens[0];
        char* id = tokens[1];
        char* major = tokens[2];
        char* courseCode = tokens[3];
        char* courseTitle = tokens[4];
        int hours = atoi(tokens[5]);
        char* semester = tokens[6];
        CourseRecord* course = createCourseRecord(courseCode, courseTitle, hours, semester);
        if (!course) continue;
        if (!hashInsert(table, name, id, major, course)) {
            free(course);
        }
    }
    fclose(file);
}

// load registrations from reg.txt into the AVL tree.
static AVLNode* loadFromFile(const char* filename, AVLNode* root) {
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("[INFO] File %s not found. Starting with empty records.\n", filename);
        return root;
    }
    char line[LINE_BUFFER];
    int loaded = 0;
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';
        trim(line);
        if (strlen(line) == 0) continue;
        char* tokens[7];
        int count = 0;
        char* token = strtok(line, "#");
        while (token && count < 7) {
            tokens[count++] = token;
            token = strtok(NULL, "#");
        }
        if (count != 7) continue;
        char* name = tokens[0];
        char* id = tokens[1];
        char* major = tokens[2];
        char* courseCode = tokens[3];
        char* courseTitle = tokens[4];
        int hours = atoi(tokens[5]);
        char* semester = tokens[6];
        CourseRecord* course = createCourseRecord(courseCode, courseTitle, hours, semester);
        if (!course) continue;
        root = insertRegistration(root, name, id, major, course, NULL, NULL);
        loaded++;
    }
    fclose(file);
    printf("[OK] Loaded %d registrations from %s\n", loaded, filename);
    return root;
}

// free all nodes in the AVL tree.
static void freeTree(AVLNode* root) {
    if (!root) return;
    freeTree(root->left);
    freeTree(root->right);
    freeCourseRecords(root->data.courses);
    free(root);
}

// check if a number is prime.
static int isPrime(int n) {
    if (n <= 1) return 0;
    if (n <= 3) return 1;
    if (n % 2 == 0 || n % 3 == 0) return 0;
    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0) return 0;
    }
    return 1;
}

// get the next prime number.
static int nextPrime(int n) {
    while (!isPrime(n)) n++;
    return n;
}

// initialize the hash table.
static void initHashTable(HashTable* table, int capacity) {
    table->capacity = capacity;
    table->size = 0;
    table->entries = (HashEntry*)malloc(sizeof(HashEntry) * capacity);
    for (int i = 0; i < capacity; i++) {
        table->entries[i].state = HASH_EMPTY;
        table->entries[i].data.name[0] = '\0';
        table->entries[i].data.studentID[0] = '\0';
        table->entries[i].data.major[0] = '\0';
        table->entries[i].data.courses = NULL;
    }
}

// free hash table memory.
static void freeHashTable(HashTable* table) {
    if (table->entries) {
        for (int i = 0; i < table->capacity; i++) {
            if (table->entries[i].state == HASH_OCCUPIED) {
                freeCourseRecords(table->entries[i].data.courses);
                table->entries[i].data.courses = NULL;
            }
        }
    }
    free(table->entries);
    table->entries = NULL;
    table->capacity = 0;
    table->size = 0;
}

// compute a hash value for a string.
static unsigned long hashString(const char* s, int mod) {
    unsigned long hash = 0;
    while (*s) {
        hash = (hash << 5) + (char)toLowerChar(*s);
        s++;
    }
    return hash % (unsigned long)mod;
}

// insert or update a record in the hash table (key = student name).
static int hashInsert(HashTable* table, const char* name, const char* id, const char* major, CourseRecord* course) {
    if (!table->entries) return 0;
    if (table->size >= table->capacity) return 0;
    unsigned long index = hashString(name, table->capacity);
    int firstDeleted = -1;
    for (int i = 0; i < table->capacity; i++) {
        unsigned long probe = (index + i) % table->capacity;
        HashEntry* entry = &table->entries[probe];
        if (entry->state == HASH_EMPTY) {
            int target = firstDeleted >= 0 ? firstDeleted : (int)probe;
            entry = &table->entries[target];
            strncpy(entry->data.name, name, MAX_NAME_LEN - 1);
            entry->data.name[MAX_NAME_LEN - 1] = '\0';
            strncpy(entry->data.studentID, id, MAX_ID_LEN - 1);
            entry->data.studentID[MAX_ID_LEN - 1] = '\0';
            strncpy(entry->data.major, major, MAX_MAJOR_LEN - 1);
            entry->data.major[MAX_MAJOR_LEN - 1] = '\0';
            entry->data.courses = NULL;
            entry->data.courses = appendCourseRecord(entry->data.courses, course, NULL);
            entry->state = HASH_OCCUPIED;
            table->size++;
            return 1;
        }
        if (entry->state == HASH_DELETED && firstDeleted < 0) firstDeleted = (int)probe;
        else if (entry->state == HASH_OCCUPIED && equalsIgnoreCase(entry->data.name, name)) {
            if (strcmp(entry->data.studentID, id) == 0) {
                strncpy(entry->data.name, name, MAX_NAME_LEN - 1);
                entry->data.name[MAX_NAME_LEN - 1] = '\0';
                strncpy(entry->data.studentID, id, MAX_ID_LEN - 1);
                entry->data.studentID[MAX_ID_LEN - 1] = '\0';
                strncpy(entry->data.major, major, MAX_MAJOR_LEN - 1);
                entry->data.major[MAX_MAJOR_LEN - 1] = '\0';
                entry->data.courses = appendCourseRecord(entry->data.courses, course, NULL);
                return 1;
            }
        }
    }
    if (firstDeleted >= 0) {
        HashEntry* entry = &table->entries[firstDeleted];
        strncpy(entry->data.name, name, MAX_NAME_LEN - 1);
        entry->data.name[MAX_NAME_LEN - 1] = '\0';
        strncpy(entry->data.studentID, id, MAX_ID_LEN - 1);
        entry->data.studentID[MAX_ID_LEN - 1] = '\0';
        strncpy(entry->data.major, major, MAX_MAJOR_LEN - 1);
        entry->data.major[MAX_MAJOR_LEN - 1] = '\0';
        entry->data.courses = NULL;
        entry->data.courses = appendCourseRecord(entry->data.courses, course, NULL);
        entry->state = HASH_OCCUPIED;
        table->size++;
        return 1;
    }
    return 0;
}

// search for a student name in the hash table.
static int hashSearch(HashTable* table, const char* name) {
    if (!table->entries) return -1;
    unsigned long index = hashString(name, table->capacity);
    for (int i = 0; i < table->capacity; i++) {
        unsigned long probe = (index + i) % table->capacity;
        HashEntry* entry = &table->entries[probe];
        if (entry->state == HASH_EMPTY) return -1;
        if (entry->state == HASH_OCCUPIED && equalsIgnoreCase(entry->data.name, name)) return (int)probe;
    }
    return -1;
}

// collect all indices that match a name in the hash table.
static int collectHashMatches(HashTable* table, const char* name, int* indices, int maxCount) {
    if (!table->entries || !indices || maxCount <= 0) return 0;
    int count = 0;
    for (int i = 0; i < table->capacity; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->state == HASH_OCCUPIED && equalsIgnoreCase(entry->data.name, name)) {
            indices[count++] = i;
            if (count == maxCount) break;
        }
    }
    return count;
}

// delete a student record at a specific index in the hash table.
static int hashDeleteAt(HashTable* table, int index) {
    if (!table->entries) return 0;
    if (index < 0 || index >= table->capacity) return 0;
    if (table->entries[index].state != HASH_OCCUPIED) return 0;
    freeCourseRecords(table->entries[index].data.courses);
    table->entries[index].data.courses = NULL;
    table->entries[index].state = HASH_DELETED;
    table->entries[index].data.name[0] = '\0';
    table->entries[index].data.studentID[0] = '\0';
    table->entries[index].data.major[0] = '\0';
    table->size--;
    return 1;
}

// check if a student ID already exists in the hash table.
static int hashIdExists(HashTable* table, const char* studentID) {
    if (!table->entries) return 0;
    for (int i = 0; i < table->capacity; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->state == HASH_OCCUPIED && strcmp(entry->data.studentID, studentID) == 0) return 1;
    }
    return 0;
}

// print the whole hash table.
static void printHashTable(HashTable* table) {
    if (!table->entries) {
        printf("[INFO] Hash table is empty.\n");
        return;
    }
    for (int i = 0; i < table->capacity; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->state == HASH_OCCUPIED) {
            printf("%d: %s (ID %s)\n", i, entry->data.name, entry->data.studentID);
        } else if (entry->state == HASH_DELETED) {
            printf("%d: <DELETED>\n", i);
        } else {
            printf("%d: <EMPTY>\n", i);
        }
    }
}

// build the hash table from students_hash.data.
static void buildHashTable(HashTable* table, AVLNode* root) {
    int studentCount = countStudents(root);
    if (studentCount == 0) {
        char path[512];
        buildPath("students_hash.data", path, sizeof(path));
        FILE* empty = fopen(path, "w");
        if (empty) fclose(empty);
        printf("[INFO] No student records to hash.\n");
        return;
    }
    saveTreeToFile("students_hash.data", root);
    if (table->entries) freeHashTable(table);
    int lineCount = countLinesInFile("students_hash.data");
    if (lineCount == 0) {
        printf("[INFO] No student records to hash.\n");
        return;
    }
    int capacity = nextPrime(lineCount * 2 + 1);
    initHashTable(table, capacity);
    loadHashTableFromFile(table, "students_hash.data");
    printf("[OK] Hash table built with %d slots.\n", capacity);
}

// save hash table data back to a file.
static void saveHashTableToFile(HashTable* table, const char* filename) {
    if (!table->entries) {
        printf("[ERROR] Hash table is not initialized.\n");
        return;
    }
    char path[512];
    buildPath(filename, path, sizeof(path));
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("[ERROR] Cannot open %s.\n", filename);
        return;
    }
    for (int i = 0; i < table->capacity; i++) {
        HashEntry* entry = &table->entries[i];
        if (entry->state == HASH_OCCUPIED) saveStudentCourses(file, &entry->data);
    }
    fclose(file);
    printf("[OK] Hash table data saved to %s\n", path);
}

// menu flow to add a registration.
static AVLNode* insertRegistrationFlow(AVLNode* root) {
    char name[MAX_NAME_LEN];
    char id[MAX_ID_LEN];
    char major[MAX_MAJOR_LEN];
    char courseCode[MAX_CODE_LEN];
    char courseTitle[MAX_TITLE_LEN];
    char semester[MAX_SEMESTER_LEN];
    printf("[INFO] Type 'exit' to return to main menu.\n");
    if (!getValidatedInput("Enter Student Name: ", name, sizeof(name), isLettersSpaces, "[ERROR] Name must contain letters only.\n")) return root;
    while (1) {
        if (!getValidatedInput("Enter Student ID: ", id, sizeof(id), isDigitsOnly, "[ERROR] Student ID must contain digits only.\n")) return root;
        if (!findByID(root, id)) break;
        printf("[ERROR] Student ID already exists. Try again.\n");
    }
    if (!getValidatedInput("Enter Major: ", major, sizeof(major), isLettersSpaces, "[ERROR] Major must contain letters only.\n")) return root;
    int canceled = 0;
    int courseCount = readIntWithPromptOrExit("How many courses to add? ", 1, 20, &canceled);
    if (canceled) return root;
    int createdStudent = 0;
    for (int i = 0; i < courseCount; ) {
        printf("Course %d of %d:\n", i + 1, courseCount);
        if (!getValidatedInput("Enter Course Code: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) return root;
        if (!getValidatedInput("Enter Course Title: ", courseTitle, sizeof(courseTitle), isLettersSpaces, "[ERROR] Course title must contain letters only.\n")) return root;
        int hours = readIntWithPromptOrExit("Enter Credit Hours: ", 1, 20, &canceled);
        if (canceled) return root;
        if (!getValidatedInput("Enter Semester: ", semester, sizeof(semester), isSemesterString, "[ERROR] Semester must contain letters and numbers only.\n")) return root;
        CourseRecord* course = createCourseRecord(courseCode, courseTitle, hours, semester);
        if (!course) {
            printf("[ERROR] Unable to create course record.\n");
            continue;
        }
        int newStudent = 0;
        int newCourse = 0;
        root = insertRegistration(root, name, id, major, course, &newStudent, &newCourse);
        if (newStudent) createdStudent = 1;
        if (newCourse) {
            printf("[OK] Added course.\n");
            i++;
        } else {
            printf("[ERROR] Cannot register the same course in the same semester. Try again.\n");
        }
    }
    if (createdStudent) printf("[OK] Student created with registrations.\n");
    return root;
}

// menu flow to find and update a student by name.
static AVLNode* findStudentByNameFlow(AVLNode* root) {
    if (!root) {
        printf("[INFO] No student data available.\n");
        return root;
    }
    char query[MAX_NAME_LEN];
    NodeList list;
    while (1) {
        printf("[INFO] Type 'exit' to return to main menu.\n");
        if (!getValidatedInput("Enter Student Name: ", query, sizeof(query), isLettersSpaces, "[ERROR] Name must contain letters only.\n")) return root;
        initNodeList(&list);
        collectByName(root, query, &list);
        if (list.size == 0) {
            printf("[INFO] No student found with that name. Try again or type 'exit'.\n");
            freeNodeList(&list);
            continue;
        }
        break;
    }
    printf("Students found with this name:\n");
    for (int i = 0; i < list.size; i++) {
        AVLNode* entry = findByID(root, list.data[i]);
        if (entry) printf("%d) %s (ID %s)\n", i + 1, entry->data.name, entry->data.studentID);
        else printf("%d) <Missing> (ID %s)\n", i + 1, list.data[i]);
    }
    
    printf("Update this student's information? (1 = Yes, 2 = No): ");
    int canceled = 0;
    int update = readIntWithPromptOrExit("", 1, 2, &canceled);
    if (canceled) {
        freeNodeList(&list);
        return root;
    }
    if (update == 1) {
        char targetId[MAX_ID_LEN];
        while (1) {
            if (!getValidatedInput("Enter Student ID from the list: ", targetId, sizeof(targetId), isDigitsOnly, "[ERROR] Student ID must contain digits only.\n")) {
                freeNodeList(&list);
                return root;
            }
            int match = 0;
            for (int i = 0; i < list.size; i++) {
                if (strcmp(list.data[i], targetId) == 0) {
                    match = 1;
                    break;
                }
            }
            if (match) break;
            printf("[ERROR] Student ID not in the list. Try again.\n");
        }
        AVLNode* target = findByID(root, targetId);
        if (!target) {
            printf("[ERROR] Student record is missing.\n");
            freeNodeList(&list);
            return root;
        }
        printStudent(target);
        char buffer[MAX_MAJOR_LEN];
        char newId[MAX_ID_LEN];
        char nameCopy[MAX_NAME_LEN];
        char majorCopy[MAX_MAJOR_LEN];
        while (1) {
            printf("Enter new name (leave blank to keep current): ");
            readLine(buffer, sizeof(buffer));
            if (isExitCommand(buffer)) {
                freeNodeList(&list);
                return root;
            }
            if (strlen(buffer) == 0) break;
            if (isLettersSpaces(buffer)) {
                strncpy(target->data.name, buffer, MAX_NAME_LEN - 1);
                target->data.name[MAX_NAME_LEN - 1] = '\0';
                break;
            } else {
                printf("[ERROR] Name must contain letters only.\n");
            }
        }
        while (1) {
            printf("Enter new student ID (leave blank to keep current): ");
            readLine(newId, sizeof(newId));
            if (isExitCommand(newId)) {
                freeNodeList(&list);
                return root;
            }
            if (strlen(newId) == 0) break;
            if (!isDigitsOnly(newId)) {
                printf("[ERROR] Student ID must contain digits only.\n");
                continue;
            }
            AVLNode* existing = findByID(root, newId);
            if (existing && existing != target) {
                printf("[ERROR] Student ID already exists.\n");
                continue;
            }
            strncpy(nameCopy, target->data.name, MAX_NAME_LEN - 1);
            nameCopy[MAX_NAME_LEN - 1] = '\0';
            strncpy(majorCopy, target->data.major, MAX_MAJOR_LEN - 1);
            majorCopy[MAX_MAJOR_LEN - 1] = '\0';
            CourseRecord* cloned = cloneCourseRecords(target->data.courses);
            root = deleteStudent(root, target->data.studentID);
            CourseRecord* current = cloned;
            while (current) {
                CourseRecord* next = current->next;
                current->next = NULL;
                root = insertRegistration(root, nameCopy, newId, majorCopy, current, NULL, NULL);
                if (!root) {
                    freeCourseRecords(next);
                    freeNodeList(&list);
                    printf("[ERROR] Failed to update student ID.\n");
                    return root;
                }
                current = next;
            }
            target = findByID(root, newId);
            if (!target) {
                printf("[ERROR] Failed to update student ID.\n");
                freeNodeList(&list);
                return root;
            }
            break;
        }
        while (1) {
            printf("Enter new major (leave blank to keep current): ");
            readLine(buffer, sizeof(buffer));
            if (isExitCommand(buffer)) {
                freeNodeList(&list);
                return root;
            }
            if (strlen(buffer) == 0) break;
            if (isLettersSpaces(buffer)) {
                strncpy(target->data.major, buffer, MAX_MAJOR_LEN - 1);
                target->data.major[MAX_MAJOR_LEN - 1] = '\0';
                break;
            }
            printf("[ERROR] Major must contain letters only.\n");
        }
        printf("Update courses? (1 = Yes, 2 = No): ");
        int updateCourses = readIntWithPromptOrExit("", 1, 2, &canceled);
        if (canceled) {
            freeNodeList(&list);
            return root;
        }
        if (updateCourses == 1) {
            while (1) {
                printf("Courses menu: 1) Add/Update 2) Remove 3) Done: ");
                int action = readIntWithPromptOrExit("", 1, 3, &canceled);
                if (canceled) {
                    freeNodeList(&list);
                    return root;
                }
                if (action == 3) break;
                if (action == 1) {
                    char courseCode[MAX_CODE_LEN];
                    char courseTitle[MAX_TITLE_LEN];
                    char semester[MAX_SEMESTER_LEN];
                    if (!getValidatedInput("Enter Course Code: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) {
                        freeNodeList(&list);
                        return root;
                    }
                    if (!getValidatedInput("Enter Course Title: ", courseTitle, sizeof(courseTitle), isLettersSpaces, "[ERROR] Course title must contain letters only.\n")) {
                        freeNodeList(&list);
                        return root;
                    }
                    int hours = readIntWithPromptOrExit("Enter Credit Hours: ", 1, 20, &canceled);
                    if (canceled) {
                        freeNodeList(&list);
                        return root;
                    }
                    if (!getValidatedInput("Enter Semester: ", semester, sizeof(semester), isSemesterString, "[ERROR] Semester must contain letters and numbers only.\n")) {
                        freeNodeList(&list);
                        return root;
                    }
                    CourseRecord* course = createCourseRecord(courseCode, courseTitle, hours, semester);
                    if (!course) {
                        printf("[ERROR] Unable to create course record.\n");
                        continue;
                    }
                    int added = 0;
                    target->data.courses = appendCourseRecord(target->data.courses, course, &added);
                    if (added) {
                        printf("[OK] Course added.\n");
                    } else {
                        printf("[ERROR] Cannot register the same course in the same semester.\n");
                    }
                } else if (action == 2) {
                    char courseCode[MAX_CODE_LEN];
                    char semester[MAX_SEMESTER_LEN];
                    if (!getValidatedInput("Enter Course Code to remove: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) {
                        freeNodeList(&list);
                        return root;
                    }
                    if (!getValidatedInput("Enter Semester: ", semester, sizeof(semester), isSemesterString, "[ERROR] Semester must contain letters and numbers only.\n")) {
                        freeNodeList(&list);
                        return root;
                    }
                    int removed = 0;
                    target->data.courses = removeCourseRecord(target->data.courses, courseCode, semester, &removed);
                    if (removed) {
                        printf("[OK] Course removed.\n");
                        if (!target->data.courses) {
                            printf("[INFO] Student has no courses left.\n");
                            root = deleteStudent(root, target->data.studentID);
                            freeNodeList(&list);
                            return root;
                        }
                    } else {
                        printf("[ERROR] Course not found for this student.\n");
                    }
                }
            }
        }
        printf("[OK] Student record updated.\n");
    }
    freeNodeList(&list);
    return root;
}

// menu flow to list students in a course.
static void listCourseFlow(AVLNode* root) {
    if (!root) {
        printf("[INFO] No student data available.\n");
        return;
    }
    char courseCode[MAX_CODE_LEN];
    printf("[INFO] Type 'exit' to return to main menu.\n");
    if (!getValidatedInput("Enter Course Code: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) return;
    int count = 0;
    listStudentsByCourse(root, courseCode, &count);
    if (count == 0) printf("[INFO] No students registered in %s.\n", courseCode);
}

// menu flow to delete a registration.
static AVLNode* deleteRegistrationFlow(AVLNode* root) {
    if (!root) {
        printf("[INFO] No student data available.\n");
        return root;
    }
    char id[MAX_ID_LEN];
    printf("[INFO] Type 'exit' to return to main menu.\n");
    if (!getValidatedInput("Enter Student ID to modify: ", id, sizeof(id), isDigitsOnly, "[ERROR] Student ID must contain digits only.\n")) return root;
    AVLNode* student = findByID(root, id);
    if (!student) {
        printf("[ERROR] Student not found.\n");
        return root;
    }
    char courseCode[MAX_CODE_LEN];
    char semester[MAX_SEMESTER_LEN];
    if (!getValidatedInput("Enter Course Code to remove: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) return root;
    if (!getValidatedInput("Enter Semester: ", semester, sizeof(semester), isSemesterString, "[ERROR] Semester must contain letters and numbers only.\n")) return root;
    int removed = 0;
    student->data.courses = removeCourseRecord(student->data.courses, courseCode, semester, &removed);
    if (!removed) {
        printf("[ERROR] Course not found for this student.\n");
        return root;
    }
    printf("[OK] Course removed.\n");
    if (!student->data.courses) {
        root = deleteStudent(root, id);
        printf("[OK] Student removed because no courses remain.\n");
    }
    return root;
}

// menu flow to add to the hash table.
static void insertIntoHashFlow(HashTable* table) {
    if (!table->entries) {
        printf("[ERROR] Build the hash table first.\n");
        return;
    }
    char name[MAX_NAME_LEN];
    char id[MAX_ID_LEN];
    char major[MAX_MAJOR_LEN];
    char courseCode[MAX_CODE_LEN];
    char courseTitle[MAX_TITLE_LEN];
    char semester[MAX_SEMESTER_LEN];
    printf("[INFO] Type 'exit' to return to main menu.\n");
    while (1) {
        if (!getValidatedInput("Enter Student Name: ", name, sizeof(name), isLettersSpaces, "[ERROR] Name must contain letters only.\n")) return;
        if (hashSearch(table, name) < 0) break;
        printf("[ERROR] Student name already exists in hash table. Try again.\n");
    }
    while (1) {
        if (!getValidatedInput("Enter Student ID: ", id, sizeof(id), isDigitsOnly, "[ERROR] Student ID must contain digits only.\n")) return;
        if (!hashIdExists(table, id)) break;
        printf("[ERROR] Student ID already exists in hash table. Try again.\n");
    }
    if (!getValidatedInput("Enter Major: ", major, sizeof(major), isLettersSpaces, "[ERROR] Major must contain letters only.\n")) return;
    int canceled = 0;
    int courseCount = readIntWithPromptOrExit("How many courses to add? ", 1, 20, &canceled);
    if (canceled) return;
    int addedAny = 0;
    for (int i = 0; i < courseCount; ) {
        printf("Course %d of %d:\n", i + 1, courseCount);
        if (!getValidatedInput("Enter Course Code: ", courseCode, sizeof(courseCode), isCourseCode, "[ERROR] Course code must be alphanumeric with no spaces.\n")) return;
        if (!getValidatedInput("Enter Course Title: ", courseTitle, sizeof(courseTitle), isLettersSpaces, "[ERROR] Course title must contain letters only.\n")) return;
        int hours = readIntWithPromptOrExit("Enter Credit Hours: ", 1, 20, &canceled);
        if (canceled) return;
        if (!getValidatedInput("Enter Semester: ", semester, sizeof(semester), isSemesterString, "[ERROR] Semester must contain letters and numbers only.\n")) return;
        CourseRecord* course = createCourseRecord(courseCode, courseTitle, hours, semester);
        if (!course) {
            printf("[ERROR] Unable to create course record.\n");
            continue;
        }
        if (hashInsert(table, name, id, major, course)) {
            addedAny = 1;
            printf("[OK] Added course.\n");
            i++;
        } else {
            free(course);
            printf("[ERROR] Unable to insert into hash table. Try again.\n");
        }
    }
    if (addedAny) printf("[OK] Student inserted/updated in hash table.\n");
}

// menu flow to search the hash table.
static void searchHashFlow(HashTable* table) {
    if (!table->entries) {
        printf("[ERROR] Hash table not initialized.\n");
        return;
    }
    char name[MAX_NAME_LEN];
    printf("[INFO] Type 'exit' to return to main menu.\n");
    if (!getValidatedInput("Enter Student Name to search: ", name, sizeof(name), isLettersSpaces, "[ERROR] Name must contain letters only.\n")) return;
    int* indices = (int*)malloc(sizeof(int) * table->capacity);
    if (!indices) {
        printf("[ERROR] Unable to allocate memory.\n");
        return;
    }
    int matchCount = collectHashMatches(table, name, indices, table->capacity);
    if (matchCount == 0) {
        printf("[INFO] Name not found in hash table.\n");
        free(indices);
        return;
    }
    printf("Matches found:\n");
    for (int i = 0; i < matchCount; i++) {
        HashEntry* entry = &table->entries[indices[i]];
        printf("%d) %s (ID %s)\n", i + 1, entry->data.name, entry->data.studentID);
    }
    if (matchCount == 1) {
        HashEntry* entry = &table->entries[indices[0]];
        printf("Selected:\n");
        printStudentRecord(&entry->data);
        free(indices);
        return;
    }
    int canceled = 0;
    int choice = readIntWithPromptOrExit("Select student number: ", 1, matchCount, &canceled);
    if (canceled) {
        free(indices);
        return;
    }
    HashEntry* entry = &table->entries[indices[choice - 1]];
    printStudentRecord(&entry->data);
    free(indices);
}

// menu flow to delete from the hash table.
static void deleteHashFlow(HashTable* table) {
    if (!table->entries) {
        printf("[ERROR] Hash table not initialized.\n");
        return;
    }
    char name[MAX_NAME_LEN];
    while (1) {
        printf("[INFO] Type 'exit' to return to main menu.\n");
        if (!getValidatedInput("Enter Student Name to delete from hash table: ", name, sizeof(name), isLettersSpaces, "[ERROR] Name must contain letters only.\n")) return;
        int* indices = (int*)malloc(sizeof(int) * table->capacity);
        if (!indices) {
            printf("[ERROR] Unable to allocate memory.\n");
            return;
        }
        int matchCount = collectHashMatches(table, name, indices, table->capacity);
        if (matchCount == 0) {
            printf("[ERROR] Name not found in hash table. Try again or type 'exit'.\n");
            free(indices);
            continue;
        }
        printf("Matches found:\n");
        for (int i = 0; i < matchCount; i++) {
            HashEntry* entry = &table->entries[indices[i]];
            printf("%d) %s (ID %s)\n", i + 1, entry->data.name, entry->data.studentID);
        }
        int canceled = 0;
        int choice = 1;
        if (matchCount > 1) {
            choice = readIntWithPromptOrExit("Select student number: ", 1, matchCount, &canceled);
            if (canceled) {
                free(indices);
                return;
            }
        }
        if (hashDeleteAt(table, indices[choice - 1])) {
            printf("[OK] Entry removed from hash table.\n");
            free(indices);
            return;
        }
        free(indices);
        printf("[ERROR] Unable to delete entry. Try again.\n");
    }
}

// print the hash function description.
static void printHashInfo(void) {
    printf("Hash Function: h(s) = ((h << 5) + c) mod table_size, case-insensitive.\n");
}

int main(int argc, char* argv[]) {
    initBaseDir(argc > 0 ? argv[0] : NULL);
    AVLNode* root = NULL;
    HashTable table;
    table.entries = NULL;
    table.capacity = 0;
    table.size = 0;
    printf("[INFO] reg.txt loaded.\n");
    root = loadFromFile("reg.txt", root);
    int running = 1;
    while (running) {
        printf("\nCOURSE REGISTRATION MANAGEMENT SYSTEM\n");
        printf("1. Insert new registration\n");
        printf("2. Find student by name and update\n");
        printf("3. List students registered in a course\n");
        printf("4. Delete a student's registration\n");
        printf("5. Build hash table and export students_hash.data\n");
        printf("6. Print hash table\n");
        printf("7. Print hash table size\n");
        printf("8. Show hash function description\n");
        printf("9. Insert student into hash table\n");
        printf("10. Search student in hash table\n");
        printf("11. Delete student from hash table\n");
        printf("12. Save hash table back to reg.txt\n");
        printf("13. Exit\n");
        int choice = readIntWithPrompt("Select option: ", 1, 13);
        switch (choice) {
            case 1:
                root = insertRegistrationFlow(root);
                break;
            case 2:
                root = findStudentByNameFlow(root);
                break;
            case 3:
                listCourseFlow(root);
                break;
            case 4:
                root = deleteRegistrationFlow(root);
                break;
            case 5:
                buildHashTable(&table, root);
                if (root) {
                    int clearChoice = readIntWithPrompt("Clear AVL? (1 = Yes, 2 = No): ", 1, 2);
                    if (clearChoice == 1) {
                        freeTree(root);
                        root = NULL;
                        printf("[INFO] AVL tree cleared.\n");
                    }
                }
                break;
            case 6:
                printHashTable(&table);
                break;
            case 7:
                if (table.entries) printf("Hash table size: %d entries, %d capacity.\n", table.size, table.capacity);
                else printf("[INFO] Hash table not built yet.\n");
                break;
            case 8:
                printHashInfo();
                break;
            case 9:
                insertIntoHashFlow(&table);
                break;
            case 10:
                searchHashFlow(&table);
                break;
            case 11:
                deleteHashFlow(&table);
                break;
            case 12:
                saveHashTableToFile(&table, "reg.txt");
                running = 0;
                break;
            case 13:
                running = 0;
                break;
        }
    }
    if (root) saveTreeToFile("reg.txt", root);
    freeTree(root);
    freeHashTable(&table);
    printf("[INFO] Goodbye.\n");
    return 0;
}

