#include <windows.h>
#include <vector>
#include <memory>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class Shape {
public:
    virtual void draw(HDC hDC) const = 0;
    virtual void drawFilled(HDC hDC) const = 0;
    virtual void moveFigure(int dx, int dy) = 0;
    virtual void rotate(double angle) = 0;
protected:
    POINT rotatePoint(const POINT& point, const POINT& center, double angle) const {
        double dx = static_cast<double>(point.x - center.x);
        double dy = static_cast<double>(point.y - center.y);
        double newX = center.x + (dx * cos(angle) - dy * sin(angle));
        double newY = center.y + (dx * sin(angle) + dy * cos(angle));
        return { static_cast<int>(newX), static_cast<int>(newY) };
    }
};

class Circle : public Shape {
private:
    POINT center;
    int radius;
public:
    Circle(POINT _center, int _radius) : center(_center), radius(_radius) {}
    void draw(HDC hDC) const override {
        Ellipse(hDC, center.x - radius, center.y - radius, center.x + radius, center.y + radius);
    }
    void drawFilled(HDC hDC) const override {
        HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
        draw(hDC);
        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);
    }
    void moveFigure(int dx, int dy) override {
        center.x += dx;
        center.y += dy;
    }
    void rotate(double) override {}
};

class Square : public Shape {
private:
    int centerX, centerY;
    int sideLength;
    double angle;
public:
    Square(int _centerX, int _centerY, int _sideLength) : centerX(_centerX), centerY(_centerY), sideLength(_sideLength), angle(0.0) {}
    void draw(HDC hDC) const override {
        POINT vertices[4];
        double cosAngle = cos(angle);
        double sinAngle = sin(angle);
        int halfLength = sideLength / 2;
        vertices[0] = { centerX + static_cast<int>(halfLength * cosAngle - halfLength * sinAngle), centerY + static_cast<int>(halfLength * sinAngle + halfLength * cosAngle) };
        vertices[1] = { centerX + static_cast<int>(halfLength * cosAngle + halfLength * sinAngle), centerY + static_cast<int>(halfLength * sinAngle - halfLength * cosAngle) };
        vertices[2] = { centerX + static_cast<int>(-halfLength * cosAngle + halfLength * sinAngle), centerY + static_cast<int>(-halfLength * sinAngle - halfLength * cosAngle) };
        vertices[3] = { centerX + static_cast<int>(-halfLength * cosAngle - halfLength * sinAngle), centerY + static_cast<int>(-halfLength * sinAngle + halfLength * cosAngle) };
        Polygon(hDC, vertices, 4);
    }
    void drawFilled(HDC hDC) const override {
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 255, 0));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
        draw(hDC);
        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);
    }
    void moveFigure(int dx, int dy) override {
        centerX += dx;
        centerY += dy;
    }
    void rotate(double rotateAngle) override {
        angle += rotateAngle;
    }
};

class Triangle : public Shape {
private:
    POINT points[3];
public:
    Triangle(POINT _p1, POINT _p2, POINT _p3) {
        points[0] = _p1;
        points[1] = _p2;
        points[2] = _p3;
    }
    void draw(HDC hDC) const override {
        Polyline(hDC, points, 3);
    }
    void drawFilled(HDC hDC) const override {
        HBRUSH hBrush = CreateSolidBrush(RGB(0, 0, 255));
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, hBrush);
        Polygon(hDC, points, 3);
        SelectObject(hDC, hOldBrush);
        DeleteObject(hBrush);
    }
    void moveFigure(int dx, int dy) override {
        for (int i = 0; i < 3; ++i) {
            points[i].x += dx;
            points[i].y += dy;
        }
    }
    void rotate(double rotateAngle) override {
        POINT center = { (points[0].x + points[1].x + points[2].x) / 3, (points[0].y + points[1].y + points[2].y) / 3 };
        for (int i = 0; i < 3; ++i) {
            points[i] = rotatePoint(points[i], center, rotateAngle);
        }
    }
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static std::vector<std::unique_ptr<Shape>> shapes;

    switch (uMsg) {
    case WM_CREATE:
        shapes.emplace_back(std::make_unique<Circle>(POINT{ 100, 150 }, 75)); 
        shapes.emplace_back(std::make_unique<Square>(250, 150, 100)); 
        shapes.emplace_back(std::make_unique<Triangle>(POINT{ 400, 50 }, POINT{ 350, 200 }, POINT{ 450, 200 })); 
        break;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hwnd, &ps);

        shapes[0]->drawFilled(hDC);
        shapes[1]->drawFilled(hDC);
        shapes[2]->drawFilled(hDC);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"ShapesWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, CLASS_NAME, L"Рисование фигур", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 500,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
