#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <regex>

class Sudoku
{

private:
    struct Cell
    {
        int number = 0;

        std::string pretty_number() const
        {
            return number == 0 ? "x" : std::to_string(number);
        }


    };
private:
    typedef std::array<std::array<Cell,9>,9> SudokuField;
    typedef std::array<Cell,9> SudokuRow;
public:
    void Load(const std::string &filename)
    {
        std::ifstream stream(filename);
        if (!stream.is_open()) throw std::invalid_argument("file doesn't exist or cannot read it.");

        std::string line;
        for (int i = 0; std::getline(stream, line); i++)
        {
            if (i > 9) throw std::invalid_argument("Sudoku had the wrong format");
            SudokuRow &row = m_field[i];

            std::regex whitespace_regex("\\s+");
            std::vector<std::string> parsed_line{
                    std::sregex_token_iterator(line.begin(), line.end(), whitespace_regex, -1), {}
            };

            if (parsed_line.size() > 9) throw std::invalid_argument("Sudoku had false format");

            for (int j = 0; j < 9; j++)
            {
                const char *num = parsed_line[j].data();

                Cell f;
                f.number = num == "x" ? 0 : atoi(num);
                row[j] = f;
            }

        }
    }

    SudokuField GetSolvedSudoku()
    {
        return m_field;
    }

    SudokuField& GetSolvedSudokuByRef()
    {
        return m_field;
    }

    void Print()
    {
        std::cout << "-------------------------";
        std::cout << std::endl;

        for(int y = 0; y<9; y++)
        {
            const SudokuRow& row = m_field[y];
            std::cout << "| ";
            for(int x = 0; x<9; x++)
            {
                const Cell& cell = row[x];
                std::cout << cell.pretty_number() << " ";
                if ((x+1)%3 == 0) std::cout << "| ";
            }
            std::cout << std::endl;
            if ((y+1)%3 == 0){ std::cout << "-------------------------";
                std::cout << std::endl;}
        }

        std::cout << std::endl << std::endl;
    }

    bool Solve()
    {
        int x,y;
        std::tie(x, y) = find_empty_cell(0,0,m_field);
        return solve_sudoku_inner(x,y,m_field);
    }

private:
    std::tuple<int, int> find_empty_cell(int xStart, int yStart, const SudokuField &field, bool exclusive = false)
    {
        if (exclusive)
        {
            xStart = (xStart + 1) % 9;
            yStart = xStart == 0 ? yStart + 1 : yStart;
            if (yStart > 8) return {-1, -1}; // Last cell
        }

        for (int y = yStart; y < 9; y++)
        {
            for (int x = xStart; x < 9; x++)
            {
                if (field[y][x].number == 0) return {x, y};
            }
            xStart = 0;
        }
        return {-1, -1};
    };

    void gen_possible_number(const SudokuField &array, int x, int y, std::vector<int> &possible_correct_number)
    {
        for (int num = 1; num < 10; num++)
        {
            bool should_continue = false;
            //Horizontal ?

            for (int yH = 0; yH < 9; yH++)
            {
                if (array[yH][x].number == num)
                {
                    should_continue = true;
                    break;
                }
            }

            if (should_continue) continue;
            // Vertical ?

            for (int xH = 0; xH < 9; xH++)
            {
                if (array[y][xH].number == num)
                {
                    should_continue = true;
                    break;
                }
            }

            if (should_continue) continue;

            // 3x3 neighbourhood ?

            int yN = y - y % 3, xN =
                    x - x % 3; // Finds the start of the neighbourhood; For example: x = 8: x - x%3 = 8 - 2 = 6
            int yEnd = yN + 3;
            int xEnd = xN + 3;

            for (int yInner = yN; yInner < yEnd; yInner++)
            {
                for (int xInner = xN; xInner < xEnd; xInner++)
                {
                    if (array[yInner][xInner].number == num)
                    {
                        should_continue = true;
                        break;
                    }
                }
            }

            if (should_continue) continue;

            possible_correct_number.push_back(num);
        }
    }

    /**
     * @brief The algorithm finds an empty cell and assigns a possible correct number to it. If no possible number can be found: Backtrack, if no cell can be found: Return true.
     * @long
     * 1. Check if c is a valid cell.
     *    Case 1: It's not
     *            Return the value of a validity check.
     * 2. Find the next empty cell c' = (x',y') starting from (x,y) so that (x,y) != (x',y')
     * 3. Foreach possible correct number n for cell c at (x,y):
     *      3.1. Set the value of c to n
     *      3.3. Call solve_sudoku(x',y', reference to current sudokufield)
     *           Case 1: False is returned:
     *                   Then continue looping.
     *           Case 2: True is returned:
     *                   Then return True exiting this function early.
     * 4. Set the value of c to 0 and return False.
     *
     * @Correctness
     *
     * Sudokufield' : The state of the Sudokufield that it had when it was passed in (Think of a local copy).
     *
     * I: If solve_sudoku(int x,int y,SudokuField& f) returns True then f is in a valid state and completed.
     *    If it returns False then f' has errors; f = f' in this case.
     *
     *    In 1. Case 1: Returning True implies that the f is completely and correctly filled in with numbers.
     *                  Completness: x and y are values that are previously returned by finding an empty cell. If there are no empty cells then x and y are in an invalid state. (Has to be implemented that way) (Fullfills I)
     *                  Correctness: Is guaranteed after a validity check (Fullfills I)
     *                  Returning False implies the same, but this time the Sudoku Field has an invalid configuration. (Fullfills I)
     *
     *    In 3. Case 1: By Induction we can assume that I holds for a call to solve_sudoku(x', y', reference to f, that was modified in 3.1).
     *                  - If True (Case 2.) is returned by this call then n was a correct choice for f (by Induction) and we return True indicating that f has no errors. Also f is completed (by Induction) which fullfills I.
     *                  - If False (Case 1.) is returned then there is a number in our current configuration that puts f' into an invalid state.
     *                    -> Assume n is the cause. We thus have to find a valid number by iterating over the rest.
     *                          -> None of them is valid. End up in 4., delete our choice to set f back to the state it was in previously (Requirement of I)(set the value of c to 0) and return False because the error was introduced earlier. Fullfills I.
     *                          -> One of them is valid. End up in Case 2.
     *                    -> Assume n is not the cause and would be a correct choice. Then looping will do no harm. solve_sudoku() will always return False in this case because the invalid number was introduced earlier somewhere in f'. So we end up in 4., delete our choice, so that f = f' and return False, fullfilling I.
     *
     *    Calling solve_sudoku(0,0,f) will thus return True and F is solved, or False and F cannot be solved.
     *
     * @param in - a sudoku field
     * @param out - the solved sudoku field
     */
    bool solve_sudoku_inner(int x, int y, SudokuField &in_out)
    {

        if (x == -1 || y == -1) return is_valid(in_out); // 1.
        int xNew, yNew;
        std::tie(xNew, yNew) = find_empty_cell(x,y, in_out, true); // 2.

        std::vector<int> possible_nums;
        gen_possible_number(in_out,x,y,possible_nums);

        for(int num : possible_nums) // 3.
        {
            in_out[y][x].number = num; // 3.1
            if (solve_sudoku_inner(xNew,yNew,in_out)) return true; // 3.2
            // 3.1 needs no additional check.
        }

        in_out[y][x].number = 0; // 4.
        return false; // 4.

    }

    // TODO: Implement
    bool is_valid(const SudokuField& in)
    {
        for(int y = 0; y<9; y++)
        {
            for(int x = 0; x<9; x++)
            {
                const Cell& cell = in[y][x];
                std::vector<int> possible_nums;
                gen_possible_number(in,x,y,possible_nums);
                if (possible_nums.empty()) return false;
            }
        }

        return true;
    }


private:
    SudokuField m_field;
};
int main()
{
    Sudoku sudoku;
    sudoku.Load("test_sudoku.txt");
    std::cout << sudoku.Solve() << std::endl;
    sudoku.Print();

    return 0;
}