# 📋 Task Manager Pro

A comprehensive task management application built with C++ featuring both a modern GUI interface and a command-line interface. This project demonstrates advanced data structures, object-oriented programming, and user interface design.

## ✨ Features

### 🎯 Core Functionality
- **Add Tasks**: Create new tasks with title, description, deadline, and priority
- **Task Management**: Edit, delete, and mark tasks as completed
- **Smart Organization**: Sort tasks by deadline or priority
- **Search & Filter**: Find tasks by title or deadline
- **Undo/Redo**: Full undo/redo functionality for all operations

### 🎨 User Interface
- **Modern GUI**: Beautiful SFML-based graphical interface
- **Command Line**: Full-featured CLI for terminal users
- **Responsive Design**: Clean, intuitive layout with color-coded priorities
- **Real-time Updates**: Instant feedback and status messages

### 📊 Advanced Features
- **Priority Queue**: MinHeap implementation for efficient priority management
- **Data Structures**: Doubly linked list for task storage
- **Date Management**: Custom date class with arithmetic operations
- **Task Analytics**: View completed, pending, and priority-based task lists

## 🏗️ Architecture

### Data Structures Used
- **Doubly Linked List**: For efficient task insertion and deletion
- **MinHeap**: For priority-based task management
- **Stack**: For undo/redo functionality
- **Vector**: For dynamic task storage and sorting

### Design Patterns
- **Object-Oriented Design**: Clean separation of concerns
- **RAII**: Proper resource management
- **Command Pattern**: For undo/redo operations

## 🚀 Getting Started

### Prerequisites
- **C++17** or higher
- **CMake** 3.25+
- **SFML 2.5.1** (for GUI version)
- **Conan** (for dependency management)

### Installation

1. **Clone the repository**
   ```bash
   git clone <repository-url>
   cd ToDoApp
   ```

2. **Install dependencies**
   ```bash
   conan install . --build=missing
   ```

3. **Build the project**
   ```bash
   mkdir build
   cd build
   cmake ..
   cmake --build .
   ```

### Running the Application

#### GUI Version (Recommended)
```bash
./ToDoApp
```

#### Command Line Version
```bash
./task_manager_cli
```

## 📖 Usage Guide

### GUI Interface
1. **Dashboard**: Overview of all tasks with statistics
2. **Add Task**: Step-by-step task creation wizard
3. **View Tasks**: Browse all, completed, or pending tasks
4. **Search**: Find tasks by title or deadline
5. **Priority Management**: View and update task priorities
6. **Task Actions**: Click any task to edit, complete, or delete

### Command Line Interface
The CLI provides a menu-driven interface with the following options:
- Add new tasks
- View all tasks (with filtering options)
- Search functionality
- Task editing and completion
- Undo/redo operations
- Priority management
- Date-based operations

## 🎮 Controls

### GUI Controls
- **Mouse**: Click buttons and select tasks
- **Keyboard**: Type in input fields
- **Enter**: Confirm input or proceed to next step
- **Escape**: Cancel current operation

### CLI Controls
- **Number Input**: Select menu options
- **Text Input**: Enter task details
- **Enter**: Confirm selections

## 🔧 Configuration

### SFML Setup
The project expects SFML 2.5.1 in the `external/` directory:
```
external/
└── SFML-2.5.1/
    ├── include/
    ├── lib/
    └── bin/
```

### Build Configuration
- **C++ Standard**: C++17
- **SFML Components**: system, window, graphics
- **Platform**: Windows (with MinGW support)

## 📁 Project Structure

```
ToDoApp/
├── CMakeLists.txt          # Build configuration
├── conanfile.txt           # Dependency management
├── gui_main.cpp            # GUI application entry point
├── task_manager_cli.cpp    # CLI application
├── task_manager.hpp        # Core task management classes
└── README.md              # This file
```

## 🧩 Key Components

### TaskManager Class
- Central task management system
- Integrates all data structures
- Handles undo/redo operations
- Provides search and filtering

### TaskManagerGUI Class
- SFML-based graphical interface
- Event handling and user input
- Screen management system
- Real-time task visualization

### Data Structures
- **Task**: Core task entity with all properties
- **Date**: Custom date handling with arithmetic
- **MinHeap**: Priority queue implementation
- **Stack**: Undo/redo action storage

## 🎯 Use Cases

### Personal Task Management
- Daily task organization
- Project deadline tracking
- Priority-based work planning

### Educational Purposes
- Data structure implementation examples
- Object-oriented programming concepts
- GUI development with SFML

### Development Learning
- CMake build system usage
- Conan package management
- Cross-platform C++ development

## 🔮 Future Enhancements

- [ ] Database persistence
- [ ] Task categories and tags
- [ ] Reminder notifications
- [ ] Task templates
- [ ] Export/import functionality
- [ ] Multi-user support
- [ ] Mobile companion app

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👨‍💻 Author

**Waniya Masood**
- Project developed as part of Technologia educational program
- Demonstrates advanced C++ programming concepts
- Showcases modern software development practices

## 🙏 Acknowledgments

- **SFML Community** for the excellent graphics library
- **Conan Team** for dependency management
- **CMake Community** for the build system
- **Technologia** for educational support

---

**Happy Task Managing! 🎉**

*Built with ❤️ using C++ and modern software engineering practices*
