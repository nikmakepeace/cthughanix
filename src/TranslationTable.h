// Immutable view over a loaded translation table.

#ifndef __TRANSLATION_TABLE_H
#define __TRANSLATION_TABLE_H

class TranslationTable {
    const char* nameValue;
    const int* dataValue;
    int widthValue;
    int heightValue;
    int sizeValue;

public:
    TranslationTable()
        : nameValue("none")
        , dataValue(0)
        , widthValue(0)
        , heightValue(0)
        , sizeValue(0) { }

    TranslationTable(const char* name_, const int* data_, int width_, int height_)
        : nameValue((name_ != 0) ? name_ : "unknown")
        , dataValue(data_)
        , widthValue(width_)
        , heightValue(height_)
        , sizeValue(width_ * height_) { }

    const char* name() const { return nameValue; }
    const int* data() const { return dataValue; }
    int width() const { return widthValue; }
    int height() const { return heightValue; }
    int size() const { return sizeValue; }
    int ready() const { return dataValue != 0 && sizeValue > 0; }

    int sameTable(const TranslationTable& other) const {
        return dataValue == other.dataValue
            && widthValue == other.widthValue
            && heightValue == other.heightValue
            && sizeValue == other.sizeValue;
    }
};

#endif
