#ifndef __FLAME_H
#define __FLAME_H

class CthughaBuffer;
class VisualFrameContext;

class Flame {
public:
    typedef void (*Function)(CthughaBuffer& buffer);

private:
    Function functionValue;
    const char* nameValue;
    const char* descriptionValue;

public:
    Flame(Function function, const char* name, const char* description);

    const char* name() const;
    const char* description() const;
    void execute(CthughaBuffer& buffer, const VisualFrameContext& context) const;
};

extern const Flame flameCatalog[];
extern const int nFlameCatalogEntries;

const Flame* flameByIndex(int index);
int init_flames();

#endif
