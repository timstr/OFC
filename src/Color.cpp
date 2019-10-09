#include <GUI/Color.hpp>

#include <algorithm>

namespace ui {



    Color::Color() : Color(0.0, 0.0, 0.0, 1.0){
    }

    Color::Color(float _r, float _g, float _b, float _a) :
        r(std::clamp(_r, 0.0f, 1.0f)),
        g(std::clamp(_g, 0.0f, 1.0f)),
        b(std::clamp(_b, 0.0f, 1.0f)),
        a(std::clamp(_a, 0.0f, 1.0f)) {

        updateHSLFromRGB();
    }

    Color::Color(uint32_t rgba) : 
        r(static_cast<float>((rgba & 0xFF000000) >> 24) / 255.0f),
        g(static_cast<float>((rgba & 0xFF0000) >> 16) / 255.0f),
        b(static_cast<float>((rgba & 0xFF00) >> 8) / 255.0f),
        a(static_cast<float>(rgba & 0xFF) / 255.0f) {

		updateHSLFromRGB();
    }

    Color::Color(const sf::Color& c) :
        r(std::clamp(c.r / 255.0f, 0.0f, 1.0f)),
        g(std::clamp(c.g / 255.0f, 0.0f, 1.0f)),
        b(std::clamp(c.b / 255.0f, 0.0f, 1.0f)),
        a(std::clamp(c.a / 255.0f, 0.0f, 1.0f)) {

        updateHSLFromRGB();
    }

    Color::operator sf::Color() const {
        return sf::Color(
            static_cast<uint8_t>(floor(255.0f * r)),
            static_cast<uint8_t>(floor(255.0f * g)),
            static_cast<uint8_t>(floor(255.0f * b)),
            static_cast<uint8_t>(floor(255.0f * a))
        );
    }

    Color Color::RGB(float r, float b, float g, float a){
        return Color(r, g, b, a);
    }

    Color Color::HSL(float h, float s, float l, float a){
        return Color();
    }

    float Color::red() const {
        return r;
    }

    float Color::green() const {
        return g;
    }

    float Color::blue() const {
        return b;
    }

    float Color::hue() const {
        return h;
    }

    float Color::saturation() const {
        return s;
    }

    float Color::lightness() const {
        return l;
    }

    float Color::alpha() const {
        return a;
    }

    void Color::setRed(float v){
        r = std::clamp(v, 0.0f, 1.0f);
        updateHSLFromRGB();
    }

    void Color::setGreen(float v){
        g = std::clamp(v, 0.0f, 1.0f);
        updateHSLFromRGB();
    }

    void Color::setBlue(float v){
        b = std::clamp(v, 0.0f, 1.0f);
        updateHSLFromRGB();
    }

    void Color::setHue(float v){
        h = v - floor(v);
        updateRGBFromHSL();
    }

    void Color::setSaturation(float v){
        s = std::clamp(v, 0.0f, 1.0f);
        updateRGBFromHSL();
    }

    void Color::setLightness(float v){
        l = std::clamp(v, 0.0f, 1.0f);
        updateRGBFromHSL();
    }

    void Color::setAlpha(float v){
        a = std::clamp(v, 0.0f, 1.0f);
    }

    uint32_t Color::toInt() const {
        return
            (static_cast<uint8_t>(floor(255.0f * r)) << 24) &
            (static_cast<uint8_t>(floor(255.0f * g)) << 16) &
            (static_cast<uint8_t>(floor(255.0f * b)) << 8) &
            static_cast<uint8_t>(floor(255.0f * a));
    }

    void Color::updateRGBFromHSL(){
        const float h1 = h * 6.0f;
        const float c = (1.0f - abs(2.0f * l - 1.0f)) * s;
        const float x = c * (1.0f - abs(1.0f - (6.0f * h1 - 2.0f * floor(3.0f * h1))));
        switch (static_cast<int>(floor(h1))){
        case 0:
            r = c;
            g = x;
            b = 0.0f;
            break;
        case 1:
            r = x;
            g = c;
            b = 0.0f;
            break;
        case 2:
            r = 0.0f;
            g = c;
            b = x;
            break;
        case 3:
            r = 0.0f;
            g = x;
            b = c;
            break;
        case 4:
            r = x;
            g = 0.0f;
            b = c;
            break;
        case 5:
            r = c;
            g = 0.0f;
            b = x;
            break;
        default:
            r = std::numeric_limits<float>::quiet_NaN();
            g = std::numeric_limits<float>::quiet_NaN();
            b = std::numeric_limits<float>::quiet_NaN();
            return;
        }
        const float m = l - c * 0.5f;
        r += m;
        g += m;
        b += m;
    }

    void Color::updateHSLFromRGB(){
        s = std::max(r, std::max(g, b)) - std::min(r, std::min(g, b));
        l = (std::min(r, std::min(g, b)) + std::max(r, std::max(g, b))) / 2.0f;

        if (s == 0.0f){
			h = 0.0f;
		} else if (r >= std::max(g, b)){
            float x = (g - b) / s / 6.0f;
			h = x - floor(x);
		} else if (g >= std::max(r, b)){
			h = ((b - r) / s + 2.0f) / 6.0f;
		} else if (b >= std::max(r, g)){
			h =  ((r - g) / s + 4.0f) / 6.0f;
		}
    }

    Color interpolate(const Color& c0, const Color& c1, float t) noexcept {
        return Color(
            (1.0f - t) * c0.red() + t * c1.red(),
            (1.0f - t) * c0.green() + t * c1.green(),
            (1.0f - t) * c0.blue() + t * c1.blue(),
            (1.0f - t) * c0.alpha() + t * c1.alpha()
        );
    }

} // namespace ui
