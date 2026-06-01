#pragma once

#include <glm/glm.hpp>
#include <map>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <glad/glad.h>

struct Character {
    unsigned int textureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    unsigned int advance;
};

class FontManager {
public:
    

    static FontManager& Instance() {
        static FontManager instance;
        return instance;
    }

    bool InitFont(const char* fontPath = "assets/fonts/Roboto-Regular.ttf", int fontSize = 48) {
        // prevent double initialization
        if (!Characters.empty())
        return true;

        FT_Library ft;

        if (FT_Init_FreeType(&ft)) {
            std::cout << "ERROR: Could not init FreeType Library\n";
            return false;
        }

        FT_Face face;

        if (FT_New_Face(ft, fontPath, 0, &face)) {
            std::cout << "ERROR: Failed to load font\n";
            FT_Done_FreeType(ft);
            return false;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for (unsigned char c = 0; c < 128; c++) {
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cout << "ERROR: Failed to load Glyph\n";
                continue;
            }

            unsigned int texture;

            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character = {
                texture,
                glm::ivec2(face->glyph->bitmap.width,
                            face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left,
                            face->glyph->bitmap_top),
                static_cast<unsigned int>(face->glyph->advance.x)
            };

            Characters.insert({c, character});
        }

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return true;
        }

    const Character& Get(char c) const {
        auto it = Characters.find(c);
        if (it != Characters.end()) {
            return it->second;
        } else {
            std::cerr << "Character '" << c << "' not found in font manager!" << std::endl;
            static Character defaultChar = {0, glm::ivec2(0), glm::ivec2(0), 0};
            return defaultChar;
        }
    }
    
    ~FontManager() {
        for (auto& pair : Characters) {
            glDeleteTextures(1, &pair.second.textureID);
        }
    }

private:
    FontManager() = default;
    std::map<char, Character> Characters;

};