#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <DirectXMath.h>

#include <3D/Obj/Object3d.h>

class AbstractGameObj
{
protected:
	size_t ppStateNum{};

	std::unique_ptr<Object3d> obj;

	std::unordered_map<std::string, std::shared_ptr<AbstractGameObj>> otherObj;

	bool alive = true;

	std::unordered_map<std::string, std::function<void()>> postUpdateProc;
	std::unordered_map<std::string, std::function<void(Light*)>> postDrawProc;

public:
	AbstractGameObj(Camera* camera,
					ObjModel* model);
	AbstractGameObj(Camera* camera) : AbstractGameObj(camera, nullptr) {}

	~AbstractGameObj();

	virtual void update() final;
	virtual void draw(Light* light) final;

	inline virtual void drawWithUpdate(Light* light) final
	{
		update();
		draw(light);
	}

#pragma region アクセッサ

	inline void setPipelineStateNum(size_t _num) { ppStateNum = _num; }
	inline size_t getPipelineStateNum() const { return ppStateNum; }

	inline Object3d* getObj() const { return obj.get(); }

	inline BaseObj* getParent() { return obj->parent; }
	inline void setParent(BaseObj* _parent) { obj->parent = _parent; }

	inline bool isAlive() const { return alive; }
	inline void setAlive(bool _alive) { alive = _alive; }
	/// @brief aliveをfalseにする
	inline void kill() { alive = false; }

	inline bool isVisible() const { return obj->visible; }
	inline void setIsVisible(bool _isVisible) { obj->visible = _isVisible; }

	inline void setPosition(const DirectX::XMFLOAT3& _pos) { obj->position = _pos; }
	inline const DirectX::XMFLOAT3& getPosition() const { return obj->position; }

	inline void setColor(const DirectX::XMFLOAT4& _color) { obj->color = _color; }
	inline const DirectX::XMFLOAT4& getColor() const { return obj->color; }

	inline void setScale(const DirectX::XMFLOAT3& _scale) { obj->scale = _scale; }
	inline void setScale(float _scale) { obj->scale = DirectX::XMFLOAT3(_scale, _scale, _scale); }
	inline void setScaleX(float _scaleX) { obj->scale.x = _scaleX; }
	inline void setScaleY(float _scaleY) { obj->scale.y = _scaleY; }
	inline void setScaleZ(float _scaleZ) { obj->scale.z = _scaleZ; }
	inline const DirectX::XMFLOAT3& getScale() const { return obj->scale; }

	inline void setRotation(const DirectX::XMFLOAT3& _rotation) { obj->rotation = _rotation; }
	inline void setRotationX(float _rotationX) { obj->rotation.x = _rotationX; }
	inline void setRotationY(float _rotationY) { obj->rotation.y = _rotationY; }
	inline void setRotationZ(float _rotationZ) { obj->rotation.z = _rotationZ; }
	inline const DirectX::XMFLOAT3& getRotation() const { return obj->rotation; }

#pragma endregion アクセッサ

#pragma region その他機能

	inline static DirectX::XMFLOAT2 calcRotationSyncVelRad(const DirectX::XMFLOAT3& vel)
	{
		return DirectX::XMFLOAT2(std::atan2(-vel.y,
											std::sqrt(vel.x * vel.x +
													  vel.z * vel.z)),
								 std::atan2(vel.x, vel.z));
	}
	inline static DirectX::XMFLOAT2 calcRotationSyncVelDeg(const DirectX::XMFLOAT3& vel)
	{
		const DirectX::XMFLOAT2 rad = calcRotationSyncVelRad(vel);
		return DirectX::XMFLOAT2(DirectX::XMConvertToDegrees(rad.x), DirectX::XMConvertToDegrees(rad.y));
	}

	/// @return 大きさベクトルの長さを返す
	inline float calcScale() const
	{
		return std::sqrt(obj->scale.x * obj->scale.x +
						 obj->scale.y * obj->scale.y +
						 obj->scale.z * obj->scale.z);
	}

	static DirectX::XMFLOAT3 calcVel(const DirectX::XMFLOAT3& targetPos,
									 const DirectX::XMFLOAT3& nowPos,
									 float velScale);
#pragma endregion その他機能
};
