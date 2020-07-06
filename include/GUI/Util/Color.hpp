#pragma once

#include <SFML/Graphics/Color.hpp>
#include <stdint.h>

namespace ui {

    // TODO: this is a stupid waste of memory.
    // HSL and RGB spaces are mutually exclusive and can be put into a just three floats
    // with a flag to distinguish which mode the color is using. Calling methods like
    // setHue or setRed will convert the color space.
    // Doing this instead of forcing an internal RGB representation can preserve the
    // values the user expects (i.e. hue is degenerate for black/white in RGB space)
    // for the cost of only a single flag member variable and some conditional checks
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

    Color interpolate(const Color& c0, const Color& c1, float t) noexcept;

    // TODO: alpha blending function(s)

};
