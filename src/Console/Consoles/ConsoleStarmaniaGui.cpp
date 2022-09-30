#include <pch.h>
#include "ConsoleStarmania.h"

#include <imgui.h>

#include "Gui/Assets/Colors.h"
#include "Gui/Assets/Fonts.h"

#include "Gui/ApplicationWindow/Window.h"

double map(double x, double in_min, double in_max, double out_min, double out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
};

class PongGame;

class Paddle{
public:
    enum class Type{
        LEFT_PADDLE,
        RIGHT_PADDLE
    };
    Paddle(std::shared_ptr<PongGame> g, Type t, std::shared_ptr<Joystick2X> stick) : game(g), type(t), joystick(stick){}
    
    void onGamesizeChange();
    void update();
    void draw(ImDrawList* drawing);
    
    Type type;
    
    //size in pixels
    //float widthToGamesizeRatio = 0.2;
    float sizeY;
    float sizeX = 20.0;
    
    float widthRatio;
    float widthMin = 0.1;
    float widthMax = 0.4;
    
    //values for ball collision
    float collisionPosX;
    float collisionMinY;
    float collisionMaxY;
    
    //game position
    glm::vec2 positionMin;
    glm::vec2 positionMax;
    
    //travel
    float minTravelY;
    float maxTravelY;
    
    bool b_shouldReleaseBall = false;
    glm::vec2 ballReleaseVector;
    
    std::shared_ptr<Joystick2X> joystick;
    std::shared_ptr<PongGame> game;
};

class Ball : public std::enable_shared_from_this<Ball>{
public:
    Ball(std::shared_ptr<PongGame> g) : game(g) {
        velocity = startVelocity;
        size = Random::getRanged(minSize, maxSize);
    }
    
    void onGamesizeChange();
    void update();
    void draw(ImDrawList* drawing);
    
    glm::vec2 position;
    glm::vec2 direction;
    float velocity;
    
    glm::vec2 positionMin;
    glm::vec2 positionMax;
    
    float minSize = 10.0;
    float maxSize = 50.0;
    float size;
    float startVelocity = 10.0;
    float bounceVelocityMultiplier = 1.05;
    bool b_isInsideBounds = true;
    bool b_ballbehindPaddle = false;
    
    std::shared_ptr<PongGame> game;
};

class PongGame : public std::enable_shared_from_this<PongGame>{
public:
    
    static std::shared_ptr<PongGame> make(){ return std::make_shared<PongGame>(); }
    
    enum class GameState{
        LEFT_PLAYER_BALL_IN_HAND,
        RIGHT_PLAYER_BALL_IN_HAND,
        PLAYING
    };
    
    GameState gameState = GameState::LEFT_PLAYER_BALL_IN_HAND;

    void intialize(std::shared_ptr<Joystick2X> leftStick, std::shared_ptr<Joystick2X> rightStick){
        leftPaddle = std::make_shared<Paddle>(shared_from_this(), Paddle::Type::LEFT_PADDLE, leftStick);
        rightPaddle = std::make_shared<Paddle>(shared_from_this(), Paddle::Type::RIGHT_PADDLE, rightStick);
        balls.push_back(std::make_shared<Ball>(shared_from_this()));
    }
    
    void onResize(){
        leftPaddle->onGamesizeChange();
        rightPaddle->onGamesizeChange();
    }
    
    void update(glm::vec2 min, glm::vec2 max){
        glm::vec2 newGameSize = max - min;
        if(newGameSize != gameSize){
            gameSize = newGameSize;
            onResize();
        }
        drawingMin = min;
        drawingMax = max;
        
        leftPaddle->update();
        rightPaddle->update();
        for(auto ball : balls) ball->update();
        
        for(auto ball : newBalls) balls.push_back(ball);
        newBalls.clear();
        
        //removed balls that left the screen
        for(int i = balls.size() - 1; i >= 0; i--){
            if(!balls[i]->b_isInsideBounds) balls.erase(balls.begin() + i);
        }
        
        if(balls.empty()){
            switch(lastScoringPlayer){
                case Paddle::Type::LEFT_PADDLE:
                    gameState = GameState::LEFT_PLAYER_BALL_IN_HAND;
                    break;
                case Paddle::Type::RIGHT_PADDLE:
                    gameState = GameState::RIGHT_PLAYER_BALL_IN_HAND;
                    break;
            }
            balls.push_back(std::make_shared<Ball>(shared_from_this()));
            paddleBounces = 0;
        }
        

    
        ImDrawList* drawing = ImGui::GetWindowDrawList();
        drawing->AddRectFilled(drawingMin, drawingMax, ImColor(0.f, 0.f, 0.f, 1.f));
        float middle = (drawingMin.x + drawingMax.x) * 0.5;
        drawing->AddLine(glm::vec2(middle, min.y), glm::vec2(middle, max.y), ImColor(1.f, 1.f, 1.f, 1.f), 1.f);
        
        static char leftScoreString[32];
        static char rightScoreString[32];
        sprintf(leftScoreString, "%i", scoreLeft);
        sprintf(rightScoreString, "%i", scoreRight);
    
        ImGui::PushFont(Fonts::mono42);
        glm::vec2 leftScoreSize = ImGui::CalcTextSize(leftScoreString);
        glm::vec2 rightScore = ImGui::CalcTextSize(rightScoreString);
        glm::vec2 leftScorePos(middle - leftScoreSize.x - ImGui::GetStyle().ItemSpacing.x, drawingMin.y + ImGui::GetStyle().ItemSpacing.y);
        glm::vec2 rightScorePos(middle + ImGui::GetStyle().ItemSpacing.x, drawingMin.y + ImGui::GetStyle().ItemSpacing.y);
        drawing->AddText(leftScorePos, ImColor(1.f, 1.f, 1.f, 1.f), leftScoreString);
        drawing->AddText(rightScorePos, ImColor(1.f, 1.f, 1.f, 1.f), rightScoreString);
        ImGui::PopFont();
        
        
        
        leftPaddle->draw(drawing);
        rightPaddle->draw(drawing);
        for(auto ball : balls) ball->draw(drawing);
    }
    
    glm::vec2 gameToScreen(glm::vec2 game){
        glm::vec2 screen;
        screen.x = map(game.x, 0.0, gameSize.x, drawingMin.x, drawingMax.x);
        screen.y = map(game.y, 0.0, gameSize.y, drawingMin.y, drawingMax.y);
        return screen;
    }
    
    void spawnExtraBallAt(std::shared_ptr<Ball> source){
        auto newBall = std::make_shared<Ball>(shared_from_this());
        newBall->position = source->position;
        newBall->direction.x = source->direction.x;
        newBall->direction.y = Random::getNormalized();
        newBalls.push_back(newBall);
    }
    
    void onBallPaddleBounce(std::shared_ptr<Ball> ball){
        paddleBounces++;
        Logger::warn("bounces {}", paddleBounces);
        int extraBallIncrement = 10;
        if(paddleBounces % extraBallIncrement == 0) spawnExtraBallAt(ball);
    }
    
    void onBallScore(Paddle::Type scoringPlayer){
        switch(scoringPlayer){
            case Paddle::Type::LEFT_PADDLE:
                scoreLeft++;
                break;
            case Paddle::Type::RIGHT_PADDLE:
                scoreRight++;
                break;
        }
        lastScoringPlayer = scoringPlayer;
    }
    
    glm::vec2 gameSize = glm::vec2(-1.0);
    glm::vec2 drawingMin;
    glm::vec2 drawingMax;

    std::shared_ptr<Paddle> leftPaddle;
    std::shared_ptr<Paddle> rightPaddle;
    std::vector<std::shared_ptr<Ball>> balls;
    std::vector<std::shared_ptr<Ball>> newBalls;

    int scoreLeft = 0;
    int scoreRight = 0;
    Paddle::Type lastScoringPlayer = Paddle::Type::LEFT_PADDLE;
    int paddleBounces = 0;
};








void Paddle::onGamesizeChange(){
    switch(type){
        case Type::LEFT_PADDLE:
            collisionPosX = sizeX;
            break;
        case Type::RIGHT_PADDLE:
            collisionPosX = game->gameSize.x - sizeX;
            break;
    }
}

void Paddle::update(){
        
    float scoreRatio;
    switch(type){
        case Type::LEFT_PADDLE:
            scoreRatio = float(game->scoreRight + 1) / float(game->scoreLeft + 1);
            break;
        case Type::RIGHT_PADDLE:
            scoreRatio = float(game->scoreLeft + 1) / float(game->scoreRight + 1);
            break;
    }
    
    widthRatio = map(scoreRatio, 0.5, 2, widthMin, widthMax);
    widthRatio = std::clamp(widthRatio, widthMin, widthMax);

    
    
    
    sizeY = game->gameSize.y * widthRatio;
    minTravelY = sizeY * 0.5;
    maxTravelY = game->gameSize.y - sizeY * 0.5;
    
    
    float normalizedYPosition = -joystick->getPosition().y;
    float normalizedXPosition = joystick->getPosition().x;
    collisionMinY = map(normalizedYPosition, -1.0, 1.0, minTravelY, maxTravelY) - sizeY * 0.5;
    collisionMaxY = collisionMinY + sizeY;
    
    switch(type){
        case Type::LEFT_PADDLE:
            positionMin.x = collisionPosX - sizeX;
            b_shouldReleaseBall = normalizedXPosition > 0.8;
            break;
        case Type::RIGHT_PADDLE:
            positionMin.x = collisionPosX;
            b_shouldReleaseBall = normalizedXPosition < -0.8;
            break;
    }
    positionMin.y = collisionMinY;
    
    positionMax.x = positionMin.x + sizeX;
    positionMax.y = collisionMaxY;
    
    if(b_shouldReleaseBall) ballReleaseVector = glm::vec2(normalizedXPosition, normalizedYPosition);
    else ballReleaseVector = glm::vec2(0.0);
}

void Paddle::draw(ImDrawList* drawing){
    glm::vec2 min = game->gameToScreen(positionMin);
    glm::vec2 max = game->gameToScreen(positionMax);
    drawing->AddRectFilled(min, max, ImColor(1.f, 1.f, 1.f, 1.f));
}


void Ball::onGamesizeChange(){
    if(position.x < 0.0) position.x = 0.0;
    else if(position.x > game->gameSize.x) position.x = game->gameSize.x;
    if(position.y < 0.0) position.y = 0.0;
    else if(position.y > game->gameSize.y) position.y = game->gameSize.y;
}






void Ball::update(){
    auto leftPaddle = game->leftPaddle;
    auto rightPaddle = game->rightPaddle;
    switch(game->gameState){
        case PongGame::GameState::LEFT_PLAYER_BALL_IN_HAND:
            position.y = (leftPaddle->collisionMaxY + leftPaddle->collisionMinY) * 0.5;
            position.x = leftPaddle->collisionPosX + size * 0.5;
            if(leftPaddle->b_shouldReleaseBall){
                direction = leftPaddle->ballReleaseVector;
                velocity = startVelocity;
                game->gameState = PongGame::GameState::PLAYING;
            }
            break;
        case PongGame::GameState::RIGHT_PLAYER_BALL_IN_HAND:
            position.y = (rightPaddle->collisionMaxY + rightPaddle->collisionMinY) * 0.5;
            position.x = rightPaddle->collisionPosX - size * 0.5;
            if(rightPaddle->b_shouldReleaseBall){
                direction = rightPaddle->ballReleaseVector;
                velocity = startVelocity;
                game->gameState = PongGame::GameState::PLAYING;
            }
            break;
        case PongGame::GameState::PLAYING:
            
            
            glm::vec2 velocityIncrement = direction * velocity;
            position = position + velocityIncrement;
            
            float radius = size * 0.5;
            
            if(position.y < radius) {
                position.y = 2.0 * radius - position.y;
                direction.y = std::abs(direction.y);
            }else if(position.y > game->gameSize.y - radius){
                position.y = 2.0 * game->gameSize.y - 2.0 * radius - position.y;
                direction.y = -std::abs(direction.y);
            }
            
            if(!b_ballbehindPaddle && position.x - radius < leftPaddle->collisionPosX){
                if(position.y > leftPaddle->collisionMinY - size && position.y < leftPaddle->collisionMaxY + size){
                    direction.x = 1.0;
                    direction.y = map(position.y, leftPaddle->collisionMinY, leftPaddle->collisionMaxY, -1.0, 1.0);
                    velocity *= bounceVelocityMultiplier;
                    position.x = leftPaddle->collisionPosX + radius;
                    game->onBallPaddleBounce(shared_from_this());
                }else b_ballbehindPaddle = true;
            }else if(!b_ballbehindPaddle && position.x + radius > rightPaddle->collisionPosX){
                if(position.y > rightPaddle->collisionMinY -size && position.y < rightPaddle->collisionMaxY + size){
                    direction.x = -1.0;
                    direction.y = map(position.y, rightPaddle->collisionMinY, rightPaddle->collisionMaxY, -1.0, 1.0);
                    velocity *= bounceVelocityMultiplier;
                    position.x = rightPaddle->collisionPosX - radius;
                    game->onBallPaddleBounce(shared_from_this());
                }else b_ballbehindPaddle = true;
            }else if(position.x + size < 0.0){
                b_isInsideBounds = false;
                game->onBallScore(Paddle::Type::RIGHT_PADDLE);
            }else if(position.x - size > game->gameSize.x){
                b_isInsideBounds = false;
                game->onBallScore(Paddle::Type::LEFT_PADDLE);
            }
            
            break;
    }
    
    
    positionMin = position - glm::vec2(size * 0.5);
    positionMax = position + glm::vec2(size * 0.5);
    
}

void Ball::draw(ImDrawList* drawing){
    glm::vec2 min = game->gameToScreen(positionMin);
    glm::vec2 max = game->gameToScreen(positionMax);
    glm::vec2 pos = game->gameToScreen(position);
    drawing->AddCircleFilled(pos, size * 0.5, ImColor(1.f, 0.f, 0.f, 1.0f));
}



















class PongGameWindow : public Window{
public:
    PongGameWindow(std::shared_ptr<ConsoleStarmania> c) : Window("Pong", true), console(c) {
        pongGame = PongGame::make();
        pongGame->intialize(console->joystickLeft, console->joystickRight);
    }
    
    std::shared_ptr<PongGame> pongGame;
    
    virtual void drawContent() override{
        float left = console->joystickLeft->getPosition().y;
        float right = console->joystickRight->getPosition().y;
        
        
        ImGui::InvisibleButton("Game", ImGui::GetContentRegionAvail());
        glm::vec2 min = ImGui::GetItemRectMin();
        glm::vec2 max = ImGui::GetItemRectMax();
        
        pongGame->update(min, max);

    }
    
    std::shared_ptr<ConsoleStarmania> console;
};



void ConsoleStarmania::gui(float height){
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	
	float joystickWidth = ImGui::GetTextLineHeight() * .2f;
    float joystickTipSize = ImGui::GetTextLineHeight() * .5f;
	
	auto drawJoystick = [&](std::shared_ptr<Joystick2X> joystick){
		ImGui::InvisibleButton("joystick1", glm::vec2(height));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 center = (max + min) / 2.0;
        glm::vec2 joystickEndPosition = center + joystick->getPosition() * glm::vec2(height * .4f) * glm::vec2(1.f, -1.f);
		drawing->AddRectFilled(min, max, ImColor(Colors::darkGray), ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
        drawing->AddCircleFilled(center, joystickWidth * .75f, ImColor(Colors::almostBlack));
		drawing->AddLine(center, joystickEndPosition, ImColor(Colors::almostBlack), joystickWidth);
		drawing->AddCircleFilled(joystickEndPosition, joystickTipSize * .5f, joystick->getPosition() == glm::vec2(0.f) ? ImColor(Colors::blue) : ImColor(Colors::darkRed));
	};
	
    ImGui::BeginGroup();
	drawJoystick(joystickLeft);
	ImGui::SameLine();
	drawJoystick(joystickRight);
    ImGui::EndGroup();
    
    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)){
        auto pongGameWindow = std::make_shared<PongGameWindow>(shared_from_this());
        pongGameWindow->open();
    }
    
}
