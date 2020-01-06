#include <GUI/Context.hpp>

#include <GUI/Window.hpp>

#include <algorithm>

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

    void Context::run(){
        while (m_windows.size() > 0){
            m_cachedTime = m_clock.getElapsedTime();
            for (auto& win : m_windows){
                win->processEvents();
            }
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
