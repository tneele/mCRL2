// Author(s): Wieger Wesselink
// Copyright: see the accompanying file COPYING or copy at
// https://github.com/mCRL2org/mCRL2/blob/master/COPYING
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
/// \file ticket_1300.cpp
/// \brief Test for find functions.

#define BOOST_TEST_MODULE ticket_1300
#include "mcrl2/atermpp/aterm_io.h"
#include "mcrl2/data/data_specification.h"
#include "mcrl2/data/detail/io.h"
#include "mcrl2/data/parse.h"
#include "mcrl2/data/print.h"
#include <boost/test/included/unit_test_framework.hpp>

using namespace mcrl2;

void test1()
{
  std::string expression_text = "DataAppl(OpId(Four_in_a_row_diagonally_column,SortArrow([SortId(Piece),SortId(Pos),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortId(Bool)),88),OpId(Red,SortId(Piece),87),DataAppl(OpId(@cDub,SortArrow([SortId(Bool),SortId(Pos)],SortId(Pos)),89),OpId(false,SortId(Bool),43),OpId(@c1,SortId(Pos),90)),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(Red,SortId(Piece),87),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),DataAppl(OpId(|>,SortArrow([SortCons(SortList,SortId(Piece)),SortCons(SortList,SortCons(SortList,SortId(Piece)))],SortCons(SortList,SortCons(SortList,SortId(Piece)))),79),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),DataAppl(OpId(|>,SortArrow([SortId(Piece),SortCons(SortList,SortId(Piece))],SortCons(SortList,SortId(Piece))),84),OpId(None,SortId(Piece),86),OpId(\"[]\",SortCons(SortList,SortId(Piece)),83)))))))),OpId(\"[]\",SortCons(SortList,SortCons(SortList,SortId(Piece))),78))))))))";

  std::string dataspec_text =
  "sort Piece = struct Red | Yellow | None;\n"
  "     Row = List(Piece);\n"
  "     Board = List(Row);\n"
  "     S1;\n"
  "     S2;\n"
  "\n"
  "cons c_,c_1: S1;\n"
  "     c_2,c_3: S2;\n"
  "\n"
  "map  N,M,R: Pos;\n"
  "     At: Nat # Nat # Board -> Piece;\n"
  "     At: Nat # Row -> Piece;\n"
  "     PutColumn: Piece # Pos # Board -> Board;\n"
  "     PutColumn,Put: Piece # Pos # Pos # Board -> Board;\n"
  "     Put: Piece # Pos # Row -> Row;\n"
  "     Four_in_a_row,Four_in_a_row_horizontally: Piece # Board -> Bool;\n"
  "     Four_in_a_row_horizontally: Piece # Pos # Board -> Bool;\n"
  "     Count_rows_horizontally: Piece # Pos # Pos # Board # Nat -> Bool;\n"
  "     Four_in_a_row_vertically: Piece # Board -> Bool;\n"
  "     Four_in_a_row_vertically: Piece # Pos # Board -> Bool;\n"
  "     Count_rows_vertically: Piece # Pos # Pos # Board # Nat -> Bool;\n"
  "     Four_in_a_row_diagonally: Piece # Board -> Bool;\n"
  "     Four_in_a_row_diagonally_column,Four_in_a_row_diagonally_row: Piece # Pos # Board -> Bool;\n"
  "     Count_rows_diagonally,Count_rows_diagonally': Piece # Pos # Pos # Board # Nat -> Bool;\n"
  "     initial_board: Board;\n"
  "     pi_S1_: List(List(Piece)) -> List(Piece);\n"
  "     pi_S1_1: List(List(Piece)) -> List(List(Piece));\n"
  "     C_S1_: S1 # List(Piece) # List(Piece) -> List(Piece);\n"
  "     C_S1_: S1 # List(List(Piece)) # List(List(Piece)) -> List(List(Piece));\n"
  "     C_S1_: S1 # S1 # S1 -> S1;\n"
  "     Det_S1_: List(List(Piece)) -> S1;\n"
  "     pi_S2_: List(Piece) -> Piece;\n"
  "     pi_S2_1: List(Piece) -> List(Piece);\n"
  "     C_S2_: S2 # Piece # Piece -> Piece;\n"
  "     C_S2_: S2 # List(Piece) # List(Piece) -> List(Piece);\n"
  "     C_S2_: S2 # S2 # S2 -> S2;\n"
  "     Det_S2_: List(Piece) -> S2;\n"
  "\n"
  "var  b,b': Board;\n"
  "     r: Row;\n"
  "     p,p',p'': Piece;\n"
  "     x,y: Nat;\n"
  "     c: Bool;\n"
  "     z: Pos;\n"
  "eqn  N  =  7;\n"
  "     M  =  1;\n"
  "     R  =  4;\n"
  "     if(c, true, false)  =  c;\n"
  "     if(c, false, true)  =  !c;\n"
  "     if(c, p, p') == p''  =  if(c, p == p'', p' == p'');\n"
  "     y == 1  ->  At(x, y, r |> b)  =  At(x, r);\n"
  "     1 < y && y <= M  ->  At(x, y, r |> b)  =  At(x, Int2Nat(y - 1), b);\n"
  "     y == 0 || y > M || x == 0 || x > N  ->  At(x, y, b)  =  None;\n"
  "     At(x, y, if(c, b, b'))  =  if(c, At(x, y, b), At(x, y, b'));\n"
  "     x == 1  ->  At(x, p |> r)  =  p;\n"
  "     1 < x && x <= N  ->  At(x, p |> r)  =  At(Int2Nat(x - 1), r);\n"
  "     x == 0 || x > N  ->  At(x, p |> r)  =  None;\n"
  "     At(x, Put(p, z, r))  =  if(x == z, p, At(x, r));\n"
  "\n"
  "var  b,b': Board;\n"
  "     r: Row;\n"
  "     p,p': Piece;\n"
  "     x,y: Pos;\n"
  "     dx,dy: Int;\n"
  "     c,othercolorseen: Bool;\n"
  "     counter: Nat;\n"
  "     y3,y2,d1: List(List(Piece));\n"
  "     y1: S1;\n"
  "     d: List(Piece);\n"
  "eqn  y == 1  ->  Put(p, x, y, r |> b)  =  Put(p, x, r) |> b;\n"
  "     y > 1 && y <= M  ->  Put(p, x, y, r |> b)  =  r |> Put(p, x, Int2Pos(y - 1), b);\n"
  "     Put(p, x, y, if(c, b, b'))  =  if(c, Put(p, x, y, b), Put(p, x, y, b'));\n"
  "     x == 1  ->  Put(p, x, p' |> r)  =  p |> r;\n"
  "     x > 1 && x <= N  ->  Put(p, x, p' |> r)  =  p' |> Put(p, Int2Pos(x - 1), r);\n"
  "     PutColumn(p, x, b)  =  PutColumn(p, x, 1, b);\n"
  "     y < M  ->  PutColumn(p, x, y, b)  =  if(At(x, y, b) == None, Put(p, x, y, b), PutColumn(p, x, y + 1, b));\n"
  "     y == M  ->  PutColumn(p, x, y, b)  =  Put(p, x, M, b);\n"
  "     Four_in_a_row(p, b)  =  Four_in_a_row_horizontally(p, b) || Four_in_a_row_vertically(p, b) || Four_in_a_row_diagonally(p, b);\n"
  "     Four_in_a_row_horizontally(p, b)  =  Four_in_a_row_horizontally(p, 1, b);\n"
  "     y < M  ->  Four_in_a_row_horizontally(p, y, b)  =  Count_rows_horizontally(p, 1, y, b, 0) || Four_in_a_row_horizontally(p, y + 1, b);\n"
  "     y == M  ->  Four_in_a_row_horizontally(p, y, b)  =  Count_rows_horizontally(p, 1, M, b, 0);\n"
  "     x < N  ->  Count_rows_horizontally(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1 || Count_rows_horizontally(p, x + 1, y, b, counter + 1), Count_rows_horizontally(p, x + 1, y, b, 0));\n"
  "     x == N  ->  Count_rows_horizontally(p, x, y, b, counter)  =  if(At(N, y, b) == p, counter >= R - 1, false);\n"
  "     Four_in_a_row_vertically(p, b)  =  Four_in_a_row_vertically(p, 1, b);\n"
  "     x < N  ->  Four_in_a_row_vertically(p, x, b)  =  Count_rows_vertically(p, x, 1, b, 0) || Four_in_a_row_vertically(p, x + 1, b);\n"
  "     x == N  ->  Four_in_a_row_vertically(p, x, b)  =  Count_rows_vertically(p, N, 1, b, 0);\n"
  "     y < M  ->  Count_rows_vertically(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1 || Count_rows_vertically(p, x, y + 1, b, counter + 1), Count_rows_vertically(p, x, y + 1, b, 0));\n"
  "     y == M  ->  Count_rows_vertically(p, x, y, b, counter)  =  if(At(x, M, b) == p, counter >= R - 1, false);\n"
  "     Four_in_a_row_diagonally(p, b)  =  Four_in_a_row_diagonally_column(p, 2, b) || Four_in_a_row_diagonally_row(p, 1, b);\n"
  "     x < N  ->  Four_in_a_row_diagonally_row(p, x, b)  =  Count_rows_diagonally(p, x, 1, b, 0) || Count_rows_diagonally'(p, x, 1, b, 0) || Four_in_a_row_diagonally_row(p, x + 1, b);\n"
  "     x == N  ->  Four_in_a_row_diagonally_row(p, x, b)  =  Count_rows_diagonally'(p, N, 1, b, 0);\n"
  "     y < M  ->  Four_in_a_row_diagonally_column(p, y, b)  =  Count_rows_diagonally(p, 1, y, b, 0) || Count_rows_diagonally'(p, N, y, b, 0) || Four_in_a_row_diagonally_column(p, y + 1, b);\n"
  "     y == M  ->  Four_in_a_row_diagonally_column(p, y, b)  =  false;\n"
  "     x < N && y < M  ->  Count_rows_diagonally(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1 || Count_rows_diagonally(p, x + 1, y + 1, b, counter + 1), Count_rows_diagonally(p, x + 1, y + 1, b, 0));\n"
  "     x == N || y == M  ->  Count_rows_diagonally(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1, false);\n"
  "     x > 1 && y < M  ->  Count_rows_diagonally'(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1 || Count_rows_diagonally'(p, max(1, x - 1), y + 1, b, counter + 1), Count_rows_diagonally'(p, max(1, x - 1), y + 1, b, 0));\n"
  "     x == 1 || y == M  ->  Count_rows_diagonally'(p, x, y, b, counter)  =  if(At(x, y, b) == p, counter >= R - 1, false);\n"
  "     initial_board  =  [[None, None, None, None, None, None, None], [None, None, None, None, None, None, None], [None, None, None, None, None, None, None], [None, None, None, None, None, None, None], [None, None, None, None, None, None, None], [None, None, None, None, None, None, None]];\n"
  "     C_S1_(c_, y2, y3)  =  y2;\n"
  "     C_S1_(c_1, y2, y3)  =  y3;\n"
  "     C_S1_(y1, y3, y3)  =  y3;\n"
  "     Det_S1_([])  =  c_;\n"
  "     Det_S1_(d |> d1)  =  c_1;\n"
  "     pi_S1_(d |> d1)  =  d;\n"
  "\n"
  "var  b,b2,b4: Bool;\n"
  "     d2,d3,d4,d5,d1,d6,d7,y15,y14,d8,d9,y19,y18,d10,d11,d12,d13: List(List(Piece));\n"
  "     y7,y6,y11,y10,d,y3,y2: List(Piece);\n"
  "     y5,b1,y9,y13,b3,y17,y23,y22,y21,b5: S1;\n"
  "     y1: S2;\n"
  "eqn  pi_S1_(if(b, d2, d3))  =  if(b, pi_S1_(d2), pi_S1_(d3));\n"
  "     C_S1_(c_, y6, y7)  =  y6;\n"
  "     C_S1_(c_1, y6, y7)  =  y7;\n"
  "     C_S1_(y5, y7, y7)  =  y7;\n"
  "     pi_S1_(C_S1_(b1, d4, d5))  =  C_S1_(b1, pi_S1_(d4), pi_S1_(d5));\n"
  "     C_S1_(c_, y10, y11)  =  y10;\n"
  "     C_S1_(c_1, y10, y11)  =  y11;\n"
  "     C_S1_(y9, y11, y11)  =  y11;\n"
  "     pi_S1_1(d |> d1)  =  d1;\n"
  "     pi_S1_1(if(b2, d6, d7))  =  if(b2, pi_S1_1(d6), pi_S1_1(d7));\n"
  "     C_S1_(c_, y14, y15)  =  y14;\n"
  "     C_S1_(c_1, y14, y15)  =  y15;\n"
  "     C_S1_(y13, y15, y15)  =  y15;\n"
  "     pi_S1_1(C_S1_(b3, d8, d9))  =  C_S1_(b3, pi_S1_1(d8), pi_S1_1(d9));\n"
  "     C_S1_(c_, y18, y19)  =  y18;\n"
  "     C_S1_(c_1, y18, y19)  =  y19;\n"
  "     C_S1_(y17, y19, y19)  =  y19;\n"
  "     Det_S1_(if(b4, d10, d11))  =  if(b4, Det_S1_(d10), Det_S1_(d11));\n"
  "     C_S1_(c_, y22, y23)  =  y22;\n"
  "     C_S1_(c_1, y22, y23)  =  y23;\n"
  "     C_S1_(y21, y23, y23)  =  y23;\n"
  "     Det_S1_(C_S1_(b5, d12, d13))  =  C_S1_(b5, Det_S1_(d12), Det_S1_(d13));\n"
  "     C_S2_(c_2, y2, y3)  =  y2;\n"
  "     C_S2_(c_3, y2, y3)  =  y3;\n"
  "     C_S2_(y1, y3, y3)  =  y3;\n"
  "     Det_S2_([])  =  c_2;\n"
  "\n"
  "var  d,y7,y6,y11,y10: Piece;\n"
  "     d1,d2,d3,d4,d5,d6,d7,y15,y14,d8,d9,y19,y18,d10,d11,d12,d13: List(Piece);\n"
  "     b,b2,b4: Bool;\n"
  "     y5,b1,y9,y13,b3,y17,y23,y22,y21,b5: S2;\n"
  "eqn  Det_S2_(d |> d1)  =  c_3;\n"
  "     pi_S2_(d |> d1)  =  d;\n"
  "     pi_S2_(if(b, d2, d3))  =  if(b, pi_S2_(d2), pi_S2_(d3));\n"
  "     C_S2_(c_2, y6, y7)  =  y6;\n"
  "     C_S2_(c_3, y6, y7)  =  y7;\n"
  "     C_S2_(y5, y7, y7)  =  y7;\n"
  "     pi_S2_(C_S2_(b1, d4, d5))  =  C_S2_(b1, pi_S2_(d4), pi_S2_(d5));\n"
  "     C_S2_(c_2, y10, y11)  =  y10;\n"
  "     C_S2_(c_3, y10, y11)  =  y11;\n"
  "     C_S2_(y9, y11, y11)  =  y11;\n"
  "     pi_S2_1(d |> d1)  =  d1;\n"
  "     pi_S2_1(if(b2, d6, d7))  =  if(b2, pi_S2_1(d6), pi_S2_1(d7));\n"
  "     C_S2_(c_2, y14, y15)  =  y14;\n"
  "     C_S2_(c_3, y14, y15)  =  y15;\n"
  "     C_S2_(y13, y15, y15)  =  y15;\n"
  "     pi_S2_1(C_S2_(b3, d8, d9))  =  C_S2_(b3, pi_S2_1(d8), pi_S2_1(d9));\n"
  "     C_S2_(c_2, y18, y19)  =  y18;\n"
  "     C_S2_(c_3, y18, y19)  =  y19;\n"
  "     C_S2_(y17, y19, y19)  =  y19;\n"
  "     Det_S2_(if(b4, d10, d11))  =  if(b4, Det_S2_(d10), Det_S2_(d11));\n"
  "     C_S2_(c_2, y22, y23)  =  y22;\n"
  "     C_S2_(c_3, y22, y23)  =  y23;\n"
  "     C_S2_(y21, y23, y23)  =  y23;\n"
  "     Det_S2_(C_S2_(b5, d12, d13))  =  C_S2_(b5, Det_S2_(d12), Det_S2_(d13));\n"
  ;

  data::data_specification dataspec = data::parse_data_specification(dataspec_text);

  atermpp::aterm t = atermpp::read_term_from_string(expression_text);
  t = data::detail::remove_index(t);
  t = data::detail::add_index(t);
  const data::data_expression& d = atermpp::down_cast<data::data_expression>(t);
  std::string s = data::pp(d);
  std::cout << s << std::endl;
}

BOOST_AUTO_TEST_CASE(test_main)
{
  test1();
}
