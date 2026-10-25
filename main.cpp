#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <algorithm>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Car.h"

enum GameState {
    STATE_SPLASH,
    STATE_MENU,
    STATE_GAME,
    STATE_CREDITS,
    STATE_SETTINGS
};

struct GameSettings {
    bool soundEnabled = true;
    bool musicEnabled = true;
    bool showFPS = false; 
    bool crtEffect = false;
    int difficulty = 1; 
};

struct Button {
    SDL_Rect rect;
    bool isClicked(int mouseX, int mouseY) {
        return (mouseX >= rect.x && mouseX <= rect.x + rect.w &&
                mouseY >= rect.y && mouseY <= rect.y + rect.h);
    }
};

void RenderText(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, int x, int y, bool center = true) {
    if (!font) return;
    SDL_Surface* surface = TTF_RenderUTF8_Blended(font, text.c_str(), color);
    if (!surface) return;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_Rect destRect;
    if (center) {
        destRect = { x - surface->w / 2, y - surface->h / 2, surface->w, surface->h };
    } else {
        destRect = { x, y - surface->h / 2, surface->w, surface->h };
    }
    
    SDL_RenderCopy(renderer, texture, NULL, &destRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void RenderTextWithOutline(SDL_Renderer* renderer, TTF_Font* font, const std::string& text, SDL_Color color, SDL_Color outlineColor, int x, int y, int outlineSize, bool center = true) {
    if (!font) return;
    RenderText(renderer, font, text, outlineColor, x - outlineSize, y, center);
    RenderText(renderer, font, text, outlineColor, x + outlineSize, y, center);
    RenderText(renderer, font, text, outlineColor, x, y - outlineSize, center);
    RenderText(renderer, font, text, outlineColor, x, y + outlineSize, center);
    
    RenderText(renderer, font, text, outlineColor, x - outlineSize, y - outlineSize, center);
    RenderText(renderer, font, text, outlineColor, x + outlineSize, y - outlineSize, center);
    RenderText(renderer, font, text, outlineColor, x - outlineSize, y + outlineSize, center);
    RenderText(renderer, font, text, outlineColor, x + outlineSize, y + outlineSize, center);

    RenderText(renderer, font, text, color, x, y, center);
}

void DrawPremiumCheckbox(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, bool checked, int x, int y, int w, int h) {
    SDL_Rect shadowRect = { x + 5, y + 5, w, h };
    SDL_SetRenderDrawColor(renderer, 20, 10, 10, 150);
    SDL_RenderFillRect(renderer, &shadowRect);

    SDL_Rect btnRect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, 45, 45, 50, 255);
    SDL_RenderFillRect(renderer, &btnRect);

    SDL_SetRenderDrawColor(renderer, 100, 100, 110, 255);
    SDL_RenderDrawRect(renderer, &btnRect);

    int boxSize = 24;
    int boxX = x + 15;
    int boxY = y + h / 2; 
    SDL_Rect checkSquare = { boxX, boxY - boxSize / 2, boxSize, boxSize };
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &checkSquare);
    SDL_SetRenderDrawColor(renderer, 150, 150, 150, 255);
    SDL_RenderDrawRect(renderer, &checkSquare);

    if (checked) {
        SDL_Rect checkFill = { boxX + 4, checkSquare.y + 4, boxSize - 8, boxSize - 8 };
        SDL_SetRenderDrawColor(renderer, 57, 255, 20, 255); 
        SDL_RenderFillRect(renderer, &checkFill);
    }

    SDL_Color white = { 255, 255, 255, 255 };
    RenderText(renderer, font, label, white, boxX + boxSize + 15, boxY - 2, false);
}

void DrawPremiumCycle(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const std::string& value, int x, int y, int w, int h) {
    SDL_Rect shadowRect = { x + 5, y + 5, w, h };
    SDL_SetRenderDrawColor(renderer, 20, 10, 10, 150);
    SDL_RenderFillRect(renderer, &shadowRect);

    SDL_Rect btnRect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, 45, 45, 50, 255);
    SDL_RenderFillRect(renderer, &btnRect);

    SDL_SetRenderDrawColor(renderer, 100, 100, 110, 255);
    SDL_RenderDrawRect(renderer, &btnRect);

    SDL_Color white = { 255, 255, 255, 255 };
    SDL_Color yellow = { 255, 215, 0, 255 };
    
    int textY = y + h / 2 - 2;
    RenderText(renderer, font, label + ":", white, x + 15, textY, false);
    RenderText(renderer, font, "< " + value + " >", yellow, x + 150, textY, false); 
}

void DrawPremiumButton(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, int x, int y, int w, int h) {
    SDL_Rect shadowRect = { x + 5, y + 5, w, h };
    SDL_SetRenderDrawColor(renderer, 20, 10, 10, 150);
    SDL_RenderFillRect(renderer, &shadowRect);

    SDL_Rect btnRect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, 255, 180, 0, 255); 
    SDL_RenderFillRect(renderer, &btnRect);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &btnRect);

    SDL_Color black = { 10, 10, 10, 255 };
    RenderText(renderer, font, label, black, x + w / 2, y + h / 2 - 2, true);
}

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER) < 0) return -1;
    if (TTF_Init() < 0) return -1;

    SDL_Window* window = SDL_CreateWindow("Kitty Racer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    stbi_set_flip_vertically_on_load(0);

    GameSettings gameSettings;

    TTF_Font* fontTitle = TTF_OpenFont("font.ttf", 64);
    TTF_Font* fontBody  = TTF_OpenFont("font.ttf", 26);
    TTF_Font* fontFPS   = TTF_OpenFont("font.ttf", 28); 

    SDL_Joystick* ps3Controller = NULL;
    if (SDL_NumJoysticks() > 0) {
        ps3Controller = SDL_JoystickOpen(0);
    }

    int splashW, splashH, splashC;
    unsigned char* splashData = stbi_load("kittystudios.jpg", &splashW, &splashH, &splashC, STBI_rgb_alpha);
    SDL_Surface* splashSurface = SDL_CreateRGBSurfaceWithFormatFrom(splashData, splashW, splashH, 32, splashW * 4, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* splashTexture = SDL_CreateTextureFromSurface(renderer, splashSurface);
    SDL_FreeSurface(splashSurface); stbi_image_free(splashData);

    int bgW, bgH, bgC;
    unsigned char* bgData = stbi_load("bg.png", &bgW, &bgH, &bgC, STBI_rgb_alpha);
    SDL_Surface* bgSurface = SDL_CreateRGBSurfaceWithFormatFrom(bgData, bgW, bgH, 32, bgW * 4, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* bgTexture = SDL_CreateTextureFromSurface(renderer, bgSurface);
    SDL_FreeSurface(bgSurface); stbi_image_free(bgData);

    int bannerW, bannerH, bannerC;
    unsigned char* bannerData = stbi_load("banner.png", &bannerW, &bannerH, &bannerC, STBI_rgb_alpha);
    SDL_Surface* bannerSurface = SDL_CreateRGBSurfaceWithFormatFrom(bannerData, bannerW, bannerH, 32, bannerW * 4, SDL_PIXELFORMAT_RGBA32);
    Uint32 bannerColorkey = SDL_MapRGB(bannerSurface->format, 255, 255, 255);
    SDL_SetColorKey(bannerSurface, SDL_TRUE, bannerColorkey);
    SDL_Texture* bannerTexture = SDL_CreateTextureFromSurface(renderer, bannerSurface);
    SDL_FreeSurface(bannerSurface); stbi_image_free(bannerData);

    int startW, startH, startC;
    unsigned char* startData = stbi_load("start.png", &startW, &startH, &startC, STBI_rgb_alpha);
    SDL_Surface* startSurface = SDL_CreateRGBSurfaceWithFormatFrom(startData, startW, startH, 32, startW * 4, SDL_PIXELFORMAT_RGBA32);
    Uint32 startColorkey = SDL_MapRGB(startSurface->format, 255, 255, 255);
    SDL_SetColorKey(startSurface, SDL_TRUE, startColorkey);
    SDL_Texture* startTexture = SDL_CreateTextureFromSurface(renderer, startSurface);
    SDL_FreeSurface(startSurface); stbi_image_free(startData);

    int creditsW, creditsH, creditsC;
    unsigned char* creditsData = stbi_load("credits.png", &creditsW, &creditsH, &creditsC, STBI_rgb_alpha);
    SDL_Surface* creditsSurface = SDL_CreateRGBSurfaceWithFormatFrom(creditsData, creditsW, creditsH, 32, creditsW * 4, SDL_PIXELFORMAT_RGBA32);
    Uint32 creditsColorkey = SDL_MapRGB(creditsSurface->format, 255, 255, 255);
    SDL_SetColorKey(creditsSurface, SDL_TRUE, creditsColorkey);
    SDL_Texture* creditsTexture = SDL_CreateTextureFromSurface(renderer, creditsSurface);
    SDL_FreeSurface(creditsSurface); stbi_image_free(creditsData);

    int settingsW, settingsH, settingsC;
    unsigned char* settingsData = stbi_load("settings.png", &settingsW, &settingsH, &settingsC, STBI_rgb_alpha);
    SDL_Surface* settingsSurface = SDL_CreateRGBSurfaceWithFormatFrom(settingsData, settingsW, settingsH, 32, settingsW * 4, SDL_PIXELFORMAT_RGBA32);
    Uint32 settingsColorkey = SDL_MapRGB(settingsSurface->format, 255, 255, 255);
    SDL_SetColorKey(settingsSurface, SDL_TRUE, settingsColorkey);
    SDL_Texture* settingsTexture = SDL_CreateTextureFromSurface(renderer, settingsSurface);
    SDL_FreeSurface(settingsSurface); stbi_image_free(settingsData);

    int creditsImgW, creditsImgH, creditsImgC;
    unsigned char* creditsImgData = stbi_load("credits.jpg", &creditsImgW, &creditsImgH, &creditsImgC, STBI_rgb_alpha);
    SDL_Surface* creditsImgSurface = SDL_CreateRGBSurfaceWithFormatFrom(creditsImgData, creditsImgW, creditsImgH, 32, creditsImgW * 4, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* creditsImgTexture = SDL_CreateTextureFromSurface(renderer, creditsImgSurface);
    SDL_FreeSurface(creditsImgSurface); stbi_image_free(creditsImgData);

    int bg1W, bg1H, bg1C;
    unsigned char* bg1Data = stbi_load("bg1.jpg", &bg1W, &bg1H, &bg1C, STBI_rgb_alpha);
    SDL_Surface* bg1Surface = SDL_CreateRGBSurfaceWithFormatFrom(bg1Data, bg1W, bg1H, 32, bg1W * 4, SDL_PIXELFORMAT_RGBA32);
    SDL_Texture* bg1Texture = SDL_CreateTextureFromSurface(renderer, bg1Surface);
    SDL_FreeSurface(bg1Surface); stbi_image_free(bg1Data);

    int carW, carH, carC;
    unsigned char* carData = stbi_load("car.png", &carW, &carH, &carC, STBI_rgb_alpha);
    SDL_Texture* carTexture = nullptr;
    if (carData) {
        SDL_Surface* carSurface = SDL_CreateRGBSurfaceWithFormatFrom(carData, carW, carH, 32, carW * 4, SDL_PIXELFORMAT_RGBA32);
        carTexture = SDL_CreateTextureFromSurface(renderer, carSurface);
        SDL_FreeSurface(carSurface);
        stbi_image_free(carData);
    }

    Car playerCar;
    playerCar.init(carTexture, 180, 530, 0.20f); 

    Uint32 startTime = SDL_GetTicks();
    const Uint32 fadeStartTime = 3000;
    const Uint32 fadeEndTime   = 4000;
    const Uint32 menuStartTime = 5000;

    float bgX = 0.0f;
    float scrollSpeed = 200.0f;

    GameState currentState = STATE_SPLASH;

    Uint32 lastTime = SDL_GetTicks();
    Uint32 fpsTimer = SDL_GetTicks();
    int frameCounter = 0;
    int realFPS = 0;

    bool running = true;
    SDL_Event event;

    while (running) {
        Uint32 currentTime = SDL_GetTicks();
        float deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        frameCounter++;
        if (currentTime - fpsTimer >= 1000) {
            realFPS = frameCounter;
            frameCounter = 0;
            fpsTimer = currentTime;
        }

        int winW, winH;
        SDL_GetRendererOutputSize(renderer, &winW, &winH);

        int btnWidth = 460;
        int btnHeight = (int)(btnWidth * ((float)startH / startW));
        int btnX = winW - btnWidth - 20;

        int startY = (int)(winH * 0.60f);
        Button btnStart = { { btnX, startY, btnWidth, btnHeight } };
        Button btnCredits = { { btnX, startY - btnHeight + 50, btnWidth, btnHeight } };
        Button btnSettings = { { btnX, startY - 2 * btnHeight + 100, btnWidth, btnHeight } };

        int opW = 420;
        int opH = 50;
        int opX = winW / 2 - opW / 2;
        int gap = 60;
        int startListY = 220; 

        Button chkSound   = { { opX, startListY, opW, opH } };
        Button chkMusic   = { { opX, startListY + gap, opW, opH } };
        Button btnDiff    = { { opX, startListY + gap * 2, opW, opH } };
        Button chkCRT     = { { opX, startListY + gap * 3, opW, opH } };
        Button chkFPS     = { { opX, startListY + gap * 4, opW, opH } };
        Button btnBack    = { { opX, startListY + gap * 5 + 10, opW, opH } };

        if (currentState == STATE_SPLASH && (currentTime - startTime) >= menuStartTime) {
            currentState = STATE_MENU;
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;

            if (currentState == STATE_GAME) {
                playerCar.handleInput(event, winH);
            }

            if (event.type == SDL_JOYBUTTONDOWN) {
                if (event.jbutton.button == 0 || event.jbutton.button == 14 || event.jbutton.button == 2) {
                    if (currentState == STATE_MENU) currentState = STATE_GAME;
                    else if (currentState == STATE_CREDITS) currentState = STATE_MENU;
                }
                if (event.jbutton.button == 1 || event.jbutton.button == 13 || event.jbutton.button == 3) {
                    if (currentState == STATE_SETTINGS || currentState == STATE_CREDITS) currentState = STATE_MENU;
                }
            }

            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
                    if (currentState == STATE_MENU) currentState = STATE_GAME;
                    else if (currentState == STATE_CREDITS) currentState = STATE_MENU;
                }
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    if (currentState == STATE_SETTINGS || currentState == STATE_CREDITS) currentState = STATE_MENU;
                }
            }

            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;

                if (currentState == STATE_MENU) {
                    if (btnStart.isClicked(mouseX, mouseY)) currentState = STATE_GAME;
                    else if (btnCredits.isClicked(mouseX, mouseY)) currentState = STATE_CREDITS;
                    else if (btnSettings.isClicked(mouseX, mouseY)) currentState = STATE_SETTINGS;
                }
                else if (currentState == STATE_SETTINGS) {
                    if (chkSound.isClicked(mouseX, mouseY)) gameSettings.soundEnabled = !gameSettings.soundEnabled;
                    else if (chkMusic.isClicked(mouseX, mouseY)) gameSettings.musicEnabled = !gameSettings.musicEnabled;
                    else if (chkFPS.isClicked(mouseX, mouseY)) gameSettings.showFPS = !gameSettings.showFPS;
                    else if (chkCRT.isClicked(mouseX, mouseY)) gameSettings.crtEffect = !gameSettings.crtEffect;
                    else if (btnDiff.isClicked(mouseX, mouseY)) gameSettings.difficulty = (gameSettings.difficulty + 1) % 3;
                    else if (btnBack.isClicked(mouseX, mouseY)) currentState = STATE_MENU;
                }
                else if (currentState == STATE_CREDITS) {
                    currentState = STATE_MENU;
                }
            }
        }

        SDL_Rect fullScreenRect = { 0, 0, winW, winH };
        Uint32 elapsedTime = currentTime - startTime;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        if (currentState == STATE_SPLASH) {
            if (elapsedTime < fadeEndTime) {
                float scale = std::min((float)winW / splashW, (float)winH / splashH);
                int destW = (int)(splashW * scale);
                int destH = (int)(splashH * scale);
                int destX = (winW - destW) / 2;
                int destY = (winH - destH) / 2;
                SDL_Rect splashRect = { destX, destY, destW, destH };
                SDL_RenderCopy(renderer, splashTexture, NULL, &splashRect);

                if (elapsedTime > fadeStartTime) {
                    float fadeProgress = (float)(elapsedTime - fadeStartTime) / (fadeEndTime - fadeStartTime);
                    Uint8 alpha = (Uint8)(std::min(fadeProgress, 1.0f) * 255);
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, alpha);
                    SDL_RenderFillRect(renderer, &fullScreenRect);
                }
            }
            else if (elapsedTime < menuStartTime) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderFillRect(renderer, &fullScreenRect);
            }
        } 
        else if (currentState == STATE_MENU) {
            bgX -= scrollSpeed * deltaTime;
            if (bgX <= -winW) bgX += winW;
            SDL_Rect bgRect1 = { (int)bgX, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect1);
            SDL_Rect bgRect2 = { (int)bgX + winW, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect2);

            int bX = 60, bY = -60, bWidth = 800; 
            int bHeight = (int)(bWidth * ((float)bannerH / bannerW));
            SDL_Rect bannerRect = { bX, bY, bWidth, bHeight };
            SDL_RenderCopy(renderer, bannerTexture, NULL, &bannerRect);

            if (startTexture) SDL_RenderCopy(renderer, startTexture, NULL, &btnStart.rect);
            if (creditsTexture) SDL_RenderCopy(renderer, creditsTexture, NULL, &btnCredits.rect);
            if (settingsTexture) SDL_RenderCopy(renderer, settingsTexture, NULL, &btnSettings.rect);
        } 
        else if (currentState == STATE_GAME) {
            bgX -= (scrollSpeed * 1.5f) * deltaTime;
            if (bgX <= -winW) bgX += winW;
            SDL_Rect bgRect1 = { (int)bgX, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect1);
            SDL_Rect bgRect2 = { (int)bgX + winW, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &bgRect2);

            playerCar.update(deltaTime);
            playerCar.render(renderer);
        }
        else if (currentState == STATE_CREDITS) {
            SDL_Rect staticBgRect = { 0, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &staticBgRect);

            if (creditsImgTexture) {
                float scale = std::min((float)winW / creditsImgW, (float)winH / creditsImgH);
                int destW = (int)(creditsImgW * scale);
                int destH = (int)(creditsImgH * scale);
                int destX = (winW - destW) / 2;
                int destY = (winH - destH) / 2;
                SDL_Rect creditsImgRect = { destX, destY, destW, destH };
                SDL_RenderCopy(renderer, creditsImgTexture, NULL, &creditsImgRect);
            }
        }
        else if (currentState == STATE_SETTINGS) {
            SDL_Rect staticBgRect = { 0, 0, winW, winH };
            SDL_RenderCopy(renderer, bgTexture, NULL, &staticBgRect);

            if (bg1Texture) {
                float scale = std::min((float)winW / bg1W, (float)winH / bg1H);
                int destW = (int)(bg1W * scale);
                int destH = (int)(bg1H * scale);
                int destX = (winW - destW) / 2;
                int destY = (winH - destH) / 2;
                SDL_Rect bg1Rect = { destX, destY, destW, destH };
                SDL_RenderCopy(renderer, bg1Texture, NULL, &bg1Rect);

                SDL_Color redText = { 240, 40, 60, 255 };
                SDL_Color whiteOutline = { 255, 255, 255, 255 };
                RenderTextWithOutline(renderer, fontTitle, "SETTINGS", redText, whiteOutline, winW / 2, destY + 120, 3, true);

                DrawPremiumCheckbox(renderer, fontBody, "Sound FX", gameSettings.soundEnabled, opX, startListY, opW, opH);
                DrawPremiumCheckbox(renderer, fontBody, "Music", gameSettings.musicEnabled, opX, startListY + gap, opW, opH);
                
                std::string diffStr = (gameSettings.difficulty == 0) ? "EASY" : (gameSettings.difficulty == 1) ? "NORMAL" : "HARD";
                DrawPremiumCycle(renderer, fontBody, "Difficulty", diffStr, opX, startListY + gap * 2, opW, opH);
                
                DrawPremiumCheckbox(renderer, fontBody, "CRT Scanlines", gameSettings.crtEffect, opX, startListY + gap * 3, opW, opH);
                DrawPremiumCheckbox(renderer, fontBody, "Show FPS", gameSettings.showFPS, opX, startListY + gap * 4, opW, opH);
                
                DrawPremiumButton(renderer, fontBody, "BACK TO MENU", opX, startListY + gap * 5 + 10, opW, opH);
            }
        }

        if (gameSettings.crtEffect) {
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 60); 
            for (int i = 0; i < winH; i += 4) {
                SDL_RenderDrawLine(renderer, 0, i, winW, i);
            }
        }

        if (gameSettings.showFPS) {
            SDL_Color white = { 255, 255, 255, 255 };
            SDL_Color black = { 0, 0, 0, 255 }; 
            RenderTextWithOutline(renderer, fontFPS, "FPS: " + std::to_string(realFPS), white, black, 20, 20, 2, false);
        }

        SDL_RenderPresent(renderer);
    }

    if (carTexture) SDL_DestroyTexture(carTexture);
    if (fontTitle) TTF_CloseFont(fontTitle);
    if (fontBody) TTF_CloseFont(fontBody);
    if (fontFPS) TTF_CloseFont(fontFPS);
    TTF_Quit();

    if (ps3Controller) SDL_JoystickClose(ps3Controller);
    if (bg1Texture) SDL_DestroyTexture(bg1Texture);
    if (settingsTexture) SDL_DestroyTexture(settingsTexture);
    if (creditsImgTexture) SDL_DestroyTexture(creditsImgTexture);
    if (creditsTexture) SDL_DestroyTexture(creditsTexture);
    if (startTexture) SDL_DestroyTexture(startTexture);
    SDL_DestroyTexture(splashTexture);
    SDL_DestroyTexture(bgTexture);
    SDL_DestroyTexture(bannerTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
