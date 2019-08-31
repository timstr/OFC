#pragma once

#include <GUI/Container.hpp>

namespace ui {

    class GridContainer : public Container {
    public:
        GridContainer();
        GridContainer(size_t rows, size_t columns);

        void setRows(size_t);
        void setColumns(size_t);
        void setDimensions(size_t rows, size_t columns);
        void addRow(float size = 1.0f);
        void addColumn(float size = 1.0f);

        size_t rows() const;
        size_t columns() const;

        void setRowHeight(size_t row, float height);
        void setColumnWidth(size_t column, float width);
        float rowHeight(size_t row) const;
        float columnHeight(size_t column) const;

        template<typename T, typename... Args>
        T& putCell(size_t row, size_t column, Args&&... args);

        void putCell(size_t row, size_t column, std::unique_ptr<Element>);
        void clearCell(size_t row, size_t column);

        Element* getCell(size_t row, size_t column);
        const Element* getCell(size_t row, size_t column) const;

        void updateContents() override;

    private:
        size_t m_rows;
        size_t m_cols;
        std::vector<std::vector<Element*>> m_cells;
        std::vector<float> m_widths;
        std::vector<float> m_heights;
    };

    // Template definitions

    template<typename T, typename... Args>
    T& GridContainer::putCell(size_t row, size_t column, Args&&... args){
        auto uptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ret = *uptr;
        putCell(row, column, std::move(uptr));
        return ret;
    }

} // namespace ui