#pragma once

#include "cell.h"
#include "common.h"

#include <functional>

class Sheet : public SheetInterface {
public:
    ~Sheet() override = default;
    
    void SetCell(Position pos, std::string text) override;
    
    [[nodiscard]] const CellInterface* GetCell(Position pos) const override;
    [[nodiscard]] CellInterface* GetCell(Position pos) override;
    
    void ClearCell(Position pos) override;
    
    [[nodiscard]] Size GetPrintableSize() const override;
    
    void PrintValues(std::ostream& output) const override;
    void PrintTexts(std::ostream& output) const override;
    
private:
    std::vector<std::vector<std::unique_ptr<Cell>>> table_; //таблица (row - индекс внешнего вектора, col - внутреннего)
};