#pragma once

#include <memory>
#include <vector>

namespace ui {

    class Window;

    class Context {
    public:

        static Context& get();

        void add_window(std::unique_ptr<Window> w);

        void remove_window(const Window* w);

        void run();

    private:
        Context() = default;
        Context(Context&&) = delete;
        Context(const Context&) = delete;
        Context& operator=(Context&&) = delete;
        Context& operator=(const Context&) = delete;
        ~Context() = default;

        std::vector<std::unique_ptr<Window>> m_windows;
    };

} // namespace ui
