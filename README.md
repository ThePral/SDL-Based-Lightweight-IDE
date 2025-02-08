# 🚀 SDL-Based Lightweight IDE  

A simple yet powerful **lightweight IDE** built using **SDL2 and SDL_ttf**. This project features syntax highlighting, a dark/light mode toggle, a sidebar with useful buttons, and basic text editing functionalities like **undo, redo, copy, paste, and compile/run** support.  

---

## 🎯 Features  

✅ **Text Editor** with syntax highlighting  
✅ **Sidebar** with buttons for easy access to actions  
✅ **Dark/Light Mode Toggle**  
✅ **Undo/Redo** functionality  
✅ **Compile & Run** C++ code  
✅ **Auto-closing brackets & parentheses**  
✅ **Error Panel** to display compilation errors  
✅ **Keyboard Shortcuts:**  
   - `Ctrl + S` → Save project  
   - `Ctrl + Z` → Undo  
   - `Ctrl + Y` → Redo  
   - `Ctrl + C` → Copy  
   - `Ctrl + V` → Paste  
   - `Ctrl + A` → Select All  
   - `Ctrl + X` → Cut  

---

## 📥 Installation  

### 1️⃣ Prerequisites  

- **C++ Compiler:** `g++` (MinGW on Windows, GCC on Linux/macOS)  
- **SDL2 & SDL_ttf:**  

#### Windows (MSYS2)  
```bash
pacman -S mingw-w64-x86_64-SDL2 mingw-w64-x86_64-SDL2_ttf
```

#### Linux  
```bash
sudo apt install libsdl2-dev libsdl2-ttf-dev
```

#### macOS  
```bash
brew install sdl2 sdl2_ttf
```

---

### 2️⃣ Clone the Repository  

```bash
git clone https://github.com/your-username/sdl-ide.git
cd sdl-ide
```

---

### 3️⃣ Build & Run  

#### Windows  
```bash
g++ main.cpp -o sdl-ide.exe -I include -L lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
./sdl-ide.exe
```

#### Linux/macOS  
```bash
g++ main.cpp -o sdl-ide -lSDL2 -lSDL2_ttf
./sdl-ide
```

---

## 🎮 Usage  

1️⃣ **Edit your C++ code** in the editor.  
2️⃣ **Use sidebar buttons** to compile, run, or switch themes.  
3️⃣ **Check error panel** for compilation messages.  
4️⃣ **Use shortcuts** to speed up editing.  

---

## 🖥️ Screenshots  

| Dark Mode | Light Mode |
|-----------|-----------|
| ![Dark Mode](screenshots/dark-mode.png) | ![Light Mode](screenshots/light-mode.png) |

---

## 🛠️ Contributing  

Contributions are welcome! Feel free to:  
- **Fork the repo** and submit a PR.  
- **Report issues** in the [Issues](https://github.com/your-username/sdl-ide/issues) tab.  

---

## 📜 License  

This project is licensed under the **MIT License**. See [LICENSE](LICENSE) for details.  

---

