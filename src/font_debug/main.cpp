#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

// ============================================================
// 简易 BMP 写入器（24-bit RGB，无需第三方库）
// ============================================================
static bool write_bmp(const char* filename, int w, int h,
                      const unsigned char* rgbData) {
    // 每行字节数（BMP 要求每行对齐到 4 字节）
    int rowSize = (w * 3 + 3) & ~3;
    int pixelDataSize = rowSize * h;
    int fileSize = 14 + 40 + pixelDataSize;

    std::vector<unsigned char> bmp(fileSize);
    int offset = 0;

    // --- BITMAPFILEHEADER (14 字节) ---
    bmp[offset++] = 'B'; bmp[offset++] = 'M';
    *(int*)&bmp[offset] = fileSize;       offset += 4;
    *(int*)&bmp[offset] = 0;              offset += 4;  // reserved
    *(int*)&bmp[offset] = 14 + 40;        offset += 4;  // data offset

    // --- BITMAPINFOHEADER (40 字节) ---
    *(int*)&bmp[offset] = 40;             offset += 4;  // header size
    *(int*)&bmp[offset] = w;              offset += 4;
    *(int*)&bmp[offset] = h;              offset += 4;
    *(short*)&bmp[offset] = 1;            offset += 2;  // planes
    *(short*)&bmp[offset] = 24;           offset += 2;  // bpp
    *(int*)&bmp[offset] = 0;              offset += 4;  // compression
    *(int*)&bmp[offset] = pixelDataSize;  offset += 4;
    *(int*)&bmp[offset] = 2835;           offset += 4;  // hres
    *(int*)&bmp[offset] = 2835;           offset += 4;  // vres
    *(int*)&bmp[offset] = 0;              offset += 4;  // colors used
    *(int*)&bmp[offset] = 0;              offset += 4;  // important colors

    // --- Pixel data (BGR, bottom-up) ---
    for (int y = h - 1; y >= 0; y--) {
        for (int x = 0; x < w; x++) {
            int srcIdx = (y * w + x) * 3;
            bmp[offset++] = rgbData[srcIdx + 2];  // B
            bmp[offset++] = rgbData[srcIdx + 1];  // G
            bmp[offset++] = rgbData[srcIdx + 0];  // R
        }
        // padding
        for (int p = 0; p < rowSize - w * 3; p++)
            bmp[offset++] = 0;
    }

    FILE* f = nullptr;
    if (fopen_s(&f, filename, "wb") != 0 || !f) return false;
    fwrite(bmp.data(), 1, fileSize, f);
    fclose(f);
    return true;
}

// ============================================================
// 在 RGB 图像上画一个像素
// ============================================================
static void set_pixel(std::vector<unsigned char>& img, int w, int h,
                      int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= w || y < 0 || y >= h) return;
    int idx = (y * w + x) * 3;
    img[idx + 0] = r;
    img[idx + 1] = g;
    img[idx + 2] = b;
}

// ============================================================
// 画一个矩形的边框（指定颜色）
// ============================================================
static void draw_rect(std::vector<unsigned char>& img, int w, int h,
                      int x0, int y0, int x1, int y1,
                      unsigned char r, unsigned char g, unsigned char b) {
    for (int x = x0; x <= x1; x++) {
        set_pixel(img, w, h, x, y0, r, g, b);
        set_pixel(img, w, h, x, y1, r, g, b);
    }
    for (int y = y0; y <= y1; y++) {
        set_pixel(img, w, h, x0, y, r, g, b);
        set_pixel(img, w, h, x1, y, r, g, b);
    }
}

// ============================================================
// 填充一个矩形区域（用来画大字数字标）
// ============================================================
static void fill_rect(std::vector<unsigned char>& img, int w, int h,
                      int x0, int y0, int x1, int y1,
                      unsigned char r, unsigned char g, unsigned char b) {
    for (int y = y0; y <= y1 && y < h; y++)
        for (int x = x0; x <= x1 && x < w; x++)
            set_pixel(img, w, h, x, y, r, g, b);
}

// ============================================================
// 画一个简单的数字（5x7 位图字体），用于标注图集
// ============================================================
static const unsigned char DIGIT_BITMAP[10][7] = {
    {0x7C, 0x82, 0x82, 0x82, 0x7C, 0x00, 0x00}, // 0
    {0x00, 0x84, 0xFE, 0x80, 0x00, 0x00, 0x00}, // 1
    {0x84, 0xC2, 0xA2, 0x92, 0x8C, 0x00, 0x00}, // 2
    {0x42, 0x82, 0x8A, 0x8A, 0x76, 0x00, 0x00}, // 3
    {0x30, 0x28, 0x24, 0xFE, 0x20, 0x00, 0x00}, // 4
    {0x4E, 0x8A, 0x8A, 0x8A, 0x72, 0x00, 0x00}, // 5
    {0x7C, 0x92, 0x92, 0x92, 0x62, 0x00, 0x00}, // 6
    {0x02, 0x02, 0xE2, 0x1A, 0x06, 0x00, 0x00}, // 7
    {0x6C, 0x92, 0x92, 0x92, 0x6C, 0x00, 0x00}, // 8
    {0x4C, 0x92, 0x92, 0x92, 0x7C, 0x00, 0x00}, // 9
};

static void draw_digit(std::vector<unsigned char>& img, int w, int h,
                       int ox, int oy, int digit,
                       unsigned char r, unsigned char g, unsigned char b) {
    if (digit < 0 || digit > 9) return;
    for (int row = 0; row < 7; row++) {
        unsigned char bits = DIGIT_BITMAP[digit][row];
        for (int col = 0; col < 7; col++) {
            if (bits & (1 << (6 - col))) {
                set_pixel(img, w, h, ox + col, oy + row, r, g, b);
            }
        }
    }
}

static void draw_number(std::vector<unsigned char>& img, int w, int h,
                        int ox, int oy, int num,
                        unsigned char r, unsigned char g, unsigned char b) {
    // 从右往左画每个数字位
    if (num == 0) {
        draw_digit(img, w, h, ox, oy, 0, r, g, b);
        return;
    }
    int x = ox;
    while (num > 0) {
        draw_digit(img, w, h, x, oy, num % 10, r, g, b);
        num /= 10;
        x -= 8; // 每个数字占 8 像素宽（含间距）
    }
}

// ============================================================
// 主程序
// ============================================================
int main(int argc, char** argv) {
    // 设置控制台为 UTF-8，避免中文乱码
    SetConsoleOutputCP(CP_UTF8);

    // ---- ① 解析命令行参数 ----
    std::string fontPath = "C:\\Windows\\Fonts\\arial.ttf";
    float pixelHeight = 48.0f;

    if (argc >= 2) fontPath = argv[1];
    if (argc >= 3) pixelHeight = static_cast<float>(atof(argv[2]));

    // ---- ② 读取 TTF 文件 ----
    std::ifstream file(fontPath, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "无法打开字体文件: " << fontPath << "\n";
        std::cerr << "用法: font_debug [字体路径] [像素高度]\n";
        std::cerr << "示例: font_debug C:\\Windows\\Fonts\\arial.ttf 48\n";
        return 1;
    }

    std::streamsize fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<unsigned char> fontBuffer(static_cast<size_t>(fileSize));
    file.read(reinterpret_cast<char*>(fontBuffer.data()), fileSize);
    file.close();

    // ---- ③ 初始化 stb_truetype ----
    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, fontBuffer.data(), 0)) {
        std::cerr << "stbtt_InitFont 失败\n";
        return 1;
    }

    // 读出字体度量（ascent, descent 用于行高说明）
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&fontInfo, &ascent, &descent, &lineGap);
    float scale = stbtt_ScaleForPixelHeight(&fontInfo, pixelHeight);

    // ---- ④ 烘焙图集 ----
    int atlasW = 512, atlasH = 512;
    std::vector<unsigned char> atlasBitmap(static_cast<size_t>(atlasW) * atlasH, 0);

    stbtt_bakedchar bakedChars[96];
    int result = stbtt_BakeFontBitmap(fontBuffer.data(), 0, pixelHeight,
                                      atlasBitmap.data(), atlasW, atlasH,
                                      32, 96, bakedChars);

    if (result < 0) {
        // 512 不够，改用 1024
        atlasW = 1024;
        atlasH = 1024;
        atlasBitmap.resize(static_cast<size_t>(atlasW) * atlasH, 0);
        result = stbtt_BakeFontBitmap(fontBuffer.data(), 0, pixelHeight,
                                      atlasBitmap.data(), atlasW, atlasH,
                                      32, 96, bakedChars);
    }

    if (result <= 0) {
        std::cerr << "stbtt_BakeFontBitmap 失败，result=" << result << "\n";
        return 1;
    }

    // ---- ⑤ 构建 GlyphData 数组（和 TextRenderer 完全一致） ----
    struct GlyphData {
        int x0, y0, x1, y1;
        float s0, t0, s1, t1;
        float xoff, yoff;
        float xadvance;
    };
    std::vector<GlyphData> glyphs(96);

    // 第一遍：从 bakedChars 拷贝原始值
    for (int i = 0; i < 96; i++) {
        stbtt_bakedchar& src = bakedChars[i];
        GlyphData& dst = glyphs[i];
        dst.x0 = src.x0;
        dst.y0 = src.y0;
        dst.x1 = src.x1;
        dst.y1 = src.y1;
        dst.xoff = src.xoff;
        dst.yoff = src.yoff;
        dst.xadvance = src.xadvance;
    }

    // 第二遍：计算 UV 坐标
    for (int i = 0; i < 96; i++) {
        GlyphData& dst = glyphs[i];
        dst.s0 = static_cast<float>(dst.x0) / static_cast<float>(atlasW);
        dst.t0 = static_cast<float>(dst.y0) / static_cast<float>(atlasH);
        dst.s1 = static_cast<float>(dst.x1) / static_cast<float>(atlasW);
        dst.t1 = static_cast<float>(dst.y1) / static_cast<float>(atlasH);
    }

    // ============================================================
    // ⑥ 输出到控制台
    // ============================================================
    std::cout << "\n";
    std::cout << "======================================================================\n";
    std::cout << "                   TextRenderer GlyphData 调试工具\n";
    std::cout << "======================================================================\n\n";
    std::cout << "字体:      " << fontPath << "\n";
    std::cout << "像素高度:  " << pixelHeight << "px\n";
    std::cout << "图集尺寸:  " << atlasW << " x " << atlasH << "\n";
    std::cout << "字符范围:  ASCII 32 ~ 127 (共96个字符)\n\n";
    std::cout << "字体度量:\n";
    std::cout << "  ascent  = " << ascent << " (设计单位),  * scale = " << ascent * scale << " px\n";
    std::cout << "  descent = " << descent << " (设计单位),  * scale = " << descent * scale << " px\n";
    std::cout << "  lineGap = " << lineGap << " (设计单位),  * scale = " << lineGap * scale << " px\n";
    std::cout << "  行高    = " << (ascent - descent + lineGap) * scale << " px\n\n";

    // ---- 打印字段含义说明 ----
    std::cout << "──────────────────────────────────────────────────────────────────────\n";
    std::cout << " GlyphData 字段含义\n";
    std::cout << "──────────────────────────────────────────────────────────────────────\n";
    std::cout << " x0,y0  该字符 bitmap 在图集纹理中的 左上角像素坐标\n";
    std::cout << " x1,y1  该字符 bitmap 在图集纹理中的 右下角像素坐标\n";
    std::cout << " s0,t0  UV 坐标版 = (x0/atlasW, y0/atlasH)\n";
    std::cout << " s1,t1  UV 坐标版 = (x1/atlasW, y1/atlasH)\n";
    std::cout << " xoff   位图左边缘相对光标的水平偏移（正=偏右）\n";
    std::cout << " yoff   位图顶边缘相对基线的垂直偏移（负=基线上方）\n";
    std::cout << " xadv   打完该字符后光标前进的距离\n";
    std::cout << "──────────────────────────────────────────────────────────────────────\n\n";

    // ---- 打印详细数据表 ----
    std::cout << std::left;
    std::cout << "  " << std::setw(6) << "字符"
              << std::setw(5) << "ASCII"
              << std::setw(6) << "x0" << std::setw(6) << "y0"
              << std::setw(6) << "x1" << std::setw(6) << "y1"
              << std::setw(10) << "s0" << std::setw(10) << "t0"
              << std::setw(10) << "s1" << std::setw(10) << "t1"
              << std::setw(8) << "xoff" << std::setw(8) << "yoff"
              << std::setw(8) << "xadv" << "\n";
    std::cout << "  " << std::string(103, '-') << "\n";

    for (int i = 0; i < 96; i++) {
        char c = static_cast<char>(i + 32);
        std::string display;
        if (c == ' ') display = "' '";
        else if (c == '\'') display = "'\\''";
        else display = std::string("'") + c + "'";

        GlyphData& g = glyphs[i];
        std::cout << "  " << std::setw(6) << display
                  << std::setw(5) << (i + 32)
                  << std::setw(6) << g.x0 << std::setw(6) << g.y0
                  << std::setw(6) << g.x1 << std::setw(6) << g.y1
                  << std::setw(10) << std::fixed << std::setprecision(4) << g.s0
                  << std::setw(10) << std::fixed << std::setprecision(4) << g.t0
                  << std::setw(10) << std::fixed << std::setprecision(4) << g.s1
                  << std::setw(10) << std::fixed << std::setprecision(4) << g.t1
                  << std::setw(8) << std::fixed << std::setprecision(1) << g.xoff
                  << std::setw(8) << std::fixed << std::setprecision(1) << g.yoff
                  << std::setw(8) << std::fixed << std::setprecision(1) << g.xadvance
                  << "\n";
    }

    // ---- 详细分析几个典型字符 ----
    std::cout << "\n\n";
    std::cout << "======================================================================\n";
    std::cout << "                 典型字符逐字段详解\n";
    std::cout << "======================================================================\n";

    // 选 4 个典型字符：' '(空格), 'A', 'g', 'j'
    const char sampleChars[] = { ' ', 'A', 'g', 'j' };
    const char* sampleNames[] = { "空格", "大写A", "小写g", "小写j" };
    const char* sampleNotes[] = {
        "- 空格没有像素轮廓，只有 xadvance，x0=y0=x1=y1=0\n"
        "  bitmap 宽高为 0 → 渲染时 charW=0,charH=0 → 跳过 DrawQuad\n"
        "  只有 cursorX += xadvance，相当于空出一个字的距离",

        "- xadvance(70) = 打完'A'后光标前进的距离\n"
        "  xoff(0) = bitmap 左边缘对齐光标，没有额外偏移\n"
        "  yoff(-33) = bitmap 顶部在基线上方 33 像素\n"
        "  渲染时 charX = cursorX + xoff*scale, charY = cursorY + yoff*scale",

        "- yoff = -20(比'A'小) 因为 g 的顶部比 A 矮\n"
        "  xadvance = 33(比'A'小很多) 因为 g 比 A 窄\n"
        "  注意 g 的 bitmap 包含了延伸到基线以下的勾\n"
        "  所以 y1-y0 比'A'大（bitmap 更宽）",

        "- yoff = -25(比 A 和 g 都大)，因为 j 的点在上面\n"
        "  xadvance 很小(22)，j 本身就很窄\n"
        "  注意 y1-y0 也大，因为 bitmap 包含了上方的点和下方的勾"
    };

    for (int si = 0; si < 4; si++) {
        char c = sampleChars[si];
        int idx = c - 32;
        GlyphData& g = glyphs[idx];

        int charW_px = g.x1 - g.x0;
        int charH_px = g.y1 - g.y0;
        float charX = g.xoff - static_cast<float>(g.x0);
        float charY = g.yoff;

        std::cout << "\n";
        std::cout << "──────────────────────────────────────────────────────────────────────\n";
        std::cout << " 字符 '" << c << "'  (" << sampleNames[si] << ")\n";
        std::cout << "──────────────────────────────────────────────────────────────────────\n\n";
        std::cout << "  原始数据:\n";
        std::cout << "    x0=" << g.x0 << ", y0=" << g.y0 << ", x1=" << g.x1 << ", y1=" << g.y1 << "\n";
        std::cout << "    s0=" << g.s0 << ", t0=" << g.t0 << ", s1=" << g.s1 << ", t1=" << g.t1 << "\n";
        std::cout << "    xoff=" << g.xoff << ", yoff=" << g.yoff << ", xadvance=" << g.xadvance << "\n\n";

        std::cout << "  换算结果 (scale=1):\n";
        std::cout << "    charW (屏幕宽度)  = x1 - x0           = " << charW_px << " px\n";
        std::cout << "    charH (屏幕高度)  = y1 - y0           = " << charH_px << " px\n";
        std::cout << "    charX (屏幕位置x) = cursorX + (xoff - x0) = cursorX + " << charX << "\n";
        std::cout << "    charY (屏幕位置y) = cursorY + yoff        = cursorY + " << charY << "\n";
        std::cout << "    cursor 推进       = xadvance          = +" << g.xadvance << "\n\n";

        // ASCII 图：在图集中的位置
        std::cout << "  在图集纹理中的位置:\n";
        int diagW = 30, diagH = 12;
        if (charW_px > 0 && charH_px > 0) {
            float scaleX = static_cast<float>(diagW - 4) / static_cast<float>(atlasW);
            float scaleY = static_cast<float>(diagH - 4) / static_cast<float>(atlasH);
            int sx0 = 2 + static_cast<int>(g.x0 * scaleX);
            int sy0 = 2 + static_cast<int>(g.y0 * scaleY);
            int sx1 = 2 + static_cast<int>(g.x1 * scaleX);
            int sy1 = 2 + static_cast<int>(g.y1 * scaleY);
            if (sx1 > diagW - 2) sx1 = diagW - 2;
            if (sy1 > diagH - 2) sy1 = diagH - 2;

            for (int row = 0; row < diagH; row++) {
                std::cout << "    ";
                for (int col = 0; col < diagW; col++) {
            bool onBorder =
                        (row == sy0 || row == sy1) && (col >= sx0 && col <= sx1) ||
                        (col == sx0 || col == sx1) && (row >= sy0 && row <= sy1);
            bool inRect = row >= sy0 && row <= sy1 && col >= sx0 && col <= sx1;
            if (onBorder)      std::cout << "#";   // 边界
            else if (inRect)   std::cout << ".";   // 内部
            else               std::cout << " ";   // 外部
                }
                std::cout << "\n";
            }
            std::cout << "    红色框 = 该字符 bitmap 在图集中的区域\n";
        } else {
            std::cout << "    (此字符没有像素数据，bitmap 宽高为 0)\n";
        }

        // ASCII 渲染位置图
        if (charW_px > 0) {
            std::cout << "\n  屏幕上的渲染位置 (cursorX=0, cursorY=0, scale=1):\n";
            int renderW = 70;
            int bx0 = static_cast<int>((g.xoff - g.x0) + 35);  // charX + 35 offset for display
            int bx1 = bx0 + charW_px;
            int by0 = static_cast<int>(g.yoff + 6);
            int by1 = by0 + charH_px;

            // 基线
            std::cout << "    基线(y=0): ─────────────────────────────────────────────\n    ";
            for (int col = 0; col < renderW; col++) {
            bool inGlyphH = col >= bx0 && col <= bx1;
            std::cout << (inGlyphH ? "=" : "─");
            }
            std::cout << "\n";

            // 字符区域
            for (int row = -1; row <= 1; row++) {
                std::cout << "    ";
                for (int col = 0; col < renderW; col++) {
                    bool inGlyph = col >= bx0 && col <= bx1 &&
                                   row >= by0 && row <= by1;
            std::cout << (inGlyph ? "#" : " ");
                }
                std::cout << "\n";
            }

            std::cout << "    ▲\n    │\n    charX = " << charX << ", charY = " << charY
                      << ", width = " << charW_px << ", height = " << charH_px << "\n";
        }

        std::cout << "\n  说明:\n" << sampleNotes[si] << "\n";
    }

    // ============================================================
    // ⑦ 保存图集图像
    // ============================================================
    // 7a. 原始灰度图 → BMP
    {
        std::vector<unsigned char> rgb(static_cast<size_t>(atlasW) * atlasH * 3);
        for (int i = 0; i < atlasW * atlasH; i++) {
            unsigned char v = atlasBitmap[i];
            rgb[i*3+0] = v;
            rgb[i*3+1] = v;
            rgb[i*3+2] = v;
        }
        write_bmp("atlas_raw.bmp", atlasW, atlasH, rgb.data());
        std::cout << "\n\n已保存: atlas_raw.bmp  (原始灰度图集)\n";
    }

    // 7b. 调试覆盖图 → 标出每个 glyph 的 bounding box
    {
        std::vector<unsigned char> debug(static_cast<size_t>(atlasW) * atlasH * 3);

        // 先复制原图（灰度转 RGB）
        for (int y = 0; y < atlasH; y++) {
            for (int x = 0; x < atlasW; x++) {
                unsigned char v = atlasBitmap[y * atlasW + x];
                int idx = (y * atlasW + x) * 3;
                // 用灰度作为底色
                debug[idx + 0] = v;
                debug[idx + 1] = v;
                debug[idx + 2] = v;
            }
        }

        // 给每个字符画红框 + 数字标号
        for (int i = 0; i < 96; i++) {
            GlyphData& g = glyphs[i];
            if (g.x1 <= g.x0 || g.y1 <= g.y0) continue; // 跳过空字符

            // 红框
            draw_rect(debug, atlasW, atlasH, g.x0, g.y0, g.x1, g.y1, 255, 40, 40);

            // 在框上方画字符索引数字（绿色）
            int numY = g.y0 - 10;
            if (numY < 0) numY = g.y0 + 2;
            draw_number(debug, atlasW, atlasH, g.x0 + 2, numY, i, 40, 255, 40);
        }

        // 画坐标参考标记（每隔 50px 画一个点 + 数字标注）
        for (int x = 0; x < atlasW; x += 50) {
            for (int y = 0; y < 3; y++)
                set_pixel(debug, atlasW, atlasH, x, y, 100, 100, 255);
            if (x % 100 == 0 && x > 0) {
                // draw x coordinate number
                int numX = x - 10;
                if (numX < 0) numX = 0;
                int val = x;
                int digits = 0;
                int tmp = val;
                while (tmp > 0) { digits++; tmp /= 10; }
                if (digits == 0) digits = 1;
                while (val > 0) {
                    draw_digit(debug, atlasW, atlasH, numX + (digits-1)*8, 6, val % 10, 100, 100, 255);
                    val /= 10;
                    digits--;
                }
            }
        }
        for (int y = 0; y < atlasH; y += 50) {
            for (int x = 0; x < 3; x++)
                set_pixel(debug, atlasW, atlasH, x, y, 100, 100, 255);
        }

        write_bmp("atlas_debug.bmp", atlasW, atlasH, debug.data());
        std::cout << "已保存: atlas_debug.bmp  (标有红框+索引号的调试图)\n";
    }

    // 7c. 单独放大显示几个特殊字符
    {
        // 把 'A', 'g', 'j', 空格 的 area 单独放大输出
        struct ZoomInfo { char c; int idx; int pad; };
        ZoomInfo zooms[] = { {'A', 'A'-32, 5}, {'g', 'g'-32, 5}, {'j', 'j'-32, 5} };

        for (ZoomInfo& z : zooms) {
            GlyphData& g = glyphs[z.idx];
            if (g.x1 <= g.x0 || g.y1 <= g.y0) continue;

            int zoomW = (g.x1 - g.x0) + z.pad * 2;
            int zoomH = (g.y1 - g.y0) + z.pad * 2;
            if (zoomW > 512 || zoomH > 512) continue;

            std::vector<unsigned char> zoom(static_cast<size_t>(zoomW) * zoomH * 3, 0);

            for (int y = 0; y < zoomH; y++) {
                for (int x = 0; x < zoomW; x++) {
                    int srcX = g.x0 - z.pad + x;
                    int srcY = g.y0 - z.pad + y;
                    unsigned char v = 0;
                    if (srcX >= 0 && srcX < atlasW && srcY >= 0 && srcY < atlasH) {
                        v = atlasBitmap[srcY * atlasW + srcX];
                    }
                    int idx = (y * zoomW + x) * 3;
                    zoom[idx+0] = v;
                    zoom[idx+1] = v;
                    zoom[idx+2] = v;
                }
            }

            // 画框
            draw_rect(zoom, zoomW, zoomH,
                      z.pad, z.pad, zoomW-z.pad-1, zoomH-z.pad-1,
                      255, 40, 40);
            // 标记 (x0,y0) 点
            fill_rect(zoom, zoomW, zoomH,
                      z.pad-2, z.pad-2, z.pad+2, z.pad+2,
                      255, 200, 40);

            char fname[64];
            sprintf_s(fname, "zoom_%c.bmp", z.c);
            write_bmp(fname, zoomW, zoomH, zoom.data());
            std::cout << "已保存: " << fname << "  ('" << z.c << "' 放大图)\n";
        }
    }

    // ============================================================
    // ⑧ 输出说明
    // ============================================================
    std::cout << "\n\n";
    std::cout << "======================================================================\n";
    std::cout << " 如何看图\n";
    std::cout << "======================================================================\n";
    std::cout << " 1. atlas_raw.bmp   → 图集原始图像，灰度图为每个字符的 bitmap\n";
    std::cout << " 2. atlas_debug.bmp → 红色框 = Glyph 的 (x0,y0)-(x1,y1) 区域\n";
    std::cout << "                      绿色数字 = 字符索引号 (0=' ', 1='!', ..., 33='A', ...)\n";
    std::cout << "                      顶部蓝色数字 = x 坐标参考线 (每100px)\n";
    std::cout << " 3. zoom_*.bmp      → 单个字符放大图\n";
    std::cout << "                      红色框 = (x0,y0)-(x1,y1) 边界\n";
    std::cout << "                      橙色方块 = (x0,y0) 起点\n\n";
    std::cout << " 对照控制台打印的表格，找到对应索引号，即可看到该字符的完整字段值\n";
    std::cout << "======================================================================\n";

    return 0;
}
