#include <iostream>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>     /* srand, rand */
#include <time.h>

struct coord{
    int x;
    int y;
};

const int HEIGHT = 20, WIDTH = 10;

// Symbols
char symbols[] = " #O*-.";

// Array which represents the field
// as to be drawn
char field[HEIGHT][WIDTH];
// the static field
char static_field[HEIGHT][WIDTH];

// the active moving block
coord block[5];

// Game over state
bool game_over = false;

// score counter
int score = 0;

// Render the field array and a frame
void render(char field[][WIDTH])
{
    // Draw frame
    for(int i = 0; i<HEIGHT+2; i++)
    {
        for(int j = 0; j<WIDTH+2; j++)
        {
            if(i == 0 || i == HEIGHT+1)
            {
                mvaddch(i, j, symbols[1]);
            }
            else
            {
                if(j == 0 || j == WIDTH+1)
                {
                    mvaddch(i, j, symbols[1]);
                }
                // Draw field
                else
                    mvaddch(i, j, field[i-1][j-1]);    
            }
        }
    }
    move(HEIGHT+2,0);
    printw("Score: %d", score);
}

// Initialize arrays (fill with spaces)
void init_array(char arr[][WIDTH])
{
    for (int i=0;i<HEIGHT;i++)
    {
        for(int j=0;j<WIDTH;j++)
        {
            arr[i][j] = symbols[0];
        }
    }    
}

// Check if there are intersections between the 
// static field / the top border and the block
bool collision()
{
    for (int i=0; i<4; i++)
    {
        if(static_field[block[i].x][block[i].y] == symbols[2]
            || block[i].x > HEIGHT-1)
        {
            return true;
        }
    }
    return false;
}

// takes also the side borders into consideration
bool collision_sides()
{
    for (int i=0; i<4; i++)
    {
        if(static_field[block[i].x][block[i].y] == symbols[2]
            || block[i].x > HEIGHT-1
            || block[i].y > WIDTH-1 || block[i].y < 0)
        {
            return true;
        }
    }
    return false;
}

// Spawn a random block
void spawn_block()
{
    // initialize random seed:
    srand(time(NULL));
    // get random block template
    int r = 10 * (rand() % 7); // random number (0,6) for offset

    // array of all available blocks
    // the first 8 values are the occupied fields
    // followed by 2 values for the rotation center
    int allblocks[70] = {0,0,0,1,0,2,0,3, 1,1, // a
                         0,0,0,1,1,0,1,1, 1,1, // b
                         0,0,0,1,1,1,1,2, 1,1, // c
                         0,1,0,2,1,0,1,1, 1,1, // d
                         0,0,1,0,1,1,1,2, 1,1, // e
                         0,1,1,0,1,1,1,2, 1,1, // f
                         0,2,1,0,1,1,1,2, 1,1, // e_mirrored
                        };

    // initialize active block
    // a block consists of 5 coord tuples:
    // 4 represent the fields, 1 the rotation center
    for (int i=0; i<5; i++)
    {
        block[i].x = allblocks[2*i+r];
        block[i].y = allblocks[2*i+r+1] + WIDTH/2-2;
    }

    if (collision()){
        game_over = true;
    }

}

// Merge the active block into the static field
void merge_into_static()
{
    for (int i=0; i<4; i++)
    {
        static_field[block[i].x][block[i].y] = symbols[2];
    }
}

// Move the block down
bool move_block_down()
{
    // Move down
    for (int i=0; i<5; i++)
    {
        block[i].x++;
    }

    if (collision())
        {
            // Undo last movement
            for (int i=0; i<5; i++)
            {
                block[i].x--;
            }
            merge_into_static();
            return false;
        }
    return true;
}

void rotate_block(int dir)
{
    coord old_pos[4];

    for (int i=0; i<4; i++)
    {
        // remember former position
        old_pos[i].x = block[i].x;
        old_pos[i].y = block[i].y;

        // calculate position relative to center
        int x = block[i].x - block[4].x;
        int y = block[i].y - block[4].y;
        
        // rotate 90 left
        if (dir == 1)
        {
            block[i].x = y + block[4].x;
            block[i].y = -x + block[4].y;
        }
        // right
        // xNew = -y (+ xCenter)
        // yNew = x (+ yCenter)
        else
        {
            block[i].x = -y + block[4].x;
            block[i].y = x + block[4].y;
        } 
    }
    if (collision_sides())
    {
        // Undo last movement
        for (int i=0; i<4; i++)
        {
            block[i].x = old_pos[i].x;
            block[i].y = old_pos[i].y;
        }
    }
}

void move_block_side(int dir)
{
    // Move left
    if(dir == 0)
    {
        for (int i=0; i<5; i++)
        {
            block[i].y--;
        }
    }
    // Move right
    else
    {
        for (int i=0; i<5; i++)
        {
            block[i].y++;
        }
    }
    if (collision_sides())
    {
        // Undo last movement
        for (int i=0; i<5; i++)
        {
            if(dir == 0)
                block[i].y++;
            else
                block[i].y--;
        }
    }
}


// Update the field with the new block position
void update_field()
{
    // Compare the field array with the 
    // static field array and reset it
    // deleting the former block position
    for (int i=0;i<HEIGHT;i++)
    {
        for(int j=0;j<WIDTH;j++)
        {
            if (static_field[i][j] == symbols[0])
                field[i][j] = symbols[0];
            else
                field[i][j] = symbols[2];
        }
    }

    // Draw in the new block position
    for (int i=0; i<4; i++)
    {
        field[block[i].x][block[i].y] = symbols[2];
    }
}


// Move everything above nr in static_field 1 down.
void delete_line(int nr)
{
    for (int i = nr; i>0; i--)
    {
        for (int j = 0; j<WIDTH; j++)
        {
            static_field[i][j] = static_field[i-1][j];
        }
    }
}

// check for lines in static_field
void check_for_lines()
{
    int score_count = 0;        // keep track of the num of lines removed
    for (int i = HEIGHT-1; i>-1; i--)
    {
        int cnt = 0;
        for (int j = 0; j<WIDTH; j++)
        {
            if (static_field[i][j] == symbols[2])
                 cnt++;
            else break;
        }
        if (cnt == WIDTH)
        // Delete line
        {
            // Animation
            for (int l=0; l<WIDTH; l++)
            {
                field[i][l] = symbols[3];
            }
            render(field);
            refresh();
            usleep(50000);
            for (int l=0; l<WIDTH; l++)
            {
                field[i][l] = symbols[4];
            }
            render(field);
            refresh();
            usleep(50000);
            for (int l=0; l<WIDTH; l++)
            {
                field[i][l] = symbols[5];
            }
            render(field);
            refresh();
            usleep(50000);
            // again old symbol but dont render
            // fixes problem
            for (int l=0; l<WIDTH; l++)
            {
                field[i][l] = symbols[2];
            }

            delete_line(i);
            // Look again at this line
            i++;
            score_count++;
            continue;
        }
    }
    if (score_count > 0)
    {
        switch(score_count)
        {
            case 1:
                score += 40;
                break;
            case 2:
                score += 100;
                break;
            case 3:
                score += 300;
                break;
            case 4:
                score += 1200;
                break;
        }
    }
}


int main(int argc, char const *argv[])
{
    init_array(field);
    init_array(static_field);

    // initialize the screen
    // set up memory and clear screen
    initscr();

    curs_set(0);
    noecho();

    spawn_block();

    int game_speed = 20;
    int time_count = 0;

    // make getch() non-blocking
    nodelay(stdscr,TRUE);

    // Main Game Loop
    while(!game_over)
    {
        update_field();
        render(field);

        refresh();  // refresh the screen to match whats in memory
                    /* Print it on to the real screen */
        
        // Timestep of the game 50ms
        usleep(50000);



        // Handle if keys are pressed
        switch(getch()) {
            // for arrows
            case '\033': { // if the first value is esc
                getch(); // skip the [
                switch(getch()) {
                    case 65:    // key up
                        break;
                    case 66:    // key down
                        if(!move_block_down())
                        {
                            check_for_lines();
                            spawn_block();
                            //time_count = 0;
                        }
                        
                        break;
                    case 67:    // key right
                        move_block_side(1);
                        break;
                    case 68:    // key left
                        move_block_side(0);
                        break;
                    }
                break;
            }
            // for letters
            case 113: // q
                rotate_block(0);
                break;
            case 119: // w
                rotate_block(1);
                break;
            }

        // Move block down (and check)
        if(time_count == game_speed)
        {
            if(!move_block_down())
            {
                check_for_lines();
                spawn_block();
            }
            time_count = 0;
        }

        time_count++;
    }

    move(HEIGHT+2,0);
    printw("GAME OVER! Your score is: %d", score);
    move(HEIGHT+3,0);
    printw("Press q to quit");
    // make getch() blocking
    nodelay(stdscr,FALSE);
    
    while(getch() != 113)
        continue;
    
    endwin();   // deallocate memory and end ncurses


    return 0;
}