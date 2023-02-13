#include "CanvasLibrary.h"

#include <ofxLua.h>
#include <imgui.h>
#include <imgui_internal.h>

namespace Scripting::CanvasLibrary{

#define DECLARE_LUA_FUNCTION(L, functionName)\
	lua_pushcfunction(L, functionName);\
	lua_setfield(L, -2, #functionName);\
	
	int getWidth(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getWidth(), expected 0");
		lua_pushnumber(L, ImGui::GetItemRectSize().x);
		return 1;
	}

	int getHeight(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getHeight(), expected 0");
		lua_pushnumber(L, ImGui::GetItemRectSize().y);
		return 1;
	}

	int getSize(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getSize(), expected 0");
		ImVec2 size = ImGui::GetItemRectSize();
		lua_pushnumber(L, size.x);
		lua_pushnumber(L, size.y);
		return 2;
	}



	int isHovered(lua_State* L){
		lua_pushboolean(L, ImGui::IsItemHovered());
		return 1;
	}

	int isClicked(lua_State* L){
		int size = lua_gettop(L);
		if(size == 0) lua_pushboolean(L, ImGui::IsItemClicked());
		if(size == 1){
			int button = luaL_checkinteger(L, -1);
			luaL_argcheck(L, button >= 0 && button < 5, -1, "Mouse Button ID out of range, should be 0 -> 4");
			lua_pushboolean(L, ImGui::IsMouseClicked(button));
		}
		else if(size > 1) luaL_error(L, "Too many arguments provided to Canvas.isClicked(), expected 1 or 0");
		return 1;
	}

	int isReleased(lua_State* L){
		int size = lua_gettop(L);
		if(size == 0) lua_pushboolean(L, ImGui::IsItemHovered() && ImGui::IsMouseReleased(ImGuiMouseButton_Left));
		if(size == 1){
			int button = luaL_checkinteger(L, -1);
			luaL_argcheck(L, button >= 0 && button < 5, -1, "Mouse Button ID out of range, should be 0 -> 4");
			lua_pushboolean(L, ImGui::IsItemHovered() && ImGui::IsMouseReleased(button));
		}
		else if(size > 1) luaL_error(L, "Too many arguments provided to Canvas.isReleased(), expected 1 or 0");
		return 1;
	}

	int isPressed(lua_State* L){
		int size = lua_gettop(L);
		if(size == 0) lua_pushboolean(L, ImGui::IsItemActive());
		if(size == 1){
			int button = luaL_checkinteger(L, -1);
			luaL_argcheck(L, button >= 0 && button < 5, -1, "Mouse Button ID out of range, should be 0 -> 4");
			lua_pushboolean(L, ImGui::IsItemHovered() && ImGui::IsMouseDown(button));
		}
		else if(size > 1) luaL_error(L, "Too many arguments provided to Canvas.isPressed(), expected 1 or 0");
		return 1;
		return 1;
	}

	int isDragged(lua_State* L){
		int size = lua_gettop(L);
		if(size == 0) lua_pushboolean(L, ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left));
		if(size == 1){
			int button = luaL_checkinteger(L, -1);
			luaL_argcheck(L, button >= 0 && button < 5, -1, "Mouse Button ID out of range, should be 0 -> 4");
			lua_pushboolean(L, ImGui::IsItemHovered() && ImGui::IsMouseDragging(button));
		}
		else if(size > 1) luaL_error(L, "Too many arguments provided to Canvas.isDragged(), expected 1 or 0");
		return 1;
	}

	int isDragReleased(lua_State* L){
		int size = lua_gettop(L);
		if(size == 0) {
			bool dragged = false;
			if(ImGui::IsMouseReleased(ImGuiMouseButton_Left) && ImGui::GetItemID() == ImGui::GetCurrentContext()->ActiveIdPreviousFrame){
				ImVec2 delta = ImGui::GetMouseDragDelta();
				dragged = delta.x > 0.0 || delta.y > 0.0;
			}
			lua_pushboolean(L, dragged);
		}
		if(size == 1){
			int button = luaL_checkinteger(L, -1);
			luaL_argcheck(L, button >= 0 && button < 5, -1, "Mouse Button ID out of range, should be 0 -> 4");
			bool dragged = false;
			if(ImGui::IsMouseReleased(button) && ImGui::IsItemHovered()){
				ImVec2 delta = ImGui::GetMouseDragDelta();
				dragged = delta.x > 0.0 || delta.y > 0.0;
			}
			lua_pushboolean(L, dragged);
		}
		else if(size > 1) luaL_error(L, "Too many arguments provided to Canvas.isDragged(), expected 1 or 0");
		return 1;
	}



	int getMousePosition(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getPosition(), expected 0");
		ImVec2 posMin = ImGui::GetItemRectMin();
		ImVec2 mouseAbsolute = ImGui::GetMousePos();
		ImVec2 pos(mouseAbsolute.x - posMin.x, mouseAbsolute.y - posMin.y);
		lua_pushnumber(L, pos.x);
		lua_pushnumber(L, pos.y);
		return 2;
	}

	int getDragStartPosition(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getDragStartPosition(), expected 0");
		ImVec2 mouse = ImGui::GetMousePos();
		ImVec2 dragDelta = ImGui::GetMouseDragDelta();
		lua_pushnumber(L, mouse.x - dragDelta.x);
		lua_pushnumber(L, mouse.y - dragDelta.y);
		return 2;
	}

	int getDragDelta(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getDragDelta(), expected 0");
		ImVec2 dragDelta = ImGui::GetMouseDragDelta();
		lua_pushnumber(L, dragDelta.x);
		lua_pushnumber(L, dragDelta.y);
		return 2;
	}

	int getScrollDelta(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getScrollDelta(), expected 0");
		lua_pushnumber(L, ImGui::GetIO().MouseWheelH);
		lua_pushnumber(L, ImGui::GetIO().MouseWheel);
		return 2;
	}

	int getZoomDelta(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getZoomDelta(), expected 0");
		//lua_pushnumber(L, ApplicationWindow::getMacOsTrackpadZoom());
		return 1;
	}

	int getRotationDelta(lua_State* L){
		if(lua_gettop(L) > 0) luaL_error(L, "Too many arguments provided to Canvas.getRotationDelta(), expected 0");
		//lua_pushnumber(L, ApplicationWindow::getMacOsTrackpadRotate());
		return 1;
	}


	void openLib(lua_State* L){
		
		lua_newtable(L);
		DECLARE_LUA_FUNCTION(L, getWidth)
		DECLARE_LUA_FUNCTION(L, getHeight)
		DECLARE_LUA_FUNCTION(L, getSize)
		DECLARE_LUA_FUNCTION(L, isHovered)
		DECLARE_LUA_FUNCTION(L, isClicked)
		DECLARE_LUA_FUNCTION(L, isReleased)
		DECLARE_LUA_FUNCTION(L, isPressed)
		DECLARE_LUA_FUNCTION(L, isDragged)
		DECLARE_LUA_FUNCTION(L, isDragReleased)
		DECLARE_LUA_FUNCTION(L, getMousePosition)
		DECLARE_LUA_FUNCTION(L, getDragStartPosition)
		DECLARE_LUA_FUNCTION(L, getDragDelta)
		DECLARE_LUA_FUNCTION(L, getScrollDelta)
		DECLARE_LUA_FUNCTION(L, getZoomDelta)
		DECLARE_LUA_FUNCTION(L, getRotationDelta)
		lua_setglobal(L, "Canvas");
		
	}

};
