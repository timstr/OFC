#pragma once

#include <OFC/DOM/Control.hpp>
#include <OFC/DOM/BoxElement.hpp>
#include <OFC/DOM/Draggable.hpp>
#include <OFC/Component/Component.hpp>
#include <OFC/Component/Containers.hpp>
#include <OFC/Observer.hpp>

#include <OFC/Util/TemplateMetaProgramming.hpp>
#include <OFC/Util/UniqueAny.hpp>

namespace ofc::ui {
    
    //------------------------
    // Tag classes used to represent mixable element traits

    class Boxy {};
    // Properties: backgroundColor, borderColor, borderThickness, borderRadius

    class Resizable {};
    // Properties: width,height,minWidth,minHeight,maxWidth,maxHeight,widthForce,heightForce

    class Positionable {};
    // Properties: left, top

    class HitTestable {};
    // Handlers: hitTest

    class Draggable {};
    // Handlers: onDrag
    // Actions: startDrag, startDragWith<T>, stopDrag, drop<T>

    class Scrollable {};
    // Handlers: onScroll

    class Hoverable {};
    // Handlers: onMouseEnter, onMousEnterWith<T>, onMouseLeave, onMouseLeaveWith<T>, onDrop<T>, 

    class Clickable {};
    // Handlers: onLeftClick, onLeftRelease, onMiddleClick, onMiddleRelease, onRightClick, onRightRelease

    class KeyPressable {};
    // Handlers: onKeyDown, onKeyUp

    class Focusable {};
    // Handlers: onGainFocus, onLoseFocus
    // Actions: grabFocus

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
        template<typename Tag, template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase;

        // Partial specializations shall specialize Tag but not BaseElement, and shall inherit from BaseElement.
        // BaseElement shall derive from the ElementBaseType specialized for the same tag. The constructor of this
        // class must accept an l-value reference to MixedComponent<AllTags...> and use that reference
        // to assign any properties to the element upon initialization.
        template<typename Tag, typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
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

        template<template<typename> typename ContainerComponentType, typename... Tags>
        class MixedContainerComponent;

        template<typename Unused>
        class NoContainer {};

        template<template<typename> typename ContainerComponentType, typename... Tags>
        struct DerivedComponentImpl {
            using Type = MixedContainerComponent<ContainerComponentType, Tags...>;
        };

        template<typename... Tags>
        struct DerivedComponentImpl<NoContainer, Tags...> {
            using Type = MixedComponent<Tags...>;
        };

        template<template<typename> typename ContainerComponentType, typename... Tags>
        using DerivedComponent = typename DerivedComponentImpl<ContainerComponentType, Tags...>::Type;
        
        //------------------------
        // CRTP-style helper for ComponentBase specializations
        template<typename CurrentTag, template<typename> typename ContainerComponentType, typename... Tags>
        class ComponentBaseHelper {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, Tags...>;
            using ComponentBaseDerived = ComponentBase<CurrentTag, ContainerComponentType, Tags...>;
        
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

            decltype(auto) element() noexcept {
                auto e = self().getDerivedElement();
                assert(e);
                return *e;
            }
        };

        //------------------------
        // Specializations for Boxy

        template<>
        struct ElementBase<Boxy> {
            using Type = dom::BoxElement;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Boxy, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Boxy, ContainerComponentType, AllTags...> {
        public:
            ComponentBase()
                : m_backgroundColorObserver(selfPtr(), memFn(&ComponentBase::onUpdateBackgroundColor))
                , m_borderColorObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderColor))
                , m_borderThicknessObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderThickness))
                , m_borderRadiusObserver(selfPtr(), memFn(&ComponentBase::onUpdateBorderRadius)) {

            }

            decltype(auto) backgroundColor(Valuelike<Color> p) {
                m_backgroundColorObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderColor(Valuelike<Color> p) {
                m_borderColorObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderThickness(Valuelike<float> p) {
                m_borderThicknessObserver.assign(std::move(p));
                return self();
            }
            decltype(auto) borderRadius(Valuelike<float> p) {
                m_borderRadiusObserver.assign(std::move(p));
                return self();
            }

        private:
            Observer<Color> m_backgroundColorObserver;
            Observer<Color> m_borderColorObserver;
            Observer<float> m_borderThicknessObserver;
            Observer<float> m_borderRadiusObserver;
        
            void onUpdateBackgroundColor(const Color& c) {
                element().setBackgroundColor(c);
            }

            void onUpdateBorderColor(const Color& c) {
                element().setBorderColor(c);
            }

            void onUpdateBorderThickness(float v) {
                element().setBorderThickness(v);
            }

            void onUpdateBorderRadius(float v) {
                element().setBorderRadius(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Boxy, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::BoxElement, BaseElement>, "Base class must derive from ui::dom::BoxElement");
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

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Resizable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Resizable, ContainerComponentType, AllTags...> {
        public:
            ComponentBase()
                : m_widthObserver(selfPtr(), memFn(&ComponentBase::onUpdateWidth))
                , m_heightObserver(selfPtr(), memFn(&ComponentBase::onUpdateHeight))
                , m_minWidthObserver(selfPtr(), memFn(&ComponentBase::onUpdateMinWidth))
                , m_minHeightObserver(selfPtr(), memFn(&ComponentBase::onUpdateMinHeight))
                , m_maxWidthObserver(selfPtr(), memFn(&ComponentBase::onUpdateMaxWidth))
                , m_maxHeightObserver(selfPtr(), memFn(&ComponentBase::onUpdateMaxHeight)) {

            }

            decltype(auto) width(Valuelike<float> p) {
                m_widthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) height(Valuelike<float> p) {
                m_heightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) size(Valuelike<vec2> p) {
                m_widthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_heightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) minWidth(Valuelike<float> p) {
                m_minWidthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) minHeight(Valuelike<float> p) {
                m_minHeightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) minSize(Valuelike<vec2> p) {
                m_minWidthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_minHeightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) maxWidth(Valuelike<float> p) {
                m_maxWidthObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) maxHeight(Valuelike<float> p) {
                m_maxHeightObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) maxSize(Valuelike<vec2> p) {
                m_maxWidthObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_maxHeightObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) widthForce(Valuelike<float> p) {
                m_widthObserver.assign(std::move(p));
                m_minWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                m_maxWidthObserver.assign(m_widthObserver.getPropertyOrValue().view());
                return self();
            }

            decltype(auto) heightForce(Valuelike<float> p) {
                m_heightObserver.assign(std::move(p));
                m_minHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                m_maxHeightObserver.assign(m_heightObserver.getPropertyOrValue().view());
                return self();
            }

            decltype(auto) sizeForce(Valuelike<vec2> p) {
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
                element().setWidth(v);
            }

            void onUpdateHeight(float v) {
                element().setHeight(v);
            }

            void onUpdateMinWidth(float v) {
                element().setMinWidth(v);
            }

            void onUpdateMinHeight(float v) {
                element().setMinHeight(v);
            }

            void onUpdateMaxWidth(float v) {
                element().setMaxWidth(v);
            }

            void onUpdateMaxHeight(float v) {
                element().setMaxHeight(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Resizable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;

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

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Positionable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Positionable, ContainerComponentType, AllTags...> {
        public:

            ComponentBase()
                : m_leftObserver(selfPtr(), memFn(&ComponentBase::onUpdateLeft))
                , m_topObserver(selfPtr(), memFn(&ComponentBase::onUpdateTop)) {

            }

            decltype(auto) position(Valuelike<vec2> p) {
                m_leftObserver.assign(p.map([](vec2 v){ return v.x; }));
                m_topObserver.assign(p.map([](vec2 v){ return v.y; }));
                return self();
            }

            decltype(auto) left(Valuelike<float> p) {
                m_leftObserver.assign(std::move(p));
                return self();
            }

            decltype(auto) top(Valuelike<float> p) {
                m_topObserver.assign(std::move(p));
                return self();
            }


        private:
            Observer<float> m_leftObserver;
            Observer<float> m_topObserver;

            void onUpdateLeft(float v) {
                element().setLeft(v);
            }

            void onUpdateTop(float v) {
                element().setTop(v);
            }

            friend ElementMixin;
        };

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Positionable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;

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
        // Specializations for HitTestable

        template<>
        struct ElementBase<HitTestable> {
            using Type = dom::Element;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<HitTestable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<HitTestable, ContainerComponentType, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) hitTest(std::function<bool(vec2)> f) {
                m_hitTest = std::move(f);
                return self();
            }

        private:
            std::function<bool(vec2)> m_hitTest;

            friend ElementMixin;
        };

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<HitTestable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {
                static_assert(std::is_base_of_v<dom::Element, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            bool hit(vec2 p) const override final {
                auto& f = component().m_hitTest;
                if (f) {
                    return f(p);
                }
                return Element::hit(p);
            }
        };

        //------------------------
        // Specializations for Draggable

        template<typename DerivedAction>
        class ActionBase<Draggable, DerivedAction> {
        public:
            void startDrag() {
                auto self = static_cast<DerivedAction*>(this);
                auto e = self->element().toDraggable();
                assert(d);
                auto vd = dynamic_cast<ValueDraggable*>(d);
                assert(vd);
                vd->clearValue();
                vd->startDrag();
            }

            template<typename T>
            void startDrag(tmp::DontDeduce<T> value) {
                auto self = static_cast<DerivedAction*>(this);
                auto d = self->element().toDraggable();
                assert(d);
                auto vd = dynamic_cast<ValueDraggable*>(d);
                assert(vd);
                vd->setValue(makeUniqueAny<T>(std::move(value)));
                vd->startDrag();
            }

            void stopDrag() {
                auto self = static_cast<DerivedAction*>(this);
                auto c = self->element().toDraggable();
                assert(c);
                c->stopDrag();
            }

            // TODO: make location choosable (maybe from presets like top-left, middle, mouse position, etc)
            // Mouse position is used for now
            template<typename T>
            void drop(tmp::DontDeduce<T> value) {
                auto self = static_cast<DerivedAction*>(this);
                auto d = self->element().toDraggable();
                assert(d);
                auto vd = dynamic_cast<ValueDraggable*>(d);
                assert(vd);
                vd->setValue(makeUniqueAny<T>(std::move(value)));
                vd->drop(vd->localMousePos());
            }
        };

        // Custom dom::Element type, non-templated so that it can be identified via dynamic_cast
        // in Hoverable::onDrop (and value can be further inspected
        class ValueDraggable : public dom::Draggable {
        public:
            void setValue(UniqueAny value);

            void clearValue();

            const UniqueAny& getValue() const noexcept;

        private:
            UniqueAny m_value;
        };

        template<>
        struct ElementBase<Draggable> {
            using Type = ValueDraggable;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Draggable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Draggable, ContainerComponentType, AllTags...> {
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

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Draggable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
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
        // Specializations for Scrollable

        template<>
        struct ElementBase<Scrollable> {
            using Type = dom::Control;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Scrollable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Scrollable, ContainerComponentType, AllTags...> {
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

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Scrollable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
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
        
        template<>
        struct ElementBase<Hoverable> {
            using Type = dom::Control;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Hoverable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Hoverable, ContainerComponentType, AllTags...> {
        public:
            using Action = MixedAction<AllTags...>;
        
            decltype(auto) onMouseEnter(std::function<void()> f) {
                assert(!static_cast<bool>(m_onMouseEnter));
                m_onMouseEnter = std::move(f);
                return self();
            }

            template<typename T>
            decltype(auto) onMouseEnterWith(std::function<void(CRefOrValue<T>)> f) {
                wrapAndAdd<T, void>(std::move(f), m_onMouseEnterWithHandlers);
                return self();
            }
        
            decltype(auto) onMouseLeave(std::function<void()> f) {
                assert(!static_cast<bool>(m_onMouseLeave));
                m_onMouseLeave = std::move(f);
                return self();
            }

            template<typename T>
            decltype(auto) onMouseLeaveWith(std::function<void(CRefOrValue<T>)> f) {
                wrapAndAdd<T, void>(std::move(f), m_onMouseLeaveWithHandlers);
                return self();
            }

            template<typename T>
            decltype(auto) onDrop(std::function<bool(CRefOrValue<T>)> f) {
                wrapAndAdd<T, bool>(std::move(f), m_onDropHandlers);
                return self();
            }

        private:
            std::function<void()> m_onMouseEnter;
            std::function<void()> m_onMouseLeave;

            template<typename R>
            using TypeDispatchMap = std::map<std::type_info const*, std::function<R(const UniqueAny&)>>;

            TypeDispatchMap<void> m_onMouseEnterWithHandlers;
            TypeDispatchMap<void> m_onMouseLeaveWithHandlers;
            TypeDispatchMap<bool> m_onDropHandlers;

            template<typename T, typename R>
            void wrapAndAdd(std::function<R(CRefOrValue<T>)>&& f, TypeDispatchMap<R>& m) {
                auto ti = &typeid(T);
                assert(m.find(ti) == m.end());
                auto ff = [f = std::move(f)](const UniqueAny& ua){
                    auto p = ua.getIf<T>();
                    assert(p);
                    return f(*p);
                };
                m.emplace(ti, std::move(ff));
            }

            friend ElementMixin;
        };

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Hoverable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
            using Action = MixedAction<AllTags...>;

            ElementMixin(ComponentType& component)
                : BaseElement(component) {

                static_assert(std::is_base_of_v<dom::Control, BaseElement>, "Base class must derive from ui::dom::Control");
            }

            void onMouseEnter(dom::Draggable* d) override final {
                if (d) {
                    if (lookupAndCall<void>(d, component().m_onMouseEnterWithHandlers)) {
                        return;
                    }
                }
                auto& f = component().m_onMouseEnter;
                if (f) {
                    f();
                }
            }

            void onMouseLeave(dom::Draggable* d) override final {
                if (d) {
                    if (lookupAndCall<void>(d, component().m_onMouseLeaveWithHandlers)) {
                        return;
                    }
                }
                auto& f = component().m_onMouseLeave;
                if (f) {
                    f();
                }
            }

            bool onDrop(dom::Draggable* d) override final {
                return lookupAndCall<bool>(d, component().m_onDropHandlers);
            }

        private:
            template<typename R>
            using TypeDispatchMap = std::map<std::type_info const*, std::function<R(const UniqueAny&)>>;

            template<typename R>
            bool lookupAndCall(dom::Draggable* d, const TypeDispatchMap<R>& m) {
                assert(d);
                if (auto vd = dynamic_cast<ValueDraggable*>(d)) {
                    auto& v = vd->getValue();
                    if (!v.hasValue()) {
                        return false;
                    }
                    auto t = v.getType();
                    assert(t);
                    auto it = m.find(t);
                    if (it == m.end()) {
                        return false;
                    }
                    if constexpr (std::is_void_v<R>){
                        it->second(v);
                        return true;
                    } else {
                        return it->second(v);
                    }
                }
                return false;
            }
        };
    

        //------------------------
        // Specializations for Clickable

        template<>
        struct ElementBase<Clickable> {
            using Type = dom::Control;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Clickable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Clickable, ContainerComponentType, AllTags...> {
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

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Clickable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
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

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<KeyPressable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<KeyPressable, ContainerComponentType, AllTags...> {
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

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<KeyPressable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
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
                auto e = self->element().toControl();
                assert(c);
                c->grabFocus();
            }
        };

        template<>
        struct ElementBase<Focusable> {
            using Type = dom::Control;
        };

        template<template<typename> typename ContainerComponentType, typename... AllTags>
        class ComponentBase<Focusable, ContainerComponentType, AllTags...>
            : private ComponentBaseHelper<Focusable, ContainerComponentType, AllTags...> {
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

        template<typename BaseElement, template<typename> typename ContainerComponentType, typename... AllTags>
        class ElementMixin<Focusable, BaseElement, ContainerComponentType, AllTags...> : public BaseElement {
        public:
            using ComponentType = DerivedComponent<ContainerComponentType, AllTags...>;
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
        // Element base class from list of required bases
        template<template<typename> typename ContainerComponentType, typename... Tags>
        class CommonElementBase
            : public std::conditional_t<
                // If all bases are dom::Element
                (std::is_same_v<ElementBaseType<Tags>, dom::Element> && ...),

                // then just inherit from dom::Element
                dom::Element,

                // otherwise, remove all occurences of dom::Element, remove any duplicates,
                // and inherit from all the resulting types in parallel
                tmp::Reapply<
                    tmp::Reapply<
                        tmp::RemoveDuplicates<
                            ElementBaseType<Tags>...
                        >,
                        tmp::Curry<tmp::Filter, dom::Element>::template Result
                    >,
                    tmp::InheritParallel
                >
            > {
        public:

            CommonElementBase(DerivedComponent<ContainerComponentType, Tags...>& component)
                : m_component(component) {
                static_assert(
                    (std::is_base_of_v<dom::Element, ElementBaseType<Tags>> && ...),
                    "All element bases must derive from ui::dom::Element"
                );
            }

            DerivedComponent<ContainerComponentType, Tags...>& component() const noexcept {
                return m_component;
            }

        private:
            DerivedComponent<ContainerComponentType, Tags...>& m_component;
        };

        template<typename Tag, template<typename> typename ContainerComponentType, typename... Tags>
        struct ElementMixinPartial {
            template<typename BaseElement>
            using Result = ElementMixin<Tag, BaseElement, ContainerComponentType, Tags...>;
        };
        
        template<template<typename> typename ContainerComponentType, typename... Tags>
        using MixedElement = tmp::InheritSerial<
            CommonElementBase<ContainerComponentType, Tags...>,
            ElementMixinPartial<Tags, ContainerComponentType, Tags...>::template Result...
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
            : public SimpleComponent<MixedElement<NoContainer, Tags...>>
            , public ComponentBase<Tags, NoContainer, Tags...>... {
        public:

            // NOTE: various named-parameter-idiom methods are inherited from CRTP 'Features' base classes
            using Action = MixedAction<Tags...>;

            using ElementType = MixedElement<NoContainer, Tags...>;

            ElementType* getDerivedElement() const noexcept {
                return SimpleComponent<MixedElement<NoContainer, Tags...>>::element();
            }

        private:
            std::unique_ptr<ElementType> createElement() override final {
                static_assert(
                    std::is_constructible_v<ElementType, MixedComponent<Tags...>&>,
                    "The Element must be constructible from a reference to the mixed component type"
                );
                return std::make_unique<ElementType>(*this);
            }
        };


        template<template<typename> typename ContainerComponentType, typename DOMContainerType, typename... Tags>
        class MixedContainerElement
            : public DOMContainerType
            , public MixedElement<ContainerComponentType, Tags...> {
        public:
            using ComponentType = MixedContainerComponent<ContainerComponentType, Tags...>;
            using ElementType = MixedElement<ContainerComponentType, Tags...>;
            using ContainerComponentSpecialized = ContainerComponentType<ComponentType>;

            class MixedContainerElement(ComponentType& component)
                : ElementType(component) {
                static_assert(std::is_base_of_v<ContainerComponent, ContainerComponentSpecialized>);
                static_assert(std::is_base_of_v<dom::Container, DOMContainerType>);
                static_assert(std::is_base_of_v<dom::Element, ElementType>);
                static_assert(!std::is_base_of_v<dom::Container, ElementType>);
            }

        private:
            void render(sf::RenderWindow& rw) override final {
                ElementType::render(rw);
                DOMContainerType::render(rw);
            }
        };


        template<template<typename> typename ContainerComponentType, typename... Tags>
        class MixedContainerComponent
            : public ContainerComponentType<MixedContainerComponent<ContainerComponentType, Tags...>>
            , public ComponentBase<Tags, ContainerComponentType, Tags...>... {
        public:
            using SelfType = MixedContainerComponent<ContainerComponentType, Tags...>;
            using ContainerComponentSpecialized = ContainerComponentType<SelfType>;

            template<typename... Args>
            MixedContainerComponent(Args&&... args)
                : ContainerComponentSpecialized(std::forward<Args>(args)...) {
                static_assert(std::is_base_of_v<ContainerComponent, ContainerComponentSpecialized>);
                static_assert(std::is_base_of_v<Component, SelfType>);
            }
            using BaseDOMContainer = typename ContainerComponentType<SelfType>::Container;
            using MixedDOMContainer = MixedContainerElement<
                ContainerComponentType,
                typename ContainerComponentType<SelfType>::Container,
                Tags...
            >;

            MixedDOMContainer* getDerivedElement() noexcept {
                auto c = container();
                assert(!c || dynamic_cast<MixedDOMContainer*>(c));
                return static_cast<MixedDOMContainer*>(c);
            }

        private:
            std::unique_ptr<BaseDOMContainer> createContainer() override final {
                static_assert(std::is_constructible_v<MixedDOMContainer, SelfType&>);
                static_assert(std::is_base_of_v<BaseDOMContainer, MixedDOMContainer>);
                return std::make_unique<MixedDOMContainer>(*this);
            }
        };

    } // namespace mix

    template<typename... Tags>
    using MixedComponent = mix::MixedComponent<Tags...>;

    template<template<typename> typename ContainerComponentType, typename... Tags>
    using MixedContainerComponent = mix::MixedContainerComponent<ContainerComponentType, Tags...>;

} // namespace ofc::ui
