#pragma once

#include <GUI/Component/Component.hpp>
#include <GUI/Component/List.hpp>

#include <GUI/DOM/FreeContainer.hpp>
#include <GUI/DOM/FlowContainer.hpp>
#include <GUI/DOM/VerticalList.hpp>
#include <GUI/DOM/HorizontalList.hpp>
#include <GUI/DOM/GridContainer.hpp>

namespace ui {

    // TODO: get rid of the following template
    // TODO: add ContainerComponent for interface reasons
    // TODO: in Window, just accept something deriving from ContainerComponent

    class ContainerComponent : public InternalComponent {
    public:

    };


    template<typename ContainerType>
    class ContainerComponentTemplate : public ContainerComponent {
    public:
        ContainerComponentTemplate() : m_container(nullptr) {
            static_assert(std::is_base_of_v<dom::Container, ContainerType>, "ContainerType must derive from dom::Container");
        }

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



    class FreeContainer : public ContainerComponentTemplate<dom::FreeContainer> {
    public:
        FreeContainer(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

        template<typename... ComponentTypes>
        FreeContainer(ComponentTypes&&... components)
            : m_childComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            // TODO: insert element before `beforeElement`
            c->adopt(std::move(element));
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



    class FlowContainer : public ContainerComponentTemplate<dom::FlowContainer> {
    public:
        FlowContainer(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

        template<typename... ComponentTypes>
        FlowContainer(ComponentTypes&&... components)
            : m_childComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        // TODO: add some clean way to specify flow style
        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            c->adopt(std::move(element), dom::LayoutStyle::Inline, beforeElement);
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



    class VerticalList : public ContainerComponentTemplate<dom::VerticalList> {
    public:
        // TODO: allow any number of arguments
        VerticalList(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

        template<typename... ComponentTypes>
        VerticalList(ComponentTypes&&... components)
            : m_childComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            if (beforeElement && beforeElement->getParentContainer() != c) {
                beforeElement = nullptr;
            }
            c->insertBefore(beforeElement, std::move(element));
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



    class HorizontalList : public ContainerComponentTemplate<dom::HorizontalList> {
    public:
        HorizontalList(AnyComponent c)
            : m_childComponent(std::move(c)) {

        }

        template<typename... ComponentTypes>
        HorizontalList(ComponentTypes&&... components)
            : m_childComponent(List(std::forward<ComponentTypes>(components)...)) {

        }

    private:
        AnyComponent m_childComponent;

        void onMountContainer(const dom::Element* beforeElement) override final {
            m_childComponent.tryMount(this, beforeElement);
        }

        void onUnmountContainer() override final {
            m_childComponent.tryUnmount();
        }

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            auto c = container();
            assert(c);
            c->insertBefore(beforeElement, std::move(element));
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

    // TODO: specialize other containers

    // TODO: grid container will need special interface
    // Some desirable options:
    // Grid with fixed number of columns that fills horizontally, then vertically
    // Example:
    //    RowMajorWrappingGrid<3>("1", "2", "3", "4", "5", "6", "7", "8", "9")
    // Output:
    //    1 2 3
    //    4 5 6
    //    7 8 9
    // Grid with fixed number of rows that fills vertically, then horizontally
    // Example:
    //    ColumnMajorWrappingGrid<3>("1", "2", "3", "4", "5", "6", "7", "8", "9")
    // Output:
    //    1 4 7
    //    2 5 8
    //    3 6 9
    // Row-oriented grid with distinct rows
    // Example:
    //     RowMajorGrid(
    //         Row("1", "2,", "3"),
    //         Row("4", "5,", "6"),
    //         Row("7", "8,", "9")
    //     )
    // Output:
    //    1 2 3
    //    4 5 6
    //    7 8 9
    // Column-oriented grid with distinct columns
    // Example:
    //     ColumnMajorGrid(
    //         Column("1", "2,", "3"),
    //         Column("4", "5,", "6"),
    //         Column("7", "8,", "9")
    //     )
    // Output:
    //    1 4 7 
    //    2 5 8
    //    3 6 9
    // Generalizing on Column and Row (to work with internal components like ForEach and If)
    // will be a bit tricky
    // When a Row/Column is mounted:
    // - walk up the component hierarchy and find the nearest RowMajorGrid/ColumnMajorGrid
    //   (error in case of a mismatch)
    // - register the component with the grid component which will assign it its own
    //   row/column index and maintain that index if other components mount or unmount
    // - when that row/column inserts a dom element, the grid component will process
    //   the insertion and place the element into the correct place in the grid element

    enum HorizontalDirection : std::uint8_t {
        RightToLeft,
        LeftToRight
    };

    enum VerticalDirection : std::uint8_t {
        TopToBottom,
        BottomToTop
    };

    class WrapGrid : public ContainerComponentTemplate<dom::GridContainer> {
    public:

        WrapGrid(VerticalDirection vdir, std::size_t numColumns, HorizontalDirection hdir = RightToLeft)
            : m_rowMajor(false)
            , m_verticalDirection(vdir)
            , m_horizontalDirection(hdir)
            , m_size(numColumns) {

        }

        WrapGrid(HorizontalDirection hdir, std::size_t numRows, VerticalDirection vdir = TopToBottom)
            : m_rowMajor(true)
            , m_verticalDirection(vdir)
            , m_horizontalDirection(hdir)
            , m_size(numRows) {

        }

        WrapGrid& Containing(AnyComponent c) {
            m_childComponent = std::move(c);
            return *this;
        }

        template<
            typename... ComponentTypes,
            std::enable_if_t<(sizeof...(ComponentTypes) > 1)>* = nullptr
        >
        WrapGrid& Containing(ComponentTypes&&... c) {
            m_childComponent = List(std::forward<ComponentTypes>(c)...);
            return *this;
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

        void onInsertChildElement(std::unique_ptr<dom::Element> element, const Component* /* whichDescendent */, const dom::Element* beforeElement) override final {
            assert(element);
            auto it = beforeElement ?
                find(begin(m_mountedElements), end(m_mountedElements), beforeElement) :
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

    // TODO
    class ColumnGrid : public ContainerComponentTemplate<dom::GridContainer> {
    public:
        ColumnGrid();

        class Column : public InternalComponent {
        public:
            Column(AnyComponent);

        private:
        };
    };

} // namespace ui
