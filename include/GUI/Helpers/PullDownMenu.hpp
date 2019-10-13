#pragma once

#include <GUI/Control.hpp>
#include <GUI/FreeContainer.hpp>
#include <GUI/GridContainer.hpp>

#include <cassert>

namespace ui {

    namespace detail {
        template<typename T>
        class PullDownMenuBase : public FreeContainer {
        public:
            PullDownMenuBase(std::vector<std::pair<T, String>> options, const sf::Font& font, std::function<void(const T&)> onChange = {});
            
            void setOnChange(std::function<void(const T&)>);

		    void setNormalColor(Color);
		    Color getNormalColor() const;
		    void setHoverColor(Color);
		    Color getHoverColor() const;
		    void setActiveColor(Color);
		    Color getActiveColor() const;

        private:
            void expand();

            void collapse();

            std::vector<std::pair<T, String>> m_options;
            std::function<void(const T&)> m_onChange;
            CallbackButton& m_label;
            GridContainer& m_listItems;
            std::unique_ptr<GridContainer> m_listItemsUP;
            std::vector<ui::CallbackButton*> m_buttons;
        };
    }

    template<typename T>
    class PullDownMenu : public detail::PullDownMenuBase<T> {
    public:
        using detail::PullDownMenuBase<T>::PullDownMenuBase;
    };

    template<>
    class PullDownMenu<String> : public detail::PullDownMenuBase<String> {
    public:
        PullDownMenu(std::vector<String> options, const sf::Font& font, std::function<void(const String&)> onChange = {});

    private:
        std::vector<std::pair<String, String>> adaptOptions(const std::vector<String>& options) const;
    };

    namespace detail {

        template<typename T>
        inline PullDownMenuBase<T>::PullDownMenuBase(
            std::vector<std::pair<T, String>> options,
            const sf::Font& font,
            std::function<void(const T&)> onChange
        )
            : m_options(std::move(options))
            , m_onChange(std::move(onChange))
            , m_label(add<CallbackButton>(
                FreeContainer::InsideLeft,
                FreeContainer::InsideTop,
                m_options.front().second,
                font,
                [this](){ m_listItemsUP ? expand() : collapse(); }
              ))
            , m_listItems(
                add<GridContainer>(
                    FreeContainer::InsideLeft,
                    FreeContainer::OutsideBottom,
                    m_options.size(),
                    1
              )) {
            
            m_label.setMinWidth(50.0f);

            m_label.getCaption().setText(m_options.front().second);

            m_listItems.setMinWidth(50.0f);

            for (std::size_t i = 0; i < m_options.size(); ++i){
                auto cb = std::make_unique<CallbackButton>(
                    m_options[i].second,
                    font,
                    [this,i](){
                        m_label.getCaption().setText(m_options[i].second);
                        m_onChange(m_options[i].first);
                        collapse();
                    }
                );
                cb->setBorderRadius(0.0f);
                m_buttons.push_back(cb.get());
                m_listItems.putCell(i, 0, std::move(cb));
                m_listItems.setHorizontalFill(i, 0, true);
            }

            collapse();
        }

        template<typename T>
        inline void PullDownMenuBase<T>::setOnChange(std::function<void(const T&)> f){
            assert(f);
            m_onChange = std::move(f);
        }

        template<typename T>
        inline void PullDownMenuBase<T>::setNormalColor(Color c){
            m_label.setNormalColor(c);
            for (const auto& b : m_buttons){
                b->setNormalColor(c);
            }
        }

        template<typename T>
        inline Color PullDownMenuBase<T>::getNormalColor() const {
            return m_label.getNormalColor();
        }

        template<typename T>
        inline void PullDownMenuBase<T>::setHoverColor(Color c){
            m_label.setHoverColor(c);
            for (const auto& b : m_buttons){
                b->setHoverColor(c);
            }
        }

        template<typename T>
        inline Color PullDownMenuBase<T>::getHoverColor() const {
            return m_label.getHoverColor();
        }

        template<typename T>
        inline void PullDownMenuBase<T>::setActiveColor(Color c){
            m_label.setActiveColor(c);
            for (const auto& b : m_buttons){
                b->setActiveColor(c);
            }
        }

        template<typename T>
        inline Color PullDownMenuBase<T>::getActiveColor() const {
            return m_label.getActiveColor();
        }

        template<typename T>
        inline void PullDownMenuBase<T>::expand(){
            assert(m_listItemsUP);
            assert(m_listItems.getParentContainer() == nullptr);
            adopt(
                FreeContainer::InsideLeft,
                FreeContainer::OutsideBottom,
                std::move(m_listItemsUP)
            );
            assert(m_listItems.getParentContainer() == this);
            bringToFront();
        }

        template<typename T>
        inline void PullDownMenuBase<T>::collapse(){
            assert(!m_listItemsUP);
            assert(m_listItems.getParentContainer() == this);
            m_listItemsUP = makeOrphan(m_listItems);
            assert(m_listItems.getParentContainer() == nullptr);
        }

    } // namespace detail

    //template<>
    PullDownMenu<String>::PullDownMenu(std::vector<String> options, const sf::Font& font, std::function<void(const String&)> onChange)
        : detail::PullDownMenuBase<String>(adaptOptions(options), font, std::move(onChange)) {
        
    }
    
    std::vector<std::pair<String, String>> PullDownMenu<String>::adaptOptions(const std::vector<String>& options) const {
        std::vector<std::pair<String, String>> ret;
        for (const auto& opt : options){
            ret.push_back({opt, opt});
        }
        return ret;
    }

} // namespace ui
