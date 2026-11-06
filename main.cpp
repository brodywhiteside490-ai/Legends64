#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/Popup.hpp>
#include "ShopData.hpp"
#include <algorithm>
#include <string>

using namespace geode::prelude;

namespace shopmaker {
    static std::string trim(std::string s) {
        auto a = s.find_first_not_of(" \t\r\n");
        if (a == std::string::npos) return {};
        auto b = s.find_last_not_of(" \t\r\n"); return s.substr(a, b-a+1);
    }
    static CCTextFieldTTF* field(CCNode* parent, char const* hint, std::string const& value, CCPoint pos, float width=160.f) {
        auto f = CCTextFieldTTF::create(width, 24.f, "bigFont.fnt");
        f->setPlaceHolder(hint); f->setString(value.c_str()); f->setScale(.48f); f->setPosition(pos); parent->addChild(f); return f;
    }
    static ccColor3B textPalette(int index) {
        static const ccColor3B colors[8]={{245,245,245},{255,220,90},{120,225,255},{150,255,160},{255,150,150},{210,160,255},{255,190,110},{30,30,30}};
        return colors[std::clamp(index,0,7)];
    }
    static CCLabelBMFont* label(CCNode* parent, std::string const& text, CCPoint pos, float scale=.38f) {
        auto l = CCLabelBMFont::create(text.c_str(), "chatFont.fnt"); l->setScale(scale); l->setPosition(pos); parent->addChild(l); return l;
    }
    static CCMenuItemSpriteExtra* button(CCObject* target, SEL_MenuHandler selector, char const* text, float width=100.f) {
        return CCMenuItemSpriteExtra::create(ButtonSprite::create(text, width, true, "goldFont.fnt", "GJ_button_01.png", 30, .65f), target, selector);
    }
    static void alert(std::string const& message) { FLAlertLayer::create("Custom Shop Maker", message.c_str(), "OK")->show(); }

    class ItemEditorPopup : public geode::Popup<> {
        Shop* m_shop = nullptr; Item* m_item = nullptr;
        CCTextFieldTTF *m_title=nullptr, *m_price=nullptr, *m_stock=nullptr, *m_iconType=nullptr, *m_iconID=nullptr, *m_kind=nullptr, *m_repeatable=nullptr;
        bool setup() override {
            setTitle("Merchandise Editor"); auto sz=m_mainLayer->getContentSize();
            label(m_mainLayer,"Item title",{sz.width*.28f,190}); m_title=field(m_mainLayer,"Item title",m_item->title,{sz.width*.69f,190});
            label(m_mainLayer,"Price (orbs)",{sz.width*.28f,158}); m_price=field(m_mainLayer,"Price",std::to_string(m_item->price),{sz.width*.69f,158});
            label(m_mainLayer,"Stock (-1 = unlimited)",{sz.width*.28f,126}); m_stock=field(m_mainLayer,"-1 unlimited",std::to_string(m_item->stock),{sz.width*.69f,126});
            label(m_mainLayer,"Icon type",{sz.width*.28f,94}); m_iconType=field(m_mainLayer,"0=Cube, 1=Ship...",std::to_string(m_item->iconType),{sz.width*.69f,94});
            label(m_mainLayer,"Icon ID",{sz.width*.28f,62}); m_iconID=field(m_mainLayer,"Icon ID",std::to_string(m_item->iconID),{sz.width*.69f,62});
            label(m_mainLayer,"Reward kind",{sz.width*.28f,32}); m_kind=field(m_mainLayer,"icon / more-icons / custom",m_item->kind,{sz.width*.69f,32},155);
            label(m_mainLayer,"Repeatable? 1=yes, 0=no",{sz.width*.32f,8},.31f); m_repeatable=field(m_mainLayer,"0 or 1",m_item->repeatable?"1":"0",{sz.width*.75f,8},70);
            auto save=button(this,menu_selector(ItemEditorPopup::onSave),"Save item",100);
            auto menu=CCMenu::create(); menu->addChild(save); menu->setPosition({sz.width/2,-12}); m_mainLayer->addChild(menu);
            return true;
        }
        void onSave(CCObject*) {
            auto integer=[](CCTextFieldTTF* f,int fallback){ try { return std::stoi(f->getString()); } catch (...) { return fallback; } };
            m_item->title=trim(m_title->getString()); if(m_item->title.empty()) m_item->title="Item";
            m_item->price=std::max(0,integer(m_price,0)); m_item->stock=integer(m_stock,-1);
            if(m_item->stock < -1) m_item->stock=-1;
            m_item->iconType=std::max(0,integer(m_iconType,0)); m_item->iconID=std::max(1,integer(m_iconID,1));
            m_item->kind=trim(m_kind->getString()); if(m_item->kind!="icon" && m_item->kind!="more-icons" && m_item->kind!="custom") m_item->kind="icon";
            m_item->repeatable=integer(m_repeatable,0)!=0;
            Store::get().save(); onClose(nullptr);
        }
    public:
        static ItemEditorPopup* create(Shop* shop, Item* item) {
            if(!shop||!item)return nullptr; auto p=new ItemEditorPopup(); p->m_shop=shop; p->m_item=item;
            if(p->initAnchored(340,230)){p->autorelease();return p;} delete p;return nullptr;
        }
    };

    class ShopDesignPopup : public geode::Popup<> {
        Shop* m_shop=nullptr;
        CCTextFieldTTF *m_name=nullptr,*m_keeper=nullptr,*m_greeting=nullptr,*m_dialogue=nullptr;
        bool setup() override {
            setTitle("Shop Design"); auto sz=m_mainLayer->getContentSize();
            auto add=[&](char const* t,CCTextFieldTTF*& f,std::string const& val,float y){label(m_mainLayer,t,{sz.width*.27f,y});f=field(m_mainLayer,t,val,{sz.width*.68f,y});};
            add("Shop name",m_name,m_shop->name,210); add("Keeper name",m_keeper,m_shop->keeperName,180);
            add("Greeting",m_greeting,m_shop->greeting,150); add("Click dialogue",m_dialogue,m_shop->dialogue,120);
            label(m_mainLayer,"Background palette",{sz.width*.28f,86});
            label(m_mainLayer,"Text palette",{sz.width*.28f,62});
            label(m_mainLayer,"Keeper head / mouth / body",{sz.width*.42f,35},.33f);
            auto prevBg=button(this,menu_selector(ShopDesignPopup::onBgPrev),"<",34);
            auto nextBg=button(this,menu_selector(ShopDesignPopup::onBgNext),">",34);
            auto prevTx=button(this,menu_selector(ShopDesignPopup::onTextPrev),"<",34);
            auto nextTx=button(this,menu_selector(ShopDesignPopup::onTextNext),">",34);
            auto parts=button(this,menu_selector(ShopDesignPopup::onParts),"Cycle keeper parts",120);
            auto menu=CCMenu::create(); menu->addChild(prevBg);menu->addChild(nextBg);menu->addChild(prevTx);menu->addChild(nextTx);menu->addChild(parts);
            prevBg->setPosition({sz.width*.55f,86});nextBg->setPosition({sz.width*.72f,86});
            prevTx->setPosition({sz.width*.55f,62});nextTx->setPosition({sz.width*.72f,62});parts->setPosition({sz.width/2,12});
            m_mainLayer->addChild(menu);
            auto colors=label(m_mainLayer,"BG "+std::to_string(m_shop->backgroundColor)+"  TEXT "+std::to_string(m_shop->textColor),{sz.width*.84f,74},.3f); colors->setTag(701);
            auto presets=label(m_mainLayer,"H"+std::to_string(m_shop->headPreset)+" / M"+std::to_string(m_shop->mouthPreset)+" / B"+std::to_string(m_shop->bodyPreset),{sz.width*.84f,42},.3f); presets->setTag(702);
            auto save=button(this,menu_selector(ShopDesignPopup::onSave),"Save design",100);
            auto sm=CCMenu::create();sm->addChild(save);sm->setPosition({sz.width/2,-14});m_mainLayer->addChild(sm);
            return true;
        }
        void onBgPrev(CCObject*){m_shop->backgroundColor=(m_shop->backgroundColor+7)%8;refreshLabels();}
        void onBgNext(CCObject*){m_shop->backgroundColor=(m_shop->backgroundColor+1)%8;refreshLabels();}
        void onTextPrev(CCObject*){m_shop->textColor=(m_shop->textColor+7)%8;refreshLabels();}
        void onTextNext(CCObject*){m_shop->textColor=(m_shop->textColor+1)%8;refreshLabels();}
        void onParts(CCObject*){m_shop->headPreset=(m_shop->headPreset+1)%8;m_shop->mouthPreset=(m_shop->mouthPreset+3)%8;m_shop->bodyPreset=(m_shop->bodyPreset+5)%8;refreshLabels();}
        void refreshLabels(){
            // The palette/preset values are stored here. A renderer can map these indices to the game's shopkeeper sprite frames.
            auto l=static_cast<CCLabelBMFont*>(m_mainLayer->getChildByTag(701));
            if(l) l->setString(("BG "+std::to_string(m_shop->backgroundColor)+"  TEXT "+std::to_string(m_shop->textColor)).c_str());
            auto p=static_cast<CCLabelBMFont*>(m_mainLayer->getChildByTag(702));
            if(p) p->setString(("H"+std::to_string(m_shop->headPreset)+" / M"+std::to_string(m_shop->mouthPreset)+" / B"+std::to_string(m_shop->bodyPreset)).c_str());
        }
        void onSave(CCObject*){
            m_shop->name=trim(m_name->getString()); if(m_shop->name.empty())m_shop->name="My Shop";
            m_shop->keeperName=trim(m_keeper->getString()); if(m_shop->keeperName.empty())m_shop->keeperName="Shopkeeper";
            m_shop->greeting=m_greeting->getString();m_shop->dialogue=m_dialogue->getString();Store::get().save();onClose(nullptr);
        }
    public:
        static ShopDesignPopup* create(Shop* shop){if(!shop)return nullptr;auto p=new ShopDesignPopup();p->m_shop=shop;if(p->initAnchored(340,275)){p->autorelease();return p;}delete p;return nullptr;}
    };

    class ShopViewPopup : public geode::Popup<> {
        Shop* m_shop=nullptr;
        bool setup() override {
            setTitle(m_shop->name.c_str());auto sz=m_mainLayer->getContentSize();
            auto greeting=label(m_mainLayer,m_shop->greeting,{sz.width/2,sz.height-38},.42f); greeting->setColor(textPalette(m_shop->textColor));
            static const ccColor3B palette[8]={{40,40,55},{45,85,150},{40,125,90},{150,70,65},{120,75,155},{180,130,50},{80,100,115},{220,220,220}};
            m_mainLayer->setColor(palette[std::clamp(m_shop->backgroundColor,0,7)]);
            auto dialogue=label(m_mainLayer,(m_shop->keeperName+": "+m_shop->dialogue).substr(0,70),{sz.width/2,sz.height-62},.34f); dialogue->setColor(textPalette(m_shop->textColor));
            float y=sz.height-100; int shown=0;
            for(auto& item:m_shop->items){
                if(shown>=5)break;
                std::string status=item.title+"  |  "+std::to_string(item.price)+" orbs  |  "+(item.stock<0?"∞":"stock "+std::to_string(item.stock));
                if(!item.repeatable&&Store::get().owns(item.id))status+="  [OWNED]";
                auto entry=label(m_mainLayer,status,{sz.width*.43f,y},.31f); entry->setColor(textPalette(m_shop->textColor));
                auto b=button(this,menu_selector(ShopViewPopup::onBuy),"Buy",55);b->setTag(shown);auto menu=CCMenu::create();menu->addChild(b);b->setPosition({sz.width*.86f,0});menu->setPosition({sz.width*.86f,y});m_mainLayer->addChild(menu);
                ++shown;y-=27;
            }
            if(m_shop->items.empty())label(m_mainLayer,"This shop has no merchandise yet.",{sz.width/2,sz.height/2},.4f);
            label(m_mainLayer,"Purchases use your regular orbs.",{sz.width/2,24},.32f);
            return true;
        }
        void onBuy(CCObject* sender){
            int index=static_cast<CCNode*>(sender)->getTag();
            if(index<0||index>=static_cast<int>(m_shop->items.size()))return;
            auto& item=m_shop->items[index];
            if(item.kind=="more-icons") { alert("More Icons support needs its mod-specific unlock adapter; this item cannot be purchased yet."); return; }
            // GD's normal orb wallet is the transaction balance. Only charge after all shop checks pass.
            auto stats=GameStatsManager::sharedState();
            int balance=stats->getCurrencyForType(CurrencyType::Orbs);
            std::string error;
            if(!Store::get().purchase(m_shop->id,item.id,balance,error)){alert(error);return;}
            stats->incrementCurrencyForType(CurrencyType::Orbs,-item.price);
            // Vanilla GD icon rewards can be unlocked here. External More Icons rewards are intentionally left
            // to the adapter interface; their IDs must not be passed into the vanilla unlock table.
            if(item.kind=="icon") {
                GameManager::sharedState()->unlockIcon(item.iconID, static_cast<IconType>(item.iconType));
            }
            alert("Purchase complete!");onClose(nullptr);
        }
    public:
        static ShopViewPopup* create(Shop* shop){if(!shop)return nullptr;auto p=new ShopViewPopup();p->m_shop=shop;if(p->initAnchored(360,300)){p->autorelease();return p;}delete p;return nullptr;}
    };

    class ShopEditorPopup : public geode::Popup<> {
        Shop* m_shop=nullptr;
        bool setup() override {
            setTitle("Shop Workshop");auto sz=m_mainLayer->getContentSize();
            label(m_mainLayer,m_shop->name,{sz.width/2,sz.height-38},.5f);
            auto design=button(this,menu_selector(ShopEditorPopup::onDesign),"Design shop",105);
            auto add=button(this,menu_selector(ShopEditorPopup::onAddItem),"Add item",85);
            auto view=button(this,menu_selector(ShopEditorPopup::onPreview),"Preview / open",105);
            auto menu=CCMenu::create();menu->addChild(design);menu->addChild(add);menu->addChild(view);menu->alignItemsHorizontallyWithPadding(8);menu->setPosition({sz.width/2,sz.height-70});m_mainLayer->addChild(menu);
            float y=sz.height-112;int i=0;
            for(auto& item:m_shop->items){if(i>=4)break;
                label(m_mainLayer,item.title+"  ($"+std::to_string(item.price)+")",{sz.width*.38f,y},.32f);
                auto edit=button(this,menu_selector(ShopEditorPopup::onEditItem),"Edit",48);edit->setTag(i);
                auto del=button(this,menu_selector(ShopEditorPopup::onDeleteItem),"X",32);del->setTag(i);
                auto row=CCMenu::create();row->addChild(edit);row->addChild(del);edit->setPosition({0,0});del->setPosition({48,0});row->setPosition({sz.width*.83f,y});m_mainLayer->addChild(row);
                ++i;y-=30;
            }
            label(m_mainLayer,"Up to 4 items shown here; save each item after editing.",{sz.width/2,22},.29f);
            return true;
        }
        void onDesign(CCObject*){ShopDesignPopup::create(m_shop)->show();}
        void onAddItem(CCObject*){auto& item=Store::get().addItem(*m_shop);ItemEditorPopup::create(m_shop,&item)->show();}
        void onPreview(CCObject*){ShopViewPopup::create(m_shop)->show();}
        void onEditItem(CCObject* sender){int i=static_cast<CCNode*>(sender)->getTag();if(i>=0&&i<(int)m_shop->items.size())ItemEditorPopup::create(m_shop,&m_shop->items[i])->show();}
        void onDeleteItem(CCObject* sender){int i=static_cast<CCNode*>(sender)->getTag();if(i>=0&&i<(int)m_shop->items.size()){auto id=m_shop->items[i].id;Store::get().eraseItem(*m_shop,id);onClose(nullptr);ShopEditorPopup::create(m_shop)->show();}}
    public:
        static ShopEditorPopup* create(Shop* shop){if(!shop)return nullptr;auto p=new ShopEditorPopup();p->m_shop=shop;if(p->initAnchored(360,280)){p->autorelease();return p;}delete p;return nullptr;}
    };

    class ShopMakerPopup : public geode::Popup<> {
        bool setup() override {
            setTitle("Custom Shop Maker");Store::get().load();auto sz=m_mainLayer->getContentSize();
            label(m_mainLayer,"Create shops, design keepers and manage merchandise",{sz.width/2,sz.height-34},.37f);
            auto create=button(this,menu_selector(ShopMakerPopup::onCreate),"Create shop",100);
            auto menu=CCMenu::create();menu->addChild(create);menu->setPosition({sz.width/2,sz.height-66});m_mainLayer->addChild(menu);
            float y=sz.height-105;int i=0;
            for(auto& shop:Store::get().shops()){
                if(i>=5)break;
                label(shop.name,{sz.width*.30f,y},.38f);
                auto edit=button(this,menu_selector(ShopMakerPopup::onEdit),"Manage",62);edit->setTag(i);
                auto open=button(this,menu_selector(ShopMakerPopup::onOpen),"Visit",52);open->setTag(i);
                auto row=CCMenu::create();row->addChild(edit);row->addChild(open);edit->setPosition({0,0});open->setPosition({58,0});row->setPosition({sz.width*.77f,y});m_mainLayer->addChild(row);
                ++i;y-=29;
            }
            if(i==0)label(m_mainLayer,"No custom shops yet.",{sz.width/2,sz.height/2},.42f);
            return true;
        }
        void onCreate(CCObject*){auto& s=Store::get().createShop();ShopEditorPopup::create(&s)->show();onClose(nullptr);}
        void onEdit(CCObject* sender){int i=static_cast<CCNode*>(sender)->getTag();auto& v=const_cast<std::vector<Shop>&>(Store::get().shops());if(i>=0&&i<(int)v.size())ShopEditorPopup::create(&v[i])->show();}
        void onOpen(CCObject* sender){int i=static_cast<CCNode*>(sender)->getTag();auto& v=const_cast<std::vector<Shop>&>(Store::get().shops());if(i>=0&&i<(int)v.size())ShopViewPopup::create(&v[i])->show();}
    public:
        static ShopMakerPopup* create(){auto p=new ShopMakerPopup();if(p->initAnchored(360,270)){p->autorelease();return p;}delete p;return nullptr;}
    };
}

class $modify(ShopMakerMenuLayer, MenuLayer) {
    bool init() {
        if(!MenuLayer::init())return false;
        shopmaker::Store::get().load();
        auto menu=getChildByID("bottom-menu");if(!menu)menu=getChildByID("main-menu");
        if(menu){auto spr=CCSprite::createWithSpriteFrameName("GJ_shopBtn_001.png");if(!spr)spr=CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
            if(spr){auto btn=CCMenuItemSpriteExtra::create(spr,this,menu_selector(ShopMakerMenuLayer::onOpenShopMaker));btn->setID("custom-shop-maker-entry");menu->addChild(btn);menu->updateLayout();}}
        return true;
    }
    void onOpenShopMaker(CCObject*){shopmaker::ShopMakerPopup::create()->show();}
};
