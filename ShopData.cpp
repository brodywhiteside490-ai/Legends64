#include "ShopData.hpp"
#include <algorithm>
#include <atomic>

using namespace geode::prelude;
namespace shopmaker {
    static constexpr char KEY[] = "shopmaker.data.v3";
    Store& Store::get() { static Store s; return s; }

    std::string Store::makeID() {
        static std::atomic<unsigned long long> n{0};
        return "shopmaker-" + std::to_string(++n);
    }

    bool Store::load() {
        m_shops.clear(); m_owned.clear();
        auto root = Mod::get()->getSavedValue<matjson::Value>(KEY, matjson::Value::object());
        if (!root.isObject()) return false;
        auto owned = root["owned"];
        if (owned.isArray()) for (auto const& id : owned) {
            auto s = id.asString(); if (s.isOk()) m_owned.push_back(s.unwrap());
        }
        auto shops = root["shops"];
        if (!shops.isArray()) return true;
        for (auto const& v : shops) {
            if (!v.isObject()) continue;
            Shop s;
            s.id = v["id"].asString().unwrapOr(makeID());
            s.name = v["name"].asString().unwrapOr("My Shop");
            s.keeperName = v["keeperName"].asString().unwrapOr("Shopkeeper");
            s.dialogue = v["dialogue"].asString().unwrapOr("Welcome!");
            s.greeting = v["greeting"].asString().unwrapOr("Welcome!");
            s.backgroundColor = v["backgroundColor"].asInt().unwrapOr(0);
            s.textColor = v["textColor"].asInt().unwrapOr(1);
            s.headPreset = v["headPreset"].asInt().unwrapOr(0);
            s.mouthPreset = v["mouthPreset"].asInt().unwrapOr(0);
            s.bodyPreset = v["bodyPreset"].asInt().unwrapOr(0);
            auto items = v["items"];
            if (items.isArray()) for (auto const& iv : items) {
                if (!iv.isObject()) continue;
                Item item;
                item.id = iv["id"].asString().unwrapOr(makeID());
                item.title = iv["title"].asString().unwrapOr("Item");
                item.kind = iv["kind"].asString().unwrapOr("icon");
                item.iconType = iv["iconType"].asInt().unwrapOr(0);
                item.iconID = iv["iconID"].asInt().unwrapOr(1);
                item.price = std::max(0, iv["price"].asInt().unwrapOr(0));
                item.stock = iv["stock"].asInt().unwrapOr(-1);
                item.repeatable = iv["repeatable"].asBool().unwrapOr(false);
                item.enabled = iv["enabled"].asBool().unwrapOr(true);
                s.items.push_back(std::move(item));
            }
            m_shops.push_back(std::move(s));
        }
        return true;
    }

    bool Store::save() const {
        matjson::Value root = matjson::Value::object();
        matjson::Value shops = matjson::Value::array();
        for (auto const& s : m_shops) {
            matjson::Value v = matjson::Value::object();
            v["id"] = s.id; v["name"] = s.name; v["keeperName"] = s.keeperName;
            v["dialogue"] = s.dialogue; v["greeting"] = s.greeting;
            v["backgroundColor"] = s.backgroundColor; v["textColor"] = s.textColor;
            v["headPreset"] = s.headPreset; v["mouthPreset"] = s.mouthPreset; v["bodyPreset"] = s.bodyPreset;
            matjson::Value items = matjson::Value::array();
            for (auto const& item : s.items) {
                matjson::Value iv = matjson::Value::object();
                iv["id"] = item.id; iv["title"] = item.title; iv["kind"] = item.kind;
                iv["iconType"] = item.iconType; iv["iconID"] = item.iconID;
                iv["price"] = item.price; iv["stock"] = item.stock;
                iv["repeatable"] = item.repeatable; iv["enabled"] = item.enabled;
                items.push(iv);
            }
            v["items"] = items; shops.push(v);
        }
        matjson::Value owned = matjson::Value::array();
        for (auto const& id : m_owned) owned.push(id);
        root["shops"] = shops; root["owned"] = owned;
        Mod::get()->setSavedValue(KEY, root);
        return true;
    }

    Shop& Store::createShop() { Shop s; s.id = makeID(); m_shops.push_back(std::move(s)); save(); return m_shops.back(); }
    bool Store::eraseShop(std::string const& id) {
        auto it = std::remove_if(m_shops.begin(), m_shops.end(), [&](auto const& s){ return s.id == id; });
        if (it == m_shops.end()) return false;
        m_shops.erase(it, m_shops.end()); save(); return true;
    }
    Shop* Store::find(std::string const& id) {
        auto it = std::find_if(m_shops.begin(), m_shops.end(), [&](auto& s){ return s.id == id; });
        return it == m_shops.end() ? nullptr : &*it;
    }
    bool Store::owns(std::string const& id) const { return std::find(m_owned.begin(), m_owned.end(), id) != m_owned.end(); }
    void Store::markOwned(std::string const& id) { if (!owns(id)) m_owned.push_back(id); save(); }
    Item& Store::addItem(Shop& shop) { Item item; item.id = makeID(); shop.items.push_back(std::move(item)); save(); return shop.items.back(); }
    Item* Store::findItem(Shop& shop, std::string const& id) {
        auto it = std::find_if(shop.items.begin(), shop.items.end(), [&](auto const& i){ return i.id == id; });
        return it == shop.items.end() ? nullptr : &*it;
    }
    bool Store::eraseItem(Shop& shop, std::string const& id) {
        auto it = std::remove_if(shop.items.begin(), shop.items.end(), [&](auto const& i){ return i.id == id; });
        if (it == shop.items.end()) return false;
        shop.items.erase(it, shop.items.end()); save(); return true;
    }
    bool Store::purchase(std::string const& shopID, std::string const& itemID, int& balance, std::string& error) {
        auto* shop = find(shopID); if (!shop) { error = "Shop not found."; return false; }
        auto* item = findItem(*shop, itemID);
        if (!item || !item->enabled) { error = "This item is unavailable."; return false; }
        if (!item->repeatable && owns(itemID)) { error = "You already own this item."; return false; }
        if (item->stock == 0) { error = "Out of stock."; return false; }
        if (item->price < 0 || balance < item->price) { error = "Not enough orbs."; return false; }
        balance -= item->price;
        if (!item->repeatable) m_owned.push_back(itemID);
        if (item->stock > 0) --item->stock;
        save(); return true;
    }
}
