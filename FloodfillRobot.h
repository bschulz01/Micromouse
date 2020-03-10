//
//  FloodfillRobot.cpp
//  MicromouseMazeSimulator
//
//  Created by Bradley Schulz on 3/9/20.
//  Copyright © 2020 Bradley Schulz. All rights reserved.
//

#include <iostream>
#include <cstdlib>  // atoi
#include <cmath>

#include "Maze.h"
#include "MazeDefinitions.h"
#include "PathFinder.h"

class ourRobot : public PathFinder {
public:
    ourRobot(bool shouldPause = false);

    MouseMovement nextMovement(unsigned x, unsigned y, const Maze &maze);

protected:

    // Helps us determine if we've made a loop around the maze without finding the center.
    bool visitedStart;
    
    // The direction the robot is facing
    Dir heading;

    // Indicates we should pause before moving to next cell.
    // Useful for command line usage.
    const bool pause;

    bool isAtCenter(unsigned x, unsigned y) const;

    // When none of the adjacent traversible cells has a lower distance than the current cell.
    void runFloodfill(unsigned int x, unsigned int y);
    
    int lowestAdjacent(int x, int y, bool possibleDirections[]);
    
    // determine if a cell has at least one lower value around it
    bool cellFinished(unsigned int x, unsigned int y);
    
    // Determine which move will move the robot to a square with a lower distance to the center
    MouseMovement bestMove( bool frontWall, bool rightWall, bool leftWall, unsigned int x, unsigned int y);
    
    void printMaze();
    
private:
    bool verticalWalls[MazeDefinitions::MAZE_LEN + 1][MazeDefinitions::MAZE_LEN + 1];
    bool horizontalWalls[MazeDefinitions::MAZE_LEN + 1][MazeDefinitions::MAZE_LEN + 1];
    int distances[MazeDefinitions::MAZE_LEN][MazeDefinitions::MAZE_LEN];
};
