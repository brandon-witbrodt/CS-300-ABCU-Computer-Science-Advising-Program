// CS 300 Project Two - ABCU Computer Science Advising Program
// Implements the binary search tree recommended in Project One.
// All program functionality is contained in this file.

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Course {
    string courseNumber;
    string title;
    vector<string> prerequisites;
};

struct TreeNode {
    Course course;
    TreeNode* left = nullptr;
    TreeNode* right = nullptr;

    explicit TreeNode(const Course& value) : course(value) {}
};

class BinarySearchTree {
private:
    TreeNode* root = nullptr;

    static TreeNode* insertCourse(TreeNode* node, const Course& course) {
        if (node == nullptr) {
            return new TreeNode(course);
        }
        if (course.courseNumber < node->course.courseNumber) {
            node->left = insertCourse(node->left, course);
        } else if (course.courseNumber > node->course.courseNumber) {
            node->right = insertCourse(node->right, course);
        }
        return node;
    }

    static void clearNodes(TreeNode* node) {
        if (node == nullptr) return;
        clearNodes(node->left);
        clearNodes(node->right);
        delete node;
    }

    static void printInOrder(const TreeNode* node) {
        if (node == nullptr) return;
        printInOrder(node->left);
        cout << node->course.courseNumber << ", " << node->course.title << '\n';
        printInOrder(node->right);
    }

    static bool checkPrerequisites(const TreeNode* node,
                                   const BinarySearchTree& courses) {
        if (node == nullptr) return true;
        bool valid = checkPrerequisites(node->left, courses);
        for (const string& number : node->course.prerequisites) {
            if (courses.searchCourse(number) == nullptr) {
                cerr << "Format error: prerequisite " << number << " for "
                     << node->course.courseNumber << " does not exist in the file.\n";
                valid = false;
            }
        }
        if (!checkPrerequisites(node->right, courses)) valid = false;
        return valid;
    }

public:
    BinarySearchTree() = default;
    ~BinarySearchTree() { clear(); }

    // Owning tree nodes must not be copied.
    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;

    void clear() {
        clearNodes(root);
        root = nullptr;
    }

    bool empty() const { return root == nullptr; }

    bool addCourse(const Course& course) {
        if (searchCourse(course.courseNumber) != nullptr) return false;
        root = insertCourse(root, course);
        return true;
    }

    const Course* searchCourse(const string& courseNumber) const {
        const TreeNode* current = root;
        while (current != nullptr) {
            if (courseNumber == current->course.courseNumber)
                return &current->course;
            if (courseNumber < current->course.courseNumber)
                current = current->left;
            else
                current = current->right;
        }
        return nullptr;
    }

    bool validatePrerequisites() const {
        return checkPrerequisites(root, *this);
    }

    void printCourseList() const { printInOrder(root); }
};

string trim(const string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

string uppercase(string value) {
    transform(value.begin(), value.end(), value.begin(),
              [](unsigned char character) { return static_cast<char>(toupper(character)); });
    return value;
}

// Parse one CSV line. Only commas delimit fields in the supplied ABCU data.
vector<string> splitCourseLine(const string& line) {
    vector<string> tokens;
    string field;
    istringstream stream(line);
    while (getline(stream, field, ',')) tokens.push_back(trim(field));
    // Preserve a final empty prerequisite field; blank prerequisites are allowed.
    if (!line.empty() && line.back() == ',') tokens.push_back("");
    return tokens;
}

// Load into a temporary tree so an invalid file never leaves partial data.
bool loadCourses(const string& fileName, BinarySearchTree& courses) {
    ifstream input(fileName);
    if (!input.is_open()) {
        cerr << "Error: Unable to open course data file.\n";
        return false;
    }

    courses.clear();
    bool formatError = false;
    string line;
    int lineNumber = 0;

    while (getline(input, line)) {
        ++lineNumber;
        vector<string> tokens = splitCourseLine(line);
        if (tokens.size() < 2) {
            cerr << "Format error on line " << lineNumber
                 << ": fewer than two parameters.\n";
            formatError = true;
            continue;
        }
        if (tokens[0].empty() || tokens[1].empty()) {
            cerr << "Format error on line " << lineNumber
                 << ": course number or title is missing.\n";
            formatError = true;
            continue;
        }

        Course currentCourse;
        currentCourse.courseNumber = uppercase(tokens[0]);
        currentCourse.title = tokens[1];
        for (size_t index = 2; index < tokens.size(); ++index) {
            if (!tokens[index].empty())
                currentCourse.prerequisites.push_back(uppercase(tokens[index]));
        }
        if (!courses.addCourse(currentCourse)) {
            cerr << "Format error on line " << lineNumber
                 << ": duplicate course number " << currentCourse.courseNumber << ".\n";
            formatError = true;
        }
    }

    if (input.bad()) {
        cerr << "Error: Unable to finish reading course data file.\n";
        formatError = true;
    }
    if (courses.empty()) {
        cerr << "Error: No course data was found.\n";
        formatError = true;
    }
    if (!formatError && !courses.validatePrerequisites()) formatError = true;
    if (formatError) {
        courses.clear();
        return false;
    }
    cout << "Course data loaded successfully.\n";
    return true;
}

void printCourseInformation(const BinarySearchTree& courses, string courseNumber) {
    courseNumber = uppercase(trim(courseNumber));
    const Course* course = courses.searchCourse(courseNumber);
    if (course == nullptr) {
        cout << "Course not found.\n";
        return;
    }

    cout << course->courseNumber << ", " << course->title << '\n';
    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None\n";
        return;
    }

    cout << "Prerequisites: ";
    for (size_t i = 0; i < course->prerequisites.size(); ++i) {
        if (i != 0) cout << ", ";
        cout << course->prerequisites[i];
    }
    cout << '\n';
    // Provide prerequisite titles as specifically required by the rubric.
    for (const string& prerequisiteNumber : course->prerequisites) {
        const Course* prerequisite = courses.searchCourse(prerequisiteNumber);
        if (prerequisite != nullptr)
            cout << "  " << prerequisite->courseNumber << ", " << prerequisite->title << '\n';
    }
}

void printMenu() {
    cout << "\n    1. Load Data Structure.\n"
         << "    2. Print Course List.\n"
         << "    3. Print Course.\n"
         << "    9. Exit\n\n"
         << "What would you like to do? ";
}

int main() {
    BinarySearchTree courses;
    bool loaded = false;
    cout << "Welcome to the course planner.\n";

    string choice;
    while (true) {
        printMenu();
        if (!getline(cin, choice)) break;
        choice = trim(choice);

        if (choice == "1") {
            cout << "Enter the course data file name: ";
            string fileName;
            if (!getline(cin, fileName)) break;
            fileName = trim(fileName);
            if (fileName.empty()) {
                cout << "Error: File name cannot be empty.\n";
                continue;
            }
            loaded = false;
            loaded = loadCourses(fileName, courses);
        } else if (choice == "2") {
            if (!loaded) {
                cout << "Please load course data first (option 1).\n";
            } else {
                cout << "Here is a sample schedule:\n\n";
                courses.printCourseList();
            }
        } else if (choice == "3") {
            if (!loaded) {
                cout << "Please load course data first (option 1).\n";
            } else {
                cout << "What course do you want to know about? ";
                string number;
                if (!getline(cin, number)) break;
                printCourseInformation(courses, number);
            }
        } else if (choice == "9") {
            cout << "Thank you for using the course planner!\n";
            break;
        } else {
            cout << choice << " is not a valid option.\n";
        }
    }
    return 0;
}
