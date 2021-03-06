#include "Terrain.hpp"

int patterns[] = { 1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,3,3,3 };
int widths[] = { 2,2,2,2,2,3,3,3,3,3,3,4,4,4,4,4,4 };
int heights[] = { 0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,3,3,3,3,3,3,4 };
int types[] = { 1,2,3,4,1,3,2,4,3,2,1,4,2,3,1,4,2,3,1,2,3,2,3,4,1,2,4,3,1,3,1,4,2,4,2,1,2,3 };

vector<int> _blockPattern(patterns, patterns + sizeof(patterns) / sizeof(int));
vector<int> _blockWidths(widths, widths + sizeof(widths) / sizeof(int));
vector<int> _blockHeights(heights, heights + sizeof(heights) / sizeof(int));
vector<int> _blockTypes(types, types + sizeof(types) / sizeof(int));

CustomTerrain::~CustomTerrain() {}

CustomTerrain::CustomTerrain()
	:_screenSize(Director::getInstance()->getWinSize())
	, _startTerrain(false)
	, _blockPoolIndex(0)
	, _currentPatternCnt(1)
	, _currentPatternIndex(0)
	, _currentTypeIndex(0)
	, _currentWidthIndex(0)
	, _currentHeightIndex(0)
	, _showGap(false)
{
}
CustomTerrain * CustomTerrain::create() {
	CustomTerrain * terrain = new CustomTerrain();
	if (terrain && terrain->initWithFile("blank.png")) {
		terrain->setAnchorPoint(Vec2(0, 0));
		terrain->initTerrain();
		terrain->autorelease();
		return terrain;
	}
	CC_SAFE_DELETE(terrain);
	return NULL;
}

void CustomTerrain::initTerrain() {
	_increaseGapInterval = 5000;
	_increaseGapTimer = 0;
	_gapSize = 2;

	for (size_t i = 0; i < 20; i++) {
		auto block = Block::create();
		this->addChild(block);
		blockPool.push_back(block);
	}

	_minTerrainWidth = _screenSize.width * 1.5f;
	random_shuffle(_blockPattern.begin(), _blockPattern.end());
	random_shuffle(_blockWidths.begin(), _blockWidths.end());
	random_shuffle(_blockHeights.begin(), _blockHeights.end());

	this->addBlocks(0);

}


void CustomTerrain::checkCollision(Player * player) {

	if (player->getState() == kPlayerDying) return;

	bool inAir = true;
	for(auto block: blocks){
		if (block->getType() == kBlockGap) continue;

		if (player->right() >= this->getPositionX() + block->left()
			&& player->left() <= this->getPositionX() + block->right()){

				if (player->bottom() >= block->top()
					&& player->next_bottom() <= block->top()
					&& player->top() > block->top()){

						player->setNextPosition(Vec2(
							player->getNextPosition().x, block->top() + player->getHeight()
						));
						player->setVector(Vec2(player->getVector().x, 0));
						player->setRotation(0.0);
						inAir = false;
						break;
					}
			}
	}

	for(auto block: blocks){
		if (block->getType() == kBlockGap) continue;

		if ((player->bottom() < block->top() &&
				player->top() > block->bottom()) || (
				player->next_bottom() < block->top() &&
			 	player->next_top() > block->bottom())
			){

				if (player->right() >= this->getPositionX() + block->getPositionX()
				&& player->left() < this->getPositionX() + block->getPositionX())	{

					player->setPositionX(this->getPositionX() + block->getPositionX()
						- player->getWidth() * 0.5f);
					player->setNextPosition(Vec2(
						this->getPositionX() + block->getPositionX() - player->getWidth() * 0.5f,
						player->getNextPosition().y
					));
					player->setVector(
						Vec2(player->getVector().x * -0.5f, player->getVector().y));

					if (player->bottom() + player->getHeight() *0.2f < block->top()){

						player->setState(kPlayerDying);
						return;

					}
					break;
				}
			}
	}

	if (inAir){
		player->setState(kPlayerFalling);
	} else {
		player->setState(kPlayerMoving);
		player->setFloating(false);
	}

}


void CustomTerrain::move(float xMove) {

	if (xMove < 0 ) return;

	if (_startTerrain){
		if (xMove > 0 && _gapSize<5){
			_increaseGapTimer += xMove;

			if (_increaseGapTimer > _increaseGapInterval){
				_increaseGapTimer = 0;
				_gapSize += 1;
			}
		}

		this->setPositionX(this->getPositionX() - xMove);

		auto block = blocks.at(0);

		if (_position.x + block->getWidth() < 0) {

			auto firstBlock = blocks.at(0);
			blocks.erase(blocks.begin());
			blocks.push_back(firstBlock);
			_position.x += block->getWidth();

			float width_cnt = this->getWidth() - block->getWidth() -
				(blocks.at(0)->getWidth());

			this->initBlock(block);
			this->addBlocks(width_cnt);
		}
	}


}

void CustomTerrain::reset() {

	this->setPosition(Vec2(0, 0));
	_startTerrain = false;
	int currentWidth = 0;

	for(auto block: blocks){
		this->initBlock(block);
		currentWidth += block->getWidth();
	}

	while(currentWidth < _minTerrainWidth){
		auto block = blockPool.at(_blockPoolIndex);
		_blockPoolIndex++;
		if (_blockPoolIndex == blockPool.size()){
			_blockPoolIndex = 0;
		}

		blocks.push_back(block);
		this->initBlock(block);
		currentWidth += block->getWidth();
	}
	this->distributeBlocks();
	_increaseGapTimer = 0;
	_gapSize = 2;
}

void CustomTerrain::addBlocks(int currentWidth) {

	while (currentWidth < _minTerrainWidth) {
		auto block = blockPool.at(_blockPoolIndex);
		_blockPoolIndex++;
		if (_blockPoolIndex == blockPool.size()) {
			_blockPoolIndex = 0;
		}
		this->initBlock(block);
		currentWidth += block->getWidth();
		blocks.push_back(block);
	}
	this->distributeBlocks();

}

void CustomTerrain::distributeBlocks() {

	int count = blocks.size();
	int i;

	for (i = 0; i< count; i++){
		auto block = blocks.at(i);
		if (i != 0){
			auto prev_block = blocks.at(i-1);
			block->setPositionX(prev_block->getPositionX() + prev_block->getWidth());
		} else {
			block->setPositionX(0);
		}
	}
}

void CustomTerrain::initBlock(Block * block) {

	int blockWidth;
	int blockHeight;

	int type = _blockTypes[_currentTypeIndex];
	_currentTypeIndex++;
	if (_currentTypeIndex == _blockTypes.size()){
		_currentTypeIndex = 0;
	}

	if (_startTerrain) {

		if (_showGap) {

			int gap = rand() % _gapSize;
			if (gap < 2) gap = 2;
			block->setupBlock(gap, 0, kBlockGap);
			_showGap = false;
		} else {
			blockWidth = _blockWidths[_currentWidthIndex];
			_currentWidthIndex++;
			if(_currentWidthIndex == _blockWidths.size()){
				random_shuffle(_blockWidths.begin(), _blockWidths.end());
				_currentWidthIndex = 0;
			}
			if(_blockHeights[_currentHeightIndex] != 0){

				blockHeight = _blockHeights[_currentHeightIndex];

				if (blockHeight - _lastBlockHeight > 2 && _gapSize == 2){
					blockHeight = 1;
				}
			} else {
				blockHeight = _lastBlockHeight;
			}
			_currentHeightIndex++;

			if (_currentHeightIndex == _blockHeights.size()) {
				_currentHeightIndex=0;
				random_shuffle(_blockHeights.begin(), _blockHeights.end());
			}

			block->setupBlock(blockWidth, blockHeight, type);
			_lastBlockWidth = blockWidth;
			_lastBlockHeight = blockHeight;

			_currentPatternCnt++;
			if(_currentPatternCnt > _blockPattern[_currentPatternIndex]){
				_showGap = true;
				_currentPatternIndex++;

				if (_currentPatternIndex == _blockPattern.size()){
					random_shuffle(_blockPattern.begin(), _blockPattern.end());
					_currentPatternIndex = 0;
				}

				_currentPatternCnt = 1;
			}
		}
	} else {
		_lastBlockHeight = 2;
		_lastBlockWidth = rand() % 2 + 2;
		block->setupBlock(_lastBlockWidth, _lastBlockHeight, type);
	}


}
