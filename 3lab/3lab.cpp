#include <windows.h> //работа с окнами и графикой
#include <vector> // для хранения точек кругов и крестов
#include <ctime> //для генерации случайных цветов
#include <shellapi.h> // Для CommandLineToArgvW
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#define DEFAULT_GRID_SIZE 50  // Размер сетки по умолчанию

// Прототипы функций
LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);  // Обработчик сообщений окна
void DrawGrid(HDC, int);  // Функция рисования сетки
void DrawCircle(HDC, int, int, int);  // Функция рисования круга
void DrawCross(HDC, int, int, int);  // Функция рисования креста

// Глобальные переменные
int cellSize = DEFAULT_GRID_SIZE;  // Размер ячейки сетки
std::vector<POINT> circles;  // Вектор для хранения точек кругов
std::vector<POINT> crosses;   // Вектор для хранения точек крестов
HBRUSH bgBrush = (HBRUSH)CreateSolidBrush(RGB(0, 0, 255)); // Задний фон окна (синий цвет)
COLORREF gridLineColor = RGB(255, 0, 0);  // Цвет линий сетки (красный)
int wndWidth = 320, wndHeight = 240;  // Размеры окна


//Стуктура конфига
struct Settings {
    int gridSize;
    int windowWidth;
    int windowHeight;
    COLORREF backgroundColor;
    COLORREF gridLineColor;
};

// Прототипы функций
void ReadSettingsFromMemoryMapping(Settings& settings);
void WriteSettingsToMemoryMapping(const Settings& settings);
void ReadSettingsFromFile(Settings& settings);
void WriteSettingsToFile(const Settings& settings);
void ReadSettingsFromStream(Settings& settings);
void WriteSettingsToStream(const Settings& settings);
void ReadSettingsFromWinAPI(Settings& settings);
void WriteSettingsToWinAPI(const Settings& settings);


Settings settings;
// Точка входа в программу
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // 1️⃣ Устанавливаем значения по умолчанию
    settings.gridSize = DEFAULT_GRID_SIZE;
    settings.windowWidth = 320;
    settings.windowHeight = 240;
    settings.backgroundColor = RGB(0, 0, 255);
    settings.gridLineColor = RGB(255, 0, 0);

    int method = 1; // Метод по умолчанию

    // 2️⃣ Парсим аргументы командной строки
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv) {
        if (argc > 2) {
            // Если есть аргументы, парсим метод
            int cmdMethod = _wtoi(argv[2]);
            if (cmdMethod >= 1 && cmdMethod <= 4) {
                method = cmdMethod;  // Метод корректен
            }
            else {
                // Если метод некорректен, используем значение по умолчанию
                MessageBox(NULL, L"Некорректный метод. Используется метод по умолчанию (1).", L"Ошибка", MB_OK | MB_ICONWARNING);
            }
        }
        else {
            // Если аргументов нет, используем значения по умолчанию
            MessageBox(NULL, L"Недостаточно аргументов, используем значения по умолчанию (размер ячеек 50, метод 1).", L"Информация", MB_OK | MB_ICONINFORMATION);
        }

        if (argc > 1) {
            // Если есть аргумент, парсим размер сетки
            int cmdGridSize = _wtoi(argv[1]);
            if (cmdGridSize > 0) {
                settings.gridSize = cmdGridSize;
            }
            else {
                // Если размер сетки некорректен, используем значение по умолчанию
                MessageBox(NULL, L"Некорректный размер сетки. Используется значение по умолчанию.", L"Ошибка", MB_OK | MB_ICONWARNING);
            }
        }

        LocalFree(argv);
    }
    else {
        // Если не удалось получить аргументы, используем значения по умолчанию
        MessageBox(NULL, L"Не удалось получить аргументы командной строки. Используются настройки по умолчанию.", L"Ошибка", MB_OK | MB_ICONWARNING);
    
    }

    cellSize = settings.gridSize;
    // 3️⃣ Читаем настройки в зависимости от метода
    switch (method) {
    case 1: ReadSettingsFromMemoryMapping(settings); break;
    case 2: ReadSettingsFromFile(settings); break;
    case 3: ReadSettingsFromStream(settings); break;
    case 4: ReadSettingsFromWinAPI(settings); break;
    default: ReadSettingsFromMemoryMapping(settings); break;
    }

    // 4️⃣ Применяем настройки после загрузки
    wndWidth = settings.windowWidth;
    wndHeight = settings.windowHeight;
    bgBrush = CreateSolidBrush(settings.backgroundColor);
    gridLineColor = settings.gridLineColor;

    // 5️⃣ Создание окна
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GridAppClass";
    wc.hbrBackground = bgBrush;
    RegisterClass(&wc);

    /*HWND hwnd = CreateWindowEx(
        0, L"GridAppClass", L"Circle & Crosses",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, wndWidth, wndHeight,
        NULL, NULL, hInstance, NULL
    );*/
    RECT rc = { 0, 0, wndWidth, wndHeight };
    AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE); // Учитываем рамки окна
    int adjustedWidth = rc.right - rc.left;
    int adjustedHeight = rc.bottom - rc.top;

    HWND hwnd = CreateWindowEx(
        0, L"GridAppClass", L"Circle & Crosses",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, adjustedWidth, adjustedHeight,
        NULL, NULL, hInstance, NULL
    );


    // 6️⃣ Основной цикл обработки сообщений
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 7️⃣ Перед выходом обновляем `settings`
    settings.gridSize = cellSize;
    settings.windowWidth = wndWidth;
    settings.windowHeight = wndHeight;
    settings.gridLineColor = gridLineColor;

    // 8️⃣ Записываем настройки перед выходом
    switch (method) {
    case 1: WriteSettingsToMemoryMapping(settings); break;
    case 2: WriteSettingsToFile(settings); break;
    case 3: WriteSettingsToStream(settings); break;
    case 4: WriteSettingsToWinAPI(settings); break;
    default: WriteSettingsToMemoryMapping(settings); break;
    }

    return 0;
}



// Обработчик сообщений окна
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg) {

        // Обработка нажатия клавиш
    case WM_KEYDOWN: {
        // Если нажата клавиша ESC или комбинация Ctrl+Q, выходим из программы
        if (wParam == VK_ESCAPE || (GetKeyState(VK_CONTROL) & 0x8000 && wParam == 'Q')) {
            PostQuitMessage(0);  // Отправляем сообщение о завершении программы
        }
        // Если нажата клавиша Enter, меняем цвет фона окна на случайный
        else if (wParam == VK_RETURN) {
            srand((unsigned)time(0));
            DeleteObject(bgBrush);

            COLORREF newColor = RGB(rand() % 256, rand() % 256, rand() % 256);
            bgBrush = CreateSolidBrush(newColor);
            SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)bgBrush);

            settings.backgroundColor = newColor;  // Сразу сохраняем новый цвет

            InvalidateRect(hwnd, NULL, TRUE);
        }

        // Если нажата комбинация Shift + C, открываем Блокнот
        else if ((GetKeyState(VK_SHIFT) & 0x8000) && wParam == 'C') {
            ShellExecute(NULL, L"open", L"notepad.exe", NULL, NULL, SW_SHOWNORMAL);  // Открываем notepad
        }
        return 0;
    }

    // Обработка прокрутки колесика мыши
    case WM_MOUSEWHEEL: {
        int wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);  // Получаем направление прокрутки
        // Изменяем цвет сетки в зависимости от прокрутки
        int r = GetRValue(gridLineColor) + (wheelDelta > 0 ? 5 : -5);
        int g = GetGValue(gridLineColor) + (wheelDelta > 0 ? 5 : -5);
        int b = GetBValue(gridLineColor) + (wheelDelta > 0 ? 5 : -5);

        // Ограничиваем значения от 0 до 255 для каждого компонента цвета
        r = max(0, min(255, r));
        g = max(0, min(255, g));
        b = max(0, min(255, b));

        gridLineColor = RGB(r, g, b);  // Обновляем цвет сетки
        InvalidateRect(hwnd, NULL, TRUE);  // Перерисовываем окно
        return 0;
    }

    case WM_PAINT: {  // Обработка перерисовки окна
        PAINTSTRUCT ps;  // Структура для хранения информации о рисовании
        HDC hdc = BeginPaint(hwnd, &ps);  // Получаем контекст устройства для рисования

        // Рисуем сетку
        DrawGrid(hdc, cellSize);

        // Рисуем все круги, которые были добавлены
        for (const auto& point : circles) {
            DrawCircle(hdc, point.x, point.y, cellSize / 2);  // Рисуем каждый круг
        }
        // Рисуем все кресты
        for (const auto& point : crosses) {
            DrawCross(hdc, point.x, point.y, cellSize / 2);  // Рисуем каждый крест
        }

        EndPaint(hwnd, &ps);  // Завершаем рисование
        return 0;
    }

                 // Обработка кликов левой кнопкой мыши
    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam) / cellSize * cellSize + cellSize / 2;  // Вычисляем центр клетки по оси X
        int y = HIWORD(lParam) / cellSize * cellSize + cellSize / 2;  // Вычисляем центр клетки по оси Y

        // Проверяем, не стоит ли уже крест в этой клетке
        for (const auto& point : crosses) {
            if (point.x == x && point.y == y) return 0;
        }

        circles.push_back({ x, y });  // Добавляем новый круг в список
        InvalidateRect(hwnd, NULL, TRUE);  // Перерисовываем окно
        return 0;
    }

     // Обработка кликов правой кнопкой мыши
    case WM_RBUTTONDOWN: {
        int x = LOWORD(lParam) / cellSize * cellSize + cellSize / 2;  // Вычисляем центр клетки по оси X
        int y = HIWORD(lParam) / cellSize * cellSize + cellSize / 2;  // Вычисляем центр клетки по оси Y

        // Проверяем, не стоит ли уже круг в этой клетке
        for (const auto& point : circles) {
            if (point.x == x && point.y == y) return 0;
        }

        crosses.push_back({ x, y });  // Добавляем новый крест в список
        InvalidateRect(hwnd, NULL, TRUE);  // Перерисовываем окно
        return 0;
    }
    case WM_SIZE: {
        //if (wndWidth != 320 && wndHeight != 240) {
        //    wndWidth = LOWORD(lParam);   // Новый размер ширины окна
        //    wndHeight = HIWORD(lParam);  // Новый размер высоты окна
        //}
        //return 0;
        wndWidth = LOWORD(lParam);   // Новый размер ширины окна
        wndHeight = HIWORD(lParam);  // Новый размер высоты окна
        return 0;
    }
    case WM_DESTROY:  // Обработка закрытия окна
        DeleteObject(bgBrush);  // Удаляем кисть фона
        PostQuitMessage(0);  // Отправляем сообщение о завершении программы
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);  // Обрабатываем остальные сообщения по умолчанию
    }
}

// Функция для рисования сетки
void DrawGrid(HDC hdc, int size) {
    HPEN pen = CreatePen(PS_SOLID, 1, gridLineColor);  // Создаем перо для рисования
    SelectObject(hdc, pen);  // Выбираем перо для рисования в контекст устройства

    RECT rect;
    GetClientRect(WindowFromDC(hdc), &rect);  // Получаем размеры окна для рисования сетки

    // Рисуем вертикальные линии
    for (int x = 0; x < rect.right; x += size) {
        MoveToEx(hdc, x, 0, NULL);  // Перемещаем перо в начало линии
        LineTo(hdc, x, rect.bottom);  // Рисуем вертикальную линию до низа окна
    }

    // Рисуем горизонтальные линии
    for (int y = 0; y < rect.bottom; y += size) {
        MoveToEx(hdc, 0, y, NULL);  // Перемещаем перо в начало линии
        LineTo(hdc, rect.right, y);  // Рисуем горизонтальную линию до конца окна
    }

    DeleteObject(pen);  // Удаляем перо
}

// Функция для рисования круга
void DrawCircle(HDC hdc, int x, int y, int radius) {
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(0, 255, 0));  // Создаем перо для рисования круга (зеленое)
    SelectObject(hdc, pen);  // Выбираем перо для рисования в контекст устройства

    HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);  // Используем прозрачную кисть для круга
    SelectObject(hdc, brush);

    Ellipse(hdc, x - radius, y - radius, x + radius, y + radius);  // Рисуем круг по заданным координатам и радиусу

    DeleteObject(pen);  // Удаляем перо
}

// Функция для рисования креста
void DrawCross(HDC hdc, int x, int y, int size) {
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 255, 0));  // Создаем перо для рисования креста (желтое)
    SelectObject(hdc, pen);  // Выбираем перо для рисования в контекст устройства

    // Рисуем первую линию креста
    MoveToEx(hdc, x - size / 2, y - size / 2, NULL);
    LineTo(hdc, x + size / 2, y + size / 2);

    // Рисуем вторую линию креста
    MoveToEx(hdc, x + size / 2, y - size / 2, NULL);
    LineTo(hdc, x - size / 2, y + size / 2);

    DeleteObject(pen);  // Удаляем перо
}

//Метод 1: Отображение файлов на память (CreateFileMapping / MapViewOfFile)
void ReadSettingsFromMemoryMapping(Settings& settings) {
    HANDLE hFile = CreateFile(TEXT("settings.ini"), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE || fileSize == 0) {
        CloseHandle(hFile);
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    HANDLE hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, fileSize, NULL);
    if (!hMapping) {
        CloseHandle(hFile);
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    LPVOID pData = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, fileSize);
    if (!pData) {
        CloseHandle(hMapping);
        CloseHandle(hFile);
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    std::string content(static_cast<char*>(pData), fileSize);
    std::stringstream ss(content);
    std::string line;

    while (std::getline(ss, line)) {
        if (line.find("GridSize=") == 0) {
            settings.gridSize = std::stoi(line.substr(9));
        }
        else if (line.find("WindowWidth=") == 0) {
            settings.windowWidth = std::stoi(line.substr(12));
        }
        else if (line.find("WindowHeight=") == 0) {
            settings.windowHeight = std::stoi(line.substr(13));
        }
        else if (line.find("BackgroundColor=") == 0) {
            int r, g, b;
            if (sscanf_s(line.substr(16).c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                settings.backgroundColor = RGB(r, g, b);
            }
        }
        else if (line.find("GridLineColor=") == 0) {
            int r, g, b;
            if (sscanf_s(line.substr(14).c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                settings.gridLineColor = RGB(r, g, b);
            }
        }
    }

    UnmapViewOfFile(pData);
    CloseHandle(hMapping);
    CloseHandle(hFile);
}

void WriteSettingsToMemoryMapping(const Settings& settings) {
    std::string data = "[Settings]\n";
    data += "GridSize=" + std::to_string(settings.gridSize) + "\n";
    data += "WindowWidth=" + std::to_string(settings.windowWidth) + "\n";
    data += "WindowHeight=" + std::to_string(settings.windowHeight) + "\n";
    data += "BackgroundColor=" + std::to_string(GetRValue(settings.backgroundColor)) + "," +
        std::to_string(GetGValue(settings.backgroundColor)) + "," +
        std::to_string(GetBValue(settings.backgroundColor)) + "\n";
    data += "GridLineColor=" + std::to_string(GetRValue(settings.gridLineColor)) + "," +
        std::to_string(GetGValue(settings.gridLineColor)) + "," +
        std::to_string(GetBValue(settings.gridLineColor)) + "\n";

    DWORD dataSize = static_cast<DWORD>(data.size());

    // 1. Открываем файл с очисткой перед записью
    HANDLE hFile = CreateFile(TEXT("settings.ini"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        printf("Ошибка: не удалось открыть файл settings.ini. Код ошибки: %d\n", GetLastError());
        return;
    }

    // 2. Записываем данные в файл
    DWORD bytesWritten;
    BOOL success = WriteFile(hFile, data.c_str(), dataSize, &bytesWritten, NULL);
    if (!success || bytesWritten != dataSize) {
        printf("Ошибка: не удалось записать данные в файл. Код ошибки: %d\n", GetLastError());
    }

    // 3. Закрываем файл
    CloseHandle(hFile);

}



//Метод 2: Файловые переменные (fopen, fread, fwrite, fclose)
void ReadSettingsFromFile(Settings& settings) {
    FILE* file = nullptr;
    if (fopen_s(&file, "settings.ini", "r") != 0 || !file) {
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, "GridSize=")) {
            sscanf_s(line, "GridSize=%d", &settings.gridSize);
        }
        else if (strstr(line, "WindowWidth=")) {
            sscanf_s(line, "WindowWidth=%d", &settings.windowWidth);
        }
        else if (strstr(line, "WindowHeight=")) {
            sscanf_s(line, "WindowHeight=%d", &settings.windowHeight);
        }
        else if (strstr(line, "BackgroundColor=")) {
            int r, g, b;
            sscanf_s(line, "BackgroundColor=%d,%d,%d", &r, &g, &b);
            settings.backgroundColor = RGB(r, g, b);
        }
        else if (strstr(line, "GridLineColor=")) {
            int r, g, b;
            sscanf_s(line, "GridLineColor=%d,%d,%d", &r, &g, &b);
            settings.gridLineColor = RGB(r, g, b);
        }
    }

    fclose(file);
}

void WriteSettingsToFile(const Settings& settings) {
    FILE* file = nullptr;
    if (fopen_s(&file, "settings.ini", "w") != 0) {
        return;
    }

    fprintf(file, "[Settings]\n");
    fprintf(file, "GridSize=%d\n", settings.gridSize);
    fprintf(file, "WindowWidth=%d\n", settings.windowWidth);
    fprintf(file, "WindowHeight=%d\n", settings.windowHeight);
    fprintf(file, "BackgroundColor=%d,%d,%d\n", GetRValue(settings.backgroundColor),
        GetGValue(settings.backgroundColor), GetBValue(settings.backgroundColor));
    fprintf(file, "GridLineColor=%d,%d,%d\n", GetRValue(settings.gridLineColor),
        GetGValue(settings.gridLineColor), GetBValue(settings.gridLineColor));

    fclose(file);
}


//Метод 3: Потоки ввода-вывода (библиотека fstream в языке C++, объекты файловых потоков ofstream и ifstream)
void ReadSettingsFromStream(Settings& settings) {
    std::ifstream file("settings.ini");
    if (!file.is_open()) {
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.find("GridSize=") == 0) {
            settings.gridSize = std::stoi(line.substr(9));
        }
        else if (line.find("WindowWidth=") == 0) {
            settings.windowWidth = std::stoi(line.substr(12));
        }
        else if (line.find("WindowHeight=") == 0) {
            settings.windowHeight = std::stoi(line.substr(13));
        }
        else if (line.find("BackgroundColor=") == 0) {
            std::string color = line.substr(16);
            int r, g, b;
            if (sscanf_s(color.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                settings.backgroundColor = RGB(r, g, b);
            }
        }
        else if (line.find("GridLineColor=") == 0) {
            std::string color = line.substr(14);
            int r, g, b;
            if (sscanf_s(color.c_str(), "%d,%d,%d", &r, &g, &b) == 3) {
                settings.gridLineColor = RGB(r, g, b);
            }
        }
    }

    file.close();
}

void WriteSettingsToStream(const Settings& settings) {
    std::ofstream file("settings.ini");
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл settings.ini для записи.\n";
        return;
    }

    file << "[Settings]\n";
    file << "GridSize=" << settings.gridSize << "\n";
    file << "WindowWidth=" << settings.windowWidth << "\n";
    file << "WindowHeight=" << settings.windowHeight << "\n";
    file << "BackgroundColor="
        << static_cast<int>(GetRValue(settings.backgroundColor)) << ","
        << static_cast<int>(GetGValue(settings.backgroundColor)) << ","
        << static_cast<int>(GetBValue(settings.backgroundColor)) << "\n";
    file << "GridLineColor="
        << static_cast<int>(GetRValue(settings.gridLineColor)) << ","
        << static_cast<int>(GetGValue(settings.gridLineColor)) << ","
        << static_cast<int>(GetBValue(settings.gridLineColor)) << "\n";

    if (!file) {
        std::cerr << "Ошибка записи в файл settings.ini\n";
    }

    file.close();
}

//Метод 4: Файловые функции WinAPI
void ReadSettingsFromWinAPI(Settings& settings) {
    HANDLE hFile = CreateFile(L"settings.ini", GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    DWORD fileSize = GetFileSize(hFile, NULL);
    if (fileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    char* buffer = new char[fileSize + 1];
    DWORD bytesRead;
    if (!ReadFile(hFile, buffer, fileSize, &bytesRead, NULL)) {
        delete[] buffer;
        CloseHandle(hFile);
        settings = { DEFAULT_GRID_SIZE, 320, 240, RGB(0, 0, 255), RGB(255, 0, 0) };
        return;
    }

    buffer[fileSize] = '\0';

    std::stringstream ss(buffer);
    std::string line;
    while (std::getline(ss, line)) {
        if (line.find("GridSize=") == 0) {
            settings.gridSize = std::stoi(line.substr(9));
        }
        else if (line.find("WindowWidth=") == 0) {
            settings.windowWidth = std::stoi(line.substr(12));
        }
        else if (line.find("WindowHeight=") == 0) {
            settings.windowHeight = std::stoi(line.substr(13));
        }
        else if (line.find("BackgroundColor=") == 0) {
            std::string color = line.substr(16);
            int r, g, b;
            sscanf_s(color.c_str(), "%d,%d,%d", &r, &g, &b);
            settings.backgroundColor = RGB(r, g, b);
        }
        else if (line.find("GridLineColor=") == 0) {
            std::string color = line.substr(14);
            int r, g, b;
            sscanf_s(color.c_str(), "%d,%d,%d", &r, &g, &b);
            settings.gridLineColor = RGB(r, g, b);
        }
    }

    delete[] buffer;
    CloseHandle(hFile);
}

void WriteSettingsToWinAPI(const Settings& settings) {
    HANDLE hFile = CreateFile(L"settings.ini", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return;

    std::string data = "[Settings]\n";
    data += "GridSize=" + std::to_string(settings.gridSize) + "\n";
    data += "WindowWidth=" + std::to_string(settings.windowWidth) + "\n";
    data += "WindowHeight=" + std::to_string(settings.windowHeight) + "\n";
    data += "BackgroundColor=" + std::to_string(GetRValue(settings.backgroundColor)) + "," +
        std::to_string(GetGValue(settings.backgroundColor)) + "," +
        std::to_string(GetBValue(settings.backgroundColor)) + "\n";
    data += "GridLineColor=" + std::to_string(GetRValue(settings.gridLineColor)) + "," +
        std::to_string(GetGValue(settings.gridLineColor)) + "," +
        std::to_string(GetBValue(settings.gridLineColor)) + "\n";

    DWORD bytesWritten;
    WriteFile(hFile, data.c_str(), data.size(), &bytesWritten, NULL);

    CloseHandle(hFile);
}
//Текст