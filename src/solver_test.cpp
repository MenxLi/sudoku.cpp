#include "config.h"
#include "solver.h"
#include <string>

// case from: https://www.kaggle.com/datasets/bryanpark/sudoku/
std::vector<std::string> cases = {
"004300209005009001070060043006002087190007400050083000600000105003508690042910300,864371259325849761971265843436192587198657432257483916689734125713528694542916378",
"040100050107003960520008000000000017000906800803050620090060543600080700250097100,346179258187523964529648371965832417472916835813754629798261543631485792254397186",
"600120384008459072000006005000264030070080006940003000310000050089700000502000190,695127384138459672724836915851264739273981546946573821317692458489715263562348197",
"497200000100400005000016098620300040300900000001072600002005870000600004530097061,497258316186439725253716498629381547375964182841572639962145873718623954534897261",
"005910308009403060027500100030000201000820007006007004000080000640150700890000420,465912378189473562327568149738645291954821637216397854573284916642159783891736425",
"100005007380900000600000480820001075040760020069002001005039004000020100000046352,194685237382974516657213489823491675541768923769352841215839764436527198978146352",
"009065430007000800600108020003090002501403960804000100030509007056080000070240090,289765431317924856645138729763891542521473968894652173432519687956387214178246395",
"000000657702400100350006000500020009210300500047109008008760090900502030030018206,894231657762495183351876942583624719219387564647159328128763495976542831435918276",
"503070190000006750047190600400038000950200300000010072000804001300001860086720005,563472198219386754847195623472638519951247386638519472795864231324951867186723945",
"060720908084003001700100065900008000071060000002010034000200706030049800215000090,163725948584693271729184365946358127371462589852917634498231756637549812215876493",
"004083002051004300000096710120800006040000500830607900060309040007000205090050803,974183652651274389283596714129835476746912538835647921568329147317468295492751863",
"000060280709001000860320074900040510007190340003006002002970000300800905500000021,431567289729481653865329174986243517257198346143756892612975438374812965598634721",
"004300000890200670700900050500008140070032060600001308001750900005040012980006005,254367891893215674716984253532698147178432569649571328421753986365849712987126435",
"008070100120090054000003020604010089530780010009062300080040607007506000400800002,958274163123698754746153928674315289532789416819462375285941637397526841461837592",
"065370002000001370000640800097004028080090001100020940040006700070018050230900060,865379412924581376713642895397164528482795631156823947541236789679418253238957164",
"005710329000362800004000000100000980083900250006003100300106000409800007070029500,865714329917362845234598761142657983783941256596283174358176492429835617671429538",
"200005300000073850000108904070009001651000040040200080300050000580760100410030096,268495317194673852735128964872549631651387249943216785326951478589764123417832596",
};

std::pair<
    std::vector<std::vector<val_t>>,
    std::vector<std::vector<val_t>>
> parse_case(const std::string& c){
    std::vector<std::vector<val_t>> input(BOARD_SIZE, std::vector<val_t>(BOARD_SIZE));
    std::vector<std::vector<val_t>> expected(BOARD_SIZE, std::vector<val_t>(BOARD_SIZE));
    std::string input_str = c.substr(0, 81);
    std::string expected_str = c.substr(82);
    ASSERT(BOARD_SIZE * BOARD_SIZE == input_str.size(), "Invalid input size");
    for (unsigned int i = 0; i < BOARD_SIZE; i++){
        for (unsigned int j = 0; j < BOARD_SIZE; j++){
            input[i][j] = input_str[i * BOARD_SIZE + j] - '0';
            expected[i][j] = expected_str[i * BOARD_SIZE + j] - '0';
        }
    };
    return std::make_pair(input, expected);
}

int main(){
    for (const auto& c : cases){
        auto [input, expected] = parse_case(c);
        Board board;
        board.load_data(input);
        Solver solver(board);
        solver.solve();

        bool correct = true;
        for (unsigned int i = 0; i < BOARD_SIZE; i++){
            for (unsigned int j = 0; j < BOARD_SIZE; j++){
                if (solver.board().get(i, j) != expected[i][j]){
                    correct = false;
                    break;
                }
            }
        }
        if (correct){
            std::cout << "Passed." << std::endl;
        } else {
            std::cout << "Failed." << std::endl;
        }
    }
    return 0;
}