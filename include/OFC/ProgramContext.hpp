#pragma once

#include <memory>
#include <vector>
#include <SFML/System.hpp>

namespace ofc::ui {

    class Window;

    class ProgramContext {
    public:

        static ProgramContext& get();

        void addWindow(std::unique_ptr<Window> w);

        void removeWindow(const Window* w);

        void run(float desiredFPS = 30.0f);

        sf::Time getProgramTime() const;

        sf::Time getDoubleClickTime() const;

    private:
        ProgramContext() = default;
        ProgramContext(ProgramContext&&) = delete;
        ProgramContext(const ProgramContext&) = delete;
        ProgramContext& operator=(ProgramContext&&) = delete;
        ProgramContext& operator=(const ProgramContext&) = delete;
        ~ProgramContext() = default;

        std::vector<std::unique_ptr<Window>> m_windows;
        sf::Clock m_clock;
        sf::Time m_cachedTime;
    };

} // namespace ofc::ui
