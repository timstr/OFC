#pragma once

#include <GUI/Component/Component.hpp>
#include <GUI/Component/List.hpp>

#include <GUI/DOM/FreeContainer.hpp>
#include <GUI/DOM/FlowContainer.hpp>
#include <GUI/DOM/ListContainer.hpp>
#include <GUI/DOM/GridContainer.hpp>

#include <GUI/Util/Direction.hpp>

namespace ui {

    class ContainerComponent : public InternalComponent {
    public:

    };



    template<typename ContainerType, typename Derived>
    class ContainerComponentTemplate : public ContainerComponent {
    public:
        ContainerComponentTemplate() : m_container(nullptr) {
            static_assert(std::is_base_of_v<dom::Container, ContainerType>, "ContainerType must derive from dom::Container");
        }

        using Container = ContainerType;

    protected:
        ContainerType* container() noexcept {
            return m_container;
        }
        const ContainerType* container() const noexcept {
            return m_container;
        }

        virtual std::unique_ptr<ContainerType> createContainer() {
            return std::make_unique<ContainerType>();
        }

        Derived& self() noexcept {
            return static_cast<Derived&>(*this);
        }

    private:
        ContainerType* m_container;

        virtual void onMountContainer(const dom::Element* beforeElement) = 0;
        virtual void onUnmountContainer() = 0;

        void onMount(const dom::Element* beforeElement) override final {
            assert(m_container == nullptr);
            auto cp = createContainer();
            assert(cp);
            m_container = cp.get();
            insertElement(std::move(cp), beforeElement);
            onMountContainer(beforeElement);
        }

        void onUnmount() override final {
            assert(m_container);
            onUnmountContainer();
            eraseElement(m_container);
            m_container = nullptr;
        }
    };



    namespace detail {
        class align_CenterVertically {};
        class align_CenterHorizontally {};
        class align_InsideLeft {};
        class align_OutsideLeft {};
        class align_InsideRight {};
        class align_OutsideRight {};
        class align_InsideTop {};
        class align_OutsideTop {};
        class align_InsideBottom {};
        class align_OutsideBottom {};
    } // namespace detail



    template<typename Derived>
    class FreeContainerBase : public ContainerComponentTemplate<dom::FreeContainer, Derived> {
    public:
        Derived& containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return static_cast<Derived&>(*this);
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Derived& containing(ComponentTypes&&... components) {
            m_childComponent = List(std::forward<ComponentTypes>(components)...);
            return static_cast<Derived&>(*this);
        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            using namespace detail;
            using Style = dom::FreeContainer::PositionStyle;
            auto c = container();
            assert(c);
            // TODO: insert element before `beforeElement`
            auto sx = Style::None;
            auto sy = Style::None;

            if (scope.has<align_InsideLeft>()) {
                sx = Style::InsideLeft;
            } else if (scope.has<align_OutsideLeft>()) {
                sx = Style::OutsideLeft;
            } else if (scope.has<align_CenterHorizontally>()){
                sx = Style::Center;
            } else if (scope.has<align_InsideRight>()) {
                sx = Style::InsideRight;
            } else if (scope.has<align_OutsideRight>()) {
                sx = Style::OutsideRight;
            }

            if (scope.has<align_InsideTop>()) {
                sy = Style::InsideTop;
            } else if (scope.has<align_OutsideTop>()) {
                sy = Style::OutsideTop;
            } else if (scope.has<align_CenterVertically>()) {
                sy = Style::Center;
            } else if (scope.has<align_InsideBottom>()) {
                sy = Style::InsideBottom;
            } else if (scope.has<align_OutsideBottom>()) {
                sy = Style::OutsideBottom;
            }

            c->adopt(sx, sy, std::move(element));
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };



    namespace detail {
        class flowStyle_Inline {};
        class flowStyle_Block {};
        class flowStyle_FloatLeft {};
        class flowStyle_FloatRight {};
        class flowStyle_Free {};
    } // namespace



    // TODO: add text flow direction
    // TODO: make text flow across lines (by breaking up individual text components into one text component per word)
    // TODO: add newline and indentation

    template<typename Derived>
    class FlowContainerBase : public ContainerComponentTemplate<dom::FlowContainer, Derived> {
    public:
        Derived& containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return static_cast<Derived&>(*this);
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Derived& containing(ComponentTypes&&... components) {
            m_childComponent = List(std::forward<ComponentTypes>(components)...);
            return static_cast<Derived&>(*this);
        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            using namespace detail;
            auto c = container();
            assert(c);
            auto style = dom::LayoutStyle::Inline;
            if (scope.has<flowStyle_Block>()) {
                style= dom::LayoutStyle::Block;
            } else if (scope.has<flowStyle_FloatLeft>()) {
                style= dom::LayoutStyle::FloatLeft;
            } else if (scope.has<flowStyle_FloatRight>()) {
                style= dom::LayoutStyle::FloatRight;
            } else if (scope.has<flowStyle_Free>()) {
                style= dom::LayoutStyle::Free;
            }
            c->adopt(std::move(element), style, scope.beforeElement());
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };



    template<typename Derived>
    class VerticalListBase : public ContainerComponentTemplate<dom::VerticalList, Derived> {
    public:
        VerticalListBase(VerticalDirection direction = TopToBottom)
            : m_direction(direction) {

        }

        Derived& containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return static_cast<Derived&>(*this);
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Derived& containing(ComponentTypes&&... components) {
            m_childComponent = List(std::forward<ComponentTypes>(components)...);
            return static_cast<Derived&>(*this);
        }

    private:
        const VerticalDirection m_direction;
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            auto c = container();
            assert(c);
            auto b = scope.beforeElement();
            if (b && b->getParentContainer() != c) {
                b = nullptr;
            }
            if (m_direction == TopToBottom){
                c->insertBefore(b, std::move(element));
            } else {
                c->insertAfter(b, std::move(element));
            }
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };



    template<typename Derived>
    class HorizontalListBase : public ContainerComponentTemplate<dom::HorizontalList, Derived> {
    public:
        HorizontalListBase(HorizontalDirection direction = LeftToRight)
            : m_direction(direction) {

        }

        Derived& containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return static_cast<Derived&>(*this);
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Derived& containing(ComponentTypes&&... components) {
            m_childComponent = List(std::forward<ComponentTypes>(components)...);
            return static_cast<Derived&>(*this);
        }

    private:
        const HorizontalDirection m_direction;
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            auto c = container();
            assert(c);
            auto b = scope.beforeElement();
            if (b && b->getParentContainer() != c) {
                b = nullptr;
            }
            if (m_direction == LeftToRight){
                c->insertBefore(b, std::move(element));
            } else {
                c->insertAfter(b, std::move(element));
            }
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            c->release(whichElement);
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return {m_childComponent.get()};
        }
    };



    template<typename Derived>
    class WrapGridBase : public ContainerComponentTemplate<dom::GridContainer, Derived> {
    public:

        WrapGridBase(VerticalDirection vdir, std::size_t numColumns, HorizontalDirection hdir = RightToLeft)
            : m_rowMajor(false)
            , m_verticalDirection(vdir)
            , m_horizontalDirection(hdir)
            , m_size(numColumns) {

        }

        WrapGridBase(HorizontalDirection hdir, std::size_t numRows, VerticalDirection vdir = TopToBottom)
            : m_rowMajor(true)
            , m_verticalDirection(vdir)
            , m_horizontalDirection(hdir)
            , m_size(numRows) {

        }

        decltype(auto) containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return self();
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        decltype(auto) containing(ComponentTypes&&... c) {
            m_childComponent = List(std::forward<ComponentTypes>(c)...);
            return self();
        }

    private:
        AnyComponent m_childComponent;
        const bool m_rowMajor;
        const VerticalDirection m_verticalDirection;
        const HorizontalDirection m_horizontalDirection;
        const std::size_t m_size;
        std::vector<dom::Element*> m_mountedElements;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void refreshContents() {
            auto c = container();
            assert(c);

            // Empty the grid and take ownership of all children
            auto elements = std::vector<std::unique_ptr<dom::Element>>{};
            for (const auto& e : m_mountedElements) {
                assert(e);
                // assert(e->getParentContainer() == c);
                elements.push_back(e->orphan());
            }
            assert(std::as_const(*c).children().size() == 0);

            // Compute how many rows/columns are needed
            const auto numElements = m_mountedElements.size();
            assert(elements.size() == numElements);
            const auto numLines = (numElements % m_size == 0) ?
                numElements / m_size :
                (numElements / m_size) + 1;

            c->setColumns(std::max(std::size_t{1}, m_rowMajor ? m_size : numLines));
            c->setRows(std::max(std::size_t{1}, m_rowMajor ? numLines : m_size));

            // Reinsert all elements where they now belong
            for (std::size_t idx = 0; idx < numElements; ++idx) {
                const auto minorLine = idx / m_size;
                const auto majorLine = idx % m_size;
                const auto x = m_rowMajor ?
                    (m_horizontalDirection == LeftToRight ? majorLine : m_size - 1 - majorLine) :
                    (m_horizontalDirection == LeftToRight ? minorLine : numLines - 1 - minorLine);
                const auto y = m_rowMajor ?
                    (m_verticalDirection == TopToBottom ? minorLine : numLines - 1 - minorLine) :
                    (m_verticalDirection == TopToBottom ? majorLine : m_size - 1 - majorLine);
                assert(c->getCell(x, y) == nullptr);
                c->putCell(x, y, std::move(elements[idx]));
            }
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            assert(element);
            auto it = scope.beforeElement() ?
                find(begin(m_mountedElements), end(m_mountedElements), scope.beforeElement()) :
                end(m_mountedElements);
            m_mountedElements.insert(it, element.get());
            auto c = container();
            assert(c);
            // NOTE: element is inserted into dummy grid position to make
            // logic of refreshContents simpler
            c->appendColumn();
            c->putCell(c->columns() - 1, 0, std::move(element));
            refreshContents();
        }

        void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
            auto c = container();
            assert(c);
            assert(count(begin(m_mountedElements), end(m_mountedElements), whichElement) == 1);
            auto it = find(begin(m_mountedElements), end(m_mountedElements), whichElement);
            assert(it != end(m_mountedElements));
            m_mountedElements.erase(it);
            assert(whichElement->getParentContainer() == c);
            whichElement->orphan();
            refreshContents();
        }

        std::vector<const Component*> getChildren() const noexcept override final {
            return { m_childComponent.get() };
        }
    };

    

    class Row;
    class Column;

    namespace detail {

        template<typename Derived, typename SliceType>
        class SliceGridBase : public ContainerComponentTemplate<dom::GridContainer, Derived> {
        public:

            decltype(auto) containing(AnyComponent c) {
                m_childComponent = std::move(c);
                return self();
            }

            template<
                typename... ComponentTypes,
                std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
            >
            decltype(auto) containing(ComponentTypes&&... c) {
                m_childComponent = List(std::forward<ComponentTypes>(c)...);
                return self();
            }

        protected:
            
            SliceGridBase(bool outerDirection, bool innerDirection)
                : m_outerDirection(outerDirection)
                , m_innerDirection(innerDirection) {
                static_assert(std::is_same_v<SliceType, Row> || std::is_same_v<SliceType, Column>);
            }

        private:
            AnyComponent m_childComponent;
            const bool m_outerDirection;
            const bool m_innerDirection;
            std::vector<std::pair<const Component*, std::vector<dom::Element*>>> m_slices;
        
            void onMountContainer(const dom::Element* beforeElement) override final {
                m_childComponent.tryMount(this, beforeElement);
            }

            void onUnmountContainer() override final {
                m_childComponent.tryUnmount();
            }

            void refreshContents() {
                const auto grid = container();

                // Clear the grid and take ownership of all elements
                std::vector<std::pair<const Component*, std::vector<std::unique_ptr<dom::Element>>>> sliceElements;

                for (std::size_t i = 0, iEnd = m_slices.size(); i != iEnd; ++i){
                    const auto& [slice, elems] = m_slices[m_outerDirection ? i : iEnd - 1 - i];
                    sliceElements.emplace_back(slice, std::vector<std::unique_ptr<dom::Element>>{});
                    for (const auto& ePtr : elems) {
                        sliceElements.back().second.push_back(ePtr->orphan());
                    }
                }
                assert(std::as_const(*grid).children().size() == 0);

                // Find the number of rows and columns
                const auto n = sliceElements.size();
                const auto largestSlice = max_element(
                    begin(sliceElements),
                    end(sliceElements),
                    [](const auto& slice1, const auto& slice2) {
                        return slice1.second.size() < slice2.second.size();
                    }
                );
                const auto m = largestSlice == end(sliceElements) ? std::size_t{0} : largestSlice->second.size();

                // Resize the grid
                if constexpr (std::is_same_v<SliceType, Column>){
                    grid->setRows(m);
                    grid->setColumns(n);
                } else {
                    grid->setRows(n);
                    grid->setColumns(m);
                }

                // Re-insert all elements where they belong
                for (std::size_t i = 0; i != n; ++i) {
                    auto& elems = sliceElements[i].second;
                    for (std::size_t j = 0, jEnd = elems.size(); j != jEnd; ++j) {
                        const auto jj = m_innerDirection ? j : m - 1 - j;

                        if constexpr (std::is_same_v<SliceType, Column>){
                            grid->putCell(i, jj, std::move(elems[j]));
                        } else {
                            grid->putCell(jj, i, std::move(elems[j]));
                        }
                    }
                }
            }

            void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
                assert(element);

                const auto s = [&]() -> const Component* {
                    if constexpr (std::is_same_v<SliceType, Column>) {
                        const auto tag = scope.get<detail::grid_column>();
                        if (!tag) {
                            throw std::runtime_error("An component was added to a ColumnGrid that was not inside a Column");
                        }
                        return tag->column;;
                    } else {
                        const auto tag = scope.get<detail::grid_row>();
                        if (!tag) {
                            throw std::runtime_error("An component was added to a RowGrid that was not inside a Row");
                        }
                        return tag->row;;
                    }
                }();
                

                // Find the column, or create a new one if it's not found
                auto it = begin(m_slices);
                while (true) {
                    if (it == end(m_slices)) {
                        // The column's position wasn't found: insert it at the end
                        m_slices.push_back({s, {}});
                        it = --end(m_slices);
                        break;
                    }
                    if (s == it->first) {
                        // The exact column is found: use it and carry on
                        break;
                    }
                    if (s->appearsBefore(it->first)) {
                        // The place where the column should be is found: insert a new column
                        it = m_slices.insert(it, {s, {}});
                        break;
                    }
                    ++it;
                }
                assert(it != end(m_slices));

                auto& [theSlice, theElements] = *it;

                // Find the new element's position in the column and insert it there
                auto eit = find_if(
                    begin(theElements),
                    end(theElements),
                    [&](const auto& e) {
                        return e == scope.beforeElement();
                    }
                );

                theElements.insert(eit, element.get());

                // insert the element at a dummy position to keep the refresh logic simple
                const auto grid = container();
                grid->appendColumn();
                grid->putCell(grid->columns() - 1, 0, std::move(element));

                refreshContents();
            }

            void onRemoveChildElement(dom::Element* whichElement, const Component* /* whichDescendent */) override final {
                assert(whichElement);
            
                for (auto it = begin(m_slices), itEnd = end(m_slices); it != itEnd; ++it){
                    auto& [theSlice, theElements] = *it;
            
                    auto eit = find(
                        begin(theElements),
                        end(theElements),
                        whichElement
                    );
                    if (eit == end(theElements)) {
                        continue;
                    }

                    theElements.erase(eit);

                    if (theElements.size() == 0) {
                        m_slices.erase(it);
                    }

                    whichElement->orphan();

                    refreshContents();

                    return;
                }
                assert(false);
            }

            std::vector<const Component*> getChildren() const noexcept override final {
                return { m_childComponent.get() };
            }
        };

    } // namespace detail
    

    template<typename Derived>
    class ColumnGridBase : public detail::SliceGridBase<Derived, Column> {
    public:
        ColumnGridBase(HorizontalDirection outerDirection, VerticalDirection innerDirection)
            : SliceGridBase(outerDirection == LeftToRight, innerDirection == TopToBottom) {

        }
    };


    template<typename Derived>
    class RowGridBase : public detail::SliceGridBase<Derived, Row> {
    public:
        RowGridBase(VerticalDirection outerDirection, HorizontalDirection innerDirection)
            : SliceGridBase(outerDirection == TopToBottom, innerDirection == LeftToRight) {

        }
    };

    //------------------------------------------

    template<typename T>
    class ScopeTagComponent : public SimpleForwardingComponent {
    public:
        using SimpleForwardingComponent::SimpleForwardingComponent;

    private:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final {
            auto s = scope;
            s.add<T>();
            SimpleForwardingComponent::onInsertChildElement(std::move(element), s);
        }
    };

    //------------------------------------------

    class CenterVertically : public ScopeTagComponent<detail::align_CenterVertically> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class CenterHorizontally : public ScopeTagComponent<detail::align_CenterHorizontally> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class Center : public SimpleForwardingComponent {
    public:
        using SimpleForwardingComponent::SimpleForwardingComponent;

    private:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final;
    };

    class AlignInsideLeft : public ScopeTagComponent<detail::align_InsideLeft> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignOutsideLeft : public ScopeTagComponent<detail::align_OutsideLeft> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignInsideRight : public ScopeTagComponent<detail::align_InsideRight> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignOutsideRight : public ScopeTagComponent<detail::align_OutsideRight> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignInsideTop : public ScopeTagComponent<detail::align_InsideTop> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignOutsideTop : public ScopeTagComponent<detail::align_OutsideTop> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignInsideBottom : public ScopeTagComponent<detail::align_InsideBottom> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class AlignOutsideBottom : public ScopeTagComponent<detail::align_OutsideBottom> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    //--------------------------------


    class FlowInline : public ScopeTagComponent<detail::flowStyle_Inline> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class FlowBlock : public ScopeTagComponent<detail::flowStyle_Block> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class FlowFloatLeft : public ScopeTagComponent<detail::flowStyle_FloatLeft> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class FlowFloatRight : public ScopeTagComponent<detail::flowStyle_FloatRight> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    class FlowFree : public ScopeTagComponent<detail::flowStyle_Free> {
    public:
        using ScopeTagComponent::ScopeTagComponent;
    };

    namespace detail {
        class grid_row {
        public:
            grid_row(const Row* theRow) noexcept;
            const Row* const row;
        };

        class grid_column {
        public:
            grid_column(const Column* theColumn) noexcept;
            const Column* const column;
        };
    }

    // TODO: "weight" for row and column (see dom::GridContainer)

    class Row : public SimpleForwardingComponent {
    public:
        Row(AnyComponent);

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Row(ComponentTypes&&... components)
            : SimpleForwardingComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final;
    };

    class Column : public SimpleForwardingComponent {
    public:
        Column(AnyComponent);

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        Column(ComponentTypes&&... components)
            : SimpleForwardingComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Scope& scope) override final;
    };

    //--------------------------------

    class FreeContainer : public FreeContainerBase<FreeContainer> {
    public:
        using FreeContainerBase::FreeContainerBase;
    };

    class FlowContainer : public FlowContainerBase<FlowContainer> {
    public:
        using FlowContainerBase::FlowContainerBase;
    };

    class VerticalList : public VerticalListBase<VerticalList> {
    public:
        using VerticalListBase::VerticalListBase;
    };

    class HorizontalList : public HorizontalListBase<HorizontalList> {
    public:
        using HorizontalListBase::HorizontalListBase;
    };

    class WrapGrid : public WrapGridBase<WrapGrid> {
    public:
        using WrapGridBase::WrapGridBase;
    };
    
    class ColumnGrid : public ColumnGridBase<ColumnGrid> {
    public:
        using ColumnGridBase::ColumnGridBase;
    };

    class RowGrid : public RowGridBase<ColumnGrid> {
    public:
        using RowGridBase::RowGridBase;
    };

} // namespace ui
