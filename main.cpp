#include <iostream>
#include <string>
#include <stdio.h>
#include "SDL.h"
#include "SDL_image.h"
#include <cmath>
//-----------------
#undef main
//-----------------

// global variables
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define PADDLE_WIDTH 50
#define PADDLE_HEIGHT SCREEN_HEIGHT/3
#define BALL_SIZE 10
SDL_Renderer* renderer;
SDL_Window* window;
bool quit = false;
SDL_Event e;
bool bPressUp{false};
bool bPressDown{false};
const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
float paddleSpeed = 300.f;
Uint32 lastTime = 0;
Uint32 currentTime = 0;
float deltatime = 0;
float ballMovementAngle = 45.f; 
int ballSpeed = 700;   
bool isPaused = false;
Uint32 pauseTime = 0;


SDL_Rect leftPaddle{0, SCREEN_HEIGHT/3, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect rightPaddle{SCREEN_WIDTH - PADDLE_WIDTH, SCREEN_HEIGHT/3, PADDLE_WIDTH, PADDLE_HEIGHT };
SDL_Rect ball{SCREEN_WIDTH/2, SCREEN_HEIGHT/2, BALL_SIZE, BALL_SIZE };





// global functions
bool Init()
{
   bool success = true;

   if(SDL_Init(SDL_INIT_EVERYTHING) < 0 )
   {
      std::cerr << "SDL could not initialized!" << SDL_GetError() << std::endl;
      success = false;
   } 
   if(IMG_Init(IMG_INIT_PNG) < 0)
   {
      std::cerr << "SDL_image could not initialized!" << IMG_GetError() << std::endl;
      success = false;
   } 

   window = SDL_CreateWindow("Pong Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
   if(window == nullptr)
   {
      std::cerr << "window not created!" << SDL_GetError() << std::endl;
      success = false;
   } 

   renderer = SDL_CreateRenderer(window, -1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
   if(renderer == nullptr)
   {
      std::cerr << "renderer not created!" << SDL_GetError() << std::endl;
      success = false;
   } 
   return success;
}

void CalculateDeltaTime()
{
   currentTime = SDL_GetTicks();
   deltatime = (currentTime - lastTime) / 1000.f;// Convert to seconds
   lastTime = currentTime;
}

void Render()
{
   //render background and make it black 
   SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
   SDL_RenderClear(renderer);

   //render paddles and make them white 
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &leftPaddle);
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &rightPaddle);
   SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
   SDL_RenderFillRect(renderer, &ball);

   // update Render
   SDL_RenderPresent(renderer);
}

void Update(float deltaSeconds)
{
   if (currentKeyStates[SDL_SCANCODE_W])
   {
      if(leftPaddle.y > 0) leftPaddle.y -= paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_S])
   {
      if(leftPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) leftPaddle.y += paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_UP])
   {
      if(rightPaddle.y > 0) rightPaddle.y -= paddleSpeed * deltaSeconds;
   }
   if (currentKeyStates[SDL_SCANCODE_DOWN])
   {
      if(rightPaddle.y < SCREEN_HEIGHT - PADDLE_HEIGHT) rightPaddle.y += paddleSpeed * deltaSeconds;
   }
}



void ResetBall()
{
   ball.x = SCREEN_WIDTH/2;
   ball.y = SCREEN_HEIGHT/2;
   ballMovementAngle = 45.0f; 
   isPaused = true;
   pauseTime = SDL_GetTicks();
}
void MoveBall(float deltaSeconds)
{
   if (isPaused)
   {
      if (SDL_GetTicks() - pauseTime > 2000)
      {
         isPaused = false;
      }
      return;
   }

    // Convert angle to radians
    float angleRadians = ballMovementAngle * M_PI / 180.0f;
    
    // Update ball position
    ball.x += static_cast<int>(ballSpeed * deltaSeconds * std::cos(angleRadians));
    ball.y += static_cast<int>(ballSpeed * deltaSeconds * std::sin(angleRadians));

    // Handle collision with left and right walls
    if (ball.x >= (SCREEN_WIDTH - BALL_SIZE) || ball.x <= 0)
    {
      ResetBall();
      return;
    }

    // Handle collision with top and bottom walls
    if (ball.y >= (SCREEN_HEIGHT - BALL_SIZE) || ball.y <= 0)
    {
        ballMovementAngle = -ballMovementAngle;
        if (ball.y >= (SCREEN_HEIGHT - BALL_SIZE)) ball.y = SCREEN_HEIGHT - BALL_SIZE - 1;
        if (ball.y <= 0) ball.y = 1;
    }

    // Handle collision with right paddle
    if(SDL_HasIntersection(&ball, & rightPaddle))
    {
      ballMovementAngle = 180.0f - ballMovementAngle;
      ball.x = rightPaddle.x - rightPaddle.w;
    }

   // Handle collision with left  paddle
    if(SDL_HasIntersection(&ball, & leftPaddle))
    {
      ballMovementAngle = 180.0f - ballMovementAngle;
      ball.x = leftPaddle.x + leftPaddle.w;
    }
}

void Close()
{
   SDL_DestroyRenderer(renderer);
   SDL_DestroyWindow(window);
   renderer = nullptr;
   window = nullptr;
   SDL_Quit();
   IMG_Quit();
}

int main(int argc, char **argv)
{

   if(!Init())
   {
      std::cerr << "something is wrong in initializing stuff in Init() function !\n";
   }
   else
   {
      lastTime = SDL_GetTicks();// Start time

      while (!quit)
      {
         CalculateDeltaTime();
         while (SDL_PollEvent(&e) != 0)
         {
            if (e.type == SDL_QUIT)
            {
               quit = true;
            }
         }
         Update(deltatime);
         MoveBall(deltatime);
         Render();

         // Simulate slower frame rate
         //SDL_Delay(100); //Add 100ms delay per frame
      }
   }
   Close();
   return 0 ;  
}






