#pragma once
#include <Camera/Camera.h>
#include <3D/Billboard/Billboard.h>

// 後々の追従を想定してObjのやつ
// CameraObjだと上ベクトルの制御が不可能になるからこちらで追従機能を追加したほうがいいかも

// Z軸回転と追従機能だけ持ったカメラクラス
class GameCamera :
	public Camera
{
private:

	// 追従Obj
	BillboardData* obj = nullptr;
	// 角度Z(最初に斜めの状態で開始するため、20,fをセット)
	float angle = 20.f;

	/// @brief カメラの状態列挙
	enum class CameraState
	{
		START,// 開始
		INPUT,// 入力受付
		FOLLOW_OFF,// 入力受付 追従オフ(画面端用)
		CLEAR,// クリア クリア時に演出でカメラを制御する必要がありそうなので追加
		OTHER, //その他(何も更新しないとき)
	};

	// カメラの状態
	CameraState cameraState = CameraState::INPUT;

#pragma region START
	// スタートするまでの時間
	unsigned short startTimer = 0;
	// 上記関数の起動フラグ
	bool addStartTimer = false;

	// 1フレームの傾き(スタート時)。動的に変えるため、変数にしている
	float startFrameAngle = 1.5f;
	// スタート演出の補間処理
	bool startLerp = false;
#pragma endregion


private:
#pragma region START

	/// @brief CameraState::STARTのupdate
	void updateStart();

	/// @brief Start時の移動
	void startAutoRot();

	/// @brief 変数 startTimer の加算処理
	void updateStartTimer();

#pragma endregion

#pragma region INPUT

	/// @brief CameraState::INPUTの時のupdate
	void updateInput();

	/// @brief 入力確認とそれに応じた角度の加算減算
	void checkInput();
#pragma endregion

#pragma region CLEAR
	/// @brief CameraState::CLEARのupdate
	void updateClear();
#pragma endregion


	void preUpdate() override;

	/// @brief 角度を上ベクトルに変換
	/// @param angle 角度
	/// @param float2 上ベクトルを格納するXMFLOAT2
	void angleToUp(float angle, DirectX::XMFLOAT2& float2);

	/// @brief 上ベクトルの回転(カメラ回転)
	void upRotate();

	/// @brief 追従
	void followObject();


public:

	/// @brief コンストラクタ
	/// @param obj プレイヤーのポインタ(追従させるために渡す)
	GameCamera(BillboardData* obj = nullptr);
	/// @brief 更新(元々のupdateと被らないように名前長くしてる)
	void gameCameraUpdate();

	inline float getAngleDeg() const { return angle; }

	/// @brief ジャイロの値のセット。
	void setGyroValue(float value) { angle = value; }

	/// @brief 追従先オブジェクト
	/// @param obj 
	void setParentObj(BillboardData* obj) { this->obj = obj; }

	/// @brief Z座標
	/// @param z 
	void setEyeZ(float z) { setEye(DirectX::XMFLOAT3(0, 0, z)); }

	/// @brief 角度の取得
	/// @return 角度
	float getAngle()const { return angle; }

	// ポーズしてるかどうか
	void pause(const bool flag)
	{
		if(flag)cameraState = CameraState::OTHER;
		else cameraState = CameraState::INPUT;
	}

	// クリア状態に変更
	void changeStateClear() { cameraState = CameraState::CLEAR; }
	void changeStateGameover(){ cameraState = CameraState::OTHER; }
	
	/// @brief 追従のオンオフ
	/// @param flag 
	void setFollowFlag(const bool flag) { cameraState = flag ? CameraState::INPUT : CameraState::FOLLOW_OFF; }
};

