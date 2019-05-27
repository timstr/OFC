#include <GUI/Context.hpp>

#include <GUI/Window.hpp>

#include <algorithm>

namespace ui {

    Context& Context::get(){
        static Context the_instance;
        return the_instance;
    }

    void Context::add_window(std::unique_ptr<Window> w){
        m_windows.push_back(std::move(w));
    }

    void Context::remove_window(const Window* w){
        m_windows.erase(std::remove_if(
            m_windows.begin(),
            m_windows.end(),
            [w](const std::unique_ptr<Window>& pw){ return pw.get() == w; }
        ), m_windows.end());
    }

    void Context::run(){
        while (m_windows.size() > 0){
            for (auto& win : m_windows){
                win->processEvents();
            }
            for (auto& win : m_windows){
                win->redraw();
            }
        }
    }

} // namespace ui
