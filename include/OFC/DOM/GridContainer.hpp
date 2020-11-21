#pragma once

#include <OFC/DOM/Container.hpp>

namespace ofc::ui::dom {
    
    class GridContainer : public Container {
    public:
        GridContainer();
        GridContainer(size_t xDim, size_t yDim);

        void setRows(size_t);
        void setColumns(size_t);
        void setDimensions(size_t xDim, size_t yDim);
        void appendRow(float weight = 1.0f);
        void appendColumn(float weight = 1.0f);
        void insertRow(size_t y, float weight = 1.0f);
        void insertColumn(size_t x, float weight = 1.0f);

        size_t rows() const;
        size_t columns() const;

        void setRowWeight(size_t y, float weight);
        void setColumnWeight(size_t x, float weight);
        float rowWeight(size_t y) const;
        float columnWeight(size_t x) const;

        void putCell(size_t x, size_t y, std::unique_ptr<Element>);
        void clearCell(size_t x, size_t y);

        Element* getCell(size_t x, size_t y);
        const Element* getCell(size_t x, size_t y) const;

    private:
        size_t m_rows;
        size_t m_cols;

        struct CellData {
            Element* child;
        };

        std::vector<std::vector<CellData>> m_cells;
        std::vector<float> m_widths;
        std::vector<float> m_heights;

        vec2 update() override;

        void onRemoveChild(const Element*) override;
    };

} // namespace ofc::ui::dom
