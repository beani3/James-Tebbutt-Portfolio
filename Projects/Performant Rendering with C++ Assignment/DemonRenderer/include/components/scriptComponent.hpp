/** @file scriptComponent.hpp */
#pragma once 
#include "events/keyEvents.hpp"
#include <entt/entt.hpp>
#include "rendering/scene.hpp"
#include <memory>

class Actor; //!< Forward declare

class Script {
public:
	Script(entt::entity entity, std::shared_ptr<Scene> scene) : m_entity(entity), m_registry(scene->m_entities) {}
	virtual void onUpdate(float timesetep) = 0; //!< Runs on update
	virtual void onImGuiRender() {} //!< Runs on ImGuiRender
	virtual void onKeyPress(KeyPressedEvent& e) = 0; //!< Runs on key press
	virtual void onKeyRelease(KeyReleasedEvent& e) = 0; //!< Runs on key release
protected:
	entt::registry& m_registry;
	entt::entity m_entity;
};

class ScriptComp
{
public:
	ScriptComp() = default; //!< Default constructor
	/** Runs on update */
	void onUpdate(float timestep) { if (m_script) m_script->onUpdate(timestep); }
	/** Runs on ImGuiRender */
	void onImGuiRender() { if (m_script) m_script->onImGuiRender(); }
	/** Runs on key press */
	void onKeyPress(KeyPressedEvent& e) { if (m_script) m_script->onKeyPress(e); }
	/** Runs on key release */
	void onKeyRelease(KeyReleasedEvent& e) { if (m_script) m_script->onKeyRelease(e); }
	/** Method which attaches the Script subclass to the the actor */
	template <typename T, typename ...Args> void attachScript(Args&& ...args)
	{
		T* ptr = new T(std::forward<Args>(args)...);
		m_script.reset(static_cast<Script*>(ptr));
	}
private:
	std::shared_ptr<Script> m_script{ nullptr }; //!< Script subclass
};