/* 
Author: Erick Kahinga
Class: ECE4122
Last Date Modified: 9/25/24
Description: Header file for the Bullet class
*/ 

#ifndef BULLET_H
#define BULLET_H

#include <SFML/Graphics.hpp>

const float BULLET_SPEED = 0.5f;

class Bullet {
public:
    sf::RectangleShape shape;

    // Constructor
    Bullet(float x, float y);

    // Method to update bullet position
    void update();
};

#endif // BULLET_H
