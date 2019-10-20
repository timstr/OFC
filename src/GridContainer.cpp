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

    GridContainer::GridContainer(size_t columns, size_t rows) :
        m_rows(rows),
        m_cols(columns) {

        //assert(rows > 0 && columns > 0);
		
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
            for (const auto& c : m_cells.back()){
                if (c.child){
                    release(c.child);
                }
            }
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
                if (auto child = row.back().child){
                    release(child);
                }
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

    void GridContainer::setDimensions(size_t columns, size_t rows){
        assert(rows > 0 && columns > 0);
        setColumns(columns);
		setRows(rows);
    }

    void GridContainer::appendRow(float weight){
		setRows(m_rows + 1);
        setRowWeight(m_rows - 1, weight);
    }

    void GridContainer::appendColumn(float weight){
		setColumns(m_cols + 1);
		setColumnWeight(m_cols - 1, weight);
    }

    size_t GridContainer::rows() const {
		return m_rows;
    }

    size_t GridContainer::columns() const {
		return m_cols;
    }

    void GridContainer::setRowWeight(size_t row, float height){
		assert(row >= 0 && row < m_rows);
        m_heights[row] = height;
    }

    void GridContainer::setColumnWeight(size_t column, float width){
		assert(column >= 0 && column < m_cols);
        m_widths[column] = width;
    }

    float GridContainer::rowWeight(size_t row) const {
		assert(row >= 0 && row < m_rows);
        return m_widths[row];
    }

    float GridContainer::columnWeight(size_t column) const {
		assert(column >= 0 && column < m_cols);
        return m_heights[column];
    }

    void GridContainer::putCell(size_t column, size_t row, std::unique_ptr<Element> e){
		assert(row >= 0 && row < m_rows);
		assert(column >= 0 && column < m_cols);
        auto prev = m_cells[row][column].child;
        if (prev){
            release(prev);
        }
        assert(!getCell(row, column));
        auto eptr = e.get();
        adopt(std::move(e));
        m_cells[row][column].child = eptr;
    }

    void GridContainer::clearCell(size_t column, size_t row){
        putCell(row, column, nullptr);
    }

    Element* GridContainer::getCell(size_t column, size_t row){
		return const_cast<Element*>(const_cast<const GridContainer*>(this)->getCell(row, column));
    }

    const Element* GridContainer::getCell(size_t column, size_t row) const {
		assert(row >= 0 && row < m_rows);
		assert(column >= 0 && column < m_cols);
        return m_cells[row][column].child;
    }

    vec2 GridContainer::update(){
        if (m_rows == 0 || m_cols == 0){
            assert(children().size() == 0);
            return {0.0f, 0.0f};
        }

        const auto placeCells = [&](const std::vector<float>& widths, const std::vector<float>& heights){
            auto rowPositions = std::vector<float>(m_rows + 1, 0.0f);
            auto colPositions = std::vector<float>(m_cols + 1, 0.0f);

            auto minWidths = std::vector<float>(m_cols, 0.0f);
            auto minHeights = std::vector<float>(m_rows, 0.0f);
        
            const auto widthSum = std::accumulate(widths.begin(), widths.end(), 0.0f);
            const auto heightSum = std::accumulate(heights.begin(), heights.end(), 0.0f);

            {
                float rowAcc = 0.0f;
                for (size_t i = 0; i < m_rows; ++i){
                    const float h = height() * heights[i] / heightSum;
                    rowPositions[i] = rowAcc;
                    rowAcc += h;
                }
                rowPositions.back() = rowAcc;
            }
            {
                float colAcc = 0.0f;
                for (size_t i = 0; i < m_cols; ++i){
                    const float w = width() * widths[i] / widthSum;
                    colPositions[i] = colAcc;
                    colAcc += w;
                }
                colPositions.back() = colAcc;
            }
            
            for (size_t i = 0; i < m_rows; ++i){
                for (size_t j = 0; j < m_cols; ++j){
                    if (auto c = m_cells[i][j]; c.child){
                        c.child->setPos({colPositions[j], rowPositions[i]});
                        setAvailableSize(c.child, {
                            colPositions[j + 1] - colPositions[j],
                            rowPositions[i + 1] - rowPositions[i]
                        });

                        const auto required = getRequiredSize(c.child);

                        minWidths[j] = std::max(minWidths[j], required.x);
                        minHeights[i] = std::max(minHeights[i], required.y);
                    }
                }
            }

            return std::make_pair(minWidths, minHeights);
        };

        const auto collapseAndDistribute = [](const std::vector<float>& minimumSizes, float availSize, const std::vector<float>& weights){
            // Mark all slots active
            // set remaining space to be availSize
            // assign weighted remaining space to all active slots
            // Loop:
            // - find an active slot whose assigned space is too small
            // - assign it the space it needs and mark it inactive
            // - subtract its minimum space from the remaining space
            // - reassign weighted remaining space to all active slots

            assert(minimumSizes.size() == weights.size());
            assert(minimumSizes.size() > 0);
            
            const auto n = minimumSizes.size();

            std::vector<std::size_t> active;
            for (std::size_t i = 0; i < n; ++i){
                active.push_back(i);
            }

            auto assignedSizes = std::vector<float>(n, 0.0f);

            const auto divideRemainingSpace = [&](){
                float totalWeight = 0.0f;
                for (const auto i : active){
                    totalWeight += weights[i];
                }
                for (const auto i : active){
                    assignedSizes[i] = availSize * weights[i] / totalWeight;
                }
            };

            divideRemainingSpace();

            while (true){
                bool found = false;
                for (auto it = active.begin(); it != active.end();){
                    const auto i = *it;
                    if (assignedSizes[i] < minimumSizes[i]){
                        availSize -= minimumSizes[i];
                        assignedSizes[i] = minimumSizes[i];
                        it = active.erase(it);
                        divideRemainingSpace();
                        found = true;
                        break;
                    } else {
                        ++it;
                    }
                }
                if (!found){
                    return assignedSizes;
                }
            }
            assert(false);
            return assignedSizes;
        };

        const auto [requiredWidths, requiredHeights] = placeCells(m_widths, m_heights);
        
        const auto minWidth = std::accumulate(requiredWidths.begin(), requiredWidths.end(), 0.0f);
        const auto minHeight = std::accumulate(requiredHeights.begin(), requiredHeights.end(), 0.0f);
        
        if (width() < minWidth){
            setWidth(minWidth);
        }
        if (height() < minHeight){
            setHeight(minHeight);
        }

        const auto availSize = size();

        const auto newWidths = collapseAndDistribute(requiredWidths, availSize.x, m_widths);
        const auto newHeights = collapseAndDistribute(requiredHeights, availSize.y, m_heights);

        // TODO: avoid this when possible
        placeCells(newWidths, newHeights);

        return {minWidth, minHeight};
    }

    void GridContainer::onRemoveChild(const Element* e){
        for (auto& row : m_cells){
            for (auto& cell : row){
                if (cell.child == e){
                    cell.child = nullptr;
                    return;
                }
            }
        }
    }

} // namespace ui
