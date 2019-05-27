#pragma once

#include <GUI/Control.hpp>
#include <GUI/Container.hpp>

namespace ui {

    class ScrollPanel : public Control, public Container {
        // TODO

    public:
        Container& inner();
    };

} // namespace ui
