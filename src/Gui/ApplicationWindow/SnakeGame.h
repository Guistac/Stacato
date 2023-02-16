#pragma once

#include "Legato/Gui/Window.h"

#include <imgui.h>
#include <GLFW/glfw3.h>

class SnakeGameWindow : public Window{
public:
	
	SnakeGameWindow() : Window("Snake", true){}
	
	virtual void onDraw() override {
		
		
		updateSnake();
		
		glm::vec2 availableSpace = ImGui::GetContentRegionAvail();
		float gameSizeRatio = gameSize.x / gameSize.y;
		float availableSpaceRatio = availableSpace.x / availableSpace.y;
		
		glm::vec2 cursorPos = ImGui::GetCursorPos();
		glm::vec2 gameDisplaySize;
		
		if(gameSizeRatio > availableSpaceRatio){
			gameDisplaySize.x = availableSpace.x;
			gameDisplaySize.y = gameDisplaySize.x / gameSizeRatio;
			ImGui::SetCursorPosY(cursorPos.y + (availableSpace.y - gameDisplaySize.y) * 0.5);
		}else{
			gameDisplaySize.y = availableSpace.y;
			gameDisplaySize.x = gameDisplaySize.y * gameSizeRatio;
			ImGui::SetCursorPosX(cursorPos.x + (availableSpace.x - gameDisplaySize.x) * 0.5);
		}
		
		glm::vec2 pixelSize(gameDisplaySize.x / float(gameSize.x));
		
		if(gameDisplaySize.x <= 0.0 || gameDisplaySize.y <= 0.0) return;
		
		ImGui::InvisibleButton("Game", gameDisplaySize);
		
		glm::vec2 gameMin = ImGui::GetItemRectMin();
		glm::vec2 gameMax = ImGui::GetItemRectMax();
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		drawing->AddRectFilled(gameMin, gameMax, ImColor(0.f, 0.f, 0.f, 1.f));
		drawing->AddRect(gameMin, gameMax, ImColor(1.f, 1.f, 1.f, 1.f), 0.0, ImDrawFlags_RoundCornersNone, 2.f);
		
		auto drawPixel = [&](glm::ivec2 pos, ImVec4 color){
			glm::vec2 min = gameMin + glm::vec2(pos) * pixelSize;
			glm::vec2 max = min + pixelSize;
			drawing->AddRectFilled(min, max, ImColor(color));
		};
		
		for(int i = 0; i < snake.size() - 1; i++) drawPixel(snake[i], ImVec4(1.f, 1.f, 1.f, 1.f));
		drawPixel(snake[snake.size() - 1], ImVec4(1.f, 1.f, 1.f, 1.f));
		
		drawPixel(applePosition, ImVec4(0.f, 1.f, 0.f, 1.f));
		
	}
	
	enum class GameState{
		NOT_STARTED,
		DEAD,
		WAITING_FOR_START,
		RUNNING
	};
	
	enum class Direction{
		LEFT,
		RIGHT,
		UP,
		DOWN,
		STOPPED
	};
	
	//game data
	glm::ivec2 gameSize = glm::ivec2(64, 32);
	bool b_wrapEdges = false;
	GameState gameState = GameState::NOT_STARTED;
	int framesPerStep = 5;
	int frameCounter = 0;
	
	//snake data
	std::deque<glm::ivec2> snake;
	int snakeLength;
	glm::ivec2 snakePosition;
	Direction snakeDirection = Direction::STOPPED;
	
	//apple
	glm::ivec2 applePosition;
	
	//inputs
	bool b_keyDown = false;
	bool b_keyUp = false;
	bool b_keyLeft = false;
	bool b_keyRight = false;
	Direction lastDirectionKey = Direction::STOPPED;
	
	
	void respawnSnake(){
		snake.clear();
		snakeLength = 4;
		snakePosition = gameSize / 2;
		for(int i = 0; i < snakeLength; i++) snake.push_front(glm::ivec2(snakePosition.x - i, snakePosition.y));
		snakeDirection = Direction::STOPPED;
		gameState = GameState::WAITING_FOR_START;
		frameCounter = 0;
		respawnApple();
	}
	
	void respawnApple(){
		auto getRandomApplePosition = [this]() -> glm::ivec2 {
			return glm::ivec2(std::round(Random::getRanged(0, gameSize.x - 1)), std::round(Random::getRanged(0, gameSize.y - 1)));
		};
		while(true){
			glm::ivec2 newApplePosition = getRandomApplePosition();
			bool b_appleIsInsideSnake = false;
			for(int i = 0; i < snake.size(); i++){
				if(snake[i] == newApplePosition) {
					b_appleIsInsideSnake = true;
					break;
				}
			}
			if(!b_appleIsInsideSnake) {
				applePosition = newApplePosition;
				break;
			}
		}
	}
	
	void updateSnake(){
		
		//spawn snake to initialize game
		if(gameState == GameState::NOT_STARTED) respawnSnake();
		
		//get last pressed direction key, or no direction if they are all released
		if(ImGui::IsKeyDown(GLFW_KEY_UP)) {
			if(!b_keyUp) {
				lastDirectionKey = Direction::UP;
				b_keyUp = true;
			}
		}else b_keyUp = false;
		if(ImGui::IsKeyDown(GLFW_KEY_DOWN)) {
			if(!b_keyDown) {
				lastDirectionKey = Direction::DOWN;
				b_keyDown = true;
			}
		}else b_keyDown = false;
		if(ImGui::IsKeyDown(GLFW_KEY_LEFT)) {
			if(!b_keyLeft) {
				lastDirectionKey = Direction::LEFT;
				b_keyLeft = true;
			}
		}else b_keyLeft = false;
		if(ImGui::IsKeyDown(GLFW_KEY_RIGHT)) {
			if(!b_keyRight) {
				lastDirectionKey = Direction::RIGHT;
				b_keyRight = true;
			}
		}else b_keyRight = false;
		if(!b_keyUp && !b_keyDown && !b_keyLeft && !b_keyRight) lastDirectionKey = Direction::STOPPED;
		
		//check frame counter to see if the current frame makes the snake move
		frameCounter++;
		if(frameCounter % framesPerStep != 0) return;
		
		if((lastDirectionKey == Direction::UP && snakeDirection != Direction::DOWN) ||
		   (lastDirectionKey == Direction::DOWN && snakeDirection != Direction::UP) ||
		   (lastDirectionKey == Direction::LEFT && snakeDirection != Direction::RIGHT) ||
		   (lastDirectionKey == Direction::RIGHT && snakeDirection != Direction::LEFT)){
			snakeDirection = lastDirectionKey;
		}
			
		//react to user input to respawn or start game
		if(snakeDirection != Direction::STOPPED){
			if(gameState == GameState::WAITING_FOR_START) gameState = GameState::RUNNING;
			else if(gameState == GameState::DEAD) respawnSnake();
		}
		
		//get next snake head position
		glm::ivec2 newSnakePosition = snakePosition;
		switch(snakeDirection){
			case Direction::STOPPED:
				break;
			case Direction::LEFT:
				newSnakePosition.x--; break;
			case Direction::RIGHT:
				newSnakePosition.x++; break;
			case Direction::UP:
				newSnakePosition.y--; break;
			case Direction::DOWN:
				newSnakePosition.y++; break;
				break;
		}
		
		//check for collisions with game edges
		if(!b_wrapEdges && (newSnakePosition.x < 0 || newSnakePosition.x >= gameSize.x || newSnakePosition.y < 0 || newSnakePosition.y >= gameSize.y)){
			gameState = GameState::DEAD;
		}
		
		
		//check for self collisions
		for(int i = 0; i < snake.size() - 1; i++){
			if(snake[i] == newSnakePosition) {
				gameState = GameState::DEAD;
				break;
			}
		}
		
		if(gameState == GameState::DEAD) return;
		else snakePosition = newSnakePosition;
		
		if(snakePosition == applePosition){
			snakeLength++;
			respawnApple();
		}
		
		snake.push_back(newSnakePosition);
		while(snake.size() > snakeLength) snake.pop_front();
		
	}
	
	SINGLETON_GET_METHOD(SnakeGameWindow)
	
};
