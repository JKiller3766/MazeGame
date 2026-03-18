/*******************************************************************************************
*
*   raylib maze game
*
*   Procedural maze generator using Maze Grid Algorithm
*
*   This game has been created using raylib (www.raylib.com)
*   raylib is licensed under an unmodified zlib/libpng license (View raylib.h for details)
*
*   Copyright (c) 2024-2025 Ramon Santamaria (@raysan5)
*
********************************************************************************************/

#include "raylib.h"

#include <stdlib.h>     // Required for: malloc(), free()

#define MAZE_WIDTH          64
#define MAZE_HEIGHT         64
#define MAZE_SCALE          10.0f

#define MAX_MAZE_ITEMS      16

// Declare new data type: Point
typedef struct Point {
    int x;
    int y;
} Point;

// Generate procedural maze image, using grid-based algorithm
// NOTE: Functions defined as static are internal to the module
static Image GenImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance);
static void UpdateMinimap(Image imMaze, Point *playerCell, Texture2D *texMaze);


static int LoadItems(Image imMaze, Point *mazeItems, bool *mazeItemPicked);

//----------------------------------------------------------------------------------
// Main entry point
//----------------------------------------------------------------------------------
int main(void)
{
    // Initialization
    //---------------------------------------------------------
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(screenWidth, screenHeight, "Delivery04 - maze game");

    // Current application mode
    int currentMode = 1;    // 0-Game, 1-Editor

    // Random seed defines the random numbers generation,
    // always the same if using the same seed
    int seed = 67218;
    SetRandomSeed(seed);

    // Generate maze image using the grid-based generator
    // TODO: [1p] Implement GenImageMaze() function with required parameters
    Image imMaze = GenImageMaze(MAZE_WIDTH, MAZE_HEIGHT, 4, 4, 1.0f);    

    // Player start-position and end-position initialization
    Point startCell = { 1, 1 };
    Point endCell = { imMaze.width - 2, imMaze.height - 2 };
    ImageDrawPixel(&imMaze, endCell.x, endCell.y, GREEN);
    
    // Load a texture to be drawn on screen from our image data
    // WARNING: If imMaze pixel data is modified, texMaze needs to be re-loaded
    Texture texMaze = LoadTextureFromImage(imMaze);
    
	//Minimap
	bool cellChange = false;

    // Maze drawing position (editor mode)
    Vector2 mazePosition = {
        GetScreenWidth()/2 - texMaze.width*MAZE_SCALE/2,
        GetScreenHeight()/2 - texMaze.height*MAZE_SCALE/2
    };

    // Define player position and size
    Rectangle player = { mazePosition.x + startCell.x*128.0f, mazePosition.y + startCell.y*128.0f + 2, 6*MAZE_SCALE, 8*MAZE_SCALE };
    float playerSpeed = 400.0f;
    Point playerCell = {startCell.x, startCell.y };
    Rectangle playerBounds[4] = {0};

    // Camera 2D for 2d gameplay mode
    // TODO: [2p] Initialize camera parameters as required
    Camera2D camera2d = { 0 };
    camera2d.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera2d.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera2d.rotation = 0.0f;
    camera2d.zoom = 1.0f;

    // Mouse selected cell for maze editing
    Point selectedCell = { 0 };
    Point imagePoint = { 0 };
    
    // Maze items position and state
    Point mazeItems[MAX_MAZE_ITEMS] = { 0 };
    bool mazeItemPicked[MAX_MAZE_ITEMS] = { 0 };
    int playerScore = 0;
    int itemsCount = LoadItems(imMaze, mazeItems, mazeItemPicked); // Number of items in map

    
    // Define textures to be used as our "biomes"
    // DONE: Load additional textures for different biomes
    Texture texBiomes[4] = { 0 };
    texBiomes[0] = LoadTexture("resources/maze_atlas01.png");
    texBiomes[1] = LoadTexture("resources/maze_atlas02.png");
    texBiomes[2] = LoadTexture("resources/maze_atlas03.png");
    texBiomes[3] = LoadTexture("resources/maze_atlas04.png");
    int currentBiome = 0;
    
    
    

    // TODO: Define all variables required for game UI elements (sprites, fonts...)
    // Player sprite texture
    Texture2D playerTex = LoadTexture("resources/player_sprite.png"); 

    Rectangle playerSrcRec = { 0.0f, 0.0f, (float)playerTex.width, (float)playerTex.height };
    Vector2 playerOrigin;

    
    

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Select current mode as desired
        
        if (IsKeyPressed(KEY_R))
        {
            // DONE: Set a new seed and re-generate maze
            
            UnloadTexture(texMaze);
            UnloadImage(imMaze);
            
            seed += 1;
            SetRandomSeed(seed);
            
            Image imMaze = GenImageMaze(MAZE_WIDTH, MAZE_HEIGHT, 4, 4, 1.0f);
            ImageDrawPixel(&imMaze, endCell.x, endCell.y, GREEN);
            itemsCount = LoadItems(imMaze, mazeItems, mazeItemPicked);
            
            texMaze = LoadTextureFromImage(imMaze);
        }
        
        if (IsKeyPressed(KEY_SPACE)) currentMode = !currentMode; // Toggle mode: 0-Game, 1-Editor

        if (currentMode == 0) // Game mode
        {
            // TODO: [2p] Player 2D movement from predefined Start-point to End-point
            // Implement maze 2D player movement logic (cursors || WASD)
            // Use imMaze pixel information to check collisions
            // Detect if current playerCell == endCell to finish game
            itemsCount = LoadItems(imMaze, mazeItems, mazeItemPicked);
            Rectangle prevPlayer = player;
            
            Point prevPlayerCell = playerCell;
            if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.y -= (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.y += (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= (playerSpeed * GetFrameTime());
            if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += (playerSpeed * GetFrameTime());
            
            playerCell.x = (int)((player.x + player.width/2 - mazePosition.x)/128.0f);
            playerCell.y = (int)((player.y + player.height/2 - mazePosition.y)/128.0f);
            
            playerBounds[0] = (Rectangle){ mazePosition.x + playerCell.x*128.0f, mazePosition.y + (playerCell.y -1)*128.0f, 128.0f, 128.0f };
            playerBounds[1] = (Rectangle){ mazePosition.x + (playerCell.x -1)*128.0f, mazePosition.y + playerCell.y*128.0f, 128.0f, 128.0f };
            playerBounds[2] = (Rectangle){ mazePosition.x + playerCell.x*128.0f, mazePosition.y + (playerCell.y +1)*128.0f, 128.0f, 128.0f };
            playerBounds[3] = (Rectangle){ mazePosition.x + (playerCell.x +1)*128.0f, mazePosition.y + playerCell.y*128.0f, 128.0f, 128.0f };
            
            if ((CheckCollisionRecs(player, playerBounds[0]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x, playerCell.y - 1), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[1]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x - 1, playerCell.y), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[2]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x, playerCell.y + 1), WHITE)) ||
                (CheckCollisionRecs(player, playerBounds[3]) && ColorIsEqual(GetImageColor(imMaze, playerCell.x + 1, playerCell.y), WHITE)))
            {
                player = prevPlayer;
            }
            
            if (playerCell.x != prevPlayerCell.x || playerCell.y != prevPlayerCell.y) {
					cellChange = true;
                }
            
            if (playerCell.x == endCell.x && playerCell.y == endCell.y) {
                currentMode = !currentMode;
            }
            
            
            
            // DONE: [1p] Camera 2D system following player movement around the map
            // Update Camera2D parameters as required to follow player and zoom control

            camera2d.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };

            // TODO: [2p] Maze items pickup logic
            for (int i = 0; i < itemsCount; i++)
            {
                if (!mazeItemPicked[i] && playerCell.x == mazeItems[i].x && playerCell.y == mazeItems[i].y)
                {
                    mazeItemPicked[i] = true;
                    playerScore += 1;                   
                    ImageDrawPixel(&imMaze, mazeItems[i].x, mazeItems[i].y, BLACK);
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                }
            }
        }
        else if (currentMode == 1) // Editor mode
        {
            // DONE: [2p] Maze editor mode, edit image pixels with mouse.
            // Implement logic to selecte image cell from mouse position -> TIP: GetMousePosition()
            // NOTE: Mouse position is returned in screen coordinates and it has to 
            // transformed into image coordinates
            // Once the cell is selected, if mouse button pressed add/remove image pixels
            
            // WARNING: Remember that when imMaze changes, texMaze must be also updated!

            selectedCell = (Point) { GetMousePosition().x, GetMousePosition().y };
            
            imagePoint.x = (selectedCell.x - mazePosition.x)/MAZE_SCALE;
            imagePoint.y = (selectedCell.y - mazePosition.y)/MAZE_SCALE;
            
            if ((imagePoint.x >= 0) && (imagePoint.y >= 0) && 
               (imagePoint.x < imMaze.width) && 
               (imagePoint.y < imMaze.height))
            {
                if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && IsKeyDown(KEY_LEFT_CONTROL))
                {
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, GREEN);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                }
                else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                {
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, WHITE);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                } 
                else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
                {
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, RED);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                } 
                else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                {
                    ImageDrawPixel(&imMaze, imagePoint.x, imagePoint.y, BLACK);
                    
                    UnloadTexture(texMaze);
                    texMaze = LoadTextureFromImage(imMaze);
                } 
            }
            // TODO: [2p] Collectible map items: player score
            // Using same mechanism than maze editor, implement an items editor, registering
            // points in the map where items should be added for player pickup -> TIP: Use mazeItems[]
        }

        // DONE: [1p] Multiple maze biomes supported
        // Implement changing between the different textures to be used as biomes
        // NOTE: For the 3d model, the current selected texture must be applied to the model material  

        if (IsKeyDown(KEY_ONE)) currentBiome = 0;
        if (IsKeyDown(KEY_TWO)) currentBiome = 1;
        if (IsKeyDown(KEY_THREE)) currentBiome = 2;
        if (IsKeyDown(KEY_FOUR)) currentBiome = 3;

        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            if (currentMode == 0) // Game mode
            {
                // Draw maze using camera2d (for automatic positioning and scale)
                BeginMode2D(camera2d);
                    
                // DONE: Draw maze walls and floor using current texture biome 
                
                DrawTextureEx(texMaze, mazePosition, 0.0f, MAZE_SCALE, WHITE);
                
                for (int y = 0; y < imMaze.height; y++)
                {
                    for (int x = 0; x < imMaze.width; x++) 
                    {
                        if (ColorIsEqual(GetImageColor(imMaze, x, y), WHITE))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle){ 0, texBiomes[currentBiome].height/2, texBiomes[currentBiome].width/2, texBiomes[currentBiome].height/2 }, (Vector2) { mazePosition.x + x* texBiomes[currentBiome].width/2, mazePosition.y + y* texBiomes[currentBiome].height/2}, WHITE);
                        }
                        else if (ColorIsEqual(GetImageColor(imMaze, x, y), BLACK))
                        {
                            DrawTextureRec(texBiomes[currentBiome], (Rectangle) { texBiomes[currentBiome].width/2, texBiomes[currentBiome].height / 2,texBiomes[currentBiome].width / 2, texBiomes[currentBiome].height / 2 }, (Vector2) { mazePosition.x + x * texBiomes[currentBiome].width / 2, mazePosition.y + y * texBiomes[currentBiome].height / 2 }, WHITE);
                        }
                    }
                }
                
                // DONE: Draw player rectangle or sprite at player position
                
                Rectangle playerDestRec = { player.x + player.width/2.0f, player.y + player.height/2.0f, (float)playerTex.width * MAZE_SCALE/2.0f, (float)playerTex.height * MAZE_SCALE/2.0f };
                Vector2 playerOrigin = { (playerDestRec.width / 2.0f), (playerDestRec.height / 2.0f) };
                DrawTexturePro(playerTex, playerSrcRec, playerDestRec, playerOrigin, 0.0f, WHITE);
                // TODO: Draw maze items 2d (using sprite texture?)
                
                for (int i = 0; i < itemsCount; i++)
                {
                    if (!mazeItemPicked[i])
                    {
                        Vector2 pos = { mazePosition.x + mazeItems[i].x * 128.0f + 64.0f, 
                                        mazePosition.y + mazeItems[i].y * 128.0f + 64.0f };
                        DrawCircleV(pos, 25, RED); 
                        DrawCircleLinesV(pos, 25, MAROON);
                    }
                }

                EndMode2D();

                // TODO: Draw game UI (score, time...) using custom sprites/fonts
                // NOTE: Game UI does not receive the camera2d transformations,
                // it is drawn in screen space coordinates directly
                
                DrawText(TextFormat("SCORE: %i", playerScore), 10, 130, 30, GOLD); // Item pickup score
                
                DrawText("GAME MODE", 10, 40, 20, BLACK);
                
				if (cellChange) {
					ImageDrawPixel(&imMaze, playerCell.x, playerCell.y, BLUE);
					UnloadTexture(texMaze);
					texMaze = LoadTextureFromImage(imMaze);
					ImageDrawPixel(&imMaze, playerCell.x, playerCell.y, BLACK);
                    cellChange = false;
				}
                
                DrawTextureEx(texMaze, (Vector2){ GetRenderWidth() -  200,  10}, 0.0f, 3, WHITE); //Minimap
                
            }
            else if (currentMode == 1) // Editor mode
            {
                // Draw generated maze texture, scaled and centered on screen 
                DrawTextureEx(texMaze, mazePosition, 0.0f, MAZE_SCALE, WHITE);

                if((imagePoint.x >= 0) && (imagePoint.y >= 0) && 
                   (imagePoint.x < imMaze.width) && 
                   (imagePoint.y < imMaze.height))
                {
                    DrawRectangleLines(mazePosition.x+imagePoint.x*MAZE_SCALE, 
                    mazePosition.y+imagePoint.y*MAZE_SCALE, MAZE_SCALE, MAZE_SCALE, GREEN);
                }
                
                DrawText("EDIT MODE", 10, 40, 20, BLACK);
                
                DrawText(TextFormat("MOUSE: [%i, %i]", selectedCell.x, selectedCell.y), 10, 70, 20, DARKBLUE);
                DrawText(TextFormat("IMAGE: [%i, %i]", imagePoint.x, imagePoint.y), 10, 100, 20, RED);

                // Draw lines rectangle over texture, scaled and centered on screen 
                DrawRectangleLines(mazePosition.x, mazePosition.y, MAZE_WIDTH*MAZE_SCALE, MAZE_HEIGHT*MAZE_SCALE, RED);

                // DONE: Draw player using a rectangle, consider maze screen coordinates!

                DrawRectangleRec((Rectangle){ (player.x - mazePosition.x) * (MAZE_SCALE / 128.0f) + mazePosition.x , (player.y - mazePosition.y) * (MAZE_SCALE / 128.0f) + mazePosition.y, player.width/MAZE_SCALE , player.height/MAZE_SCALE } , BLUE);
    
                
    
                // TODO: Draw editor UI required elements
            }

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(texMaze);     // Unload maze texture from VRAM (GPU)
    UnloadImage(imMaze);        // Unload maze image from RAM (CPU)
    UnloadTexture(playerTex); // Unload player texture
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}



// Generate procedural maze image, using grid-based algorithm
// NOTE: Color scheme used: WHITE = Wall, BLACK = Walkable, RED = Item
static Image GenImageMaze(int width, int height, int spacingRows, int spacingCols, float pointChance)
{
    // DONE: [1p] Implement maze image generation algorithm
    
    Image imMaze = GenImageColor(width, height, BLACK);
    
    Point *mazePoints = malloc(MAZE_HEIGHT*MAZE_WIDTH*sizeof(Point));
    
    int mazePointCounter = 0;
    
    for (int y = 0; y < imMaze.height; y++)
    {
        for (int x = 0; x < imMaze.width; x++)
        {
            if ((x == 0) || (y == 0) || (x == (imMaze.width - 1)) || (y == (imMaze.height - 1))) 
            {
                ImageDrawPixel(&imMaze, x, y, WHITE);
            }
            else 
            {
                if ((x%spacingRows == 0) && (y%spacingCols == 0)) 
                {
                    if (GetRandomValue(0, 100)<pointChance*100)
                    {
                        ImageDrawPixel(&imMaze, x, y, WHITE);
                        mazePoints[mazePointCounter] = (Point) { x, y }; // Inicializa los valores: x = x , y = y
                        mazePointCounter++;
                    }
                }
            }
        }
    }
    
    Point dirIncrements[4] = {
        {0,-1},
        {1,0},
        {0,1},
        {-1,0}
    };
    
    int *indexes = LoadRandomSequence(mazePointCounter, 0, mazePointCounter - 1);
    
    for (int i = 0; i < mazePointCounter; i++)
    {
        int dir = GetRandomValue(0,3);
        Point nextMazePoint = mazePoints[indexes[i]];
        nextMazePoint.x += dirIncrements[dir].x;
        nextMazePoint.y += dirIncrements[dir].y;
        
        while (ColorIsEqual(GetImageColor(imMaze, nextMazePoint.x, nextMazePoint.y), BLACK))
        {    
            ImageDrawPixel(&imMaze, nextMazePoint.x, nextMazePoint.y, WHITE);
            
            nextMazePoint.x += dirIncrements[dir].x;
            nextMazePoint.y += dirIncrements[dir].y;
        }
    }
    int nItems = 0;
    for (int y = 1; y < imMaze.height - 1; y++)
    {
        for (int x = 1; x < imMaze.width - 1; x++)
        {
            if (ColorIsEqual(GetImageColor(imMaze, x, y), BLACK))
            {
                if ((GetRandomValue(0, 1000) < 5 ) && (nItems < MAX_MAZE_ITEMS)) 
                {
                    ImageDrawPixel(&imMaze, x, y, RED);
                    
                }
            }
        }
    }

    UnloadRandomSequence(indexes);
    return imMaze;
}

// Scan map image to identify RED squares
static int LoadItems(Image imMaze, Point *mazeItems, bool *mazeItemPicked)
{
    int count = 0;
    
    for (int y = 0; y < imMaze.height; y++)
    {
        for (int x = 0; x < imMaze.width; x++)
        {
            if (ColorIsEqual(GetImageColor(imMaze, x, y), RED))
            {
                if (count < MAX_MAZE_ITEMS)
                {
                    mazeItems[count] = (Point){ x, y };
                    mazeItemPicked[count] = false;
                    count++;
                }
            }
        }
    }
    return count; // Returns number of items detected
}

