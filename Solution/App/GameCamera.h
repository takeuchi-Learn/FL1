#pragma once
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
	// 回転の角速度
	RollPitchYaw gyro{};
	// 前回の角速度
	RollPitchYaw prevGyro{};

	float accelAngle = 0.0f;

	/// @brief 加速度
	AxisVec accel{};

	float dInputAngleDeg = 0.f;
	// 角度Z(最初に斜めの状態で開始するため、20,fをセット)
	float angleDeg = 20.f;

	BillboardData* obj = nullptr;

	// todo カメラが持つべきでない（カメラの状態は、カメラの構成要素ではない。）
	// カメラの状態
	CameraState cameraState = CameraState::INPUT;

	// スティックを使用するかどうか
	bool isActiveStickControll = false;

	float angleFilterRaito{};
	float angleStopRange{};

public:
	/// @brief 入力を許可するか
	/// todo 仮実装。操作を別クラスに分けたら消す
	bool allowInput = true;

private:
	// todo pragma reginで分けられるならクラスごと分けること
#pragma region INPUT

	/// @brief CameraState::INPUTの時のupdate
	void updateInput();

	/// @brief 入力確認とそれに応じた角度の加算減算
	void rotation();
	void directionalInputRotation();
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
	inline void setGyroValue(float value) { angleDeg = value; }

	/// @brief 追従先オブジェクト
	/// @param obj
	inline void setParentObj(BillboardData* obj) { this->obj = obj; }

	/// @brief Z座標
	/// @param z
	inline void setEyeZ(float z) { setEye(DirectX::XMFLOAT3(0, 0, z)); }

	/// @brief 角度の取得
	/// @return 角度
	inline float getAngleDeg() const { return angleDeg; }
	inline void setAngleDeg(float deg) { angleDeg = deg; }

	/// @param flag ポーズしてるかどうか
	inline void pause(bool flag)
	{
		cameraState = flag ? CameraState::OTHER : CameraState::INPUT;
	}

	// クリア状態に変更
	inline void changeStateClear() { cameraState = CameraState::CLEAR; }
	inline void changeStateGameover() { cameraState = CameraState::OTHER; }

	/// @brief 追従のオンオフ
	void setFollowFlag(const bool flag);

	inline float getGetAccelUp() const { return accel.up; }

	inline CameraState getCameraState() const { return cameraState; }

	inline const auto& getGyro() const { return gyro; }
	inline const auto& getAccel() const { return accel; }
};
