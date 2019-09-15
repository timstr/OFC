#pragma once

#include <memory>
#include <vector>
#include <SFML/System.hpp>

namespace ui {

    class Window;

    class Context {
    public:

        static Context& get();

        void addWindow(std::unique_ptr<Window> w);

        void removeWindow(const Window* w);

        void run();

        sf::Time getProgramTime() const;

        sf::Time getDoubleClickTime() const;

    private:
        Context() = default;
        Context(Context&&) = delete;
        Context(const Context&) = delete;
        Context& operator=(Context&&) = delete;
        Context& operator=(const Context&) = delete;
        ~Context() = default;

        std::vector<std::unique_ptr<Window>> m_windows;
        sf::Clock m_clock;
        sf::Time m_cachedTime;
    };

} // namespace ui
