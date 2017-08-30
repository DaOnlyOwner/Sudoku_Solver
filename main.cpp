#include <iostream>
#include <string>
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
    std::tuple<int, int> find_empty_cell(int xStart, int yStart, const SudokuField &field)
    {
        for (int y = yStart; y < 9; y++)
        {
            for (int x = xStart; x < 9; x++)
            {
                if (field[y][x].number == 0) return {x,y};
            }
            xStart = 0;
        }
        return {-1,-1};
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
     * @long Explanation:
     * The algorithm starts with the first empty cell it encounters.
     * It tries to find a possible correct number.
     * Case 1: Found:
     * Based on this information it moves onto the next field and repeats the number finding recursively.
     * If no possible correct number can be found it returns and repeats with the next possible correct number.
     *
     * @param in - a sudoku field
     * @param out - the solved sudoku field
     */
    bool solve_sudoku_inner(int x, int y, const SudokuField &in)
    {
        std::vector<int> possible_nums;
        gen_possible_number(in,x,y,possible_nums);

        if (possible_nums.size() != 0)
        {
            SudokuField new_field ( in );
            for(int num : possible_nums)
            {
                new_field[y][x].number = num;

                int xNew, yNew;
                std::tie(xNew,yNew) = find_empty_cell(x,y,new_field);

                if (xNew == -1 || yNew == -1) { m_field = new_field; return true;} // Okay, cannot find empty cells anymore. Everything correctly inserted now.
                if (solve_sudoku_inner(xNew,yNew,new_field)) {return true;} // Just unwind, m_field is set already.
            }
        }
        return false; // Backtrack
    }

    // TODO: Implement
    bool CheckValidity()
    {

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