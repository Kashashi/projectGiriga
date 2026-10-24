#ifndef TEXT_TEXTURE_H
#define TEXT_TEXTURE_H

#include <glad/glad.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <vector>

// 獨立的字體載入管理類別
class FontLoader {
public:
    FontLoader(const std::string& fontPath, unsigned int fontSize);
    ~FontLoader();

    // 禁止拷貝，支援移動
    FontLoader(const FontLoader&) = delete;
    FontLoader& operator=(const FontLoader&) = delete;
    FontLoader(FontLoader&& other) noexcept;
    FontLoader& operator=(FontLoader&& other) noexcept;

    FT_Face getFace() const { return m_face; }
    unsigned int getFontSize() const { return m_fontSize; }

private:
    FT_Library m_ft = nullptr;
    FT_Face m_face = nullptr;
    unsigned int m_fontSize = 0;
};

// 符合 RAII 的動態文字紋理對象
class TextTexture {
public:
    // 構造時烘焙文字並生成單一張 OpenGL 紋理
    TextTexture(const FontLoader& font, const std::u32string& text);

    // RAII 解構釋放顯存
    ~TextTexture();

    // 禁止拷貝以防重複釋放
    TextTexture(const TextTexture&) = delete;
    TextTexture& operator=(const TextTexture&) = delete;

    // 移動構造與移動賦值
    TextTexture(TextTexture&& other) noexcept;
    TextTexture& operator=(TextTexture&& other) noexcept;

    void bind(GLuint slot = 0) const;

    GLuint getID() const { return m_textureID; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

private:
    GLuint m_textureID = 0;
    int m_width = 0;
    int m_height = 0;

    void release();

    struct GlyphLayout {
        std::vector<unsigned char> buffer;
        int width;
        int rows;
        int left;
        int top;
        int penX;
    };

    void bake(FT_Face face, const std::u32string& text);
};

#endif // TEXT_TEXTURE_H
