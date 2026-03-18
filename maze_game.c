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
static Image GenImageMaze(int width, int height, int spacing_rows, int spacing_cols, float point_chance);
static void UpdateMinimap(Image im_maze, Point *player_cell, Texture2D *tex_maze);
static void EndScene(int score);

static int LoadItems(Image im_maze, Point *maze_items, bool *maze_item_picked);

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
    
    InitAudioDevice();

    // Current application mode
    int current_mode = 1;    // 0-Game, 1-Editor

    // Random seed defines the random numbers generation,
    // always the same if using the same seed
    int seed = 67218;
    SetRandomSeed(seed);

    // Generate maze image using the grid-based generator
    // TODO: [1p] Implement GenImageMaze() function with required parameters
    Image im_maze = GenImageMaze(MAZE_WIDTH, MAZE_HEIGHT, 4, 4, 1.0f);    

    // Player start-position and end-position initialization
    Point start_cell = { 1, 1 };
    Point end_cell = { im_maze.width - 2, im_maze.height - 2 };
    ImageDrawPixel(&im_maze, end_cell.x, end_cell.y, GREEN);
    
    // Load a texture to be drawn on screen from our image data
    // WARNING: If im_maze pixel data is modified, tex_maze needs to be re-loaded
    Texture tex_maze = LoadTextureFromImage(im_maze);
    
	//Minimap
	bool cell_change = false;

    // Maze drawing position (editor mode)
    Vector2 maze_position = {
        GetScreenWidth()/2 - tex_maze.width*MAZE_SCALE/2,
        GetScreenHeight()/2 - tex_maze.height*MAZE_SCALE/2
    };

    // Define player position and size
    Rectangle player = { maze_position.x + start_cell.x*128.0f, maze_position.y + start_cell.y*128.0f + 2, 6*MAZE_SCALE, 8*MAZE_SCALE };
    float player_speed = 400.0f;
    Point player_cell = {start_cell.x, start_cell.y };
    Rectangle player_bounds[4] = {0};

    // Camera 2D for 2d gameplay mode
    // TODO: [2p] Initialize camera parameters as required
    Camera2D camera2d = { 0 };
    camera2d.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };
    camera2d.offset = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    camera2d.rotation = 0.0f;
    camera2d.zoom = 1.0f;

    // Mouse selected cell for maze editing
    Point selected_cell = { 0 };
    Point image_point = { 0 };
    
    // Maze items position and state
    Point maze_items[MAX_MAZE_ITEMS] = { 0 };
    bool maze_item_picked[MAX_MAZE_ITEMS] = { 0 };
    int player_score = 0;
    int items_count = LoadItems(im_maze, maze_items, maze_item_picked); // Number of items in map

    
    // Define textures to be used as our "biomes"
    // TODO: Load additional textures for different biomes
    Texture tex_biomes[5] = { 0 };
    tex_biomes[0] = LoadTexture("resources/maze_atlas01.png");
    tex_biomes[1] = LoadTexture("resources/maze_atlas02.png");
    tex_biomes[2] = LoadTexture("resources/maze_atlas03.png");
    tex_biomes[3] = LoadTexture("resources/maze_atlas04.png");
    tex_biomes[4] = LoadTexture("resources/maze_atlas05.png");
    int currentBiome = 0;
    

    // TODO: Define all variables required for game UI elements (sprites, fonts...)
    Texture2D player_tex = LoadTexture("resources/player_sprite.png"); 
    Texture item_tex = LoadTexture("resources/item_sprite.png");
    Rectangle player_src_rec = { 0.0f, 0.0f, (float)player_tex.width, (float)player_tex.height };
    Vector2 player_origin = {0,0};
    
    
    // Sound variables definition
    Music music = LoadMusicStream("resources/soft_music.ogg");
    music.looping = true;
    SetMusicVolume(music, 0.5f);
    PlayMusicStream(music);
    
    Sound item_sfx = LoadSound("resources/coin_sfx.wav");
    SetSoundVolume(item_sfx, 0.6f);
    
    bool ended = false;

    SetTargetFPS(60);       // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Select current mode as desired
        UpdateMusicStream(music);
        if (!ended)
        {
            if (IsKeyPressed(KEY_R))
            {
                // TODO: Set a new seed and re-generate maze
                UnloadTexture(tex_maze);
                UnloadImage(im_maze);
                
                seed += 1;
                SetRandomSeed(seed);
                
                Image im_maze = GenImageMaze(MAZE_WIDTH, MAZE_HEIGHT, 4, 4, 1.0f);
                ImageDrawPixel(&im_maze, end_cell.x, end_cell.y, GREEN);
                items_count = LoadItems(im_maze, maze_items, maze_item_picked);
                
                tex_maze = LoadTextureFromImage(im_maze);
            }
            
            if (IsKeyPressed(KEY_SPACE)) current_mode = !current_mode; // Toggle mode: 0-Game, 1-Editor

            if (current_mode == 0) // Game mode
            {
                // TODO: [2p] Player 2D movement from predefined Start-point to End-point
                // Implement maze 2D player movement logic (cursors || WASD)
                // Use im_maze pixel information to check collisions
                // Detect if current player_cell == end_cell to finish game
                items_count = LoadItems(im_maze, maze_items, maze_item_picked);
                Rectangle prev_player = player;
                
                Point prev_player_cell = player_cell;
                if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) player.y -= (player_speed * GetFrameTime());
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) player.y += (player_speed * GetFrameTime());
                if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) player.x -= (player_speed * GetFrameTime());
                if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) player.x += (player_speed * GetFrameTime());
                
                player_cell.x = (int)((player.x + player.width/2 - maze_position.x)/128.0f);
                player_cell.y = (int)((player.y + player.height/2 - maze_position.y)/128.0f);
                
                player_bounds[0] = (Rectangle){ maze_position.x + player_cell.x*128.0f, maze_position.y + (player_cell.y -1)*128.0f, 128.0f, 128.0f };
                player_bounds[1] = (Rectangle){ maze_position.x + (player_cell.x -1)*128.0f, maze_position.y + player_cell.y*128.0f, 128.0f, 128.0f };
                player_bounds[2] = (Rectangle){ maze_position.x + player_cell.x*128.0f, maze_position.y + (player_cell.y +1)*128.0f, 128.0f, 128.0f };
                player_bounds[3] = (Rectangle){ maze_position.x + (player_cell.x +1)*128.0f, maze_position.y + player_cell.y*128.0f, 128.0f, 128.0f };
                
                if ((CheckCollisionRecs(player, player_bounds[0]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x, player_cell.y - 1), WHITE)) ||
                    (CheckCollisionRecs(player, player_bounds[1]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x - 1, player_cell.y), WHITE)) ||
                    (CheckCollisionRecs(player, player_bounds[2]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x, player_cell.y + 1), WHITE)) ||
                    (CheckCollisionRecs(player, player_bounds[3]) && ColorIsEqual(GetImageColor(im_maze, player_cell.x + 1, player_cell.y), WHITE)))
                {
                    player = prev_player;
                }
                
                if (player_cell.x != prev_player_cell.x || player_cell.y != prev_player_cell.y) {
                        cell_change = true;
                    }
                
                if (player_cell.x == end_cell.x && player_cell.y == end_cell.y) {
                    ended = true;
                }
                
                // TODO: [1p] Camera 2D system following player movement around the map
                // Update Camera2D parameters as required to follow player and zoom control
                camera2d.target = (Vector2){ player.x + 20.0f, player.y + 20.0f };

                // TODO: [2p] Maze items pickup logic
                for (int i = 0; i < items_count; i++)
                {
                    if (!maze_item_picked[i] && player_cell.x == maze_items[i].x && player_cell.y == maze_items[i].y)
                    {
                        maze_item_picked[i] = true;
                        player_score += 1;                   
                        ImageDrawPixel(&im_maze, maze_items[i].x, maze_items[i].y, BLACK);
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                        PlaySound(item_sfx);
                    }
                }
            }
            else if (current_mode == 1) // Editor mode
            {
                // TODO: [2p] Maze editor mode, edit image pixels with mouse.
                // Implement logic to selecte image cell from mouse position -> TIP: GetMousePosition()
                // NOTE: Mouse position is returned in screen coordinates and it has to 
                // transformed into image coordinates
                // Once the cell is selected, if mouse button pressed add/remove image pixels
                
                // WARNING: Remember that when im_maze changes, tex_maze must be also updated!
                selected_cell = (Point) { GetMousePosition().x, GetMousePosition().y };
                
                image_point.x = (selected_cell.x - maze_position.x)/MAZE_SCALE;
                image_point.y = (selected_cell.y - maze_position.y)/MAZE_SCALE;
                
                if ((image_point.x >= 0) && (image_point.y >= 0) && 
                   (image_point.x < im_maze.width) && 
                   (image_point.y < im_maze.height))
                {
                    if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && IsKeyDown(KEY_LEFT_CONTROL))
                    {
                        ImageDrawPixel(&im_maze, end_cell.x, end_cell.y, BLACK);
                        ImageDrawPixel(&im_maze, image_point.x, image_point.y, GREEN);
                        end_cell.x = image_point.x;
                        end_cell.y = image_point.y;
                        
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    }
                    else if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
                    {
                        ImageDrawPixel(&im_maze, image_point.x, image_point.y, WHITE);
                        
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    } 
                    else if (IsMouseButtonDown(MOUSE_MIDDLE_BUTTON))
                    {
                        ImageDrawPixel(&im_maze, image_point.x, image_point.y, RED);
                        
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    } 
                    else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
                    {
                        ImageDrawPixel(&im_maze, image_point.x, image_point.y, BLACK);
                        
                        UnloadTexture(tex_maze);
                        tex_maze = LoadTextureFromImage(im_maze);
                    } 
                }
                // TODO: [2p] Collectible map items: player score
                // Using same mechanism than maze editor, implement an items editor, registering
                // points in the map where items should be added for player pickup -> TIP: Use maze_items[]
            }

            // TODO: [1p] Multiple maze biomes supported
            // Implement changing between the different textures to be used as biomes
            // NOTE: For the 3d model, the current selected texture must be applied to the model material  
            if (IsKeyDown(KEY_ONE)) currentBiome = 0;
            if (IsKeyDown(KEY_TWO)) currentBiome = 1;
            if (IsKeyDown(KEY_THREE)) currentBiome = 2;
            if (IsKeyDown(KEY_FOUR)) currentBiome = 3;
            if (IsKeyDown(KEY_FIVE)) currentBiome = 4;


            //----------------------------------------------------------------------------------

            // Draw
            //----------------------------------------------------------------------------------
            BeginDrawing();

                ClearBackground(RAYWHITE);

                   if (current_mode == 0) // Game mode
                    {
                        // Draw maze using camera2d (for automatic positioning and scale)
                        BeginMode2D(camera2d);
                            
                        // TODO: Draw maze walls and floor using current texture biome 
                        DrawTextureEx(tex_maze, maze_position, 0.0f, MAZE_SCALE, WHITE);
                        
                        for (int y = 0; y < im_maze.height; y++)
                        {
                            for (int x = 0; x < im_maze.width; x++) 
                            {
                                if (ColorIsEqual(GetImageColor(im_maze, x, y), WHITE))
                                {
                                    DrawTextureRec(tex_biomes[currentBiome], (Rectangle){ 0, tex_biomes[currentBiome].height/2, tex_biomes[currentBiome].width/2, tex_biomes[currentBiome].height/2 }, (Vector2) { maze_position.x + x* tex_biomes[currentBiome].width/2, maze_position.y + y* tex_biomes[currentBiome].height/2}, WHITE);
                                }
                                else if (ColorIsEqual(GetImageColor(im_maze, x, y), GREEN)) 
                                {
                                    DrawTextureRec(tex_biomes[currentBiome], (Rectangle){ tex_biomes[currentBiome].width/2, 0, tex_biomes[currentBiome].width/2, tex_biomes[currentBiome].height/2 }, (Vector2) { maze_position.x + x* tex_biomes[currentBiome].width/2, maze_position.y + y* tex_biomes[currentBiome].height/2}, WHITE);
                                }
                                else
                                {
                                    DrawTextureRec(tex_biomes[currentBiome], (Rectangle) { tex_biomes[currentBiome].width/2, tex_biomes[currentBiome].height / 2,tex_biomes[currentBiome].width / 2, tex_biomes[currentBiome].height / 2 }, (Vector2) { maze_position.x + x * tex_biomes[currentBiome].width / 2, maze_position.y + y * tex_biomes[currentBiome].height / 2 }, WHITE);
                                }
                            }
                        }
                        
                        // TODO: Draw player rectangle or sprite at player position
                        Rectangle player_dest_rec = { player.x + player.width/2.0f, player.y + player.height/2.0f, (float)player_tex.width * MAZE_SCALE/2.0f, (float)player_tex.height * MAZE_SCALE/2.0f };
                        Vector2 player_origin = { (player_dest_rec.width / 2.0f), (player_dest_rec.height / 2.0f) };
                        DrawTexturePro(player_tex, player_src_rec, player_dest_rec, player_origin, 0.0f, WHITE);
                        
                        // TODO: Draw maze items 2d (using sprite texture?)
                        for (int i = 0; i < items_count; i++)
                        {
                            if (!maze_item_picked[i])
                            {
                                float center_x = maze_position.x + (maze_items[i].x * tex_biomes[currentBiome].width/2 + tex_biomes[currentBiome].width/4 ); 
                                float center_y = maze_position.y + (maze_items[i].y * tex_biomes[currentBiome].height/2 + tex_biomes[currentBiome].height/4);
                                
                                Rectangle destRec = { center_x, center_y, (float)item_tex.width*4, (float)item_tex.height*4};
                                Rectangle srcRec = { 0, 0, (float)item_tex.width, (float)item_tex.height };
                                Vector2 currentItemOrigin = { destRec.width / 2.0f, destRec.height / 2.0f };
                                
                                DrawTexturePro(item_tex, srcRec, destRec, currentItemOrigin, 0.0f, WHITE);
                            }
                        }

                        EndMode2D();

                        // TODO: Draw game UI (score, time...) using custom sprites/fonts
                        // NOTE: Game UI does not receive the camera2d transformations,
                        // it is drawn in screen space coordinates directly
                        DrawText(TextFormat("SCORE: %i", player_score), 10, 40, 40, GOLD); // Item pickup score
                        
                        DrawText("GAME MODE", 10, GetScreenHeight() - 90, 20, BLACK);
                        
                        if (cell_change) {
                            ImageDrawPixel(&im_maze, player_cell.x, player_cell.y, BLUE);
                            UnloadTexture(tex_maze);
                            tex_maze = LoadTextureFromImage(im_maze);
                            ImageDrawPixel(&im_maze, player_cell.x, player_cell.y, BLACK);
                            cell_change = false;
                        }
                        DrawTextureEx(tex_maze, (Vector2){ GetRenderWidth() -  200,  10}, 0.0f, 3, WHITE); //Minimap
                    }
                    else if (current_mode == 1) // Editor mode
                    {
                        // Draw generated maze texture, scaled and centered on screen 
                        DrawTextureEx(tex_maze, maze_position, 0.0f, MAZE_SCALE, WHITE);
                    
                        if((image_point.x >= 0) && (image_point.y >= 0) && 
                           (image_point.x < im_maze.width) && 
                           (image_point.y < im_maze.height))
                        {
                            DrawRectangleLines(maze_position.x+image_point.x*MAZE_SCALE, 
                            maze_position.y+image_point.y*MAZE_SCALE, MAZE_SCALE, MAZE_SCALE, GREEN);
                        }

                        // Draw lines rectangle over texture, scaled and centered on screen 
                        DrawRectangleLines(maze_position.x, maze_position.y, MAZE_WIDTH*MAZE_SCALE, MAZE_HEIGHT*MAZE_SCALE, RED);

                        // TODO: Draw player using a rectangle, consider maze screen coordinates!
                        
                        DrawRectangleRec((Rectangle){ (player.x - maze_position.x) * (MAZE_SCALE / 128.0f) + maze_position.x , (player.y - maze_position.y) * (MAZE_SCALE / 128.0f) + maze_position.y, player.width/MAZE_SCALE , player.height/MAZE_SCALE } , BLUE);
            
                        // TODO: Draw editor UI required elements
                        DrawText("EDIT MODE", 10, GetScreenHeight() - 90, 20, BLACK);
                        
                        DrawText(TextFormat("MOUSE: [%i, %i]", selected_cell.x, selected_cell.y), 10, 40, 20, DARKBLUE);
                        DrawText(TextFormat("IMAGE: [%i, %i]", image_point.x, image_point.y), 10, 70, 20, RED);
                    }
                    
                    DrawFPS(10, 10);
                    DrawText("[SPACE] TOGGLE MODE: EDITOR/GAME", 10, GetScreenHeight() - 60, 20, LIGHTGRAY);
                    DrawText("[R]: GENERATE NEW RANDOM SEQUENCE", 10, GetScreenHeight() - 30, 20, LIGHTGRAY);
                    
                    EndDrawing();                   
                }
                else {
                    EndScene(player_score);
                    
                }
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadTexture(tex_maze);     // Unload maze texture from VRAM (GPU)
    UnloadImage(im_maze);        // Unload maze image from RAM (CPU)
    UnloadTexture(player_tex); // Unload player texture
    UnloadTexture(item_tex);
    UnloadMusicStream(music);
    UnloadSound(item_sfx);
    CloseAudioDevice();
    
    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}



// Generate procedural maze image, using grid-based algorithm
// NOTE: Color scheme used: WHITE = Wall, BLACK = Walkable, RED = Item
static Image GenImageMaze(int width, int height, int spacing_rows, int spacing_cols, float point_chance)
{
    // TODO: [1p] Implement maze image generation algorithm
    
    Image im_maze = GenImageColor(width, height, BLACK);
    
    Point *maze_points = malloc(MAZE_HEIGHT*MAZE_WIDTH*sizeof(Point));
    
    int maze_point_counter = 0;
    
    for (int y = 0; y < im_maze.height; y++)
    {
        for (int x = 0; x < im_maze.width; x++)
        {
            if ((x == 0) || (y == 0) || (x == (im_maze.width - 1)) || (y == (im_maze.height - 1))) 
            {
                ImageDrawPixel(&im_maze, x, y, WHITE);
            }
            else 
            {
                if ((x%spacing_rows == 0) && (y%spacing_cols == 0)) 
                {
                    if (GetRandomValue(0, 100)<point_chance*100)
                    {
                        ImageDrawPixel(&im_maze, x, y, WHITE);
                        maze_points[maze_point_counter] = (Point) { x, y }; // Inicializa los valores: x = x , y = y
                        maze_point_counter++;
                    }
                }
            }
        }
    }
    
    Point dir_increments[4] = {
        {0,-1},
        {1,0},
        {0,1},
        {-1,0}
    };
    
    int *indexes = LoadRandomSequence(maze_point_counter, 0, maze_point_counter - 1);
    
    for (int i = 0; i < maze_point_counter; i++)
    {
        int dir = GetRandomValue(0,3);
        Point next_maze_point = maze_points[indexes[i]];
        next_maze_point.x += dir_increments[dir].x;
        next_maze_point.y += dir_increments[dir].y;
        
        while (ColorIsEqual(GetImageColor(im_maze, next_maze_point.x, next_maze_point.y), BLACK))
        {    
            ImageDrawPixel(&im_maze, next_maze_point.x, next_maze_point.y, WHITE);
            
            next_maze_point.x += dir_increments[dir].x;
            next_maze_point.y += dir_increments[dir].y;
        }
    }
    int n_items = 0;
    for (int y = 1; y < im_maze.height - 1; y++)
    {
        for (int x = 1; x < im_maze.width - 1; x++)
        {
            if (ColorIsEqual(GetImageColor(im_maze, x, y), BLACK))
            {
                if ((GetRandomValue(0, 1000) < 5 ) && (n_items < MAX_MAZE_ITEMS)) 
                {
                    ImageDrawPixel(&im_maze, x, y, RED);
                    
                }
            }
        }
    }

    UnloadRandomSequence(indexes);
    return im_maze;
}

// Scan map image to identify RED squares
static int LoadItems(Image im_maze, Point *maze_items, bool *maze_item_picked)
{
    int count = 0;
    
    for (int y = 0; y < im_maze.height; y++)
    {
        for (int x = 0; x < im_maze.width; x++)
        {
            if (ColorIsEqual(GetImageColor(im_maze, x, y), RED))
            {
                if (count < MAX_MAZE_ITEMS)
                {
                    maze_items[count] = (Point){ x, y };
                    maze_item_picked[count] = false;
                    count++;
                }
            }
        }
    }
    return count; // Returns number of items detected
}

static void EndScene(int score){
    BeginDrawing();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
    DrawText("GAME OVER", GetScreenWidth()/2 - 300, 100, 100, RED);
    DrawText(TextFormat("SCORE: %i", score), GetScreenWidth()/2 - 140, 300, 60, WHITE);
    DrawText("Press ESC to exit", GetScreenWidth()/2 - 180, 400, 40, LIGHTGRAY);

    EndDrawing();

}