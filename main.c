#include <rui.h>
#include <utils.h>
#include <mem.h>
#include <stdint.h>

// --------------------------
// 1. 常量定义（界面尺寸、颜色等）
// --------------------------
#define WIN_WIDTH  320  // 窗口宽度
#define WIN_HEIGHT 480  // 窗口高度
#define BTN_WIDTH  70   // 按钮宽度
#define BTN_HEIGHT 60   // 按钮高度
#define DISP_HEIGHT 80  // 显示框高度
#define SPACING 10      // 元素间距

// 颜色定义（RGB888）
#define COLOR_WHITE     (rui_color_t){255, 255, 255}
#define COLOR_BLACK     (rui_color_t){0, 0, 0}
#define COLOR_GRAY      (rui_color_t){200, 200, 200}
#define COLOR_LIGHTBLUE (rui_color_t){173, 216, 230}
#define COLOR_RED       (rui_color_t){255, 99, 71}

// --------------------------
// 2. 全局变量（计算器状态）
// --------------------------
static uint32_t g_win_id;                // 窗口ID
static char g_display_text[32] = "";     // 显示框文本
static char g_current_input[16] = "";    // 当前输入的数字
static char g_prev_input[16] = "";       // 上一个输入的数字
static char g_operator = '\0';           // 当前操作符（+、-、*、/）
static int g_is_result = 0;              // 是否刚计算出结果（用于清除显示）

// --------------------------
// 3. 辅助函数（字符串与数字转换）
// --------------------------

// 字符串转整数（简化版，仅支持非负整数）
static int str_to_int(const char* str) {
    if (str == NULL || *str == '\0') return 0;
    int num = 0;
    while (*str != '\0') {
        if (!eclib_isdigit(*str)) return 0; // 非数字字符返回0
        num = num * 10 + (*str - '0');
        str++;
    }
    return num;
}

// 整数转字符串（简化版，仅支持非负整数）
static void int_to_str(int num, char* buf, size_t buf_len) {
    if (buf == NULL || buf_len < 2) return;
    if (num == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return;
    }
    // 逆序存储数字
    char temp[16];
    int i = 0;
    while (num > 0 && i < 15) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }
    // 反转并存储到buf
    int j;
    for (j = 0; j < i && j < buf_len - 1; j++) {
        buf[j] = temp[i - 1 - j];
    }
    buf[j] = '\0';
}

// --------------------------
// 4. 界面绘制函数
// --------------------------

// 绘制显示框（顶部区域）
static void draw_display(void) {
    // 1. 绘制显示框背景（浅灰色）
    // （实际需调用RUI绘制矩形的接口，这里简化为逻辑）
    rui_point_t rect_pos = {0, 0};
    rui_size_t rect_size = {WIN_WIDTH, DISP_HEIGHT};
    // eclib_rui_draw_rect(g_win_id, &rect_pos, &rect_size, &COLOR_GRAY);  // 假设的矩形绘制接口

    // 2. 绘制显示文本（右对齐）
    int text_len = eclib_strlen(g_display_text);
    rui_point_t text_pos = {
        .x = WIN_WIDTH - 20 - (text_len * 12),  // 右对齐（每个字符约占12像素宽度）
        .y = 30  // 垂直居中
    };
    eclib_rui_draw_text(g_win_id, &text_pos, g_display_text, &COLOR_BLACK, 24);
}

// 绘制单个按钮（x/y为左上角坐标，text为文字，color为背景色）
static void draw_button(int x, int y, const char* text, rui_color_t color) {
    // 1. 绘制按钮背景（矩形）
    rui_point_t rect_pos = {x, y};
    rui_size_t rect_size = {BTN_WIDTH, BTN_HEIGHT};
    // eclib_rui_draw_rect(g_win_id, &rect_pos, &rect_size, &color);  // 假设的矩形绘制接口

    // 2. 绘制按钮文字（居中）
    int text_len = eclib_strlen(text);
    rui_point_t text_pos = {
        .x = x + (BTN_WIDTH - text_len * 10) / 2,  // 水平居中
        .y = y + (BTN_HEIGHT - 20) / 2             // 垂直居中（字体大小20）
    };
    eclib_rui_draw_text(g_win_id, &text_pos, text, &COLOR_BLACK, 20);
}

// 绘制所有按钮（4列5行布局）
static void draw_all_buttons(void) {
    int start_y = DISP_HEIGHT + SPACING;  // 按钮起始Y坐标（显示框下方）

    // 第1行：C、÷、×、-
    draw_button(SPACING, start_y, "C", COLOR_RED);
    draw_button(SPACING + BTN_WIDTH + SPACING, start_y, "/", COLOR_LIGHTBLUE);
    draw_button(SPACING + 2*(BTN_WIDTH + SPACING), start_y, "*", COLOR_LIGHTBLUE);
    draw_button(SPACING + 3*(BTN_WIDTH + SPACING), start_y, "-", COLOR_LIGHTBLUE);

    // 第2行：7、8、9、+
    start_y += BTN_HEIGHT + SPACING;
    draw_button(SPACING, start_y, "7", COLOR_GRAY);
    draw_button(SPACING + BTN_WIDTH + SPACING, start_y, "8", COLOR_GRAY);
    draw_button(SPACING + 2*(BTN_WIDTH + SPACING), start_y, "9", COLOR_GRAY);
    draw_button(SPACING + 3*(BTN_WIDTH + SPACING), start_y, "+", COLOR_LIGHTBLUE);

    // 第3行：4、5、6、=
    start_y += BTN_HEIGHT + SPACING;
    draw_button(SPACING, start_y, "4", COLOR_GRAY);
    draw_button(SPACING + BTN_WIDTH + SPACING, start_y, "5", COLOR_GRAY);
    draw_button(SPACING + 2*(BTN_WIDTH + SPACING), start_y, "6", COLOR_GRAY);
    draw_button(SPACING + 3*(BTN_WIDTH + SPACING), start_y, "=", COLOR_LIGHTBLUE);

    // 第4行：1、2、3、(空)
    start_y += BTN_HEIGHT + SPACING;
    draw_button(SPACING, start_y, "1", COLOR_GRAY);
    draw_button(SPACING + BTN_WIDTH + SPACING, start_y, "2", COLOR_GRAY);
    draw_button(SPACING + 2*(BTN_WIDTH + SPACING), start_y, "3", COLOR_GRAY);

    // 第5行：0、(空)、(空)、(空)
    start_y += BTN_HEIGHT + SPACING;
    draw_button(SPACING, start_y, "0", COLOR_GRAY);
}

// 刷新整个界面
static void refresh_ui(void) {
    // 1. 清屏（用白色填充窗口）
    rui_point_t clear_pos = {0, 0};
    rui_size_t clear_size = {WIN_WIDTH, WIN_HEIGHT};
    // eclib_rui_draw_rect(g_win_id, &clear_pos, &clear_size, &COLOR_WHITE);  // 假设的清屏接口

    // 2. 重绘所有元素
    draw_display();
    draw_all_buttons();
}

// --------------------------
// 5. 计算器逻辑处理
// --------------------------

// 更新显示框
static void update_display(void) {
    eclib_strncpy(g_display_text, g_current_input, sizeof(g_display_text)-1);
    refresh_ui();
}

// 清除输入（C键）
static void clear_all(void) {
    g_current_input[0] = '\0';
    g_prev_input[0] = '\0';
    g_operator = '\0';
    g_is_result = 0;
    update_display();
}

// 输入数字（0-9）
static void input_digit(char digit) {
    if (g_is_result) {  // 刚计算完，输入新数字时清空当前输入
        g_current_input[0] = '\0';
        g_is_result = 0;
    }
    // 限制最大输入长度（15位）
    size_t len = eclib_strlen(g_current_input);
    if (len < 15) {
        g_current_input[len] = digit;
        g_current_input[len + 1] = '\0';
        update_display();
    }
}

// 输入操作符（+、-、*、/）
static void input_operator(char op) {
    if (eclib_strlen(g_current_input) == 0) return;  // 无当前输入则不处理
    // 保存当前输入到上一个值，清空当前输入
    eclib_strncpy(g_prev_input, g_current_input, sizeof(g_prev_input)-1);
    g_current_input[0] = '\0';
    g_operator = op;
    update_display();
}

// 计算结果（=键）
static void calculate_result(void) {
    if (eclib_strlen(g_prev_input) == 0 || eclib_strlen(g_current_input) == 0 || g_operator == '\0') {
        return;  // 输入不完整
    }

    // 字符串转数字
    int prev = str_to_int(g_prev_input);
    int current = str_to_int(g_current_input);
    int result = 0;
    int error = 0;

    // 执行运算
    switch (g_operator) {
        case '+': result = prev + current; break;
        case '-': result = prev - current; break;
        case '*': result = prev * current; break;
        case '/': 
            if (current == 0) {
                error = 1;  // 除零错误
            } else {
                result = prev / current;  // 简化：整数除法
            }
            break;
        default: return;
    }

    // 处理结果或错误
    if (error) {
        eclib_strncpy(g_current_input, "错误", sizeof(g_current_input)-1);
    } else {
        int_to_str(result, g_current_input, sizeof(g_current_input));
    }

    // 重置状态
    g_prev_input[0] = '\0';
    g_operator = '\0';
    g_is_result = 1;
    update_display();
}

// 处理鼠标点击事件
static void handle_click(rui_point_t pos) {
    // 点击区域判断：仅处理按钮区域（Y坐标在显示框下方）
    if (pos.y < DISP_HEIGHT) return;

    int start_y = DISP_HEIGHT + SPACING;
    int row = (pos.y - start_y) / (BTN_HEIGHT + SPACING);  // 计算点击的行
    int col = (pos.x - SPACING) / (BTN_WIDTH + SPACING);    // 计算点击的列

    // 边界检查（只处理0-3列，0-4行）
    if (row < 0 || row > 4 || col < 0 || col > 3) return;

    // 根据行列判断按钮并执行逻辑
    switch (row) {
        case 0:  // 第1行：C、/、*、-
            if (col == 0) clear_all();
            else if (col == 1) input_operator('/');
            else if (col == 2) input_operator('*');
            else if (col == 3) input_operator('-');
            break;
        case 1:  // 第2行：7、8、9、+
            if (col == 0) input_digit('7');
            else if (col == 1) input_digit('8');
            else if (col == 2) input_digit('9');
            else if (col == 3) input_operator('+');
            break;
        case 2:  // 第3行：4、5、6、=
            if (col == 0) input_digit('4');
            else if (col == 1) input_digit('5');
            else if (col == 2) input_digit('6');
            else if (col == 3) calculate_result();
            break;
        case 3:  // 第4行：1、2、3、(空)
            if (col == 0) input_digit('1');
            else if (col == 1) input_digit('2');
            else if (col == 2) input_digit('3');
            break;
        case 4:  // 第5行：0、(空)、(空)、(空)
            if (col == 0) input_digit('0');
            break;
    }
}

// --------------------------
// 6. 主函数（程序入口）
// --------------------------
int main() {
    // 1. 初始化RUI通信
    if (eclib_rui_init() != ECLIB_OK) {
        // 无法初始化RUI，退出（实际可添加错误提示）
        return 1;
    }

    // 2. 创建计算器窗口
    rui_point_t win_pos = {100, 100};  // 窗口左上角坐标
    rui_size_t win_size = {WIN_WIDTH, WIN_HEIGHT};
    rui_color_t win_bg = COLOR_WHITE;
    g_win_id = eclib_rui_create_window(&win_pos, &win_size, "简易计算器", &win_bg);
    if (g_win_id == 0) {
        return 1;  // 创建窗口失败
    }

    // 3. 注册窗口点击事件监听
    eclib_rui_register_event(g_win_id, RUI_EVENT_CLICK);

    // 4. 初始化界面（首次绘制）
    refresh_ui();

    // 5. 事件循环（处理用户输入）
    rui_event_t event;
    int running = 1;
    while (running) {
        // 等待RUI事件（超时1000ms，避免死等）
        eclib_err_t err = eclib_rui_wait_event(&event, 1000);
        if (err != ECLIB_OK) continue;

        // 处理事件
        switch (event.event_type) {
            case RUI_EVENT_CLICK:
                // 仅处理当前窗口的点击事件
                if (event.data.click.window_id == g_win_id) {
                    handle_click(event.data.click.pos);
                }
                break;
            case RUI_EVENT_WINDOW_CLOSE:
                // 窗口被关闭，退出循环
                if (event.data.click.window_id == g_win_id) {
                    running = 0;
                }
                break;
        }
    }

    // 6. 退出前清理
    eclib_rui_close_window(g_win_id);
    return 0;
}