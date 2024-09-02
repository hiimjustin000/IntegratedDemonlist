#include "IntegratedDemonlist.hpp"

class IDPackCell : public CCLayer {
public:
    static IDPackCell* create(IDDemonPack);
protected:
    bool init(IDDemonPack);
};
