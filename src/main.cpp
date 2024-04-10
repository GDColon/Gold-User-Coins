#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/CCSprite.hpp>

using namespace geode::prelude;

bool swappedTextures = false;

void swapTexture(std::string oldKey, std::string newKey) {
	auto textureCache = CCSpriteFrameCache::get();
	auto goldTexture = textureCache->spriteFrameByName(oldKey.c_str());
	textureCache->removeSpriteFrameByName(newKey.c_str());
	textureCache->addSpriteFrame(goldTexture, newKey.c_str());
}

class $modify(MenuLayer) {

	bool init() {
		if (!MenuLayer::init()) return false;

		if (swappedTextures) return true;
		else swappedTextures = true;

		// swaps all user coin textures with their golden counterparts
		for (int i = 1; i <= 4; i++) {
			std::string suffix = ("_00" + std::to_string(i) + ".png");
			swapTexture("secretCoin_01" + suffix, "secretCoin_2_01" + suffix);
			swapTexture("secretCoin_b_01" + suffix, "secretCoin_2_b_01" + suffix);
		}
		
		return true;
	} 
};

class $modify(CCSprite) {

	// removes the bronze tint on unverified coins
	void setColor(cocos2d::ccColor3B const& col) {
		GameObject* gameObj = typeinfo_cast<GameObject*>(this);
		if (gameObj && gameObj->m_objectID == 1329) CCSprite::setColor({255, 255, 255});
		else CCSprite::setColor(col);
	}
};

class $modify(LoadingLayer) {
	bool init(bool p0) {
		swappedTextures = false;
		return LoadingLayer::init(p0);
	}
};

$execute {
	// applies a patch that prevents the game from making the particles in coinEffect.plist silver
	// it's somewhere in EnhancedGameObject::updateUserCoin
	// first one makes it so (if (isCoin && objectID != 142) { ... }) never runs, 142 is secret coin ID
	// second and third changes the coin pickup effect to not be silver for user coins 
	Mod::get()->patch(reinterpret_cast<void*>(geode::base::get() + 0x14740a), { 0xEB });
	Mod::get()->patch(reinterpret_cast<void*>(geode::base::get() + 0x14afa8), { 0xEB }); // something 0xc8ffff
	Mod::get()->patch(reinterpret_cast<void*>(geode::base::get() + 0x14b035), { 0xEB }); // something concat21 0xffff
	
	// todo: android support. but that requires arm knowledge
}
