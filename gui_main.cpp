#include "task_manager.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <functional>
#include <map>
#include <stdexcept>

using namespace sf;
using namespace std;

class TaskManagerGUI {
private:
    RenderWindow window;
    TaskManager taskManager;
    Font font;
    
    // Constants
    const unsigned int WINDOW_WIDTH = 1400;         //unsigned is integer that can only be zero or positive
    const unsigned int WINDOW_HEIGHT = 900;
    const float SIDEBAR_WIDTH = 280;
    const float HEADER_HEIGHT = 60;
    
    // Colors
    const Color BG_COLOR = Color(245, 246, 250);
    const Color PRIMARY_COLOR = Color(66, 139, 202);
    const Color SECONDARY_COLOR = Color(91, 192, 222);
    const Color SUCCESS_COLOR = Color(92, 184, 92);
    const Color DANGER_COLOR = Color(217, 83, 79);
    const Color WARNING_COLOR = Color(240, 173, 78);
    const Color TEXT_COLOR = Color(51, 51, 51);
    const Color LIGHT_TEXT = Color(119, 119, 119);
    
    // UI State
    enum class Screen {
        DASHBOARD,
        ADD_TASK,
        VIEW_TASKS,
        SEARCH_TASKS,
        SEARCH_BY_TITLE,
        SEARCH_BY_DEADLINE,
        EDIT_TASK,
        PRIORITY_TASKS,
        TOP_N_PRIORITY,
        COMPLETED_TASKS,
        PENDING_TASKS,
        PRIORITY_QUEUE,
        UPDATE_PRIORITY,
        SETTINGS
    };
    
    struct UIState {
        Screen currentScreen = Screen::DASHBOARD;
        string inputBuffer;
        bool isTyping = false;
        int selectedTaskId = -1;
        string searchQuery;
        vector<Task> displayTasks;
        float scrollOffset = 0;
        string statusMessage;
        bool showStatusMessage = false;
        
        // Add Task Form
        struct {
            string title;
            string description;
            string deadline;
            int priority = 1;
            int step = 0;
            bool showError = false;
            string errorMsg;
        } addTaskForm;
        
        // Edit Task Form
        struct {
            bool isEditing = false;
            int taskId = -1;
            string title;
            string description;
            string deadline;
            int priority = 1;
            int step = 0;
        } editForm;
        
        // Search states
        struct {
            string searchDate;
            bool isSearching = false;
        } deadlineSearch;
        
        struct {
            string searchTitle;
            bool isSearching = false;
        } titleSearch;
        
        // Priority view
        struct {
            int topN = 5;
            bool isViewing = false;
            bool isInputting = false;
        } priorityView;
        
        // Update priority
        struct {
            int taskId = -1;
            bool isUpdating = false;
        } priorityUpdate;
    } state;

    struct Button {
        RectangleShape shape;
        Text text;
        function<void()> onClick;
        bool isEnabled = true;
        Color originalColor;
        
        Button(const string& label, const Vector2f& size, const Vector2f& position,
               const Font& font, const Color& color) {
            shape.setSize(size);
            shape.setPosition(position);
            shape.setFillColor(color);
            originalColor = color;
            
            text.setFont(font);
            text.setString(label);
            text.setCharacterSize(14);
            text.setFillColor(Color::White);
            
            // Center text
            FloatRect textBounds = text.getLocalBounds();   //center the text inside another shape... like a button
            text.setPosition(
                position.x + (size.x - textBounds.width) / 2,
                position.y + (size.y - textBounds.height) / 2
            );
        }
        
        bool contains(Vector2i point) const {           //2D vector with integers
            return shape.getGlobalBounds().contains(point.x, point.y);
        }       //checks if a point (like a mouse click) is inside a shape (like a button)
        
        void draw(RenderWindow& window) const {
            window.draw(shape);
            window.draw(text);
        }
        
        void setEnabled(bool enabled) {
            isEnabled = enabled;
            shape.setFillColor(enabled ? originalColor : Color(150, 150, 150));
        }
    };
    
    vector<Button> buttons;
    map<string, function<void()>> actions;

    void initWindow() {
        VideoMode desktop = VideoMode::getDesktopMode();
        window.create(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
            "Task Manager Pro",
            Style::Default);
        
        window.setPosition(Vector2i(
            (desktop.width - WINDOW_WIDTH) / 2,
            (desktop.height - WINDOW_HEIGHT) / 2
        ));
        
        window.setFramerateLimit(60);       //limit the number of frames per second (FPS) to 60
        
        if (!font.loadFromFile("C:\\Windows\\Fonts\\segoeui.ttf")) {
            throw runtime_error("Failed to load font");
        }
        
        initActions();
        createButtons();
    }
    
    void initActions() {
        actions["add_task"] = [this]() { 
            state.currentScreen = Screen::ADD_TASK;
            resetAddTaskForm();
        };
        
        actions["view_tasks"] = [this]() {
            state.currentScreen = Screen::VIEW_TASKS;
            state.displayTasks = taskManager.getAllTasks();
        };
        
        actions["search_title"] = [this]() {
            state.currentScreen = Screen::SEARCH_BY_TITLE;
            state.titleSearch.isSearching = true;
            state.isTyping = true;
            state.inputBuffer.clear();
        };
        
        actions["search_deadline"] = [this]() {
            state.currentScreen = Screen::SEARCH_BY_DEADLINE;
            state.deadlineSearch.isSearching = true;
            state.isTyping = true;
            state.inputBuffer.clear();
        };
        
        actions["dashboard"] = [this]() {
            state.currentScreen = Screen::DASHBOARD;
            updateDashboard();
        };
        
        actions["completed_tasks"] = [this]() {
            state.currentScreen = Screen::COMPLETED_TASKS;
            state.displayTasks = taskManager.getCompletedTasks();
        };
        
        actions["pending_tasks"] = [this]() {
            state.currentScreen = Screen::PENDING_TASKS;
            state.displayTasks = taskManager.getPendingTasks();
        };
        
        actions["priority_tasks"] = [this]() {
            state.currentScreen = Screen::PRIORITY_TASKS;
            state.displayTasks = taskManager.getTasksByPriority();
        };
        
        actions["priority_tasks_all"] = [this]() {
            state.currentScreen = Screen::PRIORITY_TASKS;
            vector<Task> allTasks = taskManager.getAllTasks();
            sort(allTasks.begin(), allTasks.end(),
                 [](const Task& a, const Task& b) { return a.priority < b.priority; });
            state.displayTasks = allTasks;
        };
        
        actions["priority_tasks_pending"] = [this]() {
            state.currentScreen = Screen::PRIORITY_TASKS;
            showPriorityQueue(); // Keep existing behavior
        };
        
        actions["top_n_priority"] = [this]() {
            state.currentScreen = Screen::TOP_N_PRIORITY;
            state.priorityView.isInputting = true;
            state.isTyping = true;
            state.inputBuffer.clear();
        };
        
        actions["priority_queue"] = [this]() {
            state.currentScreen = Screen::PRIORITY_QUEUE;
            showPriorityQueue();
        };
        
        actions["sort_deadline"] = [this]() {
            taskManager.sortByDeadline();
            refreshCurrentView();
            showStatusMessage("Tasks sorted by deadline!");
        };
        
        actions["undo"] = [this]() {
            taskManager.undo();
            refreshCurrentView();
            showStatusMessage("Undo completed!");
        };
        
        actions["redo"] = [this]() {
            taskManager.redo();
            refreshCurrentView();
            showStatusMessage("Redo completed!");
        };
    }
    
    void createButtons() {
        buttons.clear();
        
        // Sidebar buttons - FIXED: Only create once, not every frame
        float y = HEADER_HEIGHT + 20;
        vector<pair<string, string>> menuItems = {
            {"Dashboard", "dashboard"},
            {"Add Task", "add_task"},
            {"All Tasks", "view_tasks"},
            {"Pending Tasks", "pending_tasks"},
            {"Completed Tasks", "completed_tasks"},
            {"Search by Title", "search_title"},
            {"Search by Deadline", "search_deadline"},
            {"Priority View", "priority_tasks"},
            {"Top N Priority", "top_n_priority"},
            {"Priority Queue", "priority_queue"},
            {"Sort by Deadline", "sort_deadline"}
        };
        
        for (const auto& [label, action] : menuItems) {
            Button btn(label, Vector2f(240, 35), Vector2f(20, y), font, PRIMARY_COLOR);
            if (actions.find(action) != actions.end()) {
                btn.onClick = actions[action];
            }
            buttons.push_back(btn);
            y += 45;
        }
        
        // Undo/Redo buttons
        y += 20;
        Button undoBtn("Undo", Vector2f(115, 35), Vector2f(20, y), font, WARNING_COLOR);
        undoBtn.onClick = actions["undo"];
        buttons.push_back(undoBtn);
        
        Button redoBtn("Redo", Vector2f(115, 35), Vector2f(145, y), font, WARNING_COLOR);
        redoBtn.onClick = actions["redo"];
        buttons.push_back(redoBtn);
    }
    
    void handleInput(Event& event) {
        if (event.type == Event::MouseButtonPressed) {
            if (event.mouseButton.button == Mouse::Left) {
                handleMouseClick(event.mouseButton.x, event.mouseButton.y);
            }
        }
        else if (event.type == Event::TextEntered) {
            handleTextInput(event);
        }
        else if (event.type == Event::KeyPressed) {
            if (event.key.code == Keyboard::Return) {
                handleEnterKey();
            }
            else if (event.key.code == Keyboard::Escape) {
                handleEscapeKey();
            }
        }
    }
    
    void handleMouseClick(int x, int y) {
        Vector2i mousePos(x, y);
        
        // Check button clicks
        for (auto& button : buttons) {
            if (button.isEnabled && button.contains(mousePos)) {
                button.onClick();
                return;
            }
        }
        
        // Check task list clicks
        if (state.currentScreen == Screen::VIEW_TASKS || 
            state.currentScreen == Screen::COMPLETED_TASKS ||
            state.currentScreen == Screen::PENDING_TASKS ||
            state.currentScreen == Screen::PRIORITY_TASKS) {
            handleTaskListClick(mousePos);
        }
    }
    
    void handleTaskListClick(Vector2i mousePos) {
        float y = HEADER_HEIGHT + 20 - state.scrollOffset;
        for (const auto& task : state.displayTasks) {
            FloatRect taskBounds(SIDEBAR_WIDTH + 20, y, 
                               WINDOW_WIDTH - SIDEBAR_WIDTH - 40, 80);
            
            if (taskBounds.contains(mousePos.x, mousePos.y)) {
                state.selectedTaskId = task.id;
                showTaskActions(task);
                return;
            }
            y += 90;
        }
    }
    
    void showTaskActions(const Task& task) {
        // Create action buttons for the selected task
        createButtons(); // Recreate sidebar buttons
        
        float x = SIDEBAR_WIDTH + 300;
        float y = HEADER_HEIGHT + 100;
        
        // Complete button
        if (!task.completed) {
            Button completeBtn("Complete", Vector2f(100, 30), Vector2f(x, y), font, SUCCESS_COLOR);
            completeBtn.onClick = [this, task]() {
                taskManager.markTaskCompleted(task.id);
                refreshCurrentView();
                state.selectedTaskId = -1;
                createButtons(); // Clear action buttons
                showStatusMessage("Task completed!");
            };
            buttons.push_back(completeBtn);
        }
        
        // Edit button
        x += 120;
        Button editBtn("Edit", Vector2f(100, 30), Vector2f(x, y), font, SECONDARY_COLOR);
        editBtn.onClick = [this, task]() {
            startEditingTask(task);
        };
        buttons.push_back(editBtn);
        
        // Delete button
        x += 120;
        Button deleteBtn("Delete", Vector2f(100, 30), Vector2f(x, y), font, DANGER_COLOR);
        deleteBtn.onClick = [this, task]() {
            taskManager.deleteTask(task.id);
            refreshCurrentView();
            state.selectedTaskId = -1;
            createButtons(); // Clear action buttons
            showStatusMessage("Task deleted!");
        };
        buttons.push_back(deleteBtn);
        
        // Move to Tomorrow button
        y += 40;
        x = SIDEBAR_WIDTH + 300;
        Button tomorrowBtn("Move to Tomorrow", Vector2f(150, 30), Vector2f(x, y), font, WARNING_COLOR);
        tomorrowBtn.onClick = [this, taskId = task.id]() {
            try {
                moveTaskToTomorrow(taskId);
                refreshCurrentView();
                state.selectedTaskId = -1;
                createButtons(); // Clear action buttons
                showStatusMessage("Task moved to tomorrow!");
            } catch (const std::exception& e) {
                showStatusMessage("Error moving task to tomorrow!");
            }
        };
        buttons.push_back(tomorrowBtn);
        
        // Update Priority button
        x += 170;
        Button priorityBtn("Update Priority", Vector2f(150, 30), Vector2f(x, y), font, SECONDARY_COLOR);
        priorityBtn.onClick = [this, task]() {
            startPriorityUpdate(task);
        };
        buttons.push_back(priorityBtn);
        
        // Cancel button - NEW
        x += 170;
        Button cancelBtn("Cancel", Vector2f(100, 30), Vector2f(x, y), font, Color(128, 128, 128));
        cancelBtn.onClick = [this]() {
            state.selectedTaskId = -1;
            createButtons(); // Clear action buttons and return to normal sidebar
            showStatusMessage("Selection cancelled");
        };
        buttons.push_back(cancelBtn);
    }
    
    void startEditingTask(const Task& task) {
        state.currentScreen = Screen::EDIT_TASK;
        state.editForm.isEditing = true;
        state.editForm.taskId = task.id;
        state.editForm.title = task.title;
        state.editForm.description = task.description;
        state.editForm.deadline = task.deadline;
        state.editForm.priority = task.priority;
        state.editForm.step = 0;
        state.isTyping = true;
        state.inputBuffer = task.title;
    }
    
    void startPriorityUpdate(const Task& task) {
        state.priorityUpdate.taskId = task.id;
        state.priorityUpdate.isUpdating = true;
        state.isTyping = true;
        state.inputBuffer.clear();
        showStatusMessage("Enter new priority (1-10) for task: " + task.title);
    }
    
    void handlePriorityUpdate() {
        try {
            int newPriority = stoi(state.inputBuffer);
            if (newPriority >= 1 && newPriority <= 10) {
                Task* task = taskManager.getTask(state.priorityUpdate.taskId);
                if (task) {
                    taskManager.updateTask(task->id, task->title, task->description, 
                                         task->deadline, newPriority);
                    refreshCurrentView();
                    showStatusMessage("Priority updated successfully!");
                }
                state.priorityUpdate.isUpdating = false;
                state.isTyping = false;
                state.inputBuffer.clear();
            }
            else {
                showStatusMessage("Priority must be between 1 and 10");
            }
        }
        catch (...) {
            showStatusMessage("Invalid priority number");
        }
    }
    
    void moveTaskToTomorrow(int taskId) {
        Task* task = taskManager.getTask(taskId);
        if (task) {
            Date date(task->deadline);
            date.addDays(1);
            taskManager.updateTask(taskId, task->title, task->description, 
                                 date.toString(), task->priority);
        }
    }
    
    void handleTextInput(Event& event) {
        if (!state.isTyping) return;
        
        if (event.text.unicode == '\b') {
            if (!state.inputBuffer.empty())
                state.inputBuffer.pop_back();
        }
        else if (event.text.unicode >= 32 && event.text.unicode < 128) {
            state.inputBuffer += static_cast<char>(event.text.unicode);
        }
    }
    
    void handleEnterKey() {
        if (state.currentScreen == Screen::ADD_TASK) {
            handleAddTaskInput();
        }
        else if (state.currentScreen == Screen::EDIT_TASK) {
            handleEditTaskInput();
        }
        else if (state.currentScreen == Screen::SEARCH_BY_TITLE) {
            performTitleSearch();
        }
        else if (state.currentScreen == Screen::SEARCH_BY_DEADLINE) {
            performDeadlineSearch();
        }
        else if (state.currentScreen == Screen::TOP_N_PRIORITY) {
            handleTopNInput();
        }
        else if (state.priorityUpdate.isUpdating) {
            handlePriorityUpdate();
        }
    }
    
    void handleEscapeKey() {
        state.isTyping = false;
        state.inputBuffer.clear();
        if (state.editForm.isEditing) {
            state.editForm.isEditing = false;
            state.currentScreen = Screen::VIEW_TASKS;
        }
        state.deadlineSearch.isSearching = false;
    }
    
    void handleAddTaskInput() {
        switch (state.addTaskForm.step) {
            case 0: // Title
                if (state.inputBuffer.empty()) {
                    showError("Title cannot be empty");
                    return;
                }
                state.addTaskForm.title = state.inputBuffer;
                break;
                
            case 1: // Description
                state.addTaskForm.description = state.inputBuffer;
                break;
                
            case 2: // Deadline
                if (!isValidDate(state.inputBuffer)) {
                    showError("Invalid date format (YYYY-MM-DD)");
                    return;
                }
                state.addTaskForm.deadline = state.inputBuffer;
                break;
                
            case 3: // Priority
                try {
                    int priority = stoi(state.inputBuffer);
                    if (priority < 1 || priority > 10) {
                        showError("Priority must be between 1 and 10");
                        return;
                    }
                    state.addTaskForm.priority = priority;
                    
                    // All input received, create task
                    taskManager.addTask(
                        state.addTaskForm.title,
                        state.addTaskForm.description,
                        state.addTaskForm.deadline,
                        state.addTaskForm.priority
                    );
                    
                    // Reset and return to dashboard
                    resetAddTaskForm();
                    state.currentScreen = Screen::DASHBOARD;
                    showStatusMessage("Task added successfully!");
                    return;
                }
                catch (...) {
                    showError("Invalid priority number");
                    return;
                }
        }
        
        // Move to next input field
        state.addTaskForm.step++;
        state.inputBuffer.clear();
    }
    
    void handleEditTaskInput() {
        switch (state.editForm.step) {
            case 0: // Title
                if (state.inputBuffer.empty()) {
                    showError("Title cannot be empty");
                    return;
                }
                state.editForm.title = state.inputBuffer;
                break;
                
            case 1: // Description
                state.editForm.description = state.inputBuffer;
                break;
                
            case 2: // Deadline
                if (!isValidDate(state.inputBuffer)) {
                    showError("Invalid date format (YYYY-MM-DD)");
                    return;
                }
                state.editForm.deadline = state.inputBuffer;
                break;
                
            case 3: // Priority
                try {
                    int priority = stoi(state.inputBuffer);
                    if (priority < 1 || priority > 10) {
                        showError("Priority must be between 1 and 10");
                        return;
                    }
                    state.editForm.priority = priority;
                    
                    // All input received, update task
                    taskManager.updateTask(
                        state.editForm.taskId,
                        state.editForm.title,
                        state.editForm.description,
                        state.editForm.deadline,
                        state.editForm.priority
                    );
                    
                    // Reset and return to view tasks
                    state.editForm.isEditing = false;
                    state.currentScreen = Screen::VIEW_TASKS;
                    refreshCurrentView();
                    showStatusMessage("Task updated successfully!");
                    return;
                }
                catch (...) {
                    showError("Invalid priority number");
                    return;
                }
        }
        
        // Move to next input field
        state.editForm.step++;
        state.inputBuffer.clear();
    }
    
    void resetAddTaskForm() {
        state.addTaskForm = {};
        state.inputBuffer.clear();
        state.isTyping = true;
    }
    
    // NEW FUNCTIONS FOR MISSING CLI FEATURES
    
    void performTitleSearch() {
        if (!state.inputBuffer.empty()) {
            state.titleSearch.searchTitle = state.inputBuffer;
            state.displayTasks = taskManager.searchTasks(state.titleSearch.searchTitle);
            state.inputBuffer.clear();
            state.isTyping = false;
            state.titleSearch.isSearching = false;
            showStatusMessage("Search by title completed!");
        }
    }
    
    void performDeadlineSearch() {
        if (!state.inputBuffer.empty()) {
            state.deadlineSearch.searchDate = state.inputBuffer;
            vector<Task> allTasks = taskManager.getAllTasks();
            state.displayTasks.clear();
            
            for (const auto& task : allTasks) {
                if (task.deadline == state.deadlineSearch.searchDate) {
                    state.displayTasks.push_back(task);
                }
            }
            
            state.inputBuffer.clear();
            state.deadlineSearch.isSearching = false;
            state.isTyping = false;
            showStatusMessage("Search by deadline completed!");
        }
    }
    
    void handleTopNInput() {
        try {
            int n = stoi(state.inputBuffer);
            if (n > 0 && n <= 50) {
                state.priorityView.topN = n;
                showTopNPriorityTasks(n);
                state.inputBuffer.clear();
                state.isTyping = false;
                state.priorityView.isInputting = false;
            }
            else {
                showStatusMessage("Please enter a number between 1 and 50");
            }
        }
        catch (...) {
            showStatusMessage("Invalid number format");
        }
    }
    
    void showTopNPriorityTasks(int n) {
        vector<Task> allTasks = taskManager.getPendingTasks();
        sort(allTasks.begin(), allTasks.end(), 
             [](const Task& a, const Task& b) { return a.priority < b.priority; });
        
        state.displayTasks.clear();
        for (int i = 0; i < min(n, (int)allTasks.size()); i++) {
            state.displayTasks.push_back(allTasks[i]);
        }
        
        showStatusMessage("Showing top " + to_string(state.displayTasks.size()) + " priority tasks");
    }
    
    void showPriorityQueue() {
        // Show tasks in priority order (MinHeap simulation)
        vector<Task> pendingTasks = taskManager.getPendingTasks();
        sort(pendingTasks.begin(), pendingTasks.end(),
             [](const Task& a, const Task& b) { return a.priority < b.priority; });
        state.displayTasks = pendingTasks;
        showStatusMessage("Priority Queue View - Tasks ordered by priority");
    }
    
    void showError(const string& message) {
        state.addTaskForm.showError = true;
        state.addTaskForm.errorMsg = message;
    }
    
    void showStatusMessage(const string& message) {
        state.statusMessage = message;
        state.showStatusMessage = true;
    }
    
    bool isValidDate(const string& date) {
        if (date.length() != 10) return false;
        if (date[4] != '-' || date[7] != '-') return false;
        
        try {
            int year = stoi(date.substr(0, 4));
            int month = stoi(date.substr(5, 2));
            int day = stoi(date.substr(8, 2));
            
            return year >= 2024 && month >= 1 && month <= 12 && day >= 1 && day <= 31;
        }
        catch (...) {
            return false;
        }
    }
    
    void updateDashboard() {
        auto allTasks = taskManager.getAllTasks();
        auto pendingTasks = taskManager.getPendingTasks();
        auto completedTasks = taskManager.getCompletedTasks();
        
        // Update display tasks with most recent tasks
        state.displayTasks = allTasks;
        if (state.displayTasks.size() > 5) {
            state.displayTasks.resize(5);
        }
    }
    
    void refreshCurrentView() {
        switch (state.currentScreen) {
            case Screen::DASHBOARD:
                updateDashboard();
                break;
            case Screen::VIEW_TASKS:
                state.displayTasks = taskManager.getAllTasks();
                break;
            case Screen::COMPLETED_TASKS:
                state.displayTasks = taskManager.getCompletedTasks();
                break;
            case Screen::PENDING_TASKS:
                state.displayTasks = taskManager.getPendingTasks();
                break;
            case Screen::PRIORITY_TASKS:
                state.displayTasks = taskManager.getTasksByPriority();
                break;
            case Screen::PRIORITY_QUEUE:
                showPriorityQueue();
                break;
            default:
                break;
        }
    }
    
    void drawScreen() {
        window.clear(BG_COLOR);
        
        drawSidebar();
        drawHeader();
        
        switch (state.currentScreen) {
            case Screen::DASHBOARD:
                drawDashboard();
                break;
            case Screen::ADD_TASK:
                drawAddTaskScreen();
                break;
            case Screen::EDIT_TASK:
                drawEditTaskScreen();
                break;
            case Screen::VIEW_TASKS:
            case Screen::COMPLETED_TASKS:
            case Screen::PENDING_TASKS:
            case Screen::PRIORITY_TASKS:
                drawTaskList();
                break;
            case Screen::PRIORITY_QUEUE:
                drawPriorityQueueScreen();
                break;
            case Screen::SEARCH_BY_TITLE:
                drawSearchByTitleScreen();
                break;
            case Screen::SEARCH_BY_DEADLINE:
                drawSearchByDeadlineScreen();
                break;
            case Screen::TOP_N_PRIORITY:
                drawTopNPriorityScreen();
                break;
            case Screen::UPDATE_PRIORITY:
                drawUpdatePriorityScreen();
                break;
            case Screen::SETTINGS:
                drawSettingsScreen();
                break;
        }
        
        // Draw status message
        if (state.showStatusMessage) {
            drawStatusMessage();
        }
        
        // Draw all buttons
        for (const auto& button : buttons) {
            button.draw(window);
        }
        
        window.display();
    }
    
    void drawSidebar() {
        RectangleShape sidebar(Vector2f(SIDEBAR_WIDTH, WINDOW_HEIGHT));
        sidebar.setFillColor(Color::White);
        sidebar.setPosition(0, 0);
        window.draw(sidebar);
        
        // Logo
        Text logo;
        logo.setFont(font);
        logo.setString("Task Manager Pro");
        logo.setCharacterSize(20);
        logo.setFillColor(PRIMARY_COLOR);
        logo.setPosition(20, 20);
        window.draw(logo);
    }
    
    void drawHeader() {
        RectangleShape header(Vector2f(WINDOW_WIDTH - SIDEBAR_WIDTH, HEADER_HEIGHT));
        header.setFillColor(Color::White);
        header.setPosition(SIDEBAR_WIDTH, 0);
        window.draw(header);
        
        Text title;
        title.setFont(font);
        title.setString(getScreenTitle());
        title.setCharacterSize(20);
        title.setFillColor(TEXT_COLOR);
        title.setPosition(SIDEBAR_WIDTH + 20, 20);
        window.draw(title);
    }
    
    string getScreenTitle() const {
        switch (state.currentScreen) {
            case Screen::DASHBOARD: return "Dashboard";
            case Screen::ADD_TASK: return "Add New Task";
            case Screen::EDIT_TASK: return "Edit Task";
            case Screen::VIEW_TASKS: return "All Tasks";
            case Screen::COMPLETED_TASKS: return "Completed Tasks";
            case Screen::PENDING_TASKS: return "Pending Tasks";
            case Screen::PRIORITY_TASKS: return "Tasks by Priority";
            case Screen::SEARCH_BY_TITLE: return "Search by Title";
            case Screen::SEARCH_BY_DEADLINE: return "Search by Deadline";
            case Screen::TOP_N_PRIORITY: return "Top N Priority Tasks";
            case Screen::PRIORITY_QUEUE: return "Priority Queue";
            case Screen::UPDATE_PRIORITY: return "Update Priority";
            case Screen::SETTINGS: return "Settings";
            default: return "";
        }
    }
    
    // FIXED: Remove the problematic dashboard button creation
    void drawDashboard() {
        float x = SIDEBAR_WIDTH + 20;
        float y = HEADER_HEIGHT + 20;
        
        // Statistics
        drawStatCard("Total Tasks", to_string(taskManager.getAllTasks().size()), x, y);
        drawStatCard("Pending", to_string(taskManager.getPendingTasks().size()), x + 220, y);
        drawStatCard("Completed", to_string(taskManager.getCompletedTasks().size()), x + 440, y);
        
        // Recent Tasks
        y += 180;
        Text recentTitle;
        recentTitle.setFont(font);
        recentTitle.setString("Recent Tasks");
        recentTitle.setCharacterSize(18);
        recentTitle.setFillColor(TEXT_COLOR);
        recentTitle.setPosition(x, y);
        window.draw(recentTitle);
        
        y += 40;
        for (const auto& task : state.displayTasks) {
            drawTaskCard(task, x, y);
            y += 90;
        }
    }
    
    void drawStatCard(const string& label, const string& value, float x, float y) {
        RectangleShape card(Vector2f(200, 100));
        card.setPosition(x, y);
        card.setFillColor(Color::White);
        card.setOutlineColor(Color(200, 200, 200));
        card.setOutlineThickness(1);
        window.draw(card);
        
        Text valueText;
        valueText.setFont(font);
        valueText.setString(value);
        valueText.setCharacterSize(32);
        valueText.setFillColor(PRIMARY_COLOR);
        valueText.setPosition(x + 20, y + 20);
        window.draw(valueText);
        
        Text labelText;
        labelText.setFont(font);
        labelText.setString(label);
        labelText.setCharacterSize(16);
        labelText.setFillColor(LIGHT_TEXT);
        labelText.setPosition(x + 20, y + 60);
        window.draw(labelText);
    }
    
    void drawTaskCard(const Task& task, float x, float y) {
        RectangleShape card(Vector2f(WINDOW_WIDTH - SIDEBAR_WIDTH - 40, 80));
        card.setPosition(x, y);
        card.setFillColor(Color::White);
        card.setOutlineColor(Color(200, 200, 200));
        card.setOutlineThickness(1);
        
        if (task.id == state.selectedTaskId) {
            card.setOutlineColor(PRIMARY_COLOR);
            card.setOutlineThickness(2);
        }
        
        window.draw(card);
        
        // Title
        Text titleText;
        titleText.setFont(font);
        titleText.setString(task.title);
        titleText.setCharacterSize(16);
        titleText.setFillColor(TEXT_COLOR);
        titleText.setPosition(x + 20, y + 15);
        window.draw(titleText);
        
        // Description
        Text descText;
        descText.setFont(font);
        string desc = task.description;
        if (desc.length() > 50) {
            desc = desc.substr(0, 47) + "...";
        }
        descText.setString(desc);
        descText.setCharacterSize(14);
        descText.setFillColor(LIGHT_TEXT);
        descText.setPosition(x + 20, y + 40);
        window.draw(descText);
        
        // Deadline
        Text deadlineText;
        deadlineText.setFont(font);
        deadlineText.setString("Due: " + task.deadline);
        deadlineText.setCharacterSize(14);
        deadlineText.setFillColor(LIGHT_TEXT);
        deadlineText.setPosition(x + 400, y + 15);
        window.draw(deadlineText);
        
        // Priority
        drawPriorityIndicator(task.priority, x + card.getSize().x - 120, y + 15);
        
        // Status
        if (task.completed) {
            CircleShape checkmark(8);
            checkmark.setFillColor(SUCCESS_COLOR);
            checkmark.setPosition(x + card.getSize().x - 30, y + 15);
            window.draw(checkmark);
            
            Text completedText;
            completedText.setFont(font);
            completedText.setString("✓");
            completedText.setCharacterSize(14);
            completedText.setFillColor(Color::White);
            completedText.setPosition(x + card.getSize().x - 26, y + 17);
            window.draw(completedText);
        }
    }
    
    void drawPriorityIndicator(int priority, float x, float y) {
        CircleShape indicator(8);
        indicator.setPosition(x, y);
        
        if (priority <= 3) indicator.setFillColor(DANGER_COLOR);
        else if (priority <= 7) indicator.setFillColor(WARNING_COLOR);
        else indicator.setFillColor(SUCCESS_COLOR);
        
        window.draw(indicator);
        
        Text priorityText;
        priorityText.setFont(font);
        priorityText.setString("P" + to_string(priority));
        priorityText.setCharacterSize(14);
        priorityText.setFillColor(LIGHT_TEXT);
        priorityText.setPosition(x + 20, y - 4);
        window.draw(priorityText);
    }
    
    void drawAddTaskScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        vector<pair<string, string*>> fields = {
            {"Title", &state.addTaskForm.title},
            {"Description", &state.addTaskForm.description},
            {"Deadline (YYYY-MM-DD)", &state.addTaskForm.deadline}
        };
        
        for (size_t i = 0; i < fields.size(); i++) {
            drawInputField(fields[i].first, *fields[i].second,
                         x, y, i == state.addTaskForm.step);
            y += 80;
        }
        
        // Priority
        Text priorityLabel;
        priorityLabel.setFont(font);
        priorityLabel.setString("Priority (1-10): " + to_string(state.addTaskForm.priority));
        priorityLabel.setCharacterSize(16);
        priorityLabel.setFillColor(TEXT_COLOR);
        priorityLabel.setPosition(x, y);
        window.draw(priorityLabel);
        
        if (state.addTaskForm.step == 3) {
            drawInputField("Priority", state.inputBuffer, x, y + 30, true);
        }
        
        // Error message
        if (state.addTaskForm.showError) {
            Text errorText;
            errorText.setFont(font);
            errorText.setString(state.addTaskForm.errorMsg);
            errorText.setCharacterSize(14);
            errorText.setFillColor(DANGER_COLOR);
            errorText.setPosition(x, y + 100);
            window.draw(errorText);
        }
    }
    
    void drawEditTaskScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        vector<pair<string, string*>> fields = {
            {"Title", &state.editForm.title},
            {"Description", &state.editForm.description},
            {"Deadline (YYYY-MM-DD)", &state.editForm.deadline}
        };
        
        for (size_t i = 0; i < fields.size(); i++) {
            drawInputField(fields[i].first, *fields[i].second,
                         x, y, i == state.editForm.step);
            y += 80;
        }
        
        // Priority
        Text priorityLabel;
        priorityLabel.setFont(font);
        priorityLabel.setString("Priority (1-10): " + to_string(state.editForm.priority));
        priorityLabel.setCharacterSize(16);
        priorityLabel.setFillColor(TEXT_COLOR);
        priorityLabel.setPosition(x, y);
        window.draw(priorityLabel);
        
        if (state.editForm.step == 3) {
            drawInputField("Priority", state.inputBuffer, x, y + 30, true);
        }
    }
    
    void drawInputField(const string& label, const string& value,
                       float x, float y, bool isActive) {
        Text labelText;
        labelText.setFont(font);
        labelText.setString(label);
        labelText.setCharacterSize(16);
        labelText.setFillColor(TEXT_COLOR);
        labelText.setPosition(x, y);
        window.draw(labelText);
        
        RectangleShape field(Vector2f(500, 40));
        field.setPosition(x, y + 25);
        field.setFillColor(Color::White);
        field.setOutlineColor(isActive ? PRIMARY_COLOR : LIGHT_TEXT);
        field.setOutlineThickness(1);
        window.draw(field);
        
        Text valueText;
        valueText.setFont(font);
        valueText.setString(isActive ? state.inputBuffer + "|" : value);
        valueText.setCharacterSize(16);
        valueText.setFillColor(TEXT_COLOR);
        valueText.setPosition(x + 10, y + 35);
        window.draw(valueText);
    }
    
    void drawTaskList() {
        float x = SIDEBAR_WIDTH + 20;
        float y = HEADER_HEIGHT + 20 - state.scrollOffset;
        
        // Check if there are no tasks to display
        if (state.displayTasks.empty()) {
            Text noTasksText;
            noTasksText.setFont(font);
            
            string message;
            switch (state.currentScreen) {
                case Screen::COMPLETED_TASKS:
                    message = "No completed tasks to show";
                    break;
                case Screen::PENDING_TASKS:
                    message = "No pending tasks to show";
                    break;
                case Screen::PRIORITY_TASKS:
                    message = "No priority tasks to show";
                    break;
                case Screen::VIEW_TASKS:
                    message = "No tasks to show";
                    break;
                case Screen::SEARCH_BY_TITLE:
                case Screen::SEARCH_BY_DEADLINE:
                    message = "No tasks found matching your search";
                    break;
                case Screen::TOP_N_PRIORITY:
                    message = "No priority tasks to show";
                    break;
                case Screen::PRIORITY_QUEUE:
                    message = "Priority queue is empty";
                    break;
                default:
                    message = "No tasks available";
                    break;
            }
            
            noTasksText.setString(message);
            noTasksText.setCharacterSize(18);
            noTasksText.setFillColor(LIGHT_TEXT);
            noTasksText.setPosition(x + 50, y + 100);
            window.draw(noTasksText);
            return;
        }
        
        for (const auto& task : state.displayTasks) {
            if (y + 80 > HEADER_HEIGHT && y < WINDOW_HEIGHT) {
                drawTaskCard(task, x, y);
            }
            y += 90;
        }
    }
    
    // NEW DRAWING FUNCTIONS
    
    void drawSearchByTitleScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        drawInputField("Search by Title", state.titleSearch.searchTitle, x, y, state.isTyping);
        
        y += 100;
        if (state.displayTasks.empty() && !state.titleSearch.searchTitle.empty()) {
            Text noResultsText;
            noResultsText.setFont(font);
            noResultsText.setString("No tasks found matching '" + state.titleSearch.searchTitle + "'");
            noResultsText.setCharacterSize(16);
            noResultsText.setFillColor(LIGHT_TEXT);
            noResultsText.setPosition(x, y);
            window.draw(noResultsText);
        } else if (!state.displayTasks.empty()) {
            Text resultsText;
            resultsText.setFont(font);
            resultsText.setString("Search Results: (" + to_string(state.displayTasks.size()) + " found)");
            resultsText.setCharacterSize(18);
            resultsText.setFillColor(TEXT_COLOR);
            resultsText.setPosition(x, y);
            window.draw(resultsText);
            
            y += 40;
            for (const auto& task : state.displayTasks) {
                drawTaskCard(task, x, y);
                y += 90;
            }
        }
    }
    
    void drawSearchByDeadlineScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        drawInputField("Search by Deadline (YYYY-MM-DD)", state.deadlineSearch.searchDate, x, y, state.isTyping);
        
        y += 100;
        if (state.displayTasks.empty() && !state.deadlineSearch.searchDate.empty()) {
            Text noResultsText;
            noResultsText.setFont(font);
            noResultsText.setString("No tasks found for date: " + state.deadlineSearch.searchDate);
            noResultsText.setCharacterSize(16);
            noResultsText.setFillColor(LIGHT_TEXT);
            noResultsText.setPosition(x, y);
            window.draw(noResultsText);
        }
        else if (!state.displayTasks.empty()) {
            Text resultsText;
            resultsText.setFont(font);
            resultsText.setString("Tasks due on " + state.deadlineSearch.searchDate + ": (" + to_string(state.displayTasks.size()) + " found)");
            resultsText.setCharacterSize(18);
            resultsText.setFillColor(TEXT_COLOR);
            resultsText.setPosition(x, y);
            window.draw(resultsText);
            
            y += 40;
            for (const auto& task : state.displayTasks) {
                drawTaskCard(task, x, y);
                y += 90;
            }
        }
    }
    
    void drawTopNPriorityScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        if (state.priorityView.isInputting) {
            drawInputField("Enter number of top priority tasks to show", "", x, y, state.isTyping);
        }
        else {
            Text titleText;
            titleText.setFont(font);
            titleText.setString("Top " + to_string(state.priorityView.topN) + " Priority Tasks");
            titleText.setCharacterSize(20);
            titleText.setFillColor(TEXT_COLOR);
            titleText.setPosition(x, y);
            window.draw(titleText);
            
            y += 50;
            if (state.displayTasks.empty()) {
                Text noTasksText;
                noTasksText.setFont(font);
                noTasksText.setString("No priority tasks to show");
                noTasksText.setCharacterSize(16);
                noTasksText.setFillColor(LIGHT_TEXT);
                noTasksText.setPosition(x, y);
                window.draw(noTasksText);
            }
            else {
                for (const auto& task : state.displayTasks) {
                    drawTaskCard(task, x, y);
                    y += 90;
                }
            }
        }
    }
    
    void drawPriorityQueueScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        Text titleText;
        titleText.setFont(font);
        titleText.setString("Priority Queue (Ordered by Priority)");
        titleText.setCharacterSize(20);
        titleText.setFillColor(TEXT_COLOR);
        titleText.setPosition(x, y);
        window.draw(titleText);
        
        y += 50;
        if (state.displayTasks.empty()) {
            Text emptyText;
            emptyText.setFont(font);
            emptyText.setString("Priority queue is empty");
            emptyText.setCharacterSize(16);
            emptyText.setFillColor(LIGHT_TEXT);
            emptyText.setPosition(x, y);
            window.draw(emptyText);
        }
        else {
            for (const auto& task : state.displayTasks) {
                drawTaskCard(task, x, y);
                y += 90;
            }
        }
    }
    
    void drawUpdatePriorityScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        drawInputField("Enter new priority (1-10)", "", x, y, state.isTyping);
    }
    
    void drawSettingsScreen() {
        float x = SIDEBAR_WIDTH + 50;
        float y = HEADER_HEIGHT + 50;
        
        Text settingsText;
        settingsText.setFont(font);
        settingsText.setString("Settings and Advanced Features");
        settingsText.setCharacterSize(20);
        settingsText.setFillColor(TEXT_COLOR);
        settingsText.setPosition(x, y);
        window.draw(settingsText);
        
        y += 60;
        
        // Advanced features info
        vector<string> features = {
            "• Undo/Redo functionality available",
            "• Sort tasks by deadline",
            "• Move tasks to tomorrow",
            "• Update task priorities",
            "• Search by deadline",
            "• View tasks by priority",
            "• Separate completed/pending views"
        };
        
        for (const string& feature : features) {
            Text featureText;
            featureText.setFont(font);
            featureText.setString(feature);
            featureText.setCharacterSize(16);
            featureText.setFillColor(TEXT_COLOR);
            featureText.setPosition(x, y);
            window.draw(featureText);
            y += 30;
        }
    }
    
    void drawStatusMessage() {
        RectangleShape msgBox(Vector2f(400, 50));
        msgBox.setPosition(SIDEBAR_WIDTH + 50, WINDOW_HEIGHT - 100);
        msgBox.setFillColor(Color(50, 50, 50, 200));
        window.draw(msgBox);
        
        Text msgText;
        msgText.setFont(font);
        msgText.setString(state.statusMessage);
        msgText.setCharacterSize(14);
        msgText.setFillColor(Color::White);
        msgText.setPosition(SIDEBAR_WIDTH + 60, WINDOW_HEIGHT - 85);
        window.draw(msgText);
        
        // Auto-hide after some time (simplified)
        static int counter = 0;
        counter++;
        if (counter > 180) { // ~3 seconds at 60 FPS
            state.showStatusMessage = false;
            counter = 0;
        }
    }

public:
    TaskManagerGUI() {
        initWindow();
    }
    
    void run() {
        while (window.isOpen()) {
            Event event;
            while (window.pollEvent(event)) {
                if (event.type == Event::Closed)
                    window.close();
                else
                    handleInput(event);
            }
            
            drawScreen();
        }
    }
};

int main() {
    try {
        TaskManagerGUI app;
        app.run();
        return 0;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
}