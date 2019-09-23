#include <GUI/GridContainer.hpp>

#include <numeric>
#include <cassert>

namespace ui {

    GridContainer::GridContainer() : 
        m_rows(1),
        m_cols(1){
        
        m_cells.push_back({});
        m_cells.front().push_back({nullptr});

        m_widths.push_back(1.0f);
        m_heights.push_back(1.0f);
    }

    GridContainer::GridContainer(size_t rows, size_t columns) :
        m_rows(rows),
        m_cols(columns) {

        assert(rows > 0 && columns > 0);
		
        m_cells.resize(rows);
        for (auto& row : m_cells){
            row.resize(columns);
            std::fill(row.begin(), row.end(), CellData{});
        }

        m_widths.resize(columns);
        std::fill(m_widths.begin(), m_widths.end(), 1.0f);
        m_heights.resize(rows);
        std::fill(m_heights.begin(), m_heights.end(), 1.0f);
    }

    void GridContainer::setRows(size_t r){
		while (m_rows > r){
            m_cells.erase(--m_cells.end());
            m_heights.erase(--m_heights.end());
            --m_rows;
        }
        while (m_rows < r){
            m_cells.push_back({});
            m_cells.back().resize(m_cols);
            std::fill(m_cells.back().begin(), m_cells.back().end(), CellData{});
            m_heights.push_back(1.0f);
            ++m_rows;
        }
    }

    void GridContainer::setColumns(size_t c){
		while (m_cols > c){
            for (auto& row : m_cells){
                row.erase(--row.end());
            }
            m_widths.erase(--m_widths.end());
            --m_cols;
        }
        while (m_cols < c){
            for (auto& row : m_cells){
                row.push_back({});
            }
            m_widths.push_back(1.0f);
            ++m_cols;
        }
    }

    void GridContainer::setDimensions(size_t rows, size_t columns){
        assert(rows > 0 && columns > 0);
        setColumns(columns);
		setRows(rows);
    }

    void GridContainer::addRow(float size){
		setRows(m_rows + 1);
    }

    void GridContainer::addColumn(float size){
		setColumns(m_cols + 1);
    }

    size_t GridContainer::rows() const {
		return m_rows;
    }

    size_t GridContainer::columns() const {
		return m_cols;
    }

    void GridContainer::setRowHeight(size_t row, float height){
		assert(row >= 0 && row < m_rows);
        assert(height > 0.0f);
        m_heights[row] = height;
    }

    void GridContainer::setColumnWidth(size_t column, float width){
		assert(column >= 0 && column < m_cols);
        assert(width > 0.0f);
        m_widths[column] = width;
    }

    float GridContainer::rowHeight(size_t row) const {
		assert(row >= 0 && row < m_rows);
        return m_widths[row];
    }

    float GridContainer::columnHeight(size_t column) const {
		assert(column >= 0 && column < m_cols);
        return m_heights[column];
    }

    void GridContainer::putCell(size_t row, size_t column, std::unique_ptr<Element> e){
		assert(row >= 0 && row < m_rows);
		assert(column >= 0 && column < m_cols);
        auto prev = m_cells[row][column].child;
        if (prev){
            release(prev);
        }
        auto eptr = e.get();
        adopt(std::move(e));
        m_cells[row][column].child = eptr;
    }

    void GridContainer::clearCell(size_t row, size_t column){
        putCell(row, column, nullptr);
    }

    Element* GridContainer::getCell(size_t row, size_t column){
		return const_cast<Element*>(const_cast<const GridContainer*>(this)->getCell(row, column));
    }

    const Element* GridContainer::getCell(size_t row, size_t column) const {
		assert(row >= 0 && row < m_rows);
		assert(column >= 0 && column < m_cols);
        return m_cells[row][column].child;
    }

    void GridContainer::setHorizontalFill(size_t row, size_t column, bool enabled){
        assert(row < m_rows);
        assert(column < m_cols);
        m_cells[row][column].fillX = enabled;
        requireUpdate();
    }

    void GridContainer::setVerticalFill(size_t row, size_t column, bool enabled){
        assert(row < m_rows);
        assert(column < m_cols);
        m_cells[row][column].fillY = enabled;
        requireUpdate();
    }

    vec2 GridContainer::update(){
		std::vector<float> row_pos;
		std::vector<float> col_pos;
        row_pos.resize(m_rows + 1);
        col_pos.resize(m_cols + 1);

        const auto width_sum = std::accumulate(m_widths.begin(), m_widths.end(), 0.0f);
        const auto height_sum = std::accumulate(m_heights.begin(), m_heights.end(), 0.0f);

        {
            float row_acc = 0.0f;
            for (size_t i = 0; i < m_rows; ++i){
                const float h = height() * m_heights[i] / height_sum;
                row_pos[i] = row_acc;
                row_acc += h;
            }
            row_pos.back() = height();
        }
        {
            float col_acc = 0.0f;
            for (size_t i = 0; i < m_cols; ++i){
                const float w = width() * m_widths[i] / width_sum;
                col_pos[i] = col_acc;
                col_acc += w;
            }
            col_pos.back() = width();
        }
        
        for (size_t i = 0; i < m_rows; ++i){
            for (size_t j = 0; j < m_cols; ++j){
                if (auto c = m_cells[i][j]; c.child){
                    auto avail = c.child->size();
                    c.child->setPos({col_pos[j], row_pos[i]});
                    if (c.fillX){
                        avail.x = col_pos[j + 1] - col_pos[j];
                    }
                    if (c.fillY){
                        avail.y = row_pos[i + 1] - row_pos[i];
                    }
                    setAvailableSize(c.child, avail);
                    // TODO: query child's size now to see if it exceeds the bounds
                    // of its cell

                    //e->update({col_pos[j + 1] - col_pos[j], row_pos[i + 1] - row_pos[i]});
                }
            }
        }

        // TODO: allow size to be exceeded
        return size();
    }

} // namespace ui
