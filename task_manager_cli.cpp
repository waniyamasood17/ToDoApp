#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
using namespace std;

const int MAX_SIZE = 100;

// Forward declarations
class Stack;  // Tell compiler that Stack class will be defined later

// ------------------ Simple Date Class ------------------
class Date {
public:
    int year;
    int month;
    int day;

    // Constructor
    Date(string dateString) {
        // Convert "YYYY-MM-DD" to numbers
        sscanf(dateString.c_str(), "%d-%d-%d", &year, &month, &day);
    }

    // Convert date to string (YYYY-MM-DD format)
    string toString() {
        char buffer[11];  // Space for YYYY-MM-DD + null terminator
        sprintf(buffer, "%04d-%02d-%02d", year, month, day);
        return string(buffer);
    }

    // Add days to date
    void addDays(int days) {
        day += days;

        if (day > 30) {
            month++;
            day -= 30;
        }

        if (month > 12) {
            year++;
            month = 1;
        }
    }

    // Compare dates
    bool isLessThan(Date other) {
        if (year != other.year) {
            return year < other.year;        //Returns true if the current date is earlier than other
        }

        if (month != other.month) {
            return month < other.month;
        }

        return day < other.day;
    }
};

// ------------------ Task ------------------
class Task {
public:
    int id;
    string title;
    string description;
    int priority;
    string deadline;
    bool completed;

    Task(int i = 0, string t = "", string d = "", int p = 5, string dl = "", bool c = false) {
        id = i;
        title = t;
        description = d;
        priority = p;
        deadline = dl;
        completed = c;
    }

    void display() {
        cout << "ID: " << id << endl;
        cout << "Title: " << title << endl;
        cout << "Description: " << description << endl;
        cout << "Priority: " << priority << endl;
        cout << "Deadline: " << deadline << endl;
        cout << "Status: " << (completed ? "Completed" : "Pending") << endl;
        cout << "------------------------" << endl;
    }
};

// ------------------ Node ------------------
class Node {
public:
    Task task;
    Node* next;
    Node* prev;

    Node(Task t) {
        task = t;
        next = nullptr;
        prev = nullptr;
    }
};

// ------------------ UndoAction ------------------
class UndoAction {
public:
    string type;
    Task before;
    Task after;
};

// ------------------ Stack ------------------
class Stack {
private:
    UndoAction stack[MAX_SIZE];
    int top;

public:
    Stack() {
        top = -1;
    }

    int isFull() {
        return top == MAX_SIZE - 1;
    }

    int isEmpty() {
        return top == -1;
    }

    void push(UndoAction value) {
        if (isFull()) {
            cout << "Stack overflow" << endl;
        }
        else {
            top++;
            stack[top] = value;
        }
    }

    UndoAction pop() {
        if (isEmpty()) {
            cout << "Stack underflow" << endl;
            return UndoAction();
        }
        else {
            return stack[top--];
        }
    }
};

// ------------------ Global Variables ------------------
Stack undoActions;
Stack redoActions;

// ------------------ MinHeap ------------------
class MinHeap {
private:
    Task heap[MAX_SIZE];
    int size;

    int parent(int i) {
        return (i - 1) / 2;
    }

    int left(int i) {
        return 2 * i + 1;
    }

    int right(int i) {
        return 2 * i + 2;
    }

    void shiftUp(int i) {        //new element inserted at the bottom
        while (i > 0 && heap[parent(i)].priority > heap[i].priority) {
            swap(heap[i], heap[parent(i)]);
            i = parent(i);
        }
    }

    void shiftDown(int i) {        //After deleting the root
        int smallest = i;
        int l = left(i);
        int r = right(i);

        if (l < size && heap[l].priority < heap[smallest].priority) {
            smallest = l;
        }

        if (r < size && heap[r].priority < heap[smallest].priority) {
            smallest = r;
        }

        if (smallest != i) {
            swap(heap[i], heap[smallest]);
            shiftDown(smallest);
        }
    }

public:
    MinHeap() {
        size = 0;
    }

    void insert(Task task) {
        if (size >= MAX_SIZE) {
            cout << "Heap is full!" << endl;
            return;
        }
        heap[size] = task;
        shiftUp(size);
        size++;
    }

    Task extractMin() {
        if (size == 0) {
            cout << "Heap is empty!" << endl;
            return Task();
        }

        Task root = heap[0];
        heap[0] = heap[size - 1];
        size--;
        shiftDown(0);
        return root;
    }

    bool empty() {
        return size == 0;
    }

    void display() {
        MinHeap tempHeap = *this;  //Create a copy of current heap
        while (!tempHeap.empty()) {
            Task task = tempHeap.extractMin();
            cout << "- " << task.title << " (Priority: " << task.priority << ")" << endl;
        }
    }

    void updatePriority(string title, int newPriority) {
        int taskIndex = -1;
        for (int i = 0; i < size; i++) {
            if (heap[i].title == title) {
                taskIndex = i;
                break;
            }
        }

        if (taskIndex != -1) {
            int oldPriority = heap[taskIndex].priority;
            heap[taskIndex].priority = newPriority;

            // If new priority is less than old priority, shift up
            if (newPriority < oldPriority) {                //Decrease priority (higher importance)
                shiftUp(taskIndex);
            }
            // If new priority is more than old priority, shift down
            else if (newPriority > oldPriority) {            //Increase priority (lower importance)
                shiftDown(taskIndex);
            }
        }
    }
};

// ------------------ TaskList ------------------
class TaskList {
private:
    Node* head;
    Node* tail;
    int nextId;

public:
    TaskList() {
        head = nullptr;
        tail = nullptr;
        nextId = 1;
    }

    void addTask(MinHeap& heap) {
        string title, description, deadline;
        int priority;

        cin.ignore();
        cout << "Enter task title: ";
        getline(cin, title);

        cout << "Enter task description: ";
        getline(cin, description);

        cout << "Enter deadline (YYYY-MM-DD): ";
        getline(cin, deadline);

        cout << "Enter priority (1-10): ";
        cin >> priority;

        Task newTask(nextId, title, description, priority, deadline);
        insert(newTask);
        heap.insert(newTask);

        UndoAction action;
        action.type = "add";
        action.after = newTask;        //Save the task that was just added so we can undo it later
        undoActions.push(action);      //Push this action to the undo stack... most recent action on top

        nextId++;
    }

    void insert(Task task) {
        Node* newNode = new Node(task);

        if (head == nullptr) {
            head = newNode;
            tail = newNode;
        }
        else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
    }

    void removeById(int id) {
        Node* current = head;

        while (current != nullptr) {
            if (current->task.id == id) {
                if (current->prev != nullptr) {
                    current->prev->next = current->next;
                }
                else {
                    head = current->next;
                }

                if (current->next != nullptr) {
                    current->next->prev = current->prev;
                }
                else {
                    tail = current->prev;
                }

                delete current;
                return;
            }
            current = current->next;
        }
    }

    void markCompleted(int id) {
        Node* current = head;

        while (current != nullptr) {
            if (current->task.id == id) {
                current->task.completed = true;
                cout << "Task marked as completed!" << endl;
                return;
            }
            current = current->next;
        }

        cout << "Task not found!" << endl;
    }

    void viewCompletedTasks() {
        Node* current = head;
        cout << "\n=== Completed Tasks ===" << endl;

        while (current != nullptr) {
            if (current->task.completed) {
                current->task.display();
            }
            current = current->next;
        }
    }

    void viewPendingTasks() {
        Node* current = head;
        cout << "\n=== Pending Tasks ===" << endl;

        while (current != nullptr) {
            if (!current->task.completed) {
                current->task.display();
            }
            current = current->next;
        }
    }

    void searchByTitle(string title) {
        Node* current = head;

        while (current != nullptr) {
            if (current->task.title == title) {
                current->task.display();
                return;
            }
            current = current->next;
        }

        cout << "Task not found." << endl;
    }

    void editTask(int id, string newTitle, string newDeadline) {
        Node* current = head;

        while (current != nullptr) {
            if (current->task.id == id) {
                Task before = current->task;
                current->task.title = newTitle;
                current->task.deadline = newDeadline;
                UndoAction action = {"edit", before, current->task};
                undoActions.push(action);
                cout << "Task updated successfully!" << endl;
                return;
            }
            current = current->next;
        }

        cout << "Task not found." << endl;
    }

    void sortByDeadline() {
        for (Node* i = head; i != nullptr; i = i->next) {
            Node* minNode = i;

            for (Node* j = i->next; j != nullptr; j = j->next) {
                Date date1(j->task.deadline);
                Date date2(minNode->task.deadline);

                if (date1.isLessThan(date2)) {
                    minNode = j;
                }
            }

            if (minNode != i) {
                swap(i->task, minNode->task);
            }
        }

        cout << "Tasks sorted by deadline!" << endl;
    }

    void printAll() {
        Node* current = head;

        if (current == nullptr) {
            cout << "No tasks found." << endl;
            return;
        }

        cout << "\n=== All Tasks ===" << endl;

        while (current != nullptr) {
            current->task.display();
            current = current->next;
        }
    }

    Node* getHead() {
        return head;
    }
};

// ------------------ Features ------------------
void performUndo(TaskList& list, MinHeap& heap) {
    if (undoActions.isEmpty()) {
        cout << "Nothing to undo." << endl;
        return;
    }

    UndoAction action = undoActions.pop();

    if (action.type == "add") {
        list.removeById(action.after.id);
        // Remove from heap
        heap = MinHeap(); // Reset heap
        Node* current = list.getHead();
        while (current != nullptr) {
            if (!current->task.completed) {        //Rebuild the heap by inserting all uncompleted tasks from the updated task list
                heap.insert(current->task);
            }
            current = current->next;
        }
    }
    else {
        list.editTask(action.after.id, action.before.title, action.before.deadline);
        // Update heap
        heap = MinHeap(); // Reset heap
        Node* current = list.getHead();
        while (current != nullptr) {
            if (!current->task.completed) {
                heap.insert(current->task);
            }
            current = current->next;
        }
    }

    redoActions.push(action);
}

void performRedo(TaskList& list, MinHeap& heap) {
    if (redoActions.isEmpty()) {
        cout << "Nothing to redo." << endl;
        return;
    }

    UndoAction action = redoActions.pop();

    if (action.type == "add") {
        list.insert(action.after);
        if (!action.after.completed) {
            heap.insert(action.after);
        }
    }
    else {
        list.editTask(action.before.id, action.after.title, action.after.deadline);
        // Update heap
        heap = MinHeap(); // Reset heap
        Node* current = list.getHead();
        while (current != nullptr) {
            if (!current->task.completed) {
                heap.insert(current->task);
            }
            current = current->next;
        }
    }

    undoActions.push(action);
}

void viewTopNPriorityTasks(TaskList& list, int N) {
    MinHeap temp;
    Node* current = list.getHead();

    while (current != nullptr) {
        if (!current->task.completed) {
            temp.insert(current->task);
        }
        current = current->next;
    }

    cout << "\n=== Top " << N << " Priority Tasks ===" << endl;

    for (int i = 0; i < N && !temp.empty(); i++) {
        temp.extractMin().display();
    }
}

void moveTaskToTomorrow(TaskList& list, string title) {
    Node* current = list.getHead();

    while (current != nullptr) {
        if (current->task.title == title) {
            Date date(current->task.deadline);
            date.addDays(1);
            current->task.deadline = date.toString();
            cout << "Task moved to tomorrow successfully!" << endl;
            return;
        }
        current = current->next;
    }

    cout << "Task not found." << endl;
}

void searchByDeadline(TaskList& list, string date) {
    Node* current = list.getHead();
    bool found = false;

    cout << "\n=== Tasks Due on " << date << " ===" << endl;

    while (current != nullptr) {
        if (current->task.deadline == date) {
            current->task.display();
            found = true;
        }
        current = current->next;
    }

    if (!found) {
        cout << "No tasks found for this date." << endl;
    }
}

// ------------------ Main ------------------
int main() {
    // Initialize the undo/redo stacks
    undoActions = Stack();
    redoActions = Stack();

    TaskList list;
    MinHeap heap;
    int choice;

    // Declare all variables needed in switch cases here
    int id, editId, n, newPriority;
    string searchTitle, searchDate, newTitle, newDeadline, taskTitle, moveTitle;

    do {
        cout << "\n====== TASK MANAGER ======" << endl;
        cout << "1. Add Task" << endl;
        cout << "2. View All Tasks" << endl;
        cout << "3. View Pending Tasks" << endl;
        cout << "4. View Completed Tasks" << endl;
        cout << "5. Mark Task as Completed" << endl;
        cout << "6. Search by Title" << endl;
        cout << "7. Search by Deadline" << endl;
        cout << "8. Sort by Deadline" << endl;
        cout << "9. Edit Task" << endl;
        cout << "10. Undo" << endl;
        cout << "11. Redo" << endl;
        cout << "12. View Top N Priority Tasks" << endl;
        cout << "13. Update Priority" << endl;
        cout << "14. Move Task to Tomorrow" << endl;
        cout << "15. Show Priority Queue" << endl;
        cout << "16. Exit" << endl;
        cout << "Enter your choice: ";

        cin >> choice;

        switch (choice) {

            case 1:
                list.addTask(heap);
                break;

            case 2:
                list.printAll();
                break;

            case 3:
                list.viewPendingTasks();
                break;

            case 4:
                list.viewCompletedTasks();
                break;

            case 5:
                cout << "Enter task ID to mark as completed: ";
                cin >> id;
                list.markCompleted(id);
                break;

            case 6:
                cout << "Enter title to search: ";
                cin.ignore();
                getline(cin, searchTitle);
                list.searchByTitle(searchTitle);
                break;

            case 7:
                cout << "Enter deadline (YYYY-MM-DD): ";
                cin.ignore();
                getline(cin, searchDate);
                searchByDeadline(list, searchDate);
                break;

            case 8:
                list.sortByDeadline();
                break;

            case 9:
                cout << "Enter task ID to edit: ";
                cin >> editId;
                cout << "Enter new title: ";
                cin.ignore();
                getline(cin, newTitle);
                cout << "Enter new deadline (YYYY-MM-DD): ";
                getline(cin, newDeadline);
                list.editTask(editId, newTitle, newDeadline);
                break;

            case 10:
                performUndo(list, heap);
                break;

            case 11:
                performRedo(list, heap);
                break;

            case 12:
                cout << "Enter number of tasks to view: ";
                cin >> n;
                viewTopNPriorityTasks(list, n);
                break;

            case 13:
                cout << "Enter task title: ";
                cin.ignore();
                getline(cin, taskTitle);
                cout << "Enter new priority (1-10): ";
                cin >> newPriority;
                // Update priority in both TaskList and MinHeap
                {
                    Node* current = list.getHead();
                    while (current != nullptr) {
                        if (current->task.title == taskTitle) {
                            current->task.priority = newPriority;
                            break;
                        }
                        current = current->next;
                    }
                    heap = MinHeap(); // Reset heap
                    current = list.getHead();
                    while (current != nullptr) {
                        if (!current->task.completed) {
                            heap.insert(current->task);
                        }
                        current = current->next;
                    }
                }
                break;

            case 14:
                cout << "Enter task title to move: ";
                cin.ignore();
                getline(cin, moveTitle);
                moveTaskToTomorrow(list, moveTitle);
                break;

            case 15:
                cout << "\n=== Priority Queue ===" << endl;
                heap.display();
                break;

			case 16:
                cout << "Thank you for using Task Manager!" << endl;
                break;

            default:
                cin.clear();  // Clear error state
	            cin.ignore(1000, '\n');  // Discard invalid input
	            cout << "Invalid choice! Please try again." << endl;
	            continue;
        }

    } while (choice != 16);

    return 0;
}

