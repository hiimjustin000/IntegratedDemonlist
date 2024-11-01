#include "../IntegratedDemonlist.hpp"

class IDPackCell : public cocos2d::CCLayer {
public:
    static IDPackCell* create(IDDemonPack);
protected:
    bool init(IDDemonPack);
};
