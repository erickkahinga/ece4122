/* 
Author: Erick Kahinga
Class: ECE4122
Last Date Modified: 9/25/24
Description: File for the Bullet the starship shoots
*/ 

#include "Bullet.h"

Bullet::Bullet(float x, float y) {
    shape.setSize(sf::Vector2f(5, 10));
    shape.setFillColor(sf::Color::Red);
    shape.setPosition(x, y);
}

// Method to update bullet position
void Bullet::update() {
    shape.move(0, -BULLET_SPEED);
}
