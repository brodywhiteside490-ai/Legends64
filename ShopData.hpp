#pragma once
#include <Geode/Geode.hpp>
#include <string>
#include <vector>

namespace shopmaker {
    struct Item {
        std::string id;
        std::string title = "New item";
        // icon = Geometry Dash icon; more-icons = external icon supplied by an adapter; custom = non-icon reward.
        std::string kind = "icon";
        int iconType = 0;
        int iconID = 1;
        int price = 100;
        int stock = -1; // -1 = unlimited
        bool repeatable = false;
        bool enabled = true;
    };

    struct Shop {
        std::string id;
        std::string name = "My Shop";
        std::string keeperName = "Shopkeeper";
        std::string dialogue = "Welcome! Take a look around.";
        std::string greeting = "Welcome to my shop!";
        int backgroundColor = 0;
        int textColor = 1;
        int headPreset = 0;
        int mouthPreset = 0;
        int bodyPreset = 0;
        std::vector<Item> items;
    };

    class Store {
    public:
        static Store& get();
        std::vector<Shop> const& shops() const { return m_shops; }
        std::vector<std::string> const& owned() const { return m_owned; }
        bool owns(std::string const& itemID) const;
        bool load();
        bool save() const;
        Shop& createShop();
        bool eraseShop(std::string const& id);
        Shop* find(std::string const& id);
        Item& addItem(Shop& shop);
        bool eraseItem(Shop& shop, std::string const& itemID);
        Item* findItem(Shop& shop, std::string const& itemID);
        bool purchase(std::string const& shopID, std::string const& itemID,
                      int& balance, std::string& error);
        void markOwned(std::string const& itemID);
    private:
        std::vector<Shop> m_shops;
        std::vector<std::string> m_owned;
        Store() = default;
        static std::string makeID();
    };
}
