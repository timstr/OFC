#pragma once

#include <GUI/Control.hpp>
#include <GUI/Container.hpp>

#include <type_traits>

namespace ui {

    template<typename T, typename Enable = void>
    class PullDownMenu;

    template<typename T>
    class PullDownMenu<T, std::enable_if_t<std::is_same_v<T, String>>> : public CallbackButton {
    public:
        PullDownMenu(std::vector<String> options, const sf::Font& font, std::function<void(const String&)> on_select);
        // TODO
    };

    template<typename T>
    class PullDownMenu<T, std::enable_if_t<!std::is_same_v<T, String>>> : public CallbackButton {
    public:
        PullDownMenu(std::vector<std::pair<T, String>> options, const sf::Font& font, std::function<void(const T&)> on_select);
        // TODO
    };

} // namespace ui
