#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/LoadingLayer.hpp>
#include <Geode/modify/CCSprite.hpp>
#include <Geode/modify/GameObject.hpp>
#include <Geode/modify/EnhancedGameObject.hpp>
#include <Geode/modify/EndLevelLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

using namespace geode::prelude;

bool swappedTextures = false;
const int GOLD_COIN = 142;
const int USER_COIN = 1329;
const ccColor3B BRONZE_COLOR = ccColor3B{255, 175, 75};

void removeUITint(CCLayer* layer, std::string name) {
	auto spr = layer->getChildByID(name);
	if (spr) static_cast<CCSprite*>(spr)->setColor(ccWHITE);
}

// swap coin textures
class $modify(MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) return false;

		// test if already swapped textures
		if (swappedTextures) return true;
		else swappedTextures = true;

		// swaps all user coin textures with their golden counterparts
		for (int i = 1; i <= 4; i++) {
			std::string suffix = ("_00" + std::to_string(i) + ".png");
			swapTexture("secretCoin_01" + suffix, "secretCoin_2_01" + suffix);
			swapTexture("secretCoin_b_01" + suffix, "secretCoin_2_b_01" + suffix);
		}
		if (Mod::get()->getSettingValue<bool>("goldUI")) swapTexture("secretCoinUI_001.png", "secretCoinUI2_001.png");
		
		return true;
	} 

	void swapTexture(std::string goldKey, std::string userKey) {
		auto textureCache = CCSpriteFrameCache::get();
		auto goldTexture = textureCache->spriteFrameByName(goldKey.c_str());
		textureCache->removeSpriteFrameByName(userKey.c_str());
		textureCache->addSpriteFrame(goldTexture, userKey.c_str());
	}
};

// unset swapped flag if reloading textures
class $modify(LoadingLayer) {
	bool init(bool p0) {
		swappedTextures = false;
		return LoadingLayer::init(p0);
	}
};

// detect and remove the bronze tint on unverified coins
class $modify(CCSprite) {
	void setColor(cocos2d::ccColor3B const& col) {
		if (col == BRONZE_COLOR && Mod::get()->getSettingValue<bool>("noBronze")) {
			GameObject* gameObj = typeinfo_cast<GameObject*>(this); // check for user coin
			if (gameObj && gameObj->m_objectID == USER_COIN) {
				CCSprite::setColor({255, 255, 255});
				return;
			}
		}

		CCSprite::setColor(col);
	}
};

// change end screen
class $modify(EndLevelLayer) {
	void customSetup() {

		EndLevelLayer::customSetup();
		if (!Mod::get()->getSettingValue<bool>("goldUI")) return;

		removeUITint(m_mainLayer, "coin-1-sprite");
		removeUITint(m_mainLayer, "coin-2-background");
		removeUITint(m_mainLayer, "coin-3-sprite");
		removeUITint(m_mainLayer, "coin-4-background");
		removeUITint(m_mainLayer, "coin-5-sprite");
		removeUITint(m_mainLayer, "coin-6-background");

		// coin collect effect
		for (CCSprite* spr : CCArrayExt<CCSprite*>(m_coinsToAnimate)) {
			spr->setColor(ccWHITE);
		}
	}

	void coinEnterFinished(CCPoint p) {
		if (!Mod::get()->getSettingValue<bool>("goldUI")) return;
		bool notLocal = m_notLocal;
		m_notLocal = false; // use 'official level' particles
		EndLevelLayer::coinEnterFinished(p);
		m_notLocal = notLocal;
	}
};

// coins in pause menu mod!
class $modify(PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();
		if (!Mod::get()->getSettingValue<bool>("goldUI") || !Mod::get()->getSettingValue<bool>("noBronze")) return;

		auto bottomMenu = getChildByID("bottom-button-menu");
		if (bottomMenu) {
			for (auto node : CCArrayExt<CCNode*>(bottomMenu->getChildren())) {
				if (CCSprite* spr = typeinfo_cast<CCSprite*>(node)) {
					if (spr->getColor() == BRONZE_COLOR) spr->setColor(ccWHITE);
					
					auto sprChild = node->getChildByType<CCSprite>(0);
					if (sprChild && sprChild->getColor() == BRONZE_COLOR) sprChild->setColor(ccWHITE);
				}
			}
		}
	}
};

// pickup effect
class $modify(GameObject) {
	void playDestroyObjectAnim(GJBaseGameLayer* b) {
		if (this->m_objectID == USER_COIN) {
			this->m_objectType = GameObjectType::SecretCoin;
			GameObject::playDestroyObjectAnim(b);
			this->m_objectType = GameObjectType::UserCoin;
		}
		else GameObject::playDestroyObjectAnim(b);
	}
};

// coin particles
// this is so hacky but it works fine
class $modify(EnhancedGameObject) {
	void updateUserCoin() {
		if (this->m_objectID == USER_COIN) {
			this->m_objectID = GOLD_COIN;
			EnhancedGameObject::updateUserCoin();
			this->m_objectID = USER_COIN;
		}
		else EnhancedGameObject::updateUserCoin();
	}
};

// sneaky
class $modify(EditorUI) {
	void onCreateObject(int objID) {
		// 1614 ID is mini coin
		if ((objID == USER_COIN || objID == 1614) && Mod::get()->getSettingValue<bool>("goldCoinsEditor") && CCKeyboardDispatcher::get()->getAltKeyPressed()) EditorUI::onCreateObject(GOLD_COIN);
		else EditorUI::onCreateObject(objID);
	}
};