#pragma once
#include <SDL2/SDL.h>

struct Car {
    SDL_Texture* texture = nullptr;
    int x;
    float y;
    int origWidth, origHeight; 
    int width, height;         
    float scale = 1.0f;        
    
    int currentLane = 0;       // 0: Üst Şerit, 1: Alt Şerit
    float targetY;

    int laneY[2] = { 0, 0 }; 

    void init(SDL_Texture* tex, int startX, int lane1Y, int lane2Y, float customScale = 0.20f) {
        texture = tex;
        scale = customScale;
        x = startX; 
        
        laneY[0] = lane1Y;
        laneY[1] = lane2Y;

        if (texture) {
            Uint32 format;
            int access;
            SDL_QueryTexture(texture, &format, &access, &origWidth, &origHeight);
        } else {
            origWidth = 100;
            origHeight = 100;
        }

        width = (int)(origWidth * scale);
        height = (int)(origHeight * scale);

        targetY = laneY[currentLane] - (height / 2);
        y = targetY; 
    }

    void switchLane() {
        currentLane = 1 - currentLane; 
        targetY = laneY[currentLane] - (height / 2);
    }

    void handleInput(SDL_Event& event, int winH) {
        if (event.type == SDL_KEYDOWN || event.type == SDL_JOYBUTTONDOWN || event.type == SDL_MOUSEBUTTONDOWN) {
            switchLane();
        }
    }

    void update(float deltaTime) {
        y += (targetY - y) * 6.0f * deltaTime;
    }

    void render(SDL_Renderer* renderer) {
        if (texture) {
            SDL_Rect dest = { x, (int)y, width, height };
            SDL_RenderCopy(renderer, texture, NULL, &dest);
        }
    }

    // Çarpışmalar için arka planda hitbox hesaplamaya devam eder ama ekranda çizgi göstermez
    SDL_Rect getHitbox() {
        return { x + 5, (int)y + 5, width - 10, height - 10 };
    }
};
