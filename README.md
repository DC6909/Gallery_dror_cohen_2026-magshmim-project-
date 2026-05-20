# 🖼️ Gallery Management System

A robust **C++ server-side application** designed to manage an interactive media gallery system.  
The project supports structured user management, multi-user albums, dynamic photo tagging, and advanced data-driven queries — all built with a clean and scalable **Object-Oriented Design (OOD)** architecture.

create on program magshmim
---

## ✨ Features

### 👤 User Management
- Create and manage users
- Store structured user information
- Track user activity and ownership

### 📁 Album Management
- Create, open, close, and delete albums
- Multi-user album support
- Album ownership handling
- Persistent album metadata

### 🖼️ Photo Handling
- Add and remove photos
- View album contents
- Store photo metadata
- Dynamic photo tagging system

### 🏷️ Tagging System
- Tag users in photos
- Remove tags
- Query tagged photos
- Advanced relationship-based queries

### 📊 Advanced Queries
- Retrieve user statistics
- Find most tagged users
- Search across albums and photos
- Data-driven analytical queries

---

# 🏗️ System Architecture

The project follows a modular **Object-Oriented Design** approach.

## Core Components

### 📦 AlbumManager
Responsible for:
- Managing albums
- Handling business logic
- Coordinating between users, albums, and photos

### 🗄️ Database Layer
Abstracted data-access layer enabling:
- Clean separation between logic and storage
- Easy replacement of storage mechanisms
- Scalability and maintainability

### 💾 Storage Implementations
- **Memory Storage**
- **Database Storage**

The abstraction allows switching implementations without changing the application logic.

---

# 🧩 Design Principles

✔️ Object-Oriented Design (OOD)  
✔️ Separation of Concerns  
✔️ Modular Architecture  
✔️ Scalable Data Layer  
✔️ Encapsulation & Abstraction  
✔️ Clean and Maintainable Code  

---

# 🛠️ Technologies

- **Language:** C++
- **Paradigm:** Object-Oriented Programming (OOP)
- **Architecture:** Layered / Modular Design
- **Storage:** Abstracted Database & Memory Storage

---

# 📂 Project Structure

```bash
├── AlbumManager/
├── Database/
├── MemoryStorage/
├── Models/
├── Utils/
├── Main.cpp
└── README.md
