#include <OFC/Util/Color.hpp>

#include <OFC/Serialization.hpp>

#include <cassert>
#include <cmath>
#include <algorithm>

namespace ofc {

    std::array<float, 3> convertHSLToRGB(const std::array<float, 3>& values) noexcept {
        const auto& [h, s, l] = values;
        auto r = 0.0f;
        auto g = 0.0f;
        auto b = 0.0f;
        const float h1 = h * 6.0f;
        const float c = (1.0f - abs(2.0f * l - 1.0f)) * s;
        const float x = c * (1.0f - abs(1.0f - (6.0f * h1 - 2.0f * std::floor(3.0f * h1))));
        switch (static_cast<int>(std::floor(h1))){
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
            return {r, g, b};
        }
        const float m = l - c * 0.5f;
        r += m;
        g += m;
        b += m;
        return {
            std::clamp(r, 0.0f, 1.0f),
            std::clamp(g, 0.0f, 1.0f),
            std::clamp(b, 0.0f, 1.0f)
        };
    }

    std::array<float, 3> convertRGBToHSL(const std::array<float, 3>& values){
        const auto& [r, g, b] = values;
        auto h = 0.0f;
        auto s = 0.0f;
        auto l = 0.0f;

        s = std::max(r, std::max(g, b)) - std::min(r, std::min(g, b));
        l = (std::min(r, std::min(g, b)) + std::max(r, std::max(g, b))) / 2.0f;

        if (s == 0.0f){
            h = 0.0f;
        } else if (r >= std::max(g, b)){
            float x = (g - b) / s / 6.0f;
            h = x - std::floor(x);
        } else if (g >= std::max(r, b)){
            h = ((b - r) / s + 2.0f) / 6.0f;
        } else if (b >= std::max(r, g)){
            h =  ((r - g) / s + 4.0f) / 6.0f;
        }

        return {h, s, l};
    }

    void Color::makeRGB() noexcept {
        if (m_space == Space::RGB) {
            return;
        }
        assert(m_space == Space::HSL);
        m_values = convertHSLToRGB(m_values);
        m_space = Space::RGB;
    }

    void Color::makeHSL() noexcept {
        if (m_space == Space::HSL) {
            return;
        }
        assert(m_space == Space::RGB);
        m_values = convertRGBToHSL(m_values);
        m_space = Space::HSL;
    }

    std::array<float, 3> Color::asRGB() const noexcept {
        if (m_space == Space::RGB) {
            return m_values;
        }
        assert(m_space == Space::HSL);
        return convertHSLToRGB(m_values);
    }

    std::array<float, 3> Color::asHSL() const noexcept {
        if (m_space == Space::HSL) {
            return m_values;
        }
        assert(m_space == Space::RGB);
        return convertRGBToHSL(m_values);
    }

    Color::Color()
        : m_values{0.0f, 0.0f, 0.0f}
        , m_alpha(1.0f)
        , m_space(Space::RGB) {

    }

    Color::Color(Space space, float v0, float v1, float v2, float alpha)
        : m_values{v0, v1, v2}
        , m_alpha(alpha)
        , m_space(space) {

    }

    Color::Color(float _r, float _g, float _b, float _a)
        : m_values{
            std::clamp(_r, 0.0f, 1.0f),
            std::clamp(_g, 0.0f, 1.0f),
            std::clamp(_b, 0.0f, 1.0f)
        }
        , m_alpha(std::clamp(_a, 0.0f, 1.0f))
        , m_space(Space::RGB) {

    }

    Color::Color(uint32_t rgba)
        : m_values{
            static_cast<float>((rgba & 0xFF000000) >> 24) / 255.0f,
            static_cast<float>((rgba & 0xFF0000) >> 16) / 255.0f,
            static_cast<float>((rgba & 0xFF00) >> 8) / 255.0f,
        }
        , m_alpha(static_cast<float>(rgba & 0xFF) / 255.0f)
        , m_space(Space::RGB) {

    }

    Color::Color(const sf::Color& c)
        : m_values{
            static_cast<float>(c.r) / 255.0f,
            static_cast<float>(c.g) / 255.0f,
            static_cast<float>(c.b) / 255.0f
        }
        , m_alpha(static_cast<float>(c.a) / 255.0f)
        , m_space(Space::RGB) {

    }

    Color::operator sf::Color() const {
        const auto& [r, g, b] = asRGB();
        return sf::Color(
            static_cast<uint8_t>(std::floor(255.0f * r)),
            static_cast<uint8_t>(std::floor(255.0f * g)),
            static_cast<uint8_t>(std::floor(255.0f * b)),
            static_cast<uint8_t>(std::floor(255.0f * m_alpha))
        );
    }

    Color Color::RGB(float r, float b, float g, float a){
        return Color(r, g, b, a);
    }

    Color Color::HSL(float h, float s, float l, float a){
        return Color{
            Space::HSL, 
            h - std::round(h), // NOTE: hue wraps around
            std::clamp(s, 0.0f, 1.0f),
            std::clamp(l, 0.0f, 1.0f),
            std::clamp(a, 0.0f, 1.0f)
        };
    }

    float Color::red() const {
        return asRGB()[0];
    }

    float Color::green() const {
        return asRGB()[1];
    }

    float Color::blue() const {
        return asRGB()[2];
    }

    float Color::hue() const {
        return asHSL()[0];
    }

    float Color::saturation() const {
        return asHSL()[1];
    }

    float Color::lightness() const {
        return asHSL()[2];
    }

    float Color::alpha() const {
        return m_alpha;
    }

    void Color::setRed(float v){
        makeRGB();
        m_values[0] = std::clamp(v, 0.0f, 1.0f);
    }

    void Color::setGreen(float v){
        makeRGB();
        m_values[1] = std::clamp(v, 0.0f, 1.0f);
    }

    void Color::setBlue(float v){
        makeRGB();
        m_values[2] = std::clamp(v, 0.0f, 1.0f);
    }

    void Color::setHue(float v){
        makeHSL();
        m_values[0] = v - std::floor(v); // hue is modular
    }

    void Color::setSaturation(float v){
        makeHSL();
        m_values[1] = std::clamp(v, 0.0f, 1.0f);
    }

    void Color::setLightness(float v){
        makeHSL();
        m_values[2] = std::clamp(v, 0.0f, 1.0f);
    }

    void Color::setAlpha(float v){
        m_alpha = std::clamp(v, 0.0f, 1.0f);
    }

    uint32_t Color::toInt() const {
        const auto& [r, g, b] = asRGB();
        return
            (static_cast<uint8_t>(std::floor(255.0f * r)) << 24) &
            (static_cast<uint8_t>(std::floor(255.0f * g)) << 16) &
            (static_cast<uint8_t>(std::floor(255.0f * b)) << 8) &
            static_cast<uint8_t>(std::floor(255.0f * m_alpha));
    }

    bool operator==(const Color& a, const Color& b) noexcept {
        return (a.m_values == b.m_values) &&
            (a.m_alpha == b.m_alpha) &&
            (a.m_space == b.m_space);

    }
    bool operator!=(const Color& a, const Color& b) noexcept {
        return !(a == b);
    }

    void serialize(Serializer& s, const Color& c) {
        auto spaceCode = [&]() -> std::uint8_t {
            switch (c.m_space) {
            case Color::Space::RGB:
                return 0;
            case Color::Space::HSL:
                return 1;
            default:
                assert(false);
                return 0;
            }
        }();

        s.u8(spaceCode)
            .f32(c.m_values[0])
            .f32(c.m_values[1])
            .f32(c.m_values[2])
            .f32(c.m_alpha);
    }

    void deserialize(Deserializer& s, Color& c) {
        auto space = [&]() {
            switch (s.u8()) {
            case 0:
                return Color::Space::RGB;
            case 1:
                return Color::Space::HSL;
            default:
                throw SerializationException{};
            }
        }();

        c.m_space = space;
        c.m_values[0] = s.f32();
        c.m_values[1] = s.f32();
        c.m_values[2] = s.f32();
        c.m_alpha = s.f32();
    }

    Color interpolate(const Color& c0, const Color& c1, float t) noexcept {
        return Color(
            (1.0f - t) * c0.red() + t * c1.red(),
            (1.0f - t) * c0.green() + t * c1.green(),
            (1.0f - t) * c0.blue() + t * c1.blue(),
            (1.0f - t) * c0.alpha() + t * c1.alpha()
        );
    }

} // namespace ofc
