#ifndef GUI_H
#define GUI_H

#include "raygui.h"
#include "raylib.h"
#include "raymath.h"
#include "http_parser.h"
#include "common_lib.h"

class Button {
protected:
    Rectangle bounds;         // Kích thước và vị trí của button
    Color baseColor;          // Màu cơ bản
    Color hoverColor;         // Màu khi hover
    Color pressColor;         // Màu khi nhấn
    Color textColor;          // Màu của text
    std::string text;         // Nội dung text hiển thị trên button
    int fontSize;             // Kích thước font chữ
    Font font;                // Font tùy chỉnh
    bool isHovered;           // Trạng thái hover
    bool isPressed;           // Trạng thái nhấn
    float cornerRadius;       // Bán kính bo góc

public:
    // Constructor
    Button(float x, float y, float width, float height, const std::string& buttonText, int textSize = 20, 
           float radius = 10.0f, Font customFont = GetFontDefault(), Color base = GRAY, Color hover = LIGHTGRAY, Color press = DARKGRAY, Color textCol = WHITE)
        : bounds{ x, y, width, height }, text(buttonText), fontSize(textSize), cornerRadius(radius),
          font(customFont), baseColor(base), hoverColor(hover), pressColor(press), textColor(textCol),
          isHovered(false), isPressed(false) {}

    // Kiểm tra trạng thái hover hoặc nhấn chuột
    void Update() {
        Vector2 mousePoint = GetMousePosition();
        isHovered = CheckCollisionPointRec(mousePoint, bounds); // Kiểm tra chuột có hover lên button không
        isPressed = isHovered && IsMouseButtonDown(MOUSE_LEFT_BUTTON); // Kiểm tra nhấn chuột trái
    }

    // Vẽ button lên màn hình (với bo góc)
    void Draw() {
        // Chọn màu dựa trên trạng thái
        Color currentColor = baseColor;
        if (isPressed) {
            currentColor = pressColor;
        } else if (isHovered) {
            currentColor = hoverColor;
        }

        // Vẽ hình chữ nhật bo góc của button
        DrawRectangleRounded(bounds, cornerRadius / bounds.width, 6, currentColor);

        // Vẽ viền button để làm đẹp hơn (tùy chọn)
        DrawRectangleRoundedLines(bounds, cornerRadius / bounds.width, 6, BLACK);

        // Vẽ text lên button (căn giữa)
        Vector2 textSize = MeasureTextEx(font, text.c_str(), fontSize, 1);
        float textX = bounds.x + (bounds.width - textSize.x) / 2;
        float textY = bounds.y + (bounds.height - textSize.y) / 2;
        DrawTextEx(font, text.c_str(), { textX, textY }, fontSize, 1, textColor);
    }

    // Kiểm tra xem button có được nhấn không (dùng để thực hiện hành động)
    bool IsClicked() const {
        return isHovered && IsMouseButtonReleased(MOUSE_LEFT_BUTTON);
    }

    // Setter cho text (nếu muốn cập nhật nội dung button)
    void SetText(const std::string& newText) {
        text = newText;
    }

    // Setter cho bán kính bo góc
    void SetCornerRadius(float radius) {
        cornerRadius = radius;
    }

    // Setter cho font
    void SetFont(Font customFont) {
        font = customFont;
    }
};


class ToggleButton : public Button {
private:
    std::string labelOn;   // Label khi button ở trạng thái "On"
    std::string labelOff;  // Label khi button ở trạng thái "Off"
    bool isOn;             // Trạng thái hiện tại của button

public:
    // Constructor
    ToggleButton(float x, float y, float width, float height, 
                 const std::string& textOn, const std::string& textOff, int textSize = 20, 
                 float radius = 10.0f, Font customFont = GetFontDefault(), Color base = GRAY, Color hover = LIGHTGRAY, Color press = DARKGRAY, Color textCol = WHITE)
        : Button(x, y, width, height, textOff, textSize, radius, customFont, base, hover, press, textCol), // Khởi tạo class cha
          labelOn(textOn), labelOff(textOff), isOn(false) {}

    // Ghi đè phương thức Update và xử lý trạng thái chuyển đổi
    void Update() {
        Button::Update(); // Gọi hàm Update từ class cha để xử lý hover và nhấn chuột

        // Nếu button được nhấn, chuyển đổi trạng thái
        if (IsClicked()) {
            isOn = !isOn; // Đổi trạng thái
            SetText(isOn ? labelOn : labelOff); // Cập nhật label dựa trên trạng thái
        }
    }

    // Getter trạng thái
    bool GetState() const {
        return isOn;
    }
};


class TextBox {
private:
    Rectangle bounds;          // Kích thước và vị trí của TextBox
    Color borderColor;         // Màu viền khung
    Color backgroundColor;     // Màu nền
    Color textColor;           // Màu chữ
    Font font;                 // Font chữ
    int fontSize;              // Kích thước font
    float cornerRadius;        // Bán kính bo góc
    std::string text;          // Nội dung văn bản hiển thị
    float scrollOffset;        // Vị trí thanh cuộn
    float lineSpacing;         // Khoảng cách giữa các dòng
    int visibleLines;          // Số dòng có thể hiển thị trong khung
    int totalLines;            // Tổng số dòng văn bản
    std::vector<std::string> wrappedText; // Danh sách các dòng đã được "word-wrap"
    bool isDraggingScrollBar;  // Trạng thái kéo thanh cuộn
    float dragStartY;          // Điểm bắt đầu kéo chuột


    // Phương thức nội bộ để "word-wrap" văn bản
    void WrapText() {
        wrappedText.clear();
        std::istringstream stream(text);
        std::string line;
        float maxWidth = bounds.width - 20; // Trừ khoảng cách padding 10px mỗi bên

        while (std::getline(stream, line, '\n')) {
            std::string word;
            std::istringstream wordStream(line);
            std::string currentLine;

            while (wordStream >> word) {
                std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
                Vector2 textSize = MeasureTextEx(font, testLine.c_str(), fontSize, 1);
                if (textSize.x > maxWidth) {
                    wrappedText.push_back(currentLine);
                    currentLine = word;
                } else {
                    currentLine = testLine;
                }
            }
            wrappedText.push_back(currentLine);
        }

        totalLines = wrappedText.size();
        visibleLines = bounds.height / (fontSize + lineSpacing);
    }

public:
    // Constructor
    TextBox(float x, float y, float width, float height, const std::string& content,
            Font customFont = GetFontDefault(), int textSize = 20, float radius = 10.0f,
            Color bg = LIGHTGRAY, Color border = GRAY, Color textCol = BLACK, float spacing = 5.0f)
        : bounds{ x, y, width, height }, text(content), font(customFont), fontSize(textSize),
          cornerRadius(radius), backgroundColor(bg), borderColor(border), textColor(textCol),
          scrollOffset(0), lineSpacing(spacing), totalLines(0), visibleLines(0),
          isDraggingScrollBar(false), dragStartY(0.0f) {
        WrapText();
    }

    // Cập nhật logic TextBox
    void Update() {
        // Xử lý con lăn chuột
        if (CheckCollisionPointRec(GetMousePosition(), bounds)) {
            float mouseWheelMove = GetMouseWheelMove();
            if (mouseWheelMove != 0.0f) {
                float scrollSpeed = 20.0f; // Tốc độ cuộn mỗi lần con lăn
                scrollOffset -= mouseWheelMove * scrollSpeed;
            }
            scrollOffset = Clamp(scrollOffset, 0.0f, (totalLines - visibleLines) * (fontSize + lineSpacing));
        }

        // Giới hạn thanh cuộn trong khoảng cho phép

        // Xử lý kéo thanh cuộn
        if (totalLines > visibleLines) {
            float scrollBarHeight = bounds.height * visibleLines / totalLines;
            Rectangle scrollBar = {
                bounds.x + bounds.width - 5,
                bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / ((totalLines - visibleLines) * (fontSize + lineSpacing))),
                5,
                scrollBarHeight
            };

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), scrollBar)) {
                isDraggingScrollBar = true;
                dragStartY = GetMousePosition().y;
            }

            if (isDraggingScrollBar) {
                if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                    float deltaY = GetMousePosition().y - dragStartY;
                    dragStartY = GetMousePosition().y;

                    // Cập nhật scrollOffset dựa trên deltaY
                    float maxOffset = (totalLines - visibleLines) * (fontSize + lineSpacing);
                    scrollOffset += deltaY * (maxOffset / (bounds.height - scrollBarHeight));
                    scrollOffset = Clamp(scrollOffset, 0.0f, maxOffset);
                } else {
                    isDraggingScrollBar = false;
                }
            }
        }
    }

    // Vẽ TextBox lên màn hình
    void Draw() {
        // Vẽ khung bo góc
        DrawRectangleRounded(bounds, cornerRadius / bounds.width, 6, backgroundColor);
        DrawRectangleRoundedLines(bounds, cornerRadius / bounds.width, 6, borderColor);

        // Bắt đầu vùng cắt (scissor mode)
        BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);

        // Vẽ text (chỉ vẽ những dòng trong phạm vi hiển thị)
        float padding = 10.0f;
        float y = bounds.y + padding - scrollOffset;
        for (int i = 0; i < totalLines; i++) {
            if (y + fontSize > bounds.y + bounds.height) break; // Dừng nếu dòng vượt ra ngoài khung
            if (y + fontSize >= bounds.y) {                     // Chỉ vẽ nếu dòng nằm trong khung
                DrawTextEx(font, wrappedText[i].c_str(), { bounds.x + padding, y }, fontSize, 1, textColor);
            }
            y += fontSize + lineSpacing;
        }

        // Kết thúc vùng cắt
        EndScissorMode();

        // Vẽ thanh cuộn nếu cần
        if (totalLines > visibleLines) {
            float scrollBarHeight = bounds.height * visibleLines / totalLines;
            float scrollBarY = bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / ((totalLines - visibleLines) * (fontSize + lineSpacing)));

            DrawRectangle(bounds.x + bounds.width - 5, bounds.y, 5, bounds.height, DARKGRAY); // Nền thanh cuộn
            DrawRectangle(bounds.x + bounds.width - 5, scrollBarY, 5, scrollBarHeight, BLACK); // Thanh cuộn
        }
    }

    // Setter nội dung text
    void SetText(const std::string& newText) {
        text = newText;
        scrollOffset = 0;
        WrapText();
    }

    // Getter nội dung text
    std::string GetText() const {
        return text;
    }
};


class Popup {
private:
    Rectangle bounds;            // Vị trí và kích thước của Popup
    Color backgroundColor;       // Màu nền
    Color borderColor;           // Màu viền
    Color titleBarColor;         // Màu thanh tiêu đề
    Color textColor;             // Màu chữ
    std::string title;           // Tiêu đề popup
    TextBox contentBox;          // TextBox hiển thị nội dung
    float cornerRadius;          // Bán kính bo góc
    bool isDragging;             // Trạng thái kéo popup
    Vector2 dragOffset;          // Offset kéo chuột
    bool isVisible;              // Trạng thái hiển thị popup
    Font font;
public:
    // Constructor
    Popup(float x, float y, float width, float height, const std::string& titleText, const std::string& content,
          Font customFont = GetFontDefault(), int textSize = 20, float radius = 10.0f,
          Color bg = LIGHTGRAY, Color border = GRAY, Color titleColor = DARKGRAY, Color textCol = BLACK)
        : bounds{ x, y, width, height }, title(titleText), cornerRadius(radius), 
          backgroundColor(bg), borderColor(border), titleBarColor(titleColor), textColor(textCol), 
          isDragging(false), isVisible(true),
          contentBox(x + 10, y + 40, width - 20, height - 50, content, customFont, textSize, radius - 2, bg, border, textCol),
          font(customFont) {}

    // Cập nhật logic Popup
    bool Update() {
        if (!isVisible) return false;

        Vector2 mousePos = GetMousePosition();

        // Xử lý kéo di chuyển popup
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePos, { bounds.x, bounds.y, bounds.width, 30 })) {
            isDragging = true;
            dragOffset = { mousePos.x - bounds.x, mousePos.y - bounds.y };
        }

        if (isDragging) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                bounds.x = mousePos.x - dragOffset.x;
                bounds.y = mousePos.y - dragOffset.y;

                // Di chuyển TextBox cùng với Popup
                contentBox = TextBox(bounds.x + 10, bounds.y + 40, bounds.width - 20, bounds.height - 50, contentBox.GetText());
            } else {
                isDragging = false;
            }
        }

        // Phím tắt đóng popup (ESC hoặc X góc phải)
        if (IsKeyPressed(KEY_ESCAPE) || (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) &&
                                         CheckCollisionPointRec(mousePos, { bounds.x + bounds.width - 25, bounds.y + 5, 20, 20 }))) {
            isVisible = false;
            return false;
        }

        // Cập nhật TextBox
        contentBox.Update();
        return true;
    }

    // Vẽ Popup lên màn hình
    void Draw() {
        if (!isVisible) return;

        // Vẽ nền Popup
        DrawRectangleRounded(bounds, cornerRadius / bounds.width, 6, backgroundColor);
        DrawRectangleRoundedLines(bounds, cornerRadius / bounds.width, 6, borderColor);

        // Vẽ thanh tiêu đề
        DrawRectangleRounded({ bounds.x, bounds.y, bounds.width, 30 }, cornerRadius / bounds.width, 6, titleBarColor);
        DrawTextEx(font, title.c_str(), {bounds.x + 10, bounds.y + 7}, 20, 6, textColor);

        // Vẽ nút đóng (X)
        DrawTextEx(font, "X", {bounds.x + bounds.width - 20, bounds.y + 5}, 20, 6, RED);

        // Vẽ TextBox nội dung
        contentBox.Draw();
    }

    // Hiển thị hoặc ẩn Popup
    void SetVisible(bool visible) {
        isVisible = visible;
    }

    // Kiểm tra trạng thái hiển thị
    bool IsVisible() const {
        return isVisible;
    }

    // Đặt nội dung Popup
    void SetContent(const std::string& content) {
        contentBox.SetText(content);
    }
    
    Rectangle GetBounds() const {
        return bounds;
    }
};


// class Popup {
// private:
//     Rectangle bounds;          // Kích thước và vị trí của popup
//     std::string title;         // Tiêu đề
//     std::string content;       // Nội dung chi tiết
//     Font font;                 // Font chữ
//     int fontSize;              // Kích thước font
//     Color borderColor;         // Màu viền
//     Color backgroundColor;     // Màu nền
//     Color textColor;           // Màu chữ
//     float cornerRadius;        // Bán kính bo góc
//     float scrollOffset;        // Vị trí thanh cuộn
//     float lineSpacing;         // Khoảng cách giữa các dòng
//     int visibleLines;          // Số dòng có thể hiển thị
//     int totalLines;            // Tổng số dòng nội dung
//     std::vector<std::string> wrappedText; // Danh sách các dòng đã được "word-wrap"
//     float contentHeight;       // Chiều cao nội dung thực tế

//     // Phương thức nội bộ để "word-wrap" nội dung
//     void WrapText() {
//         wrappedText.clear();
//         std::istringstream stream(content);
//         std::string line;
//         float maxWidth = bounds.width - 20; // Padding 10px mỗi bên

//         while (std::getline(stream, line, '\n')) {
//             std::string word;
//             std::istringstream wordStream(line);
//             std::string currentLine;

//             while (wordStream >> word) {
//                 std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
//                 Vector2 textSize = MeasureTextEx(font, testLine.c_str(), fontSize, 1);
//                 if (textSize.x > maxWidth) {
//                     wrappedText.push_back(currentLine);
//                     currentLine = word;
//                 } else {
//                     currentLine = testLine;
//                 }
//             }
//             wrappedText.push_back(currentLine);
//         }

//         totalLines = wrappedText.size();
//         visibleLines = bounds.height / (fontSize + lineSpacing);
//         contentHeight = totalLines * (fontSize + lineSpacing);
//     }

// public:
//     // Constructor
//     Popup(float x, float y, float width, float height, const std::string& titleText, const std::string& detailContent,
//           Font customFont = GetFontDefault(), int textSize = 20, float radius = 10.0f,
//           Color bg = LIGHTGRAY, Color border = GRAY, Color textCol = BLACK, float spacing = 5.0f)
//         : bounds{ x, y, width, height }, title(titleText), content(detailContent), font(customFont),
//           fontSize(textSize), cornerRadius(radius), backgroundColor(bg), borderColor(border),
//           textColor(textCol), scrollOffset(0), lineSpacing(spacing), visibleLines(0), totalLines(0),
//           contentHeight(0) {
//         WrapText();
//     }

//     // Cập nhật logic của popup
//     void Update(Vector2 mousePosition) {
//         // Kiểm tra nếu chuột nằm trong popup
//         if (CheckCollisionPointRec(mousePosition, bounds)) {
//             float mouseWheelMove = GetMouseWheelMove();
//             if (mouseWheelMove != 0.0f) {
//                 float scrollSpeed = 20.0f; // Tốc độ cuộn
//                 scrollOffset -= mouseWheelMove * scrollSpeed;

//                 // Giới hạn thanh cuộn trong khoảng cho phép
//                 float maxScroll = std::max(0.0f, contentHeight - bounds.height);
//                 scrollOffset = Clamp(scrollOffset, 0.0f, maxScroll);
//             }
//         }
//     }

//     // Vẽ popup
//     void Draw() const {
//         // Vẽ khung bo góc
//         DrawRectangleRounded(bounds, cornerRadius / bounds.width, 6, backgroundColor);
//         DrawRectangleRoundedLines(bounds, cornerRadius / bounds.width, 6, borderColor);

//         // Vẽ tiêu đề
//         float titleHeight = fontSize + 10.0f; // Chiều cao khung tiêu đề
//         DrawRectangleRounded({ bounds.x, bounds.y, bounds.width, titleHeight }, cornerRadius / bounds.width, 6, borderColor);
//         DrawTextEx(font, title.c_str(), { bounds.x + 10, bounds.y + 5 }, fontSize, 1, WHITE);

//         // Bắt đầu vùng cắt (scissor mode)
//         BeginScissorMode(bounds.x, bounds.y + titleHeight, bounds.width, bounds.height - titleHeight);

//         // Vẽ nội dung
//         float padding = 10.0f;
//         float y = bounds.y + titleHeight + padding - scrollOffset;
//         for (const auto& line : wrappedText) {
//             if (y + fontSize > bounds.y + bounds.height) break; // Dừng nếu dòng vượt ra ngoài khung
//             if (y + fontSize >= bounds.y + titleHeight) {       // Chỉ vẽ nếu dòng nằm trong khung
//                 DrawTextEx(font, line.c_str(), { bounds.x + padding, y }, fontSize, 1, textColor);
//             }
//             y += fontSize + lineSpacing;
//         }

//         // Kết thúc vùng cắt
//         EndScissorMode();

//         // Vẽ thanh cuộn nếu cần
//         if (contentHeight > bounds.height) {
//             float scrollBarHeight = bounds.height * (bounds.height / contentHeight);
//             float scrollBarY = bounds.y + titleHeight + (bounds.height - scrollBarHeight - titleHeight) *
//                                (scrollOffset / (contentHeight - bounds.height));

//             DrawRectangle(bounds.x + bounds.width - 5, bounds.y + titleHeight, 5, bounds.height - titleHeight, DARKGRAY); // Nền thanh cuộn
//             DrawRectangle(bounds.x + bounds.width - 5, scrollBarY, 5, scrollBarHeight, BLACK); // Thanh cuộn
//         }
//     }

//     // Getter cho vùng popup (để kiểm tra va chạm)
//     Rectangle GetBounds() const {
//         return bounds;
//     }
// };

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////111111111111111
class Table {
private:
    Rectangle bounds;                         // Kích thước và vị trí của bảng
    Font font;                                // Font chữ
    int fontSize;                             // Kích thước font
    float rowHeight;                          // Chiều cao mỗi hàng
    float scrollOffset;                       // Offset thanh cuộn
    std::vector<ConnectionInfo> data;         // Dữ liệu hiển thị
    int selectedRow;                          // Hàng được chọn (-1 nếu không có hàng nào)
    float lineSpacing;                        // Khoảng cách giữa các dòng
    bool isDraggingScrollBar;                 // Trạng thái kéo thanh cuộn
    float dragStartY;                         // Điểm bắt đầu kéo chuột
    Popup* detailPopup;                       // Popup chi tiết (nullptr nếu không hiển thị)
    int visibleLines;          // Số dòng có thể hiển thị trong khung
    int totalLines; 

    // Vẽ một hàng
    void DrawRow(int rowIndex, float y, bool isHovered, bool isSelected) {
        // Màu nền hàng khi được hover hoặc chọn
        Color backgroundColor = isSelected ? DARKGRAY : (isHovered ? LIGHTGRAY : WHITE);
        DrawRectangle(bounds.x, y, bounds.width, rowHeight, backgroundColor);

        const ConnectionInfo& connection = data[rowIndex];
        std::ostringstream oss;
        oss << connection.transactions[0].request.method;  // Method
        std::string method = oss.str();

        std::string clientIp = connection.client.ip;       // Client IP
        std::string serverIp = connection.server.ip;       // Server IP
        std::string url = connection.transactions[0].request.uri; // URL

        // Vẽ dữ liệu từng cột
        float columnX[] = { bounds.x + 10, bounds.x + 120, bounds.x + 300, bounds.x + 500 };
        DrawTextEx(font, method.c_str(), { columnX[0], y + 5 }, fontSize, 1, BLACK);
        DrawTextEx(font, clientIp.c_str(), { columnX[1], y + 5 }, fontSize, 1, BLACK);
        DrawTextEx(font, serverIp.c_str(), { columnX[2], y + 5 }, fontSize, 1, BLACK);
        DrawTextEx(font, url.c_str(), { columnX[3], y + 5 }, fontSize, 1, BLACK);
    }

    void WrapText() {
        totalLines = data.size();
        visibleLines = bounds.height / (fontSize + lineSpacing);
    }

public:
    // Constructor
    Table(float x, float y, float width, float height, const std::vector<ConnectionInfo>& connectionData,
          Font customFont = GetFontDefault(), int textSize = 20, float rowSpacing = 5.0f)
        : bounds{ x, y, width, height }, data(connectionData), font(customFont), fontSize(textSize),
          rowHeight(textSize + rowSpacing), scrollOffset(0), selectedRow(-1), lineSpacing(rowSpacing),
          isDraggingScrollBar(false), dragStartY(0.0f), detailPopup(nullptr), visibleLines(0), totalLines(0) {
            WrapText();
          }

    // Cập nhật logic bảng
    void Update(Vector2 mousePosition) {
        // Xử lý cuộn chuột khi chuột nằm trong bảng và không trong popup
        if (CheckCollisionPointRec(mousePosition, bounds) && !detailPopup) {
            float mouseWheelMove = GetMouseWheelMove();
            if (mouseWheelMove != 0.0f) {
                float scrollSpeed = rowHeight; // Tốc độ cuộn
                scrollOffset -= mouseWheelMove * scrollSpeed;
            }

            // Giới hạn thanh cuộn trong khoảng cho phép
            float maxScroll = std::max(0.0f, data.size() * rowHeight - bounds.height);
            scrollOffset = Clamp(scrollOffset, 0.0f, maxScroll);
        }
                // Xử lý kéo thanh cuộn
        float scrollBarHeight = bounds.height * visibleLines / totalLines;
        Rectangle scrollBar = {
            bounds.x + bounds.width - 5,
            bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / ((totalLines - visibleLines) * (fontSize + lineSpacing))),
            5,
            scrollBarHeight
        };

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePosition, scrollBar)) {
            isDraggingScrollBar = true;
            dragStartY = mousePosition.y;
        }

        if (isDraggingScrollBar) {
            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
                float deltaY = mousePosition.y - dragStartY;
                dragStartY = mousePosition.y;

                // Cập nhật scrollOffset dựa trên deltaY
                float maxOffset = (totalLines - visibleLines) * (fontSize + lineSpacing);
                scrollOffset += deltaY * (maxOffset / (bounds.height - scrollBarHeight));
                scrollOffset = Clamp(scrollOffset, 0.0f, maxOffset);
            } else {
                isDraggingScrollBar = false;
            }
        }

        // Xử lý logic popup nếu có
        if (detailPopup) {
            if(!detailPopup->Update()) {
                delete detailPopup;
                detailPopup = nullptr;
            }
        }

        // Xử lý chọn hàng khi chuột nằm trong bảng
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (CheckCollisionPointRec(mousePosition, bounds) && !detailPopup) {
                int hoveredRow = (mousePosition.y - bounds.y + scrollOffset) / rowHeight;
                if (hoveredRow >= 0 && hoveredRow < data.size()) {
                    selectedRow = hoveredRow;
                    // std::cout << "Selected row: " << selectedRow << std::endl;
                    // std::this_thread::sleep_for(std::chrono::milliseconds(200));
                }
            } else {
                selectedRow = -1; // Bỏ chọn nếu nhấp ra ngoài bảng
            }
        }

        // Xử lý nhấp đúp để hiển thị chi tiết
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && selectedRow != -1) {
            static double lastClickTime = 0.0;
            double currentTime = GetTime();
            if (currentTime - lastClickTime < 0.3) { // Nhấp đúp
                if (detailPopup == nullptr) {
                    std::string connectionDetails = ConnectionInfoToString(data[selectedRow]);
                    detailPopup = new Popup(200, 150, 400, 300, "Connection Details", connectionDetails, font);
                }
            }
            lastClickTime = currentTime;
        }

        // Đóng popup nếu nhấp ra ngoài popup
        if (detailPopup && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (!CheckCollisionPointRec(mousePosition, bounds)) {
                delete detailPopup;
                detailPopup = nullptr;
            }
        }
    }


    // Vẽ bảng lên màn hình
    void Draw() {
        // Vẽ nền bảng
        DrawRectangle(bounds.x, bounds.y, bounds.width, bounds.height, LIGHTGRAY);

        // Bắt đầu vùng cắt
        BeginScissorMode(bounds.x, bounds.y, bounds.width, bounds.height);

        // Vẽ các hàng
        float y = bounds.y - scrollOffset;
        for (size_t i = 0; i < data.size(); ++i) {
            bool isHovered = CheckCollisionPointRec(GetMousePosition(), { bounds.x, y, bounds.width, rowHeight });
            bool isSelected = (selectedRow == i);
            DrawRow(i, y, isHovered, isSelected);
            y += rowHeight;
        }

        // Kết thúc vùng cắt
        EndScissorMode();

        // Vẽ thanh cuộn
        float maxScroll = std::max(0.0f, data.size() * rowHeight - bounds.height);
        if (maxScroll > 0) {
            float scrollBarHeight = bounds.height * bounds.height / (data.size() * rowHeight);
            float scrollBarY = bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / maxScroll);
            DrawRectangle(bounds.x + bounds.width - 5, bounds.y, 5, bounds.height, DARKGRAY); // Nền thanh cuộn
            DrawRectangle(bounds.x + bounds.width - 5, scrollBarY, 5, scrollBarHeight, BLACK); // Thanh cuộn
        }

        // Vẽ Popup chi tiết nếu có
        if (detailPopup) {
            detailPopup->Draw();
        }
    }
};


// class Table {
// private:
//     Rectangle bounds;              // Kích thước và vị trí của bảng
//     std::vector<ConnectionInfo> data; // Dữ liệu cần hiển thị
//     Font font;                     // Font chữ
//     int fontSize;                  // Kích thước font
//     Color borderColor;             // Màu viền
//     Color backgroundColor;         // Màu nền
//     Color textColor;               // Màu chữ
//     float scrollOffset;            // Vị trí thanh cuộn
//     float lineSpacing;             // Khoảng cách giữa các dòng
//     int visibleLines;              // Số dòng có thể hiển thị trong bảng
//     int totalLines;                // Tổng số dòng dữ liệu
//     bool isDraggingScrollBar;      // Trạng thái kéo thanh cuộn
//     float dragStartY;              // Điểm bắt đầu kéo chuột
//     Rectangle scrollBar;           // Vùng thanh cuộn

//     void WrapText() {
//         totalLines = data.size();
//         visibleLines = bounds.height / (fontSize + lineSpacing);
//     }

// public:
//     // Constructor
//     Table(float x, float y, float width, float height, const std::vector<ConnectionInfo>& tableData,
//           Font customFont = GetFontDefault(), int textSize = 20, float spacing = 5.0f,
//           Color bg = LIGHTGRAY, Color border = GRAY, Color textCol = BLACK)
//         : bounds{ x, y, width, height }, data(tableData), font(customFont), fontSize(textSize),
//           borderColor(border), backgroundColor(bg), textColor(textCol), scrollOffset(0),
//           lineSpacing(spacing), isDraggingScrollBar(false), dragStartY(0.0f) {
//         WrapText();
//     }

//     // Cập nhật logic của bảng
//     void Update(Vector2 mousePosition) {
//         // Kiểm tra nếu chuột nằm trong bảng và cuộn
//         if (CheckCollisionPointRec(mousePosition, bounds)) {
//             float mouseWheelMove = GetMouseWheelMove();
//             if (mouseWheelMove != 0.0f) {
//                 float scrollSpeed = 20.0f; // Tốc độ cuộn
//                 scrollOffset -= mouseWheelMove * scrollSpeed;

//                 // Giới hạn thanh cuộn trong khoảng cho phép
//                 scrollOffset = Clamp(scrollOffset, 0.0f, (totalLines - visibleLines) * (fontSize + lineSpacing));
//             }

//             // Xử lý kéo thanh cuộn
//             float scrollBarHeight = bounds.height * visibleLines / totalLines;
//             scrollBar = {
//                 bounds.x + bounds.width - 5,
//                 bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / ((totalLines - visibleLines) * (fontSize + lineSpacing))),
//                 5,
//                 scrollBarHeight
//             };

//             if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mousePosition, scrollBar)) {
//                 isDraggingScrollBar = true;
//                 dragStartY = mousePosition.y;
//             }

//             if (isDraggingScrollBar) {
//                 if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
//                     float deltaY = mousePosition.y - dragStartY;
//                     dragStartY = mousePosition.y;

//                     // Cập nhật scrollOffset dựa trên deltaY
//                     float maxOffset = (totalLines - visibleLines) * (fontSize + lineSpacing);
//                     scrollOffset += deltaY * (maxOffset / (bounds.height - scrollBarHeight));
//                     scrollOffset = Clamp(scrollOffset, 0.0f, maxOffset);
//                 } else {
//                     isDraggingScrollBar = false;
//                 }
//             }
//         }
//     }

//     // Vẽ bảng
//     void Draw() const {
//         // Vẽ khung bo góc
//         DrawRectangleRounded(bounds, 0.1f, 6, backgroundColor);
//         DrawRectangleRoundedLines(bounds, 0.1f, 6, borderColor);

//         float padding = 10.0f;
//         float y = bounds.y + padding - scrollOffset;

//         // Vẽ các dòng dữ liệu trong bảng
//         for (int i = 0; i < totalLines; i++) {
//             if (y + fontSize > bounds.y + bounds.height) break; // Dừng nếu dòng vượt ra ngoài khung
//             if (y + fontSize >= bounds.y) {                     // Chỉ vẽ nếu dòng nằm trong khung
//                 // Vẽ các dữ liệu trong dòng (Method, Client IP, Remote Server IP, URL)
//                 ConnectionInfo& row = data[i];
//                 std::string rowContent = row.request.method + " | " +
//                                          row.client.ip + " | " +
//                                          row.server.ip + " | " +
//                                          row.request.uri;
//                 DrawTextEx(font, rowContent.c_str(), { bounds.x + padding, y }, fontSize, 1, textColor);
//             }
//             y += fontSize + lineSpacing;
//         }

//         // Vẽ thanh cuộn nếu cần
//         if (totalLines > visibleLines) {
//             float scrollBarHeight = bounds.height * visibleLines / totalLines;
//             float scrollBarY = bounds.y + (bounds.height - scrollBarHeight) * (scrollOffset / ((totalLines - visibleLines) * (fontSize + lineSpacing)));

//             DrawRectangle(bounds.x + bounds.width - 5, bounds.y, 5, bounds.height, DARKGRAY); // Nền thanh cuộn
//             DrawRectangle(bounds.x + bounds.width - 5, scrollBarY, 5, scrollBarHeight, BLACK); // Thanh cuộn
//         }
//     }
// };


#endif // GUI_H