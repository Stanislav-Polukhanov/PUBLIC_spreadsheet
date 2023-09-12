#pragma once

#include "common.h"
#include "formula.h"

#include <optional>
#include <set>
#include <unordered_set>

class Cell : public CellInterface {
public:
    Cell(SheetInterface& sheet);
    ~Cell() override;

    void Set(std::string text, SheetInterface& sheet, Position pos);
    void Clear();

    [[nodiscard]] Value GetValue() const override;
    [[nodiscard]] std::string GetText() const override;
    
    //если не формула, возвращаем пустой вектор, иначе используем метод от FormulaImpl
    [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
    
    [[nodiscard]] bool IsReferenced() const;
    
    void InvalidateCache();

private:
    class Impl{
    public:
        virtual ~Impl() = default;
        [[nodiscard]] virtual std::string GetText() const = 0;
        [[nodiscard]] virtual Value GetValue() const = 0;
        [[nodiscard]] virtual std::vector<Position> GetReferencedCells() const;
        virtual bool HasCache();
        virtual void InvalidateCache() {}
    };

    class EmptyImpl : public Impl{
    public:
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
    };

    class TextImpl : public Impl{
    public:
        explicit TextImpl(std::string&& string_input);
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
    private:
        std::string text_;
    };

    class FormulaImpl : public Impl{
    public:
        explicit FormulaImpl(std::string&& string_input, SheetInterface& sheet);
        [[nodiscard]] std::string GetText() const override;
        [[nodiscard]] Value GetValue() const override;
        
        [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
        //bool HasCache() override;
        void InvalidateCache() override;
    private:
        std::unique_ptr<FormulaInterface> formula_;
        SheetInterface& sheet_;
        mutable std::optional<FormulaInterface::Value> cache_;
    };
    
    SheetInterface& sheet_; //задается в момент создания ячейки
    Position pos_;
    std::unique_ptr<Impl> impl_;
    std::set<CellInterface*> dependencies_; //зависимости - данная ячейка зависит от этих ячеек
    std::set<CellInterface*> back_dependencies_; //обратные зависимости - эти ячейки зависят от данной
    
    [[nodiscard]] bool CheckCircularDependencies(const Impl& checked_cell_impl, Position cell_pos) const;
    [[nodiscard]] bool CheckCircularDependencies(const CellInterface* checked_cell,
                                                 std::unordered_set<const CellInterface*>& visited_cells,
                                                 Position checked_cell_pos) const;
};