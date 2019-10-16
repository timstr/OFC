#pragma once

#include <GUI/TextEntry.hpp>

namespace ui {

    template<typename T>
    class NumberTextEntry : public TextEntry {
    public:
        NumberTextEntry(T defaultVal, T min, T max, const sf::Font& font, std::function<void(T)> onReturn);

        void setMinimum(T);
        T minimum() const;
        void setMaximum(T);
        T maximum() const;
        void setValue(T);
        T value() const;

    private:
        void onReturn() override;

        bool validate() const override;

        void updateFromValue();

        std::optional<T> parseText() const;

        T m_value;
        T m_minimum;
        T m_maximum;
        std::function<void(T)> m_onReturn;
    };

    // TODO: move these to a .tpp file

    template<typename T>
    inline NumberTextEntry<T>::NumberTextEntry(T defaultVal, T min, T max, const sf::Font& font, std::function<void(T)> onReturn)
        : TextEntry(font)
        , m_value(defaultVal)
        , m_minimum(min)
        , m_maximum(max)
        , m_onReturn(std::move(onReturn)) {
        
        static_assert(std::is_arithmetic_v<T>, "T must be a numeric type.");

        updateFromValue();
    }

    template<typename T>
    inline void NumberTextEntry<T>::onReturn(){
        auto v = parseText();
        assert(v);
        assert(m_onReturn);
        m_value = *v;
        m_onReturn(m_value);
        updateFromValue();
    }

    template<typename T>
    inline bool NumberTextEntry<T>::validate() const {
        return parseText().has_value();
    }

    template<typename T>
    inline void NumberTextEntry<T>::updateFromValue(){
        setText(std::to_string(m_value));
    }

    template<typename T>
    inline std::optional<T> NumberTextEntry<T>::parseText() const {
        const auto s = text().toAnsiString();
        try {
            if constexpr (std::is_integral_v<T>){
                std::size_t idx;
                const auto v = std::stoll(s, &idx);
                if (idx != s.size()){
                    return {};
                }
                const auto clamped = std::clamp(
                    v,
                    static_cast<long long>(m_minimum),
                    static_cast<long long>(m_maximum)
                );
                return static_cast<T>(clamped);
            } else {
                std::size_t idx;
                const auto v = std::stod(s, &idx);
                if (idx != s.size()){
                    return {};
                }
                const auto clamped = std::clamp(
                    v,
                    static_cast<double>(m_minimum),
                    static_cast<double>(m_maximum)
                );
                return static_cast<T>(clamped);
            }
        } catch (...) {
            return {};
        }
    }

} // namespace ui

