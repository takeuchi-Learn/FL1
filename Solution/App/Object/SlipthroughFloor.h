#pragma once
#include "StageObject.h"

class GameCamera;
class Billboard;

// 今の状態だとマップチップで指定できない為、どうにかする
// 地形判定の配列とは別の配列を用意し、すり抜け床だけ隔離する
// で、GameMapから取得できるようにし、typeid(GameMap)で地形か確認していた部分を地形とすり抜け床で分ける
// ゴールもこれでいいかもしれない

// すり抜け床 戦場のあれ
class SlipthroughFloor :
    public StageObject
{
private:

public:
	SlipthroughFloor(GameCamera* camera, const DirectX::XMFLOAT2& pos, float scale);
};

