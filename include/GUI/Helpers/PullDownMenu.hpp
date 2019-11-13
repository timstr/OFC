#pragma once

#include <GUI/Control.hpp>
#include <GUI/FreeContainer.hpp>
#include <GUI/GridContainer.hpp>

#include <cassert>

namespace ui {

    // TODO: replace GridContainer with VerticalList once it supports uniform width

    namespace detail {
        template<typename T>
        class PullDownMenuBase : public FreeContainer, public Control {
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
            void select(std::size_t i, bool invokeCallback = true);

            bool isExpanded() const;

            void expand();

            void collapse();

            void onLoseFocus() override;

            bool onKeyDown(Key) override;

            std::vector<std::pair<T, String>> m_options;
            std::size_t m_selection;
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

    // TODO: move the following definitions to a .tpp file

    namespace detail {

        template<typename T>
        inline PullDownMenuBase<T>::PullDownMenuBase(
            std::vector<std::pair<T, String>> options,
            const sf::Font& font,
            std::function<void(const T&)> onChange
        )
            : m_options(std::move(options))
            , m_selection(0)
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
                    1,
                    m_options.size()
              )) {
            
            m_label.setMinWidth(50.0f);

            m_listItems.setMinWidth(50.0f);

            for (std::size_t i = 0; i < m_options.size(); ++i){
                auto cb = std::make_unique<CallbackButton>(
                    m_options[i].second,
                    font,
                    [this,i](){
                        select(i);
                        collapse();
                    }
                );
                cb->setBorderRadius(0.0f);
                m_buttons.push_back(cb.get());
                m_listItems.putCell(0, i, std::move(cb));
            }

            select(0, false);

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
            m_buttons[m_selection]->setNormalColor(
                interpolate(getNormalColor(), 0xFFFFFFFF, 0.2f)
            );
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
        inline void PullDownMenuBase<T>::select(std::size_t i, bool invoke){
            assert(i < m_options.size());
            m_buttons[m_selection]->setNormalColor(getNormalColor());
            m_selection = i;
            m_buttons[m_selection]->setNormalColor(
                interpolate(getNormalColor(), 0xFFFFFFFF, 0.2f)
            );
            m_label.getCaption().setText(m_options[m_selection].second);
            if (invoke){
                m_onChange(m_options[i].first);
            }
        }

        template<typename T>
        inline bool PullDownMenuBase<T>::isExpanded() const {
            return !m_listItemsUP;
        }

        template<typename T>
        inline void PullDownMenuBase<T>::expand(){
            assert(!isExpanded());
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
            assert(isExpanded());
            assert(m_listItems.getParentContainer() == this);
            m_listItemsUP = makeOrphan(m_listItems);
            assert(m_listItems.getParentContainer() == nullptr);
        }

        template<typename T>
        inline void PullDownMenuBase<T>::onLoseFocus(){
            if (!m_listItemsUP){
                collapse();
            }
        }

        template<typename T>
        inline bool PullDownMenuBase<T>::onKeyDown(Key key){
            if (key == Key::Up){
                select(std::max(m_selection, 1ull) - 1);
                return true;
            } else if (key == Key::Down){
                select(std::min(m_selection + 1, m_options.size() - 1));
                return true;
            } else if (key == Key::Home){
                select(0);
                return true;
            } else if (key == Key::End){
                select(m_options.size() - 1);
                return true;
            }
            return false;
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
