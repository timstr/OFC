#pragma once

#include <SFML/Graphics/Color.hpp>
#include <stdint.h>

namespace ui {

    class Color {
    public:
        Color();
        Color(float _r, float _g, float _b, float _a = 1.0f);
        Color(uint32_t rgba);
        Color(const sf::Color&);

        static Color RGB(float r, float b, float g, float a = 1.0f);

        static Color HSL(float h, float s, float l, float a = 1.0f);
        
        float red() const;
        float green() const;
        float blue() const;

        float hue() const;
        float saturation() const;
        float lightness() const;

        float alpha() const;

        void setRed(float);
        void setGreen(float);
        void setBlue(float);

        void setHue(float);
        void setSaturation(float);
        void setLightness(float);

        void setAlpha(float);

        operator sf::Color() const;

        uint32_t toInt() const;

    private:
        float r, g, b, h, s, l, a;

        void updateRGBFromHSL();
        void updateHSLFromRGB();
    };

};
