#define SDL_MAIN_HANDLED

#ifdef _WIN32
#include <windows.h>
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <regex>
#include <stack>
#include "compile.hpp"

const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 700;
const int STATUS_BAR_HEIGHT = 24;
const int ERROR_PANEL_HEIGHT = 100;
const std::string FONT_PATH = "arial.ttf";
const std::string OUTPUT_FILE = "temp_output.exe";
const std::string ERROR_FILE = "compile_errors.txt";
std::string userInput;

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;
TTF_Font* font = nullptr;
std::vector<std::string> lines = {""};
std::vector<std::string> errorMessages;
int currentLine = 0, cursorPos = 0, scrollOffset = 0;
bool quit = false, selecting = false;
int selectionStartLine = -1, selectionStartPos = -1;
std::stack<std::vector<std::string>> undoStack, redoStack;

bool init();
void close();
void renderTextEditor();
void renderStatusBar();
void renderErrorPanel();
void renderSidebar();
void renderRightSidebar();
void handleInput(SDL_Event& e);
void handleMouseInput(SDL_Event& e);
void compileAndRun();
void highlightSyntax(std::string& line, SDL_Color& color);
void showCompileErrors();
void newProject();
void saveProject();
void toggleTheme();
void debugAndCompile();
void runProject();
void saveToFile(const std::string& filename);
void loadFromFile(const std::string& filename);
void pushUndoState();
void undo();
void redo();
void copySelection();
void pasteClipboard();

// --------- Button Structure ---------
struct Button {
    SDL_Rect rect;
    std::string label;
    void (*action)();
};

std::vector<Button> buttons = {
    {{SCREEN_WIDTH - 210, 50, 200, 40}, "New Project", newProject},
    {{SCREEN_WIDTH - 210, 100, 200, 40}, "Save Project", saveProject},
    {{SCREEN_WIDTH - 210, 150, 200, 40}, "Undo", undo},
    {{SCREEN_WIDTH - 210, 200, 200, 40}, "Redo", redo},
    {{SCREEN_WIDTH - 210, 250, 200, 40}, "Toggle Theme", toggleTheme},
    {{SCREEN_WIDTH - 210, 300, 200, 40}, "Debug & Compile", debugAndCompile},
    {{SCREEN_WIDTH - 210, 350, 200, 40}, "Run", runProject}
};

bool init() {
    std::cout << "Initializing SDL..." << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }
    if (TTF_Init() == -1) {
        std::cerr << "TTF could not initialize! SDL_ttf Error: " << TTF_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("SDL IDE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    std::cout << "Loading font from: " << FONT_PATH << std::endl;
    font = TTF_OpenFont(FONT_PATH.c_str(), 24);
    if (!font) {
        std::cerr << "Failed to load font! TTF_Error: " << TTF_GetError() << std::endl;
        return false;
    }

    std::cout << "Font loaded successfully!" << std::endl;
    SDL_StartTextInput();
    return true;
}



void close() {
    SDL_StopTextInput();
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

// --------- New Project Function ---------
void newProject() {
    lines.clear();
    lines.push_back("");
    currentLine = cursorPos = 0;
    std::cout << "New project created!\n";
}


// --------- Save Project Function ---------
void saveProject() {
    saveToFile("saved_project.cpp");
    std::cout << "Project saved!\n";
}

bool darkMode = true;

// --------- Toggle Theme Function ---------
void toggleTheme() {
    darkMode = !darkMode;
    std::cout << "Theme toggled!\n";

    renderTextEditor();
}


// --------- Debug and Compile Function ---------
void debugAndCompile() {
    std::cout << "Debugging and compiling...\n";
    compileAndRun();
}


// --------- Run Project Function ---------
void runProject() {
    std::cout << "Running project...\n";
    compileAndRun();
}


// --------- Clipboard Function ---------
void copySelection() {
    if (selectionStartLine != -1 && selecting) {
        int startLine = std::min(selectionStartLine, currentLine);
        int endLine = std::max(selectionStartLine, currentLine);

        std::string copiedText = "";
        for (int i = startLine; i <= endLine; i++) {
            copiedText += lines[i] + "\n";
        }
        SDL_SetClipboardText(copiedText.c_str());
    }
}

void pasteClipboard() {
    if (SDL_HasClipboardText()) {
        char* text = SDL_GetClipboardText();
        lines[currentLine].insert(cursorPos, text);
        cursorPos += strlen(text);
        SDL_free(text);
    }
}

// --------- Mouse Handling ---------
void handleMouseInput(SDL_Event& e) {
    if (e.type == SDL_MOUSEBUTTONDOWN) {
        int mouseX = e.button.x;
        int mouseY = e.button.y;
        int clickedLine = (mouseY + scrollOffset) / 24;
        if (clickedLine < lines.size()) {
            currentLine = clickedLine;
            selecting = true;
            selectionStartLine = currentLine;
            selectionStartPos = cursorPos;
        }

        for (auto& button : buttons) {
            if (mouseX >= button.rect.x && mouseX <= button.rect.x + button.rect.w &&
                mouseY >= button.rect.y && mouseY <= button.rect.y + button.rect.h) {
                button.action();
                return;
            }
        }
    } else if (e.type == SDL_MOUSEBUTTONUP) {
        selecting = false;
    }
}

// --------- Status Bar Rendering ---------
void renderStatusBar() {
    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_Rect statusBar = {0, SCREEN_HEIGHT, SCREEN_WIDTH, STATUS_BAR_HEIGHT};
    SDL_RenderFillRect(renderer, &statusBar);

    std::string status = "Line: " + std::to_string(currentLine + 1) + ", Pos: " + std::to_string(cursorPos);
    if (selecting) {
        status += " [SELECTING]";
    }
    SDL_Color statusColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, status.c_str(), statusColor);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect renderQuad = {10, SCREEN_HEIGHT + 2, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

// --------- Sidebar Rendering ---------
void renderSidebar() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray
    SDL_Rect sidebar = {0, 0, 60, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &sidebar);

    SDL_Color textColor = {255, 255, 255, 255};  // White text
    int y = -scrollOffset;
    for (size_t i = 0; i < lines.size(); i++, y += 28) {
        std::string lineNumber = std::to_string(i + 1);
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, lineNumber.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth = textSurface->w;
        SDL_Rect renderQuad = {30 - textWidth / 2, y + 5, textWidth, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

void renderRightSidebar() {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);  // Dark Gray
    SDL_Rect sidebar = {SCREEN_WIDTH - 220, 0, 220, SCREEN_HEIGHT};
    SDL_RenderFillRect(renderer, &sidebar);

    SDL_Color textColor = {255, 255, 255, 255};  // White text

    for (const auto& button : buttons) {
        SDL_SetRenderDrawColor(renderer, 70, 70, 70, 255);
        SDL_RenderFillRect(renderer, &button.rect);

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, button.label.c_str(), textColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        int textWidth = textSurface->w;
        SDL_Rect textRect = {button.rect.x + (button.rect.w - textWidth) / 2, button.rect.y + 10, textWidth, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }
}

// --------- Text Rendering ---------
void renderTextEditor() {
    if (darkMode) {
        SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);  // Dark mode
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);  // Light mode
    }
    SDL_RenderClear(renderer);

    renderSidebar();
    renderRightSidebar();
    renderStatusBar();

    std::vector<std::string> keywords = {"int", "if", "return", "else", "for", "while"};
    int y = -scrollOffset;

    for (size_t i = 0; i < lines.size(); i++, y += 28) {
        SDL_Color textColor = darkMode ? SDL_Color{255, 255, 255, 255} : SDL_Color{0, 0, 0, 255};  // White text in dark mode, black in light mode

        for (const std::string& keyword : keywords) {
            if (lines[i].find(keyword) != std::string::npos) {
                textColor = {255, 0, 0, 255};  // Red keywords
            }
        }

        SDL_Surface* textSurface = TTF_RenderText_Solid(font, lines[i].c_str(), textColor);
        if (!textSurface) continue;
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

        SDL_Rect renderQuad = {100, y, textSurface->w, 28};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);

        // Cursor rendering
        if (i == currentLine) {
            int cursorX = 75;
            if (cursorPos > 0) {
                TTF_SizeText(font, lines[i].substr(0, cursorPos).c_str(), &cursorX, nullptr);
                cursorX += 100;
            }
            SDL_SetRenderDrawColor(renderer, darkMode ? 255 : 0, darkMode ? 255 : 0, darkMode ? 255 : 0, 255);  // White cursor in dark mode, black in light mode
            SDL_RenderDrawLine(renderer, cursorX, y + 5, cursorX, y + 25);
        }
    }

    renderErrorPanel();
    SDL_RenderPresent(renderer);
}



// --------- Save File Function ---------
void saveToFile(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    for (const auto& line : lines) {
        file << line << "\n";
    }
    file.close();
}

// --------- Load From File Function ---------
void loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return;
    }
    lines.clear();
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
    currentLine = 0;
    cursorPos = 0;
}

// --------- Push Undo State Function ---------
void pushUndoState() {
    undoStack.push(lines);
    while (!redoStack.empty()) redoStack.pop();
}

// --------- Undo Function ---------
void undo() {
    if (!undoStack.empty()) {
        redoStack.push(lines);
        lines = undoStack.top();
        undoStack.pop();
    }
}

// --------- Redo Function ---------
void redo() {
    if (!redoStack.empty()) {
        undoStack.push(lines);
        lines = redoStack.top();
        redoStack.pop();
    }
}

// --------- Error Panel Rendering Function ---------
void renderErrorPanel() {
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255); // Dark background
    SDL_Rect errorPanel = {0, SCREEN_HEIGHT - ERROR_PANEL_HEIGHT, SCREEN_WIDTH, ERROR_PANEL_HEIGHT};
    SDL_RenderFillRect(renderer, &errorPanel);

    SDL_Color errorColor = {255, 255, 255, 255};  // White text
    int y = SCREEN_HEIGHT - ERROR_PANEL_HEIGHT + 10;

    for (const auto& message : errorMessages) {
        SDL_Surface* textSurface = TTF_RenderText_Blended(font, message.c_str(), errorColor);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect renderQuad = {15, y, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &renderQuad);
        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
        y += 28;
    }
}


// --------- Compilation & Execution ---------
void compileAndRun() {

    saveToFile("temp_code.cpp");
    std::string compileCmd = "g++ temp_code.cpp -o " + OUTPUT_FILE + " 2> " + ERROR_FILE;

    errorMessages.clear();
    std::system(compileCmd.c_str());  // Compile

    std::ifstream errorFile(ERROR_FILE);
    std::string errorLine;
    while (std::getline(errorFile, errorLine)) {
        errorMessages.push_back("Error: " + errorLine);
    }
    errorFile.close();

    if (errorMessages.empty()) {
        std::string runCmd = OUTPUT_FILE + " < input.txt > program_output.txt 2>&1"; 
        std::ofstream inputFile("input.txt");
        inputFile << userInput;
        inputFile.close();

        std::system(runCmd.c_str());

        std::ifstream outputFile("program_output.txt");
        std::string outputLine;
        while (std::getline(outputFile, outputLine)) {
            errorMessages.push_back(" " + outputLine);
        }
        outputFile.close();
    }
}

// --------- Error Panel Rendering ---------
void showCompileErrors() {
    errorMessages.clear();
    std::ifstream errorFile(ERROR_FILE);
    std::string errorLine;
    while (std::getline(errorFile, errorLine)) {
        errorMessages.push_back(errorLine);
    }
    errorFile.close();
}

// --------- Input Handling ---------
void handleInput(SDL_Event& e) {
    if (e.type == SDL_QUIT) {
        quit = true;
    } else if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_UP) {
            if (currentLine > 0) {
                currentLine--;
                cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
            }
        } else if (e.key.keysym.sym == SDLK_DOWN) {
            if (currentLine < lines.size() - 1) {
                currentLine++;
                cursorPos = std::min(cursorPos, (int)lines[currentLine].size());
            }
        } else if (e.key.keysym.sym == SDLK_s && SDL_GetModState() & KMOD_CTRL) {
            saveToFile("saved_code.cpp");  // Ctrl+S saves the file
            std::cout << "File saved as saved_code.cpp" << std::endl;
        } else if (e.key.keysym.sym == SDLK_o && SDL_GetModState() & KMOD_CTRL) {
            loadFromFile("saved_code.cpp");  // Ctrl+O opens the file
            std::cout << "File loaded from saved_code.cpp" << std::endl;
        } else if (e.key.keysym.sym == SDLK_b && SDL_GetModState() & KMOD_CTRL) {
            std::cout << "Compiling and running the code..." << std::endl;
            compileAndRun();
        } else if (e.key.keysym.sym == SDLK_LEFT) {
            if (cursorPos > 0) {
                cursorPos--;
            } else if (currentLine > 0) {
                currentLine--;
                cursorPos = lines[currentLine].size();
            }
        } else if (e.key.keysym.sym == SDLK_RIGHT) {
            if (cursorPos < lines[currentLine].size()) {
                cursorPos++;
            } else if (currentLine < lines.size() - 1) {
                currentLine++;
                cursorPos = 0;
            }
        } else if (e.key.keysym.sym == SDLK_a && SDL_GetModState() & KMOD_CTRL) { 
            // ---- Select All (Ctrl + A) ----
            selectionStartLine = 0;
            selectionStartPos = 0;
            currentLine = lines.size() - 1;
            cursorPos = lines[currentLine].size();
            selecting = true;
        } else if (e.key.keysym.sym == SDLK_x && SDL_GetModState() & KMOD_CTRL) { 
            // ---- Cut (Ctrl + X) ----
            if (selecting && selectionStartLine != -1) {
                copySelection();
                pushUndoState();

                int startLine = std::min(selectionStartLine, currentLine);
                int endLine = std::max(selectionStartLine, currentLine);

                lines.erase(lines.begin() + startLine, lines.begin() + endLine + 1);
                
                currentLine = std::max(0, startLine);
                cursorPos = 0;
                selecting = false;
            }
        } else if (e.key.keysym.sym == SDLK_BACKSPACE) {
            if (cursorPos > 0) {
                pushUndoState();
                lines[currentLine].erase(cursorPos - 1, 1);
                cursorPos--;
            }
        }
        // ---- Auto-Complete Feature ----
        else if (e.key.keysym.sym == SDLK_LEFTPAREN) { // "("
            lines[currentLine].insert(cursorPos, "()");
            cursorPos++;
        } else if (e.key.keysym.sym == SDLK_RIGHTPAREN) { // ")"
            if (cursorPos < lines[currentLine].size() && lines[currentLine][cursorPos] == ')') {
                cursorPos++;
            } else {
                lines[currentLine].insert(cursorPos, ")");
                cursorPos++;
            }
        } else if (e.key.keysym.sym == SDLK_LEFTBRACKET) { // "["
            lines[currentLine].insert(cursorPos, "[]");
            cursorPos++;
        } else if (e.key.keysym.sym == SDLK_RIGHTBRACKET) { // "]"
            if (cursorPos < lines[currentLine].size() && lines[currentLine][cursorPos] == ']') {
                cursorPos++;
            } else {
                lines[currentLine].insert(cursorPos, "]");
                cursorPos++;
            }
        } else if (e.key.keysym.sym == SDLK_RETURN) {
            pushUndoState();
            lines.insert(lines.begin() + currentLine + 1, lines[currentLine].substr(cursorPos));
            lines[currentLine] = lines[currentLine].substr(0, cursorPos);
            currentLine++;
            cursorPos = 0;
        } else if (e.key.keysym.sym == SDLK_z && SDL_GetModState() & KMOD_CTRL) {
            undo();
        } else if (e.key.keysym.sym == SDLK_y && SDL_GetModState() & KMOD_CTRL) {
            redo();
        } else if (e.key.keysym.sym == SDLK_c && SDL_GetModState() & KMOD_CTRL) {
            SDL_SetClipboardText(lines[currentLine].c_str()); // Copy
        } else if (e.key.keysym.sym == SDLK_v && SDL_GetModState() & KMOD_CTRL) {
            if (SDL_HasClipboardText()) {
                char* text = SDL_GetClipboardText();
                lines[currentLine].insert(cursorPos, text);
                cursorPos += strlen(text);
                SDL_free(text);  // Paste
            }
        }
        
    } else if (e.type == SDL_TEXTINPUT) {
        char ch = e.text.text[0];

        if (ch == '{') {
            lines[currentLine].insert(cursorPos, "{}");
            cursorPos++;
        } else if (ch == '}') {
            if (cursorPos < lines[currentLine].size() && lines[currentLine][cursorPos] == '}') {
                cursorPos++;
            } else {
                lines[currentLine].insert(cursorPos, "}");
                cursorPos++;
            }
        } else {
            lines[currentLine].insert(cursorPos, e.text.text);
            cursorPos += strlen(e.text.text);
        }
    }

    handleMouseInput(e);
}


// --------- Main Loop ---------
int main(int argc, char* argv[])
{
    std::cout << "Initializing SDL..." << std::endl;
    if (!init()) {
        std::cerr << "Initialization failed! Exiting..." << std::endl;
        return -1;
    }
    std::cout << "SDL Initialized Successfully!" << std::endl;

    SDL_Event e;
    while (!quit) {
        std::cout << "Waiting for events..." << std::endl;
        while (SDL_PollEvent(&e)) {
            std::cout << "Event detected: " << e.type << std::endl;

            if (e.type == SDL_QUIT) {
                std::cout << "SDL_QUIT event detected! Exiting program..." << std::endl;
                quit = true;
            }

            handleInput(e);
        }

        renderTextEditor();

        SDL_Delay(100);
    }

    std::cout << "Exiting program..." << std::endl;
    close();
    return 0;
}
