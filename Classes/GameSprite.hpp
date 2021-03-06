#pragma once

#include "cocos2d.h"


USING_NS_CC;
enum {

	kBackground,
	kMiddleground,
	kForeground
};

class GameSprite : public Sprite {

protected:
	Size _screenSize;

public:

	CC_SYNTHESIZE(Point, _nextPosition, NextPosition);

	CC_SYNTHESIZE(float, _width, Width);

	CC_SYNTHESIZE(float, _height, Height);

	CC_SYNTHESIZE(Point, _vector, Vector);

	GameSprite(void);
	~GameSprite(void);

	inline virtual void place() { this->setPosition(_nextPosition); };

	inline void setSize() {
		_width = this->getBoundingBox().size.width;
		_height = this->getBoundingBox().size.height;
	}


};
