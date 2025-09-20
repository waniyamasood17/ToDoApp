#pragma once
#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

using namespace std;

const int MAX_SIZE = 100;

// Forward declarations
class Stack;

// ------------------ Simple Date Class ------------------
class Date {
public:
    int year;
    int month;
    int day;

    Date(string dateString) {
        sscanf(dateString.c_str(), "%d-%d-%d", &year, &month, &day);
    }

    string toString() {
        char buffer[11];
        sprintf(buffer, "%04d-%02d-%02d", year, month, day);
        return string(buffer);
    }

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

    bool isLessThan(Date other) {           //checks if the current object (this) is earlier than other.
        if (year != other.year)
            return year < other.year;
        if (month != other.month)
            return month < other.month;
        return day < other.day;
    }
};

// ------------------ Task ------------------
struct Task {
    int id;
    std::string title;
    std::string description;
    std::string deadline;
    int priority;
    bool completed;

    // Default constructor
    Task() : id(0), title(""), description(""), deadline(""), priority(1), completed(false) {}

    // Parameterized constructor
    Task(int _id, const std::string& _title, const std::string& _desc,
         const std::string& _deadline, int _priority)
        : id(_id), title(_title), description(_desc), 
          deadline(_deadline), priority(_priority), completed(false) {}

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

    Node(Task t) : task(t), next(nullptr), prev(nullptr) {}
};

// ------------------ UndoAction ------------------
class UndoAction {
public:
    string type;
    Task before;
    Task after;
    
    // Default constructor
    UndoAction() : type(""), before(), after() {}
    
    // Parameterized constructor
    UndoAction(const string& t, const Task& b, const Task& a) 
        : type(t), before(b), after(a) {}
};

// ------------------ Stack ------------------
class Stack {
private:
    UndoAction stack[MAX_SIZE];
    int top;

public:
    Stack() : top(-1) {}

    bool isFull() {
        return top == MAX_SIZE - 1;
    }

    bool isEmpty() {
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

// ------------------ MinHeap ------------------
class MinHeap {
private:
    Task heap[MAX_SIZE];
    int size;

    int parent(int i) { return (i - 1) / 2; }
    int left(int i) { return 2 * i + 1; }
    int right(int i) { return 2 * i + 2; }

    void shiftUp(int i) {
        while (i > 0 && heap[parent(i)].priority > heap[i].priority) {
            swap(heap[i], heap[parent(i)]);
            i = parent(i);
        }
    }

    void shiftDown(int i) {
        int smallest = i;
        int l = left(i);
        int r = right(i);

        if (l < size && heap[l].priority < heap[smallest].priority)
            smallest = l;

        if (r < size && heap[r].priority < heap[smallest].priority)
            smallest = r;

        if (smallest != i) {
            swap(heap[i], heap[smallest]);
            shiftDown(smallest);
        }
    }

public:
    MinHeap() : size(0) {}

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
        MinHeap tempHeap = *this;
        while (!tempHeap.empty()) {
            Task task = tempHeap.extractMin();
            cout << "- " << task.title << " (Priority: " << task.priority << ")" << endl;
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
    TaskList() : head(nullptr), tail(nullptr), nextId(1) {}

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

    int getNextId() { return nextId++; }

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

    void sortByDeadline() {
        vector<Task> tasks;
        Node* current = head;
        while (current != nullptr) {
            tasks.push_back(current->task);
            current = current->next;
        }

        for (size_t i = 0; i < tasks.size(); i++) {
            for (size_t j = i + 1; j < tasks.size(); j++) {
                Date date1(tasks[i].deadline);
                Date date2(tasks[j].deadline);
                if (date2.isLessThan(date1)) {
                    swap(tasks[i], tasks[j]);
                }
            }
        }

        // Rebuild the list
        current = head;
        size_t index = 0;
        while (current != nullptr && index < tasks.size()) {
            current->task = tasks[index++];
            current = current->next;
        }
    }

    vector<Task> getAllTasks() {
        vector<Task> tasks;
        Node* current = head;
        while (current != nullptr) {
            tasks.push_back(current->task);
            current = current->next;
        }
        return tasks;
    }

    vector<Task> getPendingTasks() {
        vector<Task> tasks;
        Node* current = head;
        while (current != nullptr) {
            if (!current->task.completed) {
                tasks.push_back(current->task);
            }
            current = current->next;
        }
        return tasks;
    }

    vector<Task> getCompletedTasks() {
        vector<Task> tasks;
        Node* current = head;
        while (current != nullptr) {
            if (current->task.completed) {
                tasks.push_back(current->task);
            }
            current = current->next;
        }
        return tasks;
    }

    void editTask(int id, const string& newTitle, const string& newDeadline) {
        Node* current = head;
        while (current != nullptr) {
            if (current->task.id == id) {
                current->task.title = newTitle;
                current->task.deadline = newDeadline;
                cout << "Task updated successfully!" << endl;
                return;
            }
            current = current->next;
        }
        cout << "Task not found." << endl;
    }

    Node* getHead() { return head; }
};

// ------------------ TaskManager ------------------
class TaskManager {
private:
    std::vector<Task> tasks;
    int nextId = 1;
    TaskList list;
    MinHeap heap;
    Stack undoActions;
    Stack redoActions;

    // Helper method that rebuilds the TaskList and MinHeap
    // so they match the current state of the tasks vector after sorting or changes
    void syncDataStructures() {
        // Clear and rebuild TaskList
        list = TaskList();
        for (const auto& task : tasks) {
            Task taskCopy = task;
            taskCopy.id = task.id;
            list.insert(taskCopy);
        }
        
        // Clear and rebuild MinHeap with pending tasks only
        heap = MinHeap();
        for (const auto& task : tasks) {
            if (!task.completed) {
                heap.insert(task);
            }
        }
    }

public:
    TaskManager() {}

    int addTask(const std::string& title, const std::string& desc, 
                const std::string& deadline, int priority) {
        if (title.empty()) {
            throw std::invalid_argument("Title cannot be empty");
        }
        if (priority < 1 || priority > 10) {
            throw std::invalid_argument("Priority must be between 1 and 10");
        }
        
        Task newTask(nextId, title, desc, deadline, priority);
        
        // Store for undo
        UndoAction action("add", Task(), newTask);
        undoActions.push(action);
        
        // Add to main storage
        tasks.push_back(newTask);
        
        // Sync all data structures
        syncDataStructures();
        
        return nextId++;
    }

    void deleteTask(int id) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });
        if (it != tasks.end()) {
            // Store for undo
            UndoAction action("delete", *it, Task());
            undoActions.push(action);
            
            tasks.erase(it);
            syncDataStructures();
        }
    }

    void updateTask(int id, const std::string& title, const std::string& desc,
                   const std::string& deadline, int priority) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });
        if (it != tasks.end()) {
            // Store original for undo
            Task beforeTask = *it;
            
            // Update task
            it->title = title;
            it->description = desc;
            it->deadline = deadline;
            it->priority = priority;
            
            // Store for undo
            UndoAction action("edit", beforeTask, *it);
            undoActions.push(action);
            
            // Sync all data structures
            syncDataStructures();
        }
    }

    void markTaskCompleted(int id) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });
        if (it != tasks.end()) {
            Task beforeTask = *it;
            it->completed = true;
            
            UndoAction action("edit", beforeTask, *it);
            undoActions.push(action);
            
            syncDataStructures();
        }
    }

    std::vector<Task> getAllTasks() const {
        return tasks;
    }

    std::vector<Task> getPendingTasks() const {
        std::vector<Task> pending;
        std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(pending),
            [](const Task& task) { return !task.completed; });
        return pending;
    }

    std::vector<Task> getCompletedTasks() const {
        std::vector<Task> completed;
        std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(completed),
            [](const Task& task) { return task.completed; });
        return completed;
    }

    std::vector<Task> searchTasks(const std::string& query) const {
        std::vector<Task> results;
        std::copy_if(tasks.begin(), tasks.end(), std::back_inserter(results),
            [&query](const Task& task) {
                return task.title.find(query) != std::string::npos ||
                       task.description.find(query) != std::string::npos;       //no position if no match is found
            });
        return results;
    }

    std::vector<Task> getTasksByPriority() const {
        std::vector<Task> prioritized = tasks;
        std::sort(prioritized.begin(), prioritized.end(),
            [](const Task& a, const Task& b) {
                return a.priority < b.priority;
            });
        return prioritized;
    }

    Task* getTask(int id) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });      //auto it automatically deduces the iterator type
        return it != tasks.end() ? &(*it) : nullptr;        //*it: Dereferences the iterator to get the actual Task object
                                                            //&(*it): Gets the address of the found task — returns a pointer (Task*)
    }

    void sortByDeadline() {
        std::sort(tasks.begin(), tasks.end(),
            [](const Task& a, const Task& b) {
                Date dateA(a.deadline);
                Date dateB(b.deadline);
                return dateA.isLessThan(dateB);
            });
        syncDataStructures();
    }

    void editTask(int id, const string& newTitle, const string& newDeadline) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });
        if (it != tasks.end()) {
            Task beforeTask = *it;
            
            it->title = newTitle;
            it->deadline = newDeadline;
            
            UndoAction action("edit", beforeTask, *it);
            undoActions.push(action);
            
            syncDataStructures();
        }
    }

    void updatePriority(int id, int newPriority) {
        auto it = std::find_if(tasks.begin(), tasks.end(),
            [id](const Task& task) { return task.id == id; });
        if (it != tasks.end()) {
            Task beforeTask = *it;
            
            it->priority = newPriority;
            
            UndoAction action("edit", beforeTask, *it);
            undoActions.push(action);
            
            syncDataStructures();
        }
    }

    void undo() {
        if (!undoActions.isEmpty()) {
            UndoAction action = undoActions.pop();
            
            if (action.type == "add") {
                // Remove the added task
                auto it = std::find_if(tasks.begin(), tasks.end(),
                    [&action](const Task& task) { return task.id == action.after.id; });
                if (it != tasks.end()) {
                    tasks.erase(it);
                }
            }
            else if (action.type == "edit") {
                // Restore the previous version
                auto it = std::find_if(tasks.begin(), tasks.end(),
                    [&action](const Task& task) { return task.id == action.before.id; });
                if (it != tasks.end()) {
                    *it = action.before;
                }
            }
            else if (action.type == "delete") {
                // Restore the deleted task
                tasks.push_back(action.before);
            }
            
            syncDataStructures();
            redoActions.push(action);
        }
    }

    void redo() {
        if (!redoActions.isEmpty()) {
            UndoAction action = redoActions.pop();
            
            if (action.type == "add") {
                // Re-add the task
                tasks.push_back(action.after);
            }
            else if (action.type == "edit") {
                // Restore the edited version
                auto it = std::find_if(tasks.begin(), tasks.end(),
                    [&action](const Task& task) { return task.id == action.after.id; });
                if (it != tasks.end()) {
                    *it = action.after;
                }
            }
            else if (action.type == "delete") {
                // Re-delete the task
                auto it = std::find_if(tasks.begin(), tasks.end(),
                    [&action](const Task& task) { return task.id == action.before.id; });
                if (it != tasks.end()) {
                    tasks.erase(it);
                }
            }
            
            syncDataStructures();
            undoActions.push(action);
        }
    }
    
    // Getter methods for data structures
    MinHeap& getHeap() { return heap; }
    TaskList& getList() { return list; }
};