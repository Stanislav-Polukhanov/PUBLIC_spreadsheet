#include "formula.h"

#include "FormulaAST.h"

#include <algorithm>
#include <sstream>

using namespace std::literals;

bool Size::operator==(Size rhs) const {
    return rows == rhs.rows && cols == rhs.cols;
}

std::ostream& operator<<(std::ostream& output, FormulaError) {
    return output << "#DIV/0!";
}

namespace {
    class Formula : public FormulaInterface {
    public:
        explicit Formula(std::string expression)
            try : ast_(ParseFormulaAST(expression)) {}
            catch (...){throw FormulaException("formula exception");}
        

        [[nodiscard]] Value Evaluate(const SheetInterface& sheet) const override;
        
        [[nodiscard]] std::string GetExpression() const override;
        
        [[nodiscard]] std::vector<Position> GetReferencedCells() const override;
    private:
        FormulaAST ast_;
    };
    
    FormulaInterface::Value Formula::Evaluate(const SheetInterface &sheet) const {
        std::function<double(Position)> cell_value_getter = [&sheet](const Position pos) -> double {
            if (!pos.IsValid()){
                throw FormulaError(FormulaError::Category::Ref);
            }
            const CellInterface* cell_ptr = sheet.GetCell(pos);
            if (!cell_ptr){
                return 0.0;
            }
            const auto cell_value = cell_ptr->GetValue();
            if (std::holds_alternative<double>(cell_value)){
                return std::get<double>(cell_value);
            }
            if (std::holds_alternative<std::string>(cell_value)){
                const std::string& cell_string = std::get<std::string>(cell_value);
                if (cell_string.empty()){
                    return 0.0;
                }
                try {
                    return std::stod(cell_string);
                }
                catch (const std::invalid_argument&){
                    throw FormulaError(FormulaError::Category::Value);
                }
            }
            if (std::holds_alternative<FormulaError>(cell_value)){
                throw FormulaError(std::get<FormulaError>(cell_value));
            }
            assert(false);
        };
        try {
            return ast_.Execute(cell_value_getter);
        }
        catch (const FormulaError& fe){
            return fe;
        }
    }
    
    std::string Formula::GetExpression() const {
        std::ostringstream stream;
        ast_.PrintFormula(stream);
        return stream.str();
    }
    
    std::vector<Position> Formula::GetReferencedCells() const {
        std::vector<Position> cells(ast_.GetCells().begin(), ast_.GetCells().end());
        std::sort(cells.begin(), cells.end());
        cells.erase(std::unique(cells.begin(), cells.end()), cells.end());
        return cells;
    }
}  // namespace

std::unique_ptr<FormulaInterface> ParseFormula(std::string expression) {
    return std::make_unique<Formula>(std::move(expression));
}