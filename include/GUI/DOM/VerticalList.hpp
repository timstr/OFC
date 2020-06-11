#pragma once

#include <GUI/DOM/Container.hpp>

namespace ui::dom {

    // TODO: alignment options (left/center/right/justify)

    class VerticalList : public Container {
    public:
        VerticalList();
        
        using Container::children;

        std::size_t length() const noexcept;

        Element* getCell(std::size_t i);
        const Element* getCell(std::size_t i) const;

        void insert(std::size_t index, std::unique_ptr<Element>);
        template<typename T, typename... Args>
        T& insert(std::size_t index, Args&&... args);
        void erase(std::size_t index);

        void push_front(std::unique_ptr<Element>);
        template<typename T, typename... Args>
        T& push_front(Args&&... args);
        void pop_front();

        void push_back(std::unique_ptr<Element>);
        template<typename T, typename... Args>
        T& push_back(Args&&... args);
        void pop_back();

        void clear();

        float padding() const noexcept;
        void setPadding(float);

    private:
        vec2 update() override;

        void onRemoveChild(const Element*) override;

        std::vector<Element*> m_cells;
        float m_padding;
    };

    // TODO: move these to a .tpp file?

    template<typename T, typename... Args>
    inline T& VerticalList::insert(std::size_t index, Args&&... args){
        auto e = std::make_unique<T>(std::forward<Args>(args)...);
        auto& er = *e;
        this->insert(index, std::move(e));
        return er;
    }

    template<typename T, typename... Args>
    inline T& VerticalList::push_front(Args&&... args){
        auto e = std::make_unique<T>(std::forward<Args>(args)...);
        auto& er = *e;
        this->push_front(std::move(e));
        return er;
    }

    template<typename T, typename... Args>
    inline T& VerticalList::push_back(Args&&... args){
        auto e = std::make_unique<T>(std::forward<Args>(args)...);
        auto& er = *e;
        this->push_back(std::move(e));
        return er;
    }

} // namespace ui::dom
