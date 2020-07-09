#pragma once

#include <GUI/DOM/Control.hpp>
#include <GUI/DOM/BoxElement.hpp>
#include <GUI/DOM/Draggable.hpp>

namespace ui {
    
    //------------------------
    // Tag classes used to represent mixable element traits

    class Boxy;
    // Properties: backgroundColor, borderColor, borderThickness, borderRadius

    class Resizable;
    // Properties: width,height,minWidth,minHeight,maxWidth,maxHeight,widthForce,heightForce

    class Positionable;
    // Properties: left, top

    class Scrollable;
    // Handlers: onScroll

    class Hoverable;
    // Handlers: onHover, onHoverWith

    class Clickable;
    // Handlers: onLeftClick, onLeftRelease, onMiddleClick, onMiddleRelease, onRightClick, onRightRelease

    class KeyPressable;
    // Handlers: onKeyDown, onKeyUp

    class Focusable;
    // Handlers: onGainFocus, onLoseFocus
    // Actions: grabFocus

    class Draggable;
    // Handlers: onDrag
    // Actions: startDrag, stopDrag, drop

    namespace mix {
        //------------------------
        // Primary templates used to map element trait tags to various implementations
    
        // The element base class upon which 
        // specializations shall contain a member typename Type equal to the desired dom::Element subclass
        template<typename Tag>
        struct ElementBase {
            using Type = dom::Element;
        };

        template<typename Tag>
        using ElementBaseType = typename ElementBase<Tag>::Type;

        // Partial specializations shall specialize Tag but not DerivedComponent, and need not inherit from anything.
        // This will be used in the CRTP; DerivedComponent shall be left unspecialized so that it may be used as the derived type.
        // casting 'this' to DerivedComponent* is thus allowed in member functions, for example, to return a reference to the
        // component from named-parameter-idiom methods as used in other Component interfaces.
        template<typename Tag, typename... AllTags>
        class ComponentBase;

        // Partial specializations shall specialize Tag but not BaseElement, and shall inherit from BaseElement.
        // BaseElement shall derive from the ElementBaseType specialized for the same tag. The constructor of this
        // class must accept an l-value reference to MixedComponent<AllTags...> and use that reference
        // to assign any properties to the element upon initialization.
        template<typename Tag, typename BaseElement, typename... AllTags>
        class ElementMixin;

        // Partial specializations shall provide member functions for performing actions on instantiated dom::Elements
        // This will be used in the CRTP; DerivedAction shall be left unspecialized so that it may be used as the derived type.
        // casting this to DerivedAction* is thus allowed in member functions, for example, to call element() and access
        // the instantiated dom::Element.
        // The primary template is defined and provides no members. Tags which don't need any actions don't need to provide
        // a specialization.
        template<typename Tag, typename DerivedAction>
        class ActionBase {
    
        };

        //------------------------
        // Forward declarations needed by tag specializations

        template<typename... Tags>
        class MixedAction;

        template<typename... Tags>
        class MixedComponent;

        //------------------------
        // CRTP-style helper for ComponentBase specializations
        template<typename CurrentTag, typename... Tags>
        class ComponentBaseHelper {
        public:
            using ComponentType = MixedComponent<Tags...>;
            using ComponentBaseDerived = ComponentBase<CurrentTag, Tags...>;
        
            decltype(auto) self() noexcept {
                return static_cast<ComponentType&>(*this);
            }

            auto selfPtr() noexcept {
                return static_cast<ComponentType*>(this);
            }

            template<typename... Args>
            auto memFn(void (ComponentBaseDerived::* f)(Args...)) {
                return static_cast<void (ComponentType::*)(Args...)>(f);
            }

            auto getElement() noexcept {
                auto e = self().element();
                assert(e);
                return e;
            }
        };

        //------------------------
        // Specializations for Boxy

        template<>
        struct ElementBase<Boxy> {
            using Type = dom::BoxElement;
        };

        template<typename... AllTags>
        class ComponentBase<Boxy, AllTags...> : private ComponentBaseHelper<Boxy, AllTags...> {
        public:
            ComponentBase()
                : m_backgroundColorObserver(selfPtr(), memFn(&ComponentBase::onUpdateBackgroundColor))
                , m_borderColorObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderColor))
                , m_borderThicknessObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderThickness))
                , m_borderRadiusObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderRadius)) {

            }

            decltype(auto) backgroundColor(PropertyOrValue<Color> p) {
                m_backgroundColorObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderColor(PropertyOrValue<Color> p) {
                m_borderColorObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderThickness(PropertyOrValue<float> p) {
                m_borderThicknessObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderRadius(PropertyOrValue<float> p) {
                m_borderRadiusObserver.assign(std::move(p));
                return self();
            }

        private:
            Observer<Color> m_backgroundColorObserver;
            Observer<Color> m_borderColorObserver;
            Observer<float> m_borderThicknessObserver;
            Observer<float> m_borderRadiusObserver;
        
            void onUpdateBackgroundColor(const Color& c) {
                getElement()->setBackgroundColor(c);
            }

            void onUpdateBorderColor(const Color& c) {
                getElement()->setBorderColor(c);
            }

            void onUpdateBorderThickness(float v) {
                getElement()->setBorderThickness(v);
            }

            void onUpdateBorderRadius(float v) {
                getElement()->setBorderRadius(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Boxy, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::BoxElement");
            
                maybeCall(component.m_backgroundColorObserver, &ElementMixin::setBackgroundColor);
                maybeCall(component.m_borderColorObserver, &ElementMixin::setBorderColor);
                maybeCall(component.m_borderThicknessObserver, &ElementMixin::setBorderThickness);
                maybeCall(component.m_borderRadiusObserver, &ElementMixin::setBorderRadius);
            }

        private:
            template<typename T, typename U>
            void maybeCall(Observer<T>& o, void (ElementMixin::* f)(U)) {
                if (o.hasValue()) {
                    (this->*f)(o.getValueOnce());
                }
            }
        };

        //------------------------
        // Specializations for Resizable

        template<>
        struct ElementBase<Resizable> {
            using Type = dom::Element;
        };

        template<typename... AllTags>
        class ComponentBase<Resizable, AllTags...> : private ComponentBaseHelper<Resizable, AllTags...> {
        public:
            ComponentBase()
                : m_widthObserver(selfPtr(), memFn(&ComponentBase::onUpdateWidth))
                , m_heightObserver(selfPtr(), memFn(&ComponentBase::onUpdateHeight))
                , m_minWidthObserver(selfPtr(), memFn(&ComponentBase::onUpdateMinWidth))
                , m_minHeightObserver(selfPtr(), memFn(&ComponentBase::onUpdateMinHeight))
                , m_maxWidthObserver(selfPtr(), memFn(&ComponentBase::onUpdateMaxWidth))
                , m_maxHeightObserver(selfPtr(), memFn(&ComponentBase::onUpdateMaxHeight)) {

            }

            decltype(auto) width(PropertyOrValue<float> p) {
                m_widthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) height(PropertyOrValue<float> p) {
                m_heightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) size(PropertyOrValue<vec2> p) {
                m_widthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_heightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) minWidth(PropertyOrValue<float> p) {
                m_minWidthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) minHeight(PropertyOrValue<float> p) {
                m_minHeightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) minSize(PropertyOrValue<vec2> p) {
                m_minWidthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_minHeightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) maxWidth(PropertyOrValue<float> p) {
                m_maxWidthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) maxHeight(PropertyOrValue<float> p) {
                m_maxHeightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) maxSize(PropertyOrValue<vec2> p) {
                m_maxWidthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_maxHeightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) widthForce(PropertyOrValue<float> p) {
                m_widthObserver.assign(std::move(p));
                m_minWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                m_maxWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                return self();
            }

            decltype(auto) heightForce(PropertyOrValue<float> p) {
                m_heightObserver.assign(std::move(p));
                m_minHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                m_maxHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                return self();
            }

            decltype(auto) sizeForce(PropertyOrValue<vec2> p) {
                m_widthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_minWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                m_maxWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                m_heightObserver.assign(p.map([](vec2 v){ return v.y; }));
                m_minHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                m_maxHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                return self();
            }

        private:
            Observer<float> m_widthObserver;
            Observer<float> m_heightObserver;
            Observer<float> m_minWidthObserver;
            Observer<float> m_minHeightObserver;
            Observer<float> m_maxWidthObserver;
            Observer<float> m_maxHeightObserver;

            void onUpdateWidth(float v) {
                getElement()->setWidth(v);
            }

            void onUpdateHeight(float v) {
                getElement()->setHeight(v);
            }

            void onUpdateMinWidth(float v) {
                getElement()->setMinWidth(v);
            }

            void onUpdateMinHeight(float v) {
                getElement()->setMinHeight(v);
            }

            void onUpdateMaxWidth(float v) {
                getElement()->setMaxWidth(v);
            }

            void onUpdateMaxHeight(float v) {
                getElement()->setMaxHeight(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Resizable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Element, BaseElement>, "Base class must derive from ui::dom::Element");
            
                // NOTE: Element::setWidth and Element::setHeight have hidden default
                // parameters, preventing maybeCall from working
                if (component.m_widthObserver.hasValue()){
                    setWidth(component.m_widthObserver.getValueOnce());
                }
                if (component.m_heightObserver.hasValue()){
                    setHeight(component.m_heightObserver.getValueOnce());
                }
                maybeCall(component.m_minWidthObserver, &ElementMixin::setMinWidth);
                maybeCall(component.m_minHeightObserver, &ElementMixin::setMinHeight);
                maybeCall(component.m_maxWidthObserver, &ElementMixin::setMaxWidth);
                maybeCall(component.m_maxHeightObserver, &ElementMixin::setMaxHeight);
            }

        private:
            template<typename T, typename U>
            void maybeCall(Observer<T>& o, void (ElementMixin::* f)(U)) {
                if (o.hasValue()) {
                    (this->*f)(o.getValueOnce());
                }
            }

        };

        //------------------------
        // Specializations for Positionable

        template<>
        struct ElementBase<Positionable> {
            using Type = dom::Element;
        };

        template<typename... AllTags>
        class ComponentBase<Positionable, AllTags...> : private ComponentBaseHelper<Positionable, AllTags...> {
        public:

            ComponentBase()
                : m_leftObserver(selfPtr(), memFn(&ComponentBase::onUpdateLeft))
                , m_topObserver(selfPtr(), memFn(&ComponentBase::onUpdateTop)) {

            }

            decltype(auto) position(PropertyOrValue<vec2> p) {
                m_leftObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_topObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) left(PropertyOrValue<float> p) {
                m_leftObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) top(PropertyOrValue<float> p) {
                m_topObserver.assign(std::move(p));
                return self();
            }


        private:
            Observer<float> m_leftObserver;
            Observer<float> m_topObserver;

            void onUpdateLeft(float v) {
                getElement()->setLeft(v);
            }

            void onUpdateTop(float v) {
                getElement()->setTop(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Positionable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Element, BaseElement>, "Base class must derive from ui::dom::Element");
            
                maybeCall(component.m_leftObserver, &ElementMixin::setLeft);
                maybeCall(component.m_topObserver, &ElementMixin::setTop);
            }

        private:
            template<typename T, typename U>
            void maybeCall(Observer<T>& o, void (ElementMixin::* f)(U)) {
                if (o.hasValue()) {
                    (this->*f)(o.getValueOnce());
                }
            }

        };

        //------------------------
        // Specializations for Scrollable

        template<>
        struct ElementBase<Scrollable> {
            using Type = dom::Control;
        };

        template<typename... AllTags>
        class ComponentBase<Scrollable, AllTags...> : private ComponentBaseHelper<Scrollable, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onScroll(std::function<bool(vec2)> f) {
                m_onScroll = [f = std::move(f)](vec2 d, Action /* unused */){
                    return f(d);
                };
                return self();
            }
            decltype(auto) onScroll(std::function<bool(vec2, Action)> f) {
                m_onScroll = std::move(f);
                return self();
            }

        private:
            std::function<bool(vec2, Action)> m_onScroll;

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Scrollable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            bool onScroll(vec2 delta) override final {
                auto& f = component().m_onScroll;
                if (f) {
                    return f(delta, Action{this});
                }
                return false;
            }
        };

        //------------------------
        // Specializations for Hoverable

        // TODO: onHover/onDrop both take pointers to dom::Draggable elements, but these
        // are meaningless in the context of the Component API. Perhaps the Draggable
        // and Hoverable traits should agree on some kind of signal type to distinguish elements?

        //------------------------
        // Specializations for Clickable

        template<>
        struct ElementBase<Clickable> {
            using Type = dom::Control;
        };

        template<typename... AllTags>
        class ComponentBase<Clickable, AllTags...> : private ComponentBaseHelper<Clickable, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onLeftClick(std::function<bool(int)> f) {
                m_onLeftClick = [f = std::move(f)](int i, Action /* unused */){
                    return f(i);
                };
                return self();
            }
            decltype(auto) onLeftClick(std::function<bool(int, Action)> f) {
                m_onLeftClick = std::move(f);
                return self();
            }
            decltype(auto) onLeftRelease(std::function<void()> f) {
                m_onLeftRelease = [f = std::move(f)](Action /* unused */){
                    return f();
                };
                return self();
            }
            decltype(auto) onLeftRelease(std::function<void(Action)> f) {
                m_onLeftRelease = std::move(f);
                return self();
            }
            decltype(auto) onMiddleClick(std::function<bool(int)> f) {
                m_onMiddleClick = [f = std::move(f)](int i, Action /* unused */){
                    return f(i);
                };
                return self();
            }
            decltype(auto) onMiddleClick(std::function<bool(int, Action)> f) {
                m_onMiddleClick = std::move(f);
                return self();
            }
            decltype(auto) onMiddleRelease(std::function<void()> f) {
                m_onMiddleRelease = [f = std::move(f)](Action /* unused */){
                    return f();
                };
                return self();
            }
            decltype(auto) onMiddleRelease(std::function<void(Action)> f) {
                m_onMiddleRelease = std::move(f);
                return self();
            }
            decltype(auto) onRightClick(std::function<bool(int)> f) {
                m_onRightClick = [f = std::move(f)](int i, Action /* unused */){
                    return f(i);
                };
                return self();
            }
            decltype(auto) onRightClick(std::function<bool(int, Action)> f) {
                m_onRightClick = std::move(f);
                return self();
            }
            decltype(auto) onRightRelease(std::function<void()> f) {
                m_onRightRelease = [f = std::move(f)](Action /* unused */){
                    return f();
                };
                return self();
            }
            decltype(auto) onRightRelease(std::function<void(Action)> f) {
                m_onRightRelease = std::move(f);
                return self();
            }

        private:
            std::function<bool(int, Action)> m_onLeftClick;
            std::function<void(Action)> m_onLeftRelease;
            std::function<bool(int, Action)> m_onMiddleClick;
            std::function<void(Action)> m_onMiddleRelease;
            std::function<bool(int, Action)> m_onRightClick;
            std::function<void(Action)> m_onRightRelease;

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Clickable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {

                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            bool onLeftClick(int i) override final {
                auto& f = component().m_onLeftClick;
                if (f) {
                    return f(i, Action{this});
                }
                return false;
            }

            void onLeftRelease() override final {
                auto& f = component().m_onLeftRelease;
                if (f) {
                    f(Action{this});
                }
            }

            bool onMiddleClick(int i) override final {
                auto& f = component().m_onMiddleClick;
                if (f) {
                    return f(i, Action{this});
                }
                return false;
            }

            void onMiddleRelease() override final {
                auto& f = component().m_onMiddleRelease;
                if (f) {
                    f(Action{this});
                }
            }

            bool onRightClick(int i) override final {
                auto& f = component().m_onRightClick;
                if (f) {
                    return f(i, Action{this});
                }
                return false;
            }

            void onRightRelease() override final {
                auto& f = component().m_onRightRelease;
                if (f){
                    f(Action{this});
                }
            }
        };
    
        //------------------------
        // Specializations for KeyPressable

        template<>
        struct ElementBase<KeyPressable> {
            using Type = dom::Control;
        };

        template<typename... AllTags>
        class ComponentBase<KeyPressable, AllTags...> : private ComponentBaseHelper<KeyPressable, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onKeyDown(std::function<bool(Key)> f) {
                m_onKeyDown = [f = std::move(f)](Key k, Action /* unused */){
                    return f(k);
                };
                return self();
            }
            decltype(auto) onKeyDown(std::function<bool(Key, Action)> f) {
                m_onKeyDown = std::move(f);
                return self();
            }
            decltype(auto) onKeyUp(std::function<void(Key)> f) {
                m_onKeyUp = [f = std::move(f)](Key k, Action /* unused */){
                    return f(k);
                };
                return self();
            }
            decltype(auto) onKeyUp(std::function<void(Key, Action)> f) {
                m_onKeyUp = std::move(f);
                return self();
            }

        private:
            std::function<bool(Key, Action)> m_onKeyDown;
            std::function<bool(Key, Action)> m_onKeyUp;

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<KeyPressable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            bool onKeyDown(Key key) override final {
                auto& f = component().m_onKeyDown;
                if (f) {
                    return f(key, Action{this});
                }
                return false;
            }

            void onKeyUp(Key key) override final {
                auto& f = component().m_onKeyUp;
                if (f) {
                    f(key, Action{this});
                }
            }
        };

        //------------------------
        // Specializations for Focusable

        template<typename DerivedAction>
        class ActionBase<Focusable, DerivedAction> {
        public:
            void grabFocus() {
                auto self = static_cast<DerivedAction*>(this);
                auto c = self->element().toControl();
                assert(c);
                c->grabFocus();
            }
        };

        template<>
        struct ElementBase<Focusable> {
            using Type = dom::Control;
        };

        template<typename... AllTags>
        class ComponentBase<Focusable, AllTags...> : private ComponentBaseHelper<Focusable, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onGainFocus(std::function<void()> f) {
                m_onGainFocus = [f = std::move(f)](Action /* unused */){
                    f();
                };
                return self();
            }
            decltype(auto) onGainFocus(std::function<void(Action)> f) {
                m_onGainFocus = std::move(f);
                return self();
            }
            decltype(auto) onLoseFocus(std::function<void()> f) {
                m_onLoseFocus = [f = std::move(f)](Action /* unused */){
                    f();
                };
                return self();
            }
            decltype(auto) onLoseFocus(std::function<void(Key, Action)> f) {
                m_onLoseFocus = std::move(f);
                return self();
            }

        private:
            std::function<void(Action)> m_onGainFocus;
            std::function<void(Action)> m_onLoseFocus;

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Focusable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            void onGainFocus() override final {
                auto& f = component().m_onGainFocus;
                if (f) {
                    f(Action{this});
                }
            }

            void onLoseFocus() override final {
                auto& f = component().m_onLoseFocus;
                if (f) {
                    return f(Action{this});
                }
            }
        };

        //------------------------
        // Specializations for Draggable

        template<typename DerivedAction>
        class ActionBase<Draggable, DerivedAction> {
        public:
            void startDrag() {
                auto self = static_cast<DerivedAction*>(this);
                auto c = self->element().toDraggable();
                assert(c);
                c->startDrag();
            }

            void stopDrag() {
                auto self = static_cast<DerivedAction*>(this);
                auto c = self->element().toDraggable();
                assert(c);
                c->stopDrag();
            }

            void drop(vec2 where = {0.0f, 0.0f}) {
                auto self = static_cast<DerivedAction*>(this);
                auto c = self->element().toDraggable();
                assert(c);
                c->drop();
            }
        };

        template<>
        struct ElementBase<Draggable> {
            using Type = dom::Draggable;
        };

        template<typename... AllTags>
        class ComponentBase<Draggable, AllTags...> : private ComponentBaseHelper<Draggable, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onDrag(std::function<void(vec2)> f) {
                m_onDrag = [f = std::move(f)](vec2 d, Action /* unused */){
                    f(d);
                };
                return self();
            }
            decltype(auto) onGainFocus(std::function<void(vec2, Action)> f) {
                m_onDrag = std::move(f);
                return self();
            }

        private:
            std::function<void(vec2, Action)> m_onDrag;

            friend ElementMixin;
        };

        template<typename BaseElement, typename... AllTags>
        class ElementMixin<Draggable, BaseElement, AllTags...> : public BaseElement {
        public:
            using ComponentType = MixedComponent<AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Draggable, BaseElement>, "Base class must derive from ui::dom::Draggable");
            }

            void onDrag() override final {
                auto& f = component().m_onDrag;
                if (f) {
                    f(pos(), Action{this});
                }
            }
        };

        //------------------------
        // Helper template building blocks

        template<typename T, template<typename...> typename To>
        struct ReapplyImpl;

        template<template<typename...> typename From, typename... Args, template<typename...> typename To>
        struct ReapplyImpl<From<Args...>, To> {
            using Type = To<Args...>;
        };

        // Given a desired (uninstantiated) variadic template and a given instantiation
        // of another variadic template, returns the desired variadic template
        // instantiated with the same parameters
        template<typename FromApplied, template<typename...> typename To>
        using Reapply = typename ReapplyImpl<FromApplied, To>::Type;


        // Given a template taking two parameters and a type, effectively binds that type
        // to the first parameter and returns a new template taking a single parameter
        // which maps to the second parameter.
        template<template<typename, typename...> typename TT, typename... U>
        struct Curry {
    
            template<typename... T>
            using Result = TT<U..., T...>;
        };



        template<typename... Ts>
        struct FilterImpl;

        template<typename ToRemove, typename... Acc>
        struct FilterImpl<ToRemove, std::tuple<Acc...>> {
            using Type = std::tuple<Acc...>;
        };

        template<typename ToRemove, typename... Acc, typename T, typename... Rest>
        struct FilterImpl<ToRemove, std::tuple<Acc...>, T, Rest...>
            : std::conditional_t<
                std::is_same_v<ToRemove, T>,
                FilterImpl<ToRemove, std::tuple<Acc...>, Rest...>,
                FilterImpl<ToRemove, std::tuple<Acc..., T>, Rest...>
            > {

        };

        // Given a type to remove and a list of types, returns a std::tuple containing
        // that list of types minus any occurences of the type to remove
        template<typename ToRemove, typename... Ts>
        using Filter = typename FilterImpl<ToRemove, std::tuple<>, Ts...>::Type;



        // Adapted from https://stackoverflow.com/a/57528226/5023438
        template<typename... Ts>
        struct RemoveDuplicatesImpl;

        template<typename T, typename... Ts>
        struct RemoveDuplicatesImpl<T, Ts...> {
            using Type = T;
        };

        template<typename... Ts, typename U, typename... Us>
        struct RemoveDuplicatesImpl<std::tuple<Ts...>, U, Us...>
            : std::conditional_t<
                (std::is_same_v<U, Ts> || ...), // Does U match anything in TS?
                RemoveDuplicatesImpl<std::tuple<Ts...>, Us...>, // if yes, recurse but don't add the type to the tuple
                RemoveDuplicatesImpl<std::tuple<Ts..., U>, Us...> // if no, recurse and add the type to the tuple
            > {

        };

        // Given a list of types, returns a std::tuple containing the same types minus any
        // duplicates. In case of duplicate types anywhere in the list, only the leftmost
        // occurence is preserved.
        template<typename... Ts>
        using RemoveDuplicates = typename RemoveDuplicatesImpl<std::tuple<>, Ts...>::Type;


        // Given a list of types, inherits from all those types separately and independently
        template<typename... Bases>
        class InheritParallel : public Bases... {

        };


        // Given an initial base class (base case base class, hence BaseBase), and a list
        // of mixin templates, creates an inheritance chain with BaseBase at the root and
        // with each Mixin deriving in order from the previous class.
        // Each mixin is expected to be a class that derives from its template parameter.
        template<typename BaseBase, template<typename> typename... Mixins>
        class InheritSerial;

        template<typename BaseBase>
        class InheritSerial<BaseBase> : public BaseBase {
        public:
            template<typename... Args>
            InheritSerial(Args&&... args)
                : BaseBase(std::forward<Args>(args)...) {
            
            }
        };

        template<typename BaseBase, template<typename> typename Mixin, template<typename> typename... Rest>
        class InheritSerial<BaseBase, Mixin, Rest...>
            : public Mixin<InheritSerial<BaseBase, Rest...>> {
        public:
            template<typename... Args>
            InheritSerial(Args&&... args)
                : Mixin<InheritSerial<BaseBase, Rest...>>(std::forward<Args>(args)...) {
            
            }
        };
     
        //------------------------
        // Element base class from list of required bases
        template<typename... Tags>
        class CommonElementBase
            : public std::conditional_t<
                // If all bases are dom::Element
                (std::is_same_v<ElementBaseType<Tags>, dom::Element> && ...),

                // then just inherit from dom::Element
                dom::Element,

                // otherwise, remove all occurences of dom::Element, remove any duplicates,
                // and inherit from all the resulting types in parallel
                Reapply<
                    Reapply<
                        RemoveDuplicates<
                            ElementBaseType<Tags>...
                        >,
                        Curry<Filter, dom::Element>::template Result
                    >,
                    InheritParallel
                >
            > {
        public:

            CommonElementBase(MixedComponent<Tags...>& component)
                : m_component(component) {
                static_assert(
                    (std::is_base_of_v<dom::Element, ElementBaseType<Tags>> && ...),
                    "All element bases must derive from ui::dom::Element"
                );
            }

            MixedComponent<Tags...>& component() noexcept {
                return m_component;
            }

        private:
            MixedComponent<Tags...>& m_component;
        };

        //------------------------

        template<typename Tag, typename... Tags>
        struct ElementMixinPartial {
            template<typename BaseElement>
            using Result = ElementMixin<Tag, BaseElement, Tags...>;
        };

        template<typename... Tags>
        using MixedElement = InheritSerial<
            CommonElementBase<Tags...>,
            ElementMixinPartial<Tags, Tags...>::template Result...
        >;

        template<typename... Tags>
        class MixedAction : public ActionBase<Tags, MixedAction<Tags...>>... {
        public:
            MixedAction(dom::Element* e)
                : m_element(e) {
                assert(m_element);
            }

            dom::Element& element() noexcept {
                assert(m_element);
                return *m_element;
            }

        private:
            dom::Element* const m_element;
        };

        template<typename... Tags>
        class MixedComponent
            : public SimpleComponent<MixedElement<Tags...>>
            , public ComponentBase<Tags, Tags...>... {
        public:

            // NOTE: various named-parameter-idiom methods are inherited from CRTP 'Features' base classes
            using Action = MixedAction<Tags...>;

        private:
            std::unique_ptr<MixedElement<Tags...>> createElement() override final {
                static_assert(
                    std::is_constructible_v<MixedElement<Tags...>, MixedComponent<Tags...>&>,
                    "The Element must be constructible from a reference to the mixed component type"
                );
                return std::make_unique<MixedElement<Tags...>>(*this);
            }
        };

    } // namespace mix

    template<typename... Tags>
    using MixedComponent = mix::MixedComponent<Tags...>;

} // namespace ui
