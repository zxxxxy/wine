#include "AI.h"
#include<ctime>
#include<cstdlib>
#include<iostream>
using namespace std;

int AI::GetTime() {
  return (clock() - start) / CLOCKS_PER_SEC * 1000;
}

int AI::StopTime() {
  return (timeout_turn < time_left / 7) ? timeout_turn : time_left / 7;
}
// 返回最佳点
Pos AI::gobang() {
  total = 0;
  stopThink = false;
  srand(time(NULL));
  start = clock();

  // 第一步下中心点
  if (step == 0) {
    BestMove.x = size / 2;
    BestMove.y = size / 2;
    return BestMove;
  }
  // 第二，三步随机
  if (step == 1) {
    int rx, ry;
    int d = step * 2 + 1;
    do {
      rx = rand() % d + remMove[1].x - step;
      ry = rand() % d + remMove[1].y - step;
    } while (cell[rx][ry].piece != 0);
    BestMove.x = rx;
    BestMove.y = ry;
    return BestMove;
  }

    // 迭代加深搜索
    for (int i = 2; i <= SearchDepth; i += 2) {
    if (GetTime() * 14 >= StopTime() && i > 4)
      break;
    MaxDepth = i;

    _BestVal = minimax(i, -10001, 10000);
    if (!stopThink) {
      BestMove = _BestMove;
      BestVal = _BestVal;
    }
    if (BestVal == 10000) break;
  }

  ThinkTime = (clock() - start) / CLOCKS_PER_SEC * 1000;
  return BestMove;
}

// max函数
int AI::minimax(int depth, int alpha, int beta) {
  UpdateRound(2);
  Pos moves[22];
  Pos p;
  int val;
  int count = GetMove(moves, 20);

  if (count == 1) {
    _BestMove = moves[1];
    return BestVal;
  }

  moves[0] = (depth > 2) ? BestMove : moves[1];

  // 遍历所有走法
  for (int i = 0; i <= count; i++) {

    p = moves[i];
    if (i > 0 && p.x == moves[0].x && p.y == moves[0].y)
      continue;

    if (GetTime() + 1000 >= StopTime())
      return alpha;

    MakeMove(moves[i]);

    do {
      if (i > 0 && alpha + 1 < beta) {
        val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);
        if (val <= alpha || val >= beta)
          break;
      }
      val = -AlphaBeta(depth - 1, -beta, -alpha);
    } while (0);

    DelMove();

    if (val >= beta) {
      _BestMove = moves[i];
      return val;
    }

    if (val > alpha) {
      alpha = val;
      _BestMove = moves[i];
    }
  }
  return alpha;
}

// alpha-beta搜索
int AI::AlphaBeta(int depth, int alpha, int beta) {
  int val;
  static int cnt = 0;
  Pos moves[22];
  total++;

  // 对方最后一子连五
  if (CheckWin())
    return -10000;

  // 叶节点
  if (depth == 0)
    return evaluate();

  if (--cnt <= 0) {
    cnt = 1000;
    if (GetTime() + 1000 >= StopTime())
      stopThink = true;
  }
  if (stopThink)
    return alpha;

  int count = GetMove(moves, 16);
  // 遍历所有走法
  for (int i = 1; i <= count; i++) {
    if (stopThink)
      break;

    MakeMove(moves[i]);

    do {
      if (i > 1 && alpha + 1 < beta) {
        val = -AlphaBeta(depth - 1, -alpha - 1, -alpha);
        if (val <= alpha || val >= beta)
          break;
      }
      val = -AlphaBeta(depth - 1, -beta, -alpha);
    } while (0);

    DelMove();

    if (val >= beta) {
      return val;
    }
    if (val > alpha) {
      alpha = val;
    }
  }
  return alpha;
}
int AI::CutCand(Pos * moves, Point * cand, int Csize,int branch) {
  int me = color(step + 1);
  int Msize = 0;

  if (cand[1].val >= 2400) {
    Msize = 1;
    moves[Msize] = cand[1].p;
  }
  if (cand[1].val >= 1200 && cand[1].val < 2000) {
    Msize = 1;
    moves[Msize] = cand[1].p;
    if (cand[2].val >= 1200) {
      Msize = 2;
      moves[Msize] = cand[2].p;
    }
    for (int i = Msize + 1; i <= Csize&& Msize < branch;++i){
      if (IsType(cand[i].p, me, block4) || IsType(cand[i].p, 3 - me, block4)) {
        ++Msize;
        moves[Msize] = cand[i].p;
      }
    }
  }
  return Msize;
}

// 生成所有着法，并返回个数
int AI::GetMove(Pos * moves, int branch) {
  Point cand[200];
  int Csize = 0, Msize = 0;
  int val;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      if (IsCand[i][j] && cell[i][j].piece == 0) {
        val = ScoreMove(i, j);
        if (val > 0) {
          ++Csize;
          cand[Csize].p.x = i;
          cand[Csize].p.y = j;
          cand[Csize].val = val;
        }

      }
    }
  }
  // 着法排序
  sort(cand, Csize);

  // 棋型剪枝
  Msize = CutCand(moves, cand, Csize,branch);

  // 如果没有剪枝
  if (Msize == 0) {
    Msize=(Csize < branch) ? Csize : branch;
    for (int k = 1; k <= Msize; k++) {
      moves[k] = cand[k].p;
    }
  }

  return Msize;
}

// 排序
void AI::sort(Point * a, int n) {
  int i, j;
  Point key;
  for (i = 2; i <= n; i++) {
    key = a[i];
    for (j = i; j > 1 && a[j - 1].val < key.val; j--) {
      a[j] = a[j - 1];
    }
    a[j] = key;
  }
}


// 估值函数
int AI::evaluate() {
  int val;
  int max = -1;
  Pos p;
  // 选出最佳点
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      if (IsCand[i][j] && cell[i][j].piece == 0) {
        val = ScoreMove(i, j);
        if (val > max) {
          max = val;
          p.x = i;
          p.y = j;
        }
      }
    }
  }
  // 评估最佳点下了之后的局势
  MakeMove(p);
  val = evaluate2();
  DelMove();
  return val;
}

// 棋型估值函数
int AI::evaluate2() {
  int Ctype[Ntype] = { 0 };
  int Htype[Ntype] = { 0 };
  int Cscore = 0, Hscore = 0;
  int me = color(step);

  AllType(me, Ctype);
  AllType(3 - me, Htype);

  if (Ctype[win] > 0)
    return 10000;
  if (Htype[flex4] > 0 || Htype[block4] > 0)
    return -10000;
  if (Ctype[flex4] > 0 || Ctype[block4] > 1)
    return 10000;
  if (Htype[flex3] > 0 && Ctype[block4] == 0)
    return -10000;

  for (int i = 1; i <= block4; ++i) {
    Cscore += Ctype[i] * Tval[i];
    Hscore += Htype[i] * Tval[i];
  }

  return Cscore - Hscore;
}


// 着法打分
int AI::ScoreMove(int x, int y) {
  int score = 0;
  int MeType[Ntype] = { 0 };
  int YouType[Ntype] = { 0 };
  int me = color(step + 1);


  TypeCount(x, y, me, MeType);
  TypeCount(x, y, 3 - me, YouType);

  if (MeType[win] > 0)
    return 10000;
  if (YouType[win] > 0)
    return 5000;
  if (MeType[flex4] > 0 || MeType[block4] > 1)
    return 2400;

  if (MeType[block4] > 0 && MeType[flex3] > 0)
    score += 2000;
  else if (YouType[flex4] > 0 || YouType[block4] > 1)
    score += 1200;
  else if (YouType[block4] > 0 && YouType[flex3] > 0)
    score += 1000;
  else if (MeType[flex3] > 1)
    score += 400;
  else if (YouType[flex3] > 1)
    score += 200;

  for (int i = 1; i <= block4; i++) {
    score += MeVal[i] * MeType[i];
    score += YouVal[i] * YouType[i];
  }

  return score;
}

 // 统计棋盘所有棋型个数
void AI::AllType(int role, int *type) {
  int x, y, i;
  int cnt[Ntype] = { 0, 2, 2, 3, 3, 4, 4, 5 };

  for (i = role; i <= step; i += 2) {
    x = remMove[i].x;
    y = remMove[i].y;
    TypeCount(x, y, role, type);
  }

  for (i = 1; i < Ntype; ++i) {
    if (type[i] % cnt[i] == cnt[i] - 1)
      ++type[i];
    type[i] /= cnt[i];
  }
}

