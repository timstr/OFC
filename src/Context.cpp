#include <GUI/Context.hpp>

#include <GUI/Window.hpp>

#include <algorithm>
#include <cassert>

namespace ui {

    Context& Context::get(){
        static Context the_instance;
        return the_instance;
    }

    void Context::addWindow(std::unique_ptr<Window> w){
        m_windows.push_back(std::move(w));
    }

    void Context::removeWindow(const Window* w){
        m_windows.erase(std::remove_if(
            m_windows.begin(),
            m_windows.end(),
            [w](const std::unique_ptr<Window>& pw){ return pw.get() == w; }
        ), m_windows.end());
    }

    void Context::run(float desiredFPS){
        assert(desiredFPS > 0.0f);
        const auto delayPerTick = sf::milliseconds(static_cast<int>(std::ceil(1.0f / desiredFPS)));
        const auto eventInterval = sf::milliseconds(10);
        while (m_windows.size() > 0){
            const auto doneTime = m_cachedTime + delayPerTick;
            do {
                m_cachedTime = m_clock.getElapsedTime();
                for (auto& win : m_windows) {
                    win->processEvents();
                }
            } while (m_cachedTime < doneTime);
            for (auto& win : m_windows){
                win->tick();
                win->redraw();
            }
        }
    }

    sf::Time Context::getProgramTime() const {
        return m_cachedTime;
    }

    sf::Time Context::getDoubleClickTime() const {
         return sf::seconds(0.25f);
    }

} // namespace ui
