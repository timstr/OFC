#pragma once

#include <SFML/Graphics/Color.hpp>

#include <stdint.h>
#include <array>

namespace ui {

    /**
     * Color is a point in either the RGB or the HSL color space, plus an alpha channel.
     * The object is in one of the two spaces at any given time, specifically the space
     * through which it was most recently modified. For example, calling setHue ensures
     * the Color object is thereafter in the HSL space, converting from RGB if necessary.
     * Calling any getters or other const-qualified functions does not modify the object.
     * For example, calling getHue on an object that is in RGB space performs the same
     * conversion computations but leaves the object in RGB space.
     */
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
        std::array<float, 3> m_values;
        float m_alpha;

        enum class Space : std::uint8_t {
            RGB,
            HSL
        };

        Space m_space;
        
        Color(Space space, float v0, float v1, float v2, float alpha);

        void makeRGB() noexcept;
        void makeHSL() noexcept;

        std::array<float, 3> asRGB() const noexcept;
        std::array<float, 3> asHSL() const noexcept;
        
        friend bool operator==(const Color& a, const Color& b) noexcept;
        friend bool operator!=(const Color& a, const Color& b) noexcept;
    };

    

    Color interpolate(const Color& c0, const Color& c1, float t) noexcept;

    // TODO: alpha blending function(s)

};
