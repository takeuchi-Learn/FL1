﻿#pragma once
#include <Camera/Camera.h>
#include <GameObject/AbstractGameObj.h>
#include <Imu/Sensor.h>
#include <3D/Billboard/Billboard.h>
#include <DirectXMath.h>

// 後々の追従を想定してObjのやつ
// CameraObjだと上ベクトルの制御が不可能になるからこちらで追従機能を追加したほうがいいかも

/// @brief Z軸回転と追従機能だけ持ったカメラクラス
class GameCamera :
	public Camera
{
private:
	template <class T>
	struct MinMax
	{
		T min;
		T max;
	};

	MinMax<DirectX::XMFLOAT2> stickRangeL{};
	MinMax<DirectX::XMFLOAT2> stickRangeR{};
	float stickAngleShiftVal = 2.f;

public:
	// todo カメラが持つべきでない。（カメラを構成する要素ではないため。）switch文ではなくstateパターンやビヘイビアツリーを使うこと。
	// カメラ内部の要素と、カメラを操作する者の状態は、まったくの別物。これは後者。
	/// @brief カメラの状態列挙
	enum class CameraState
	{
		START,// 開始
		INPUT,// 入力受付
		FOLLOW_OFF,// 入力受付 追従オフ(画面端用)
		CLEAR,// クリア クリア時に演出でカメラを制御する必要がありそうなので追加
		OTHER, //その他(何も更新しないとき)
	};

	struct AxisVec
	{
		/// @brief 右
		float right = 0.f;
		/// @brief 天
		float up = 0.f;
		/// @brief 奥
		float forward = 0.f;
	};
	struct RollPitchYaw
	{
		/// @brief 奥行き軸回転
		float roll = 0.f;
		/// @brief 左右軸回転
		float pitch = 0.f;
		/// @brief 天地軸回転
		float yaw = 0.f;
	};

private:
	// todo センサー周りもカメラが持つべきでない。形式を指定して、外部クラスでその形式に変換して扱う方がよい。

	// todo new演算子は「絶対に」使ってはならない

	// todo カルマンフィルターとMadgwickフィルターで使用しているライブラリがGPLなので、使用不可。変更必須。

	// 回転の角速度
	RollPitchYaw gyro{};
	// 前回の角速度
	RollPitchYaw prevGyro{};

	float accelAngle = 0.0f;

	/// @brief 加速度
	AxisVec accel{};
	// 角度Z(最初に斜めの状態で開始するため、20,fをセット)
	float angleDeg = 20.f;

	BillboardData* obj = nullptr;

	// todo カメラが持つべきでない（カメラの状態は、カメラの構成要素ではない。）
	// カメラの状態
	CameraState cameraState = CameraState::START;

	// todo pragma reginで分けられるならクラスごと分けること
#pragma region START
	// todo 時間（time）ではなくフレーム（frame、count）

	// スタートするまでの時間
	// todo uint16_tとするべき
	unsigned short startTimer = 0;

	// todo 変数名が嘘をついている（タイマーとフラグは別の概念）
	// 上記関数の起動フラグ
	bool addStartTimer = false;

	// 1フレームの傾き(スタート時)。動的に変えるため、変数にしている
	float startFrameAngle = 1.5f;

	// todo 不適切な命名＆フラグを使うべきではない。最低でもstd::function等で振る舞いを変更する。
	// スタート演出の補間処理
	bool startLerp = false;

	// スティックを使用するかどうか
	bool isActiveStickControll = false;

	float angleFilterRaito{};
	float angleStopRange{};

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
	void rotation();
	void imuInputRotation();
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
	void followObject(bool followX);

	/// @brief 更新(元々のupdateと被らないように名前長くしてる)
	void gameCameraUpdate();

public:
	/// @brief コンストラクタ
	/// @param obj プレイヤーのポインタ(追従させるために渡す)
	GameCamera(BillboardData* obj = nullptr);

	bool loadYaml();

	inline bool getIsActiveStickControll() const { return isActiveStickControll; }

	/// @brief ジャイロの値のセット。
	void setGyroValue(float value) { angleDeg = value; }

	/// @brief 追従先オブジェクト
	/// @param obj
	void setParentObj(BillboardData* obj) { this->obj = obj; }

	/// @brief Z座標
	/// @param z
	void setEyeZ(float z) { setEye(DirectX::XMFLOAT3(0, 0, z)); }

	/// @brief 角度の取得
	/// @return 角度
	float getAngleDeg() const { return angleDeg; }

	/// @param flag ポーズしてるかどうか
	void pause(bool flag)
	{
		cameraState = flag ? CameraState::OTHER : CameraState::INPUT;
	}

	// クリア状態に変更
	void changeStateClear() { cameraState = CameraState::CLEAR; }
	void changeStateGameover() { cameraState = CameraState::OTHER; }

	/// @brief 追従のオンオフ
	void setFollowFlag(const bool flag);

	float getGetAccelUp() const { return accel.up; }

	CameraState getCameraState() const { return cameraState; }

	inline const auto& getGyro() const { return gyro; }
	inline const auto& getAccel() const { return accel; }
};
