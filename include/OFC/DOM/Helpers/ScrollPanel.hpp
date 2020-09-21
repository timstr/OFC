#pragma once

#include <OFC/DOM/Control.hpp>
#include <OFC/DOM/Container.hpp>

namespace ofc::ui::dom {

    template<typename ContainerType>
    class ScrollPanel : public Control, public Container {
    public:
        template<typename... Args>
        ScrollPanel(Args&&... args);

        void showHorizontalScrollBar(bool enable);

        void showVerticalScrollBar(bool enable);

        void showResizeButton(bool enable);

        ContainerType& inner();
        const ContainerType& inner() const;

    private:

        void onResize() override;

        class ScrollButton;
        class HorizontalScrollButton;
        class VerticalScrollButton;
        class ResizeButton;

        class InnerContent;

        ContainerType& m_inner;
    };

} // namespace ofc::ui::dom
